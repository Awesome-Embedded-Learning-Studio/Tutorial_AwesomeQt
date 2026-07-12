---
title: MOC 编译器原理——元对象代码生成
description: Qt 6 MOC 工具的源码级拆解——从 runMoc 三段式（预处理嚼符号 → parse 解析成 ClassDef IR → generate 生成代码），到 generateCode 四段输出（字符串池去重 → 数据表 UintData → staticMetaObject 七字段 → qt_metacall/qt_static_metacall/qt_metacast 函数），再到与运行期 metacall 的对接。
---

# 现代Qt开发教程（专家篇）1.17——MOC 编译器原理（元对象代码生成）

## 1. 前言——为什么 MOC 值得单独拆一篇

咱们写 `Q_OBJECT` 的时候都知道「moc 会处理这个类」。但 moc 这个工具内部到底干了什么？它是怎么从咱们写的那行 `class MyClass : public QObject` 加上 `signals:` 段，变出一份 `moc_myclass.cpp` 源文件的？那份源文件里都有什么？它是怎么把「信号」「槽」「属性」这些咱们声明的东西，翻译成运行期能被 `QMetaObject` 查询的静态表的？

笔者第一次琢磨这些的时候，是去看自己 build 目录下的 `moc_*.cpp`——好家伙，几千行手写感十足的 C++，`qt_metacall` 里那一坨 `switch` 跳转，看得笔者愣住了：这玩意儿真是机器生成的？

这些问题，靠「moc 是元对象编译器」这种概括答不了。咱们要把 moc 工具的源码本身打开看。前面几篇咱们反复用到 moc 的产物——[QObject 元对象系统](./01-qobject-meta-system-expert.md) 里讲 `staticMetaObject` 静态表、`qt_metacall` 函数体，[信号槽底层](./02-signal-slot-internals-expert.md) 里讲 moc 在信号函数体里偷偷插的 `QMetaObject::activate`——但每次都是「这是 moc 生成的」，没说 moc 怎么生成的。本篇就是来补这一环的：把 moc 工具拆开，看它从读头文件到吐 `moc_*.cpp` 的完整流水线。

入门篇的 [QObject 与元对象系统](../../beginner/01-qtbase/01-qobject-meta-system-beginner.md) 讲过 `Q_OBJECT` 怎么写、moc 大致起什么作用——那是知其然。本篇是知其所以然，而且这是专家层独有的深度（入门/进阶没有单独的 moc 篇，因为 moc 内部原理本来就属于源码级话题）。

边界先划清楚：本篇拆的是 moc 工具的源码——即 `qtbase/src/tools/moc/` 下那个可执行程序的实现。它怎么用 C++ 写一个「读 C++ 头文件、生成 C++ 源文件」的代码生成器。咱们追它的 `parse` → IR → `generate` 流水线，看产物怎么落地。不展开 moc 对 C++ 语法的完整解析器细节（那是一个手写的递归下降 parser，细节繁多），只看和「元对象代码生成」直接相关的关键路径。

## 2. 环境说明

本篇所有行号都来自 moc 工具源码 `qt_src/qt6.9.1/qtbase/src/tools/moc/`，是 moc 工具自身的解析器/生成器源码行号，不是 Qt 库运行期代码的行号。笔者提醒一句：这些行号随 Qt 升级漂移很快（moc 工具几乎每个小版本都在重构），对照阅读请用函数名/结构名定位。和前几篇对 `generator.cpp` 个别行挂脚注不同，本篇通篇都是 moc 工具源码，不再逐条标注。

本篇涉及的源码文件（按出现顺序）：

| 文件 | 角色 |
|---|---|
| `qt_src/qt6.9.1/qtbase/src/tools/moc/main.cpp` | runMoc 三段式入口（预处理 → parse → generate） |
| `qt_src/qt6.9.1/qtbase/src/tools/moc/moc.h` | FunctionDef / ClassDef 中间表示（IR） |
| `qt_src/qt6.9.1/qtbase/src/tools/moc/moc.cpp` | parse 解析 + generate 遍历调 Generator |
| `qt_src/qt6.9.1/qtbase/src/tools/moc/generator.h` | Generator 类声明 |
| `qt_src/qt6.9.1/qtbase/src/tools/moc/generator.cpp` | generateCode 四段输出（产物主体） |
| `qt_src/qt6.9.1/qtbase/src/corelib/kernel/qobjectdefs.h` | QMetaObject::Data 七字段（moc 的填充目标） |
| `qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtmetamacros.h` | Q_OBJECT 宏声明端（moc 负责定义） |

本篇无配套 example，原因：moc 是构建期代码生成工具，对照 `qtbase/src/tools/moc/` 翻源码就是最好的实验——你也可以随便找个 Q_OBJECT 类，看它的 build 目录下生成的 `moc_xxx.cpp`，对照本篇讲的段落结构。

## 3. 核心概念讲解

先对路线图。笔者觉得 moc 最妙的设定就是——它本质是一个「前端 parse → 中间表示 → 后端 generate」三段式编译器，只不过它产出的是 C++ 源代码而不是机器码：

```mermaid
flowchart LR
    SRC["咱们的头文件\nQ_OBJECT / signals: / slots:"] --> PP["① 预处理\npreprocessed 嚼符号"]
    PP --> PARSE["② parse 解析\n构 ClassDef IR"]
    PARSE --> IR["ClassDef\nsignalList/slotList/\nmethodList/propertyList"]
    IR --> GEN["③ generate\n遍历 classList\n每类 new Generator"]
    GEN --> OUT["generateCode 四段输出"]
    OUT --> S1["段1: 字符串池去重"]
    S1 --> S2["段2: 数据表 UintData"]
    S2 --> S3["段3: staticMetaObject 七字段"]
    S3 --> S4["段4: qt_metacall / qt_static_metacall / qt_metacast"]
    S4 --> PROD["moc_myclass.cpp 产物\n编译进二进制\n供运行期 metacall 调"]
```

咱们从链路源头 `runMoc` 开始。

### 3.1 三段式入口——runMoc

moc 的主流程在一个叫 `runMoc` 的函数里，结构非常清晰的三段：

`qt_src/qt6.9.1/qtbase/src/tools/moc/main.cpp:543-547`

```cpp
    moc.symbols += pp.preprocessed(moc.filename, &in);

    if (!pp.preprocessOnly) {
        // 2. parse
        moc.parse();
    }
```

第一段 `pp.preprocessed`——预处理器把头文件嚼成一串符号（tokens）。第二段 `moc.parse()`——解析器吃进这串符号，构造出中间表示（IR）。第三段（这里没贴出来，在后面）`moc.generate()` 把 IR 翻译成产物代码。这就是 moc 的三段式：预处理嚼符号 → parse 成 IR → generate 出代码。和教科书上的编译器结构一模一样，只是它的输入是 C++ 头文件、输出还是 C++ 源代码（所谓 transpiler / 代码生成器）。

注意 moc 的 parser 不是拿现成的 C++ 编译器前端（比如 clang），而是 moc 自己手写的一个简化的 C++ 解析器——笔者一开始还纳闷 Qt 怎么不直接复用 clang，后来才想通：moc 只要它关心和元对象系统相关的语法（`Q_OBJECT` 宏、`signals:`/`slots:` 段、`Q_PROPERTY`、`Q_INVOKABLE` 等标记），普通 C++ 语法它只是粗略扫过。这是 moc 能保持轻量、能独立于完整 C++ 编译器运行的关键，但也意味着它对 C++ 语法的理解是有限的（这点 §4.3 的坑会涉及）。

### 3.2 中间表示——FunctionDef 与 ClassDef

parse 阶段把源码嚼成结构化的中间表示。方法是声明什么、信号什么、槽什么、属性什么，全部存成 `FunctionDef` 和 `ClassDef` 结构。

方法用一组布尔标志位记录它的「角色」：

`qt_src/qt6.9.1/qtbase/src/tools/moc/moc.h:76-110`

```cpp
struct FunctionDef
{
    Type type;
    QList<ArgumentDef> arguments;
// ...
    bool isInvokable = false;
    bool isScriptable = false;
    bool isSlot = false;
    bool isSignal = false;
    bool isPrivateSignal = false;
```

一个 `FunctionDef` 描述一个成员函数，用 `isSlot` / `isSignal` / `isInvokable`（对应 `Q_INVOKABLE`）/ `isScriptable`（对应 `Q_SCRIPTABLE`）等标志位标记它是哪种角色。这些标志位是 parse 阶段根据源码里的宏标记和 `signals:`/`slots:` 段落位置填上的。

整个类则聚合成一个 `ClassDef`，它是产物的直接投影：

`qt_src/qt6.9.1/qtbase/src/tools/moc/moc.h:177-209`

```cpp
struct ClassDef : public BaseDef {
    QList<SuperClass> superclassList;
// ...
    QList<FunctionDef> constructorList;
    QList<FunctionDef> signalList, slotList, methodList, publicList;
    QList<QByteArray> nonClassSignalList;
    QList<PropertyDef> propertyList;
```

`ClassDef` 持有 `superclassList`（基类列表，用于生成 superClass 链）、`signalList` / `slotList` / `methodList`（信号、槽、普通可调用方法的 FunctionDef 列表）、`propertyList`（`Q_PROPERTY` 列表）。这些列表就是后续 generate 阶段的素材——产物代码里的每一段，都对应这里某个列表的一项。`ClassDef` 本质上就是「这个类的元信息在 moc 眼里的完整画像」。

### 3.3 Generator 类与 generateCode——产物总入口

IR 建好了，generate 阶段开始吐代码。moc 对每个 `ClassDef` 实例化一个 `Generator` 对象负责生成：

`qt_src/qt6.9.1/qtbase/src/tools/moc/moc.cpp:1233-1236`

```cpp
    for (ClassDef &def : classList) {
        Generator generator(this, &def, metaTypes, knownQObjectClasses, knownGadgets, out,
                            requireCompleteTypes);
        generator.generateCode();
```

遍历所有 `ClassDef`（一个头文件可能有多个 Q_OBJECT 类），每个 new 一个 `Generator`，调它的 `generateCode()`。`Generator` 类封装了生成所需的一切：

`qt_src/qt6.9.1/qtbase/src/tools/moc/generator.h:11-49`

```cpp
class Generator
{
    Moc *parser = nullptr;
    FILE *out;
    ClassDef *cdef;
    QList<uint> meta_data;
// ...
    void generateCode();
// ...
    void generateMetacall();
    void generateStaticMetacall();
```

`Generator` 持有 `out`（输出的 `FILE*`，即要生成的 `moc_*.cpp` 文件）、`cdef`（要生成的那个 `ClassDef`），以及三个核心方法 `generateCode` / `generateMetacall` / `generateStaticMetacall`。`generateCode` 是总入口，它按固定顺序调各段子生成函数：

`qt_src/qt6.9.1/qtbase/src/tools/moc/generator.cpp:220`

```cpp
void Generator::generateCode()
{
    bool isQObject = (cdef->classname == "QObject");
    bool isConstructible = !cdef->constructorList.isEmpty();
```

`generateCode` 开头先判断两件事：这个类是不是 `QObject` 根类本身（`isQObject`，影响后面要不要往上传 qt_metacall）、有没有构造函数（`isConstructible`，影响要不要生成 `CreateInstance` 分支）。然后它会按四段顺序输出产物——注册字符串、建数据表、建 `staticMetaObject`、生成函数。咱们下面逐段看。

### 3.4 第一段：字符串池去重

第一段把所有需要的字符串（类名、方法名、参数类型名、属性名等）收集起来去重，生成一个紧凑的字符串池。Qt 6.9 这里做过重构：

`qt_src/qt6.9.1/qtbase/src/tools/moc/generator.cpp:245-248,286-288`

```cpp
    strreg(cdef->qualified);
    registerClassInfoStrings();
    registerFunctionStrings(cdef->signalList);
    registerFunctionStrings(cdef->slotList);
```

`strreg` / `registerFunctionStrings` 这些函数把字符串注册进一个去重表——同一个字符串（比如 `void` 类型名在多个方法签名里出现）只存一份。最终这些字符串被吐成 `QtMocHelpers::StringRefStorage qt_stringData` 这样一个编译期字符串存储。为什么要去重？笔者没料到这是个性命攸关的优化：元对象表里所有名字都用整数偏移引用这个池子，去重能显著减小 `staticMetaObject` 的体积——一个大型 QObject 类可能有上百个方法签名，`void`/`int`/`QString` 这些类型名重复无数次，不去重表会膨胀得很厉害。

### 3.5 第二段：数据表 UintData

第二段生成描述方法/属性元信息的数据表。Qt 6.9 用了新的 `UintData` 包装：

`qt_src/qt6.9.1/qtbase/src/tools/moc/generator.cpp:290-295`

```cpp
    fprintf(out, "    QtMocHelpers::UintData qt_methods {\n");

    // Build signals array first, otherwise the signal indices would be wrong
    addFunctions(cdef->signalList, "Signal");
    addFunctions(cdef->slotList, "Slot");
    addFunctions(cdef->methodList, "Method");
```

这段生成 `qt_methods` 数据表。笔者是被那句注释「Build signals array first, otherwise the signal indices would be wrong」点醒的——信号必须先 add，否则信号索引会错。这一句就揭了 moc 的一个关键约定：方法索引空间是按 signal → slot → method 的固定顺序编排的，信号永远排在最前。这个顺序决定了运行期 `QMetaObject::method(i)` 拿到的索引含义，也决定了 `emit` 时 `activate` 用的信号索引（§4.1 的坑就源于此）。`qt_properties` / `qt_enums` 用类似的方式生成。

### 3.6 第三段：staticMetaObject 七字段聚合

第三段生成核心产物——`staticMetaObject` 静态表本体。先看它的填充目标（这是咱们在 [01 篇](./01-qobject-meta-system-expert.md) 已经见过的 `QMetaObject::Data` 七槽位）：

`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qobjectdefs.h:600-609`

```cpp
    struct Data { // private data
        SuperData superdata;
        const uint *stringdata;
        const uint *data;
        typedef void (*StaticMetacallFunction)(QObject *, QMetaObject::Call, int, void **);
        StaticMetacallFunction static_metacall;
        const SuperData *relatedMetaObjects;
        const QtPrivate::QMetaTypeInterface *const *metaTypes;
        void *extradata; //reserved for future use
    } d;
```

七个槽位：`superdata`（父类元对象）、`stringdata`（§3.4 的字符串池）、`data`（§3.5 的数据表）、`static_metacall`（函数指针）、`relatedMetaObjects` / `metaTypes` / `extradata`。moc 第三段就是把这七个槽填上。生成时用一个模板函数聚合：

`qt_src/qt6.9.1/qtbase/src/tools/moc/generator.cpp:335-336`

```cpp
    fprintf(out, "    return QtMocHelpers::metaObjectData<%s, %s>(%s, qt_stringData,\n"
                 "            qt_methods, qt_properties, qt_enums%s);\n"
```

`metaObjectData` 模板函数把 `qt_stringData` / `qt_methods` / `qt_properties` / `qt_enums` 聚合成一个编译期常量——这是 Qt 6.9 重构后的写法，把多个数据表用模板参数打包，比老版本平铺的 `uint[]` 数组更类型安全。

最关键的 `superdata`（父类元对象，superClass 链的源头）这样填：

`qt_src/qt6.9.1/qtbase/src/tools/moc/generator.cpp:439-451`

```cpp
    fprintf(out, "Q_CONSTINIT const QMetaObject %s::staticMetaObject = { {\n",
            cdef->qualified.constData());

    if (isQObject)
        fprintf(out, "    nullptr,\n");
    else if (cdef->superclassList.size() && !cdef->hasQGadget && !cdef->hasQNamespace)
        fprintf(out, "    QMetaObject::SuperData::link<%s::staticMetaObject>(),\n", purestSuperClass.constData());
```

这段是 superClass 链的建立点。如果当前类是 `QObject` 根类本身，`superdata` 填 `nullptr`（链的终点）；否则填 `QMetaObject::SuperData::link<基类::staticMetaObject>()`——这就是咱们在 [01 篇](./01-qobject-meta-system-expert.md) 见过的 `SuperData` 双模式，它通过 `link<>()` 模板拿到父类 `staticMetaObject` 的地址。整条继承链就是靠这个槽位一个链接一个串起来的。注意 `!cdef->hasQGadget && !cdef->hasQNamespace` 这个条件——`Q_GADGET` 和 `Q_NAMESPACE` 类的元对象处理方式略有不同（§4.2）。

后面的 `stringdata` / `data` 槽用 6.9 的新方案填：

`qt_src/qt6.9.1/qtbase/src/tools/moc/generator.cpp:451-453`

```cpp
    fprintf(out, "    qt_staticMetaObjectStaticContent%s.stringdata,\n"
            "    qt_staticMetaObjectStaticContent%s.data,\n",
```

指向 `qt_staticMetaObjectStaticContent` 这个模板静态内容的 `stringdata` / `data` 成员——又是 6.9 重构引入的编译期内容容器。

### 3.7 第四段：qt_metacall / qt_static_metacall / qt_metacast

第四段生成三个函数体。先看 `qt_static_metacall` 的 `InvokeMetaMethod` 分支——咱们在 [01 篇](./01-qobject-meta-system-expert.md) 见过它运行期长什么样，现在看 moc 怎么打印出那个 `switch`：

`qt_src/qt6.9.1/qtbase/src/tools/moc/generator.cpp:1001-1041`

```cpp
        fprintf(out, "    if (_c == QMetaObject::InvokeMetaMethod) {\n");
        fprintf(out, "        switch (_id) {\n");
        for (int methodindex = 0; methodindex < methodList.size(); ++methodindex) {
// ...
            fprintf(out, "        case %d: ", methodindex);
// ...
            fprintf(out, "_t->");
```

这段对 `methodList` 里每个方法生成一个 `case _id: _t->方法名(args);`——这就是运行期 `qt_metacall` 那个 `switch(_id)` 跳转表的来源。笔者看到这儿差点笑出声：moc 在编译期就把「方法索引 → 方法调用」的映射固化成 switch case，用最朴素的 `fprintf` 一行一行往外打印 C++ 代码，运行期一次跳转就到位——所谓「元对象代码生成」，底下居然就是一坨 `fprintf` 拼字符串。

属性的 `ReadProperty` / `WriteProperty` 分支用类似但独立的模式：

`qt_src/qt6.9.1/qtbase/src/tools/moc/generator.cpp:1148-1149`

```cpp
        if (needGet) {
            fprintf(out, "    if (_c == QMetaObject::ReadProperty) {\n");
```

每个属性操作类型（Read/Write/Reset）各自一个 `if (_c == ...)` + `switch(_id)`，case 按属性在 `propertyList` 里的索引排。

`qt_metacall` 函数体则复用了 [01 篇](./01-qobject-meta-system-expert.md) 详细讲过的「链式调父类 + 索引递减」结构：

`qt_src/qt6.9.1/qtbase/src/tools/moc/generator.cpp:849-854,870-878`

```cpp
    fprintf(out, "\nint %s::qt_metacall(QMetaObject::Call _c, int _id, void **_a)\n{\n",
             cdef->qualified.constData());

    if (!purestSuperClass.isEmpty() && !isQObject) {
        QByteArray superClass = purestSuperClass;
        fprintf(out, "    _id = %s::qt_metacall(_c, _id, _a);\n", superClass.constData());
    }
```

这里生成的就是 01 篇讲的「先把 `_id` 交给基类 `qt_metacall` 处理」的开头，`isQObject` 守卫保证根类不往上传。01 篇咱们看的是这个生成物运行期怎么被调用，本篇看的是 moc 怎么把它打印出来——同一个函数，两个视角。

还有 `qt_metacast`（类型转换函数，`qobject_cast` 的底层），它用 `strcmp` 逐类名比较：

`qt_src/qt6.9.1/qtbase/src/tools/moc/generator.cpp:486-489`

```cpp
    fprintf(out, "\nvoid *%s::qt_metacast(const char *_clname)\n{\n", cdef->qualified.constData());
    fprintf(out, "    if (!_clname) return nullptr;\n");
    fprintf(out, "    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_%s_t>.strings))\n"
                  "        return static_cast<void*>(this),\n",
```

生成的 `qt_metacast` 拿传入的类名字符串，先和本类名 `strcmp`，命中就 `return this`；否则继续和各基类/接口名比较，命中返回对应类型指针；都不中递归基类。这就是 `qobject_cast<MyClass*>(obj)` 能做安全向下转换的底层——它查的是 moc 编译期固化进 `qt_metacast` 的类名列表。

### 3.8 与运行期的对接点

最后把 moc 产物和运行期串起来。声明端是 `Q_OBJECT` 宏：

`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtmetamacros.h:118-146`

```cpp
#define Q_OBJECT \
public: \
// ...
    static const QMetaObject staticMetaObject; \
    virtual const QMetaObject *metaObject() const; \
    virtual void *qt_metacast(const char *); \
    virtual int qt_metacall(QMetaObject::Call, int, void **); \
```

`Q_OBJECT` 宏在声明端声明了 `staticMetaObject` 和三个虚函数。这些声明的定义——`staticMetaObject` 的七字段填充、三个虚函数的函数体——全部由 moc 在产物 `moc_myclass.cpp` 里提供。声明在咱们写的头文件里，定义在 moc 生成的文件里，两边一编译链接，类就「活」了。

对接的运行期入口是 `QMetaObject::metacall`：

`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qmetaobject.cpp:343-348`

```cpp
int QMetaObject::metacall(QObject *object, Call cl, int idx, void **argv)
{
    if (object->d_ptr->metaObject)
        return object->d_ptr->metaObject->metaCall(object, cl, idx, argv);
    else
        return object->qt_metacall(cl, idx, argv);
}
```

这是 01 篇讲过的总分发入口。普通对象走 `else` 分支——调虚函数 `object->qt_metacall(...)`。而这个 `qt_metacall` 正是本篇讲的 moc 产物。运行期 `metacall`（Qt 库代码）调 moc 生成的 `qt_metacall`（你的类的产物代码），这条调用边界就是「Qt 元对象框架」和「moc 生成的胶水代码」的对接点。整条元对象系统，就是靠这个对接运转的。

## 4. 踩坑预防

第一个坑是随意调整信号/槽的声明顺序导致方法索引漂移。§3.5 看到 moc 生成数据表时那句注释「Build signals array first, otherwise the signal indices would be wrong」——笔者把这条单拎出来放在坑第一位，是因为 moc 按 signal → slot → method 的固定顺序、按声明顺序给每个方法分配索引。这个索引一旦定下来，就会被运行期的 `QMetaObject::method(i)`、跨线程 `QueuedConnection`（它通过索引定位信号）、QML 的方法调用、以及任何按索引序列化的地方依赖。如果你在某次代码重构里调整了信号声明顺序，或者把一个槽删了、在中间插了新信号，moc 重新生成的 `staticMetaObject` 里所有索引都会跟着变。后果是：老的客户端代码（比如已经编译好的插件、QML 里硬编码的方法名查询可能没事，但按索引的二进制协议会错位）调到错的方法，跨线程信号连到错的槽，排查极其困难。解法：把公共信号/槽的声明顺序当作 ABI 的一部分，不要随意调整；要加新信号，追加到末尾，别插中间；确实需要调整时，评估所有按索引依赖的下游。

第二个坑是 `Q_GADGET` 和 `Q_OBJECT` 混淆。§3.6 看到 `staticMetaObject` 生成时有个 `!cdef->hasQGadget` 的条件分支——`Q_GADGET` 类的元对象和 `Q_OBJECT` 类不一样。`Q_GADGET` 是给「不需要信号槽、但想有元属性和枚举」的纯数据类（gadget）用的，它不生成 `static_metacall` 函数指针（或生成的是空壳），因此不支持信号槽、不能被 `QMetaObject::invokeMethod` 调方法。如果你本意是要一个能发信号、能被反射调用的类，却图省事写了 `Q_GADGET`，后果是 `connect` 编译报错、`invokeMethod` 运行期找不到方法、`staticMetaObject` 里方法表是空的。反过来，如果你只是一个 `struct Point { int x,y; }` 想在 QVariant 里用、想有反射，用 `Q_OBJECT` 就太重了（强制继承 QObject、引入对象树开销）。解法：需要信号槽/对象树用 `Q_OBJECT`；只需元类型 + 元属性 + 枚举反射用 `Q_GADGET`（它可以不继承 QObject）。

第三个坑是 moc 看不到的类。§3.1 提到 moc 用的是自己手写的简化 C++ 解析器，它只处理 CMake `AUTOMOC` 喂给它的、在头文件里的 `Q_OBJECT` 类。有几类类 moc 是看不到的：模板类（`template<class T> class Foo : public QObject { Q_OBJECT };`——moc 不知道 `T` 是什么，根本不处理）；定义在 `.cpp` 文件里的类（moc 默认只扫头文件，除非你显式 `#include "foo.moc"`）；只有前向声明没看到完整定义的类。如果你在这些 moc 看不到的地方写了 `Q_OBJECT`，moc 不会生成 `moc_foo.cpp`，链接阶段就会报 `undefined reference to vtable for Foo` 或 `undefined reference to Foo::staticMetaObject`。后果是编译过、链接挂，新手看到 vtable 错误一头雾水。解法：`Q_OBJECT` 类必须是非模板的、定义在头文件里的；模板类需要元对象能力，要么用 `Q_GADGET`（部分能力），要么抽一个非模板的 QObject 基类放头文件里承载 `Q_OBJECT`，模板子类继承它；`.cpp` 里的 Q_OBJECT 类，在 `.cpp` 末尾 `#include "foo.moc"`（注意是 `.moc` 不是 `_moc.cpp`）让 moc 处理它。

## 5. 官方文档参考链接

[Qt 文档 · The Meta-Object System](https://doc.qt.io/qt-6/metaobjects.html) -- 元对象系统总览，moc / Q_OBJECT / 元调用三件套的关系

[Qt 文档 · moc Tool](https://doc.qt.io/qt-6/moc.html) -- moc 工具的官方说明，命令行选项与产物

[Qt 文档 · Q_OBJECT & Q_GADGET](https://doc.qt.io/qt-6/qobject.html#Q_OBJECT) -- Q_OBJECT / Q_GADGET 宏的语义与适用区别

---

到这里，moc 这个「元对象代码生成器」咱们就从源码层面拆透了。从 `runMoc` 的三段式（预处理嚼符号 → parse 成 `ClassDef` IR → generate 出代码），到 `generateCode` 的四段输出（字符串池去重 → `UintData` 数据表 → `staticMetaObject` 七字段聚合 → `qt_metacall`/`qt_static_metacall`/`qt_metacast` 函数体），咱们看清了 moc 怎么把咱们写的 `Q_OBJECT` 类声明，翻译成运行期能被 `QMetaObject` 查询的那张静态表和那些函数。最后咱们把 moc 产物和运行期 `metacall` 的对接点也串了起来——声明端在 `Q_OBJECT` 宏，定义端在 `moc_*.cpp`，调用端在 `QMetaObject::metacall`，三段一拼就是完整的元对象系统。回过头看 [01 篇](./01-qobject-meta-system-expert.md) 和 [02 篇](./02-signal-slot-internals-expert.md) 里那些「moc 生成的」细节，现在应该全是落到实处的了。

本篇涉及的所有行号证据已按源码机制归类收在 [code-index · qtbase](../code-index/qtbase/) 下，带着行号直接去 `qt_src/qt6.9.1/qtbase/src/tools/moc/` 翻原文就能核对。
