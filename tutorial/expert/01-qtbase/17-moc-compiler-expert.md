---
title: MOC 编译器原理——元对象代码生成
description: Qt 6 MOC 工具的源码级拆解——runMoc 的三段式流水线（预处理嚼符号、parse 解析成 ClassDef 中间表示、generate 生成代码），generateCode 的四段输出（字符串池去重、UintData 数据表、staticMetaObject 七字段聚合、qt_metacall/qt_static_metacall/qt_metacast 函数体），以及 moc 产物与运行期 metacall 的对接。
---

# MOC 编译器原理——元对象代码生成

## 前言：为什么 MOC 值得单独拆一篇

咱们写下 `Q_OBJECT` 的时候都知道，接手它的会是 moc。可 moc 这个工具内部到底干了什么？它是怎么从咱们写的那行 `class MyClass : public QObject` 加上 `signals:` 段，变出一份 `moc_myclass.cpp` 源文件来的？里头都装了些什么？它又是怎么把信号、槽、属性这些咱们声明的东西，翻译成运行期能被 `QMetaObject` 查询的静态表的？

笔者头一回琢磨这些的时候，是去看自己 build 目录下的 `moc_*.cpp`。几千行手写感十足的 C++ 摆在眼前，`qt_metacall` 里头的那一大串 `switch` 跳转，看得笔者都愣住了：这玩意儿真是机器生成的？

这样的问题，靠一句 moc 是元对象编译器的概括答不上来，咱们得把 moc 工具的源码本身打开来看。前面几篇咱们反复用到 moc 的产物：[QObject 元对象系统](./01-qobject-meta-system-expert.md) 那一篇讲了 `staticMetaObject` 静态表和 `qt_metacall` 的函数体，[信号槽底层](./02-signal-slot-internals-expert.md) 那一篇讲了 moc 在信号函数体里插进来的 `QMetaObject::activate`。不过每一回咱们都只把 `generator.cpp` 的行号当证据来引用，moc 这个工具自己怎么从读头文件一路走到交出产物，整条流水线咱们还没说过。本篇就是来补上这一环的：把 moc 工具打开，看它从读头文件到交出 `moc_*.cpp` 的整条流水线。

入门篇的 [QObject 与元对象系统](../../beginner/01-qtbase/01-qobject-meta-system-beginner.md) 咱们读过，`Q_OBJECT` 怎么写、moc 大致起什么作用，那讲的是知其然。本篇要讲的是知其所以然，而且这样的深度是专家层独有的，入门和进阶都没有单独的 moc 篇，moc 的内部原理本来就属于源码级的话题。

边界咱们也划清楚：本篇看的是 moc 工具自己的源码，也就是 `qtbase/src/tools/moc/` 目录底下那个可执行程序的实现。咱们要看的是它怎么用 C++ 写出一个工具，读进 C++ 的头文件，生成出 C++ 的源文件。parse、中间表示、generate 的流水线咱们整条追一遍，看产物是怎么落地的。moc 对 C++ 语法的完整解析器细节（那是一台手写的递归下降 parser，细节繁多）咱们不展开，只走和元对象代码生成直接相关的关键路径。

## 环境说明

本篇的行号，主体来自 moc 工具自己的源码，也就是 `qt_src/qt6.9.1/qtbase/src/tools/moc/` 底下的解析器、生成器源码行号，不是 Qt 库运行期代码的行号。例外是几处对接点：`qobjectdefs.h` 的 `Data` 结构、`qtmetamacros.h` 的 `Q_OBJECT` 宏，还有 `qmetaobject.cpp` 的 `metacall` 和按名字查元对象的 `QMetaObject_findMetaObject`，它们取自的是 `qt_src/qt6.9.1/qtbase/src/corelib/kernel/`，正文里出现的时候咱们会点明。笔者提醒一句，这些行号跟着 Qt 的升级漂移得很快（moc 工具几乎每个小版本都在重构），您对照阅读的时候，拿函数名或结构名定位是最稳的。和前几篇给 `generator.cpp` 个别行挂脚注的做法不同，本篇的行号证据一条接着一条，咱们就不再逐条挂脚注了。

本篇要翻的源码文件，咱们按正文里出场的顺序列在下面：

| 文件 | 角色 |
|---|---|
| `qt_src/qt6.9.1/qtbase/src/tools/moc/main.cpp` | runMoc 三段式入口（预处理 → parse → generate） |
| `qt_src/qt6.9.1/qtbase/src/tools/moc/moc.h` | FunctionDef / ClassDef 中间表示（IR） |
| `qt_src/qt6.9.1/qtbase/src/tools/moc/moc.cpp` | parse 解析 + generate 遍历调 Generator |
| `qt_src/qt6.9.1/qtbase/src/tools/moc/generator.h` | Generator 类声明 |
| `qt_src/qt6.9.1/qtbase/src/tools/moc/generator.cpp` | generateCode 四段输出（产物主体） |
| `qt_src/qt6.9.1/qtbase/src/corelib/kernel/qobjectdefs.h` | QMetaObject::Data 七字段（moc 的填充目标） |
| `qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtmetamacros.h` | Q_OBJECT 宏声明端（moc 负责定义） |
| `qt_src/qt6.9.1/qtbase/src/corelib/kernel/qmetaobject.cpp` | QMetaObject::metacall 运行期总分发 |

本篇没有配套的 example。moc 是构建期的代码生成工具，对照 `qtbase/src/tools/moc/` 翻源码就是最好的实验，您也可以随便找一个 `Q_OBJECT` 类，看它 build 目录下生成的 `moc_xxx.cpp`，对照本篇讲的段落结构来读。

## 核心概念讲解

动身以前咱们把路线图摆出来。笔者觉得 moc 最妙的设定是，它本质是一台前端 parse、中间表示、后端 generate 的三段式编译器，只不过它交出来的产物是 C++ 源代码，而不是机器码：

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

图上的路线，咱们从链路源头 `runMoc` 走起。

### 三段式入口：runMoc

moc 的主流程住在一个叫 `runMoc` 的函数里，咱们翻到 `qt_src/qt6.9.1/qtbase/src/tools/moc/main.cpp:543-547`，三段的结构看得清清楚楚：

```cpp
    moc.symbols += pp.preprocessed(moc.filename, &in);

    if (!pp.preprocessOnly) {
        // 2. parse
        moc.parse();
    }
```

头一段的活是预处理，`pp.preprocessed` 把咱们的头文件嚼成一串符号（tokens）。第二段干的是解析，`moc.parse()` 把吃进去的符号串构造成中间表示（IR）。第三段在贴出的代码后面，`moc.generate()` 把 IR 翻译成咱们要的产物代码。这就是 moc 的三段式（预处理嚼符号、parse 成 IR、generate 出代码），和教科书上的编译器结构一模一样，只不过它的输入是 C++ 头文件，输出的也还是 C++ 源代码，也就是所谓的 transpiler（代码生成器）。

moc 的 parser 没有拿现成的 C++ 编译器前端（比如 clang），它用的是自己手写的一台简化 C++ 解析器。笔者一开始还纳闷 Qt 怎么不直接复用 clang，后来才想通：moc 关心的只是和元对象系统相关的那部分语法，`Q_OBJECT` 宏是它要的，`signals:` 和 `slots:` 段是它要的，`Q_PROPERTY`、`Q_INVOKABLE` 这样的标记也在它的名单里，普通 C++ 语法它只是粗略地扫过。这是 moc 能保持轻量、能独立于完整 C++ 编译器运行的关键，不过这也意味着它对 C++ 语法的理解是有限的，风险一节咱们会回到这一点。

### 中间表示：FunctionDef 与 ClassDef

parse 阶段把源码嚼成了结构化的中间表示。一个类里声明了哪些方法、哪些是信号、哪些是槽、哪些是属性，咱们拿 `FunctionDef` 和 `ClassDef` 两个结构分头装下。

方法用一组布尔标志位记录它的角色，咱们把 `FunctionDef` 摆开来看：

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

这几行的原文在 `qt_src/qt6.9.1/qtbase/src/tools/moc/moc.h:76-110`。咱们凑近看一个 `FunctionDef`，它描述的是一个成员函数，`isSlot`/`isSignal`/`isInvokable`（对应 `Q_INVOKABLE`）/`isScriptable`（对应 `Q_SCRIPTABLE`）这些标志位标记出它的角色。它们由 parse 阶段根据源码里的宏标记和 `signals:`/`slots:` 段落位置填上。

类级别的信息则聚合成 `ClassDef`，它是产物的直接投影，咱们把 `qt_src/qt6.9.1/qtbase/src/tools/moc/moc.h:177-209` 的声明摘下来：

```cpp
struct ClassDef : BaseDef {
    QList<SuperClass> superclassList;
// ...
    QList<FunctionDef> constructorList;
    QList<FunctionDef> signalList, slotList, methodList, publicList;
    QList<QByteArray> nonClassSignalList;
    QList<PropertyDef> propertyList;
```

开头交代一句引文里那个 `BaseDef`：它是 `ClassDef`、`NamespaceDef` 这类声明共有的底座，装的是类名、限定名、classInfo 和枚举这些公共字段，`ClassDef` 从它这里继承了这身底子。`ClassDef` 手里的家当，咱们挑生成的时候要用的点几个名：`superclassList` 记的是基类，生成 superClass 链的时候要用，`signalList`、`slotList`、`methodList` 是信号、槽、普通方法的列表，`propertyList` 装的是 `Q_PROPERTY`。`constructorList` 咱们按住不表，第四段讲 `CreateInstance` 的时候轮到它出场。引文里剩下的 `publicList`、`nonClassSignalList` 本篇用不上，就不逐个点了。generate 阶段的素材全在这个结构里，产物代码里的每一段，都能对应到上面点过名的清单。咱们可以把 `ClassDef` 读成这个类的元信息在 moc 眼里的完整画像。

### Generator 与 generateCode：产物总入口

IR 建好了，接下来轮到 generate 阶段出代码了。咱们把 `moc.cpp` 里那段循环调出来（`qt_src/qt6.9.1/qtbase/src/tools/moc/moc.cpp:1233-1236`），它遍历的是 `classList`（一个头文件里可能有多个 `Q_OBJECT` 类），给每一个 `ClassDef` 都构造出属于自己的 `Generator`，跟着调它的 `generateCode()`：

```cpp
    for (ClassDef &def : classList) {
        Generator generator(this, &def, metaTypes, knownQObjectClasses, knownGadgets, out,
                            requireCompleteTypes);
        generator.generateCode();
```

咱们跟着看 `Generator` 类的声明，生成要用的东西全封装在里头：

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

刚才那段声明的出处是 `qt_src/qt6.9.1/qtbase/src/tools/moc/generator.h:11-49`。`Generator` 手里的东西，`out` 是输出的 `FILE*`，也就是要生成的 `moc_*.cpp` 文件，`cdef` 是要生成的那个 `ClassDef`，手里的三个核心方法是 `generateCode`/`generateMetacall`/`generateStaticMetacall`，其中的 `generateCode` 是总入口，它按固定的顺序调各段子生成函数，咱们翻到 `qt_src/qt6.9.1/qtbase/src/tools/moc/generator.cpp:220` 看它的开头：

```cpp
void Generator::generateCode()
{
    bool isQObject = (cdef->classname == "QObject");
    bool isConstructible = !cdef->constructorList.isEmpty();
```

函数的开头就有两个判断，一个判的是这个类是不是 `QObject` 根类本身（`isQObject`，这决定了后面要不要往上传 `qt_metacall`），另一个判的是它有没有构造函数（`isConstructible`，这决定了要不要生成 `CreateInstance` 分支）。前文点家当的时候预告过的 `CreateInstance`，这儿多说一句：它是运行期按索引反射构造实例的那段 `switch`，[QObject 元对象系统](./01-qobject-meta-system-expert.md) 那一篇的 `qt_static_metacall` 一节见过它。跟着函数按四段的顺序输出产物（注册字符串、建数据表、建 `staticMetaObject`、生成函数体），咱们下面一段一段看。

### 第一段：字符串池去重

第一段把所有要用的字符串（类名、方法名、参数类型名、属性名这些）收集起来去重，生成一个紧凑的字符串池。Qt 6.9 在这里做了重构，咱们看它留下的证据（`qt_src/qt6.9.1/qtbase/src/tools/moc/generator.cpp:245-248,286-288`）：

```cpp
    strreg(cdef->qualified);
    registerClassInfoStrings();
    registerFunctionStrings(cdef->signalList);
    registerFunctionStrings(cdef->slotList);
```

`strreg`/`registerFunctionStrings` 这些函数把要用的字符串注册进一个去重表，同一个字符串（好比 `void` 这个类型名的重复出现）在表里只留它的一份。这些字符串最终被写成 `QtMocHelpers::StringRefStorage qt_stringData` 这样一个编译期的字符串存储。为什么要去重？笔者当初没料到这是个性命攸关的优化：元对象表里所有的名字都用整数偏移来引用这个池子，去重能显著减小 `staticMetaObject` 的体积。一个大型 `QObject` 类的方法签名能有上百个，`void`/`int`/`QString` 这些类型名在签名里一遍遍地重复，不去重的话表会膨胀得很厉害。

### 第二段：数据表 UintData

第二段生成描述方法、属性元信息的数据表，Qt 6.9 给它配的新包装叫 `UintData`，咱们看它张什么样：

```cpp
    fprintf(out, "    QtMocHelpers::UintData qt_methods {\n");

    // Build signals array first, otherwise the signal indices would be wrong
    addFunctions(cdef->signalList, "Signal");
    addFunctions(cdef->slotList, "Slot");
    addFunctions(cdef->methodList, "Method");
```

咱们对着 `qt_src/qt6.9.1/qtbase/src/tools/moc/generator.cpp:290-295` 这段看，它生成的是 `qt_methods` 数据表。笔者是被那句注释 “Build signals array first, otherwise the signal indices would be wrong” 点醒的，信号的 add 得放在最前面，否则信号索引就错了。这一句揭开了 moc 的一个关键约定：方法索引空间按 signal、slot、method 的固定顺序编排，排在最前的永远是信号。这个顺序决定了运行期 `QMetaObject::method(i)` 拿到的索引是什么含义，也决定了 `emit` 的时候 `activate` 用的信号索引，风险一节的第一处麻烦就出在这里。`qt_properties`/`qt_enums` 用的也是类似的方式。

### 第三段：staticMetaObject 七字段聚合

第三段要生成的是核心产物，也就是 `staticMetaObject` 静态表的本体。它的填充目标咱们见过，[QObject 元对象系统](./01-qobject-meta-system-expert.md) 那一篇讲过 `QMetaObject::Data` 的七个槽位（`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qobjectdefs.h:600-609`），咱们照原文搬一段下来：

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

七个槽位各有各的差事：`superdata` 装的是父类元对象，`stringdata` 接的是字符串池一节的产物，`data` 接的是数据表一节的产物，`static_metacall` 挂的是函数指针。剩下的三个槽位也有各自的差事，咱们一个一个点：`relatedMetaObjects` 指向的是相关元对象的列表，属性类型和枚举作用域牵扯到的其他类元对象，moc 生成的时候把它们链了进去（`qt_src/qt6.9.1/qtbase/src/tools/moc/generator.cpp:367-431` 的 `extraList`）。运行期要按名字找一个类的元对象，走的就是它，`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qmetaobject.cpp:1059-1061` 的 `QMetaObject_findMetaObject` 顺着它一层层往下查，解析属性枚举作用域的那一问，调用点落在 `qmetaobject.cpp:3885` 的 `QMetaProperty` 构造函数里。`metaTypes` 指向的是元类型接口表，运行期按偏移从里头取回每个参数、属性的 `QMetaType`。最末的 `extradata` 是保留位，源码注释写明了留给将来用。moc 的第三段干的就是把七个槽位挨个填上。生成的时候用一个模板函数来聚合，咱们翻到聚合的那一笔（`qt_src/qt6.9.1/qtbase/src/tools/moc/generator.cpp:335-336`）：

```cpp
    fprintf(out, "    return QtMocHelpers::metaObjectData<%s, %s>(%s, qt_stringData,\n"
                 "            qt_methods, qt_properties, qt_enums%s);\n"
```

`metaObjectData` 这个模板函数干的聚合活，是把 `qt_stringData`/`qt_methods`/`qt_properties`/`qt_enums` 打包成一份编译期常量的活。这是 Qt 6.9 重构以后的写法，多个数据表改用模板参数的方式打包，在咱们看来比老版本平铺的 `uint[]` 数组更类型安全。

最要紧的 `superdata` 槽（父类元对象，superClass 链的源头）是怎么填的，咱们到 `qt_src/qt6.9.1/qtbase/src/tools/moc/generator.cpp:439-451` 找答案：

```cpp
    fprintf(out, "Q_CONSTINIT const QMetaObject %s::staticMetaObject = { {\n",
            cdef->qualified.constData());

    if (isQObject)
        fprintf(out, "    nullptr,\n");
    else if (cdef->superclassList.size() && !cdef->hasQGadget && !cdef->hasQNamespace)
        fprintf(out, "    QMetaObject::SuperData::link<%s::staticMetaObject>(),\n", purestSuperClass.constData());
```

这一段是 superClass 链的建立点。当前的类要是 `QObject` 根类本身，`superdata` 填的就是 `nullptr`，链到这里就是终点了。不然的话，`superdata` 填的就是 `QMetaObject::SuperData::link<基类::staticMetaObject>()`，这正是咱们在 [QObject 元对象系统](./01-qobject-meta-system-expert.md) 那一篇见过的 `SuperData` 双模式，它通过 `link<>()` 模板拿到的是父类 `staticMetaObject` 的地址。整条继承链就靠这个槽位一个链接一个地串起来。您也留意一下 `!cdef->hasQGadget && !cdef->hasQNamespace` 这个条件，`Q_GADGET` 类的元对象走的是略有不同的处理，风险一节咱们就说这个分野。`Q_NAMESPACE` 本篇就不展开了。

后面的 `stringdata`/`data` 两个槽，6.9 的新方案这么填，咱们把出处放在这儿（`qt_src/qt6.9.1/qtbase/src/tools/moc/generator.cpp:448-451`）：

```cpp
    fprintf(out, "    qt_staticMetaObjectStaticContent%s.stringdata,\n"
            "    qt_staticMetaObjectStaticContent%s.data,\n",
```

咱们看到填进来的是 `qt_staticMetaObjectStaticContent` 这个模板静态内容的 `stringdata`/`data` 成员，又是 6.9 重构引进来的编译期内容容器。

### 第四段：qt_metacall、qt_static_metacall 与 qt_metacast

第四段要生成的是三个函数体。头一个是 `qt_static_metacall` 的 `InvokeMetaMethod` 分支，[QObject 元对象系统](./01-qobject-meta-system-expert.md) 那一篇见过它运行期的样子，现在轮到咱们看 moc 把那个 `switch` 打印出来，打印的现场在 `qt_src/qt6.9.1/qtbase/src/tools/moc/generator.cpp:1001-1041`：

```cpp
        fprintf(out, "    if (_c == QMetaObject::InvokeMetaMethod) {\n");
        fprintf(out, "        switch (_id) {\n");
        for (int methodindex = 0; methodindex < methodList.size(); ++methodindex) {
// ...
            fprintf(out, "        case %d: ", methodindex);
// ...
            fprintf(out, "_t->");
```

这段代码对 `methodList` 里的每个方法生成一行 `case _id: _t->方法名(args);`，运行期 `qt_metacall` 那个 `switch(_id)` 跳转表就是这么来的。笔者看到这儿差点笑出声：moc 在编译期把方法索引到方法调用的映射固化成 switch case，用最朴素的 `fprintf` 一行一行往外打印 C++ 代码，运行期一次跳转就到位了。所谓的元对象代码生成，底下干的居然就是一堆 `fprintf` 拼字符串的活。

属性的 `ReadProperty`/`WriteProperty` 分支用的是类似而独立的一套模式：每个属性操作类型（Read/Write/Reset）都有自己的一个 `if (_c == ...)` 加 `switch(_id)`，case 按属性在 `propertyList` 里的索引排。咱们把 `ReadProperty` 的两行搬下来（`qt_src/qt6.9.1/qtbase/src/tools/moc/generator.cpp:1148-1149`）：

```cpp
        if (needGet) {
            fprintf(out, "    if (_c == QMetaObject::ReadProperty) {\n");
```

`qt_metacall` 的函数体复用了 [QObject 元对象系统](./01-qobject-meta-system-expert.md) 详细讲过的链式调父类加索引递减的结构，咱们看看 moc 是怎么写它的（`qt_src/qt6.9.1/qtbase/src/tools/moc/generator.cpp:848-855,870-874`）：

```cpp
    fprintf(out, "\nint %s::qt_metacall(QMetaObject::Call _c, int _id, void **_a)\n{\n",
             cdef->qualified.constData());

    if (!purestSuperClass.isEmpty() && !isQObject) {
        QByteArray superClass = purestSuperClass;
        fprintf(out, "    _id = %s::qt_metacall(_c, _id, _a);\n", superClass.constData());
    }
```

这里生成的就是那一篇讲过的开头，把 `_id` 交给基类的 `qt_metacall` 处理，`isQObject` 的守卫保证了根类不再往上传。那一篇咱们看的是这个生成物运行期怎么被调用，本篇看的是 moc 怎么把它打印出来，这是同一个函数的两个视角。

还有一个负责类型转换的 `qt_metacast`（`qobject_cast` 的底层），咱们看它拿 `strcmp` 逐个类名比过去：

```cpp
    fprintf(out, "\nvoid *%s::qt_metacast(const char *_clname)\n{\n", cdef->qualified.constData());
    fprintf(out, "    if (!_clname) return nullptr;\n");
    fprintf(out, "    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_%s_t>.strings))\n"
                  "        return static_cast<void*>(this);\n",
```

咱们把这段的原文记在这儿（`qt_src/qt6.9.1/qtbase/src/tools/moc/generator.cpp:486-489`）。生成的 `qt_metacast` 拿到传入的类名字符串，跟本类名做的是 `strcmp`，命中了就返回 `this`。没中的就接着跟各个基类、接口名比，命中哪个就返回对应类型的指针，都不中的话就递归去问基类。`qobject_cast<MyClass*>(obj)` 能做安全的向下转换，底层查的就是 moc 在编译期固化进 `qt_metacast` 的类名列表。

### 与运行期的对接点

咱们把 moc 产物和运行期串起来。声明的那一端是 `Q_OBJECT` 宏，宏的原文摊开在 `qt_src/qt6.9.1/qtbase/src/corelib/kernel/qtmetamacros.h:118-146`：

```cpp
#define Q_OBJECT \
public: \
// ...
    static const QMetaObject staticMetaObject; \
    virtual const QMetaObject *metaObject() const; \
    virtual void *qt_metacast(const char *); \
    virtual int qt_metacall(QMetaObject::Call, int, void **); \
```

`Q_OBJECT` 宏在声明端声明了一个 `staticMetaObject` 和三个虚函数。这些声明的定义，也就是 `staticMetaObject` 的七字段填充和三个虚函数的函数体，moc 全给咱们在产物 `moc_myclass.cpp` 里提供。声明写在咱们自己的头文件里，定义在 moc 生成的文件里，编译链接一跑就把两边接上了，这个类就活了。

对接的运行期入口是 `QMetaObject::metacall`，咱们把它那七行函数体整段搬来（`qt_src/qt6.9.1/qtbase/src/corelib/kernel/qmetaobject.cpp:343-349`）：

```cpp
int QMetaObject::metacall(QObject *object, Call cl, int idx, void **argv)
{
    if (object->d_ptr->metaObject)
        return object->d_ptr->metaObject->metaCall(object, cl, idx, argv);
    else
        return object->qt_metacall(cl, idx, argv);
}
```

这就是那一篇讲过的总分发入口，[QObject 元对象系统](./01-qobject-meta-system-expert.md) 那一篇追的就是它。普通对象走的是 `else` 分支，调的是虚函数 `object->qt_metacall(...)`，而这个 `qt_metacall` 正是本篇讲的 moc 产物。运行期的 `metacall`（Qt 库代码）调的是 moc 生成的 `qt_metacall`（您类里的产物代码），调用的边界就落在这里。这一步值得您亲手踩一遍：随便找个会 `emit` 信号的程序，您给 `QMetaObject::metacall` 下一发断点，信号一触发您就单步往下走，`else` 分支一脚踏进您自己类里的 `qt_metacall`，跟着 `_id` 落进 `qt_static_metacall` 的 `switch`，那个 `switch` 正是本篇第四段看 moc 用 `fprintf` 一行一行打出来的。

## 风险与故障预防

本篇挑出来的麻烦都能在源码里直接对上号，后果和解法咱们一条条交代。

头一处麻烦是随意调整信号、槽的声明顺序，方法索引也就跟着漂移了。数据表一节里 moc 自己写的那句注释 “Build signals array first, otherwise the signal indices would be wrong”，笔者把它单拎出来放在头一位，因为 moc 排索引的顺序是固定的，signal、slot、method 一类一类地来，类内又按声明的顺序，每个方法领到自己的索引。索引一旦定下来了，运行期的 `QMetaObject::method(i)` 依赖的就是它。跨线程的 `QueuedConnection`（它靠索引定位信号）、QML 的方法调用，还有任何按索引序列化的地方，搭的也都是这同一套索引。您要是在某次重构里调整了信号的声明顺序，或者删掉了一个槽、在中间插进来一个新信号，moc 重新生成的 `staticMetaObject` 里所有索引都会跟着变。后果是这样的，老的客户端代码（比如已经编译好的插件）会调到错的方法，跨线程信号会连到错的槽。QML 里按方法名查询的大概率没事，但按索引走的二进制协议是会错位的，这类问题排查起来是极其困难的。

解法咱们也写清楚：把公共信号、槽的声明顺序当成 ABI 的一部分，不要随意地去动它。要加新信号的话，往末尾追加就是了，别去动中间原有的。确实需要调整的时候，把所有按索引依赖的下游都评估一遍。

第二处麻烦是 `Q_GADGET` 和 `Q_OBJECT` 的混用。生成 `staticMetaObject` 的时候有个 `!cdef->hasQGadget` 的条件分支（第三段的代码里咱们见过它），`Q_GADGET` 类的元对象和 `Q_OBJECT` 类不是一个样子。`Q_GADGET` 是给不需要信号槽、但想要元属性和枚举反射的纯数据类（gadget）用的，它不生成能用的 `static_metacall` 函数指针（或者生成的只是空壳），所以它不支持信号槽，也不能被 `QMetaObject::invokeMethod` 调它的方法。您要是本意想要一个能发信号、能被反射调用的类，却图省事写了 `Q_GADGET`，后果马上就来了（`connect` 编译报错、`invokeMethod` 运行期找不到方法、`staticMetaObject` 里的方法表是空的）。不过反过来，您手里只有一个 `struct Point { int x,y; }`，想放进 `QVariant`、想要反射的话，用 `Q_OBJECT` 就太重了，它强制您继承 `QObject`，把对象树的开销也带了进来。

解法要看您拿它来干什么：要信号槽、要对象树的用 `Q_OBJECT`，只要元类型加元属性加枚举反射的用 `Q_GADGET`（它可以不继承 `QObject`）。

第三处麻烦是 moc 看不见的类。三段式入口一节提到 moc 用的是自己手写的简化 C++ 解析器，它只处理 CMake 的 `AUTOMOC` 喂给它的、写在头文件里的 `Q_OBJECT` 类。有几类类 moc 是看不见的：模板类（`template<class T> class Foo : public QObject { Q_OBJECT };`，moc 不知道 `T` 是什么，它压根不在 moc 的处理范围里）、定义在 `.cpp` 文件里的类（moc 默认只扫的是头文件，除非您显式 `#include "foo.moc"`）、还有只有前向声明没见到完整定义的类。您在这些 moc 看不见的地方写了 `Q_OBJECT`，moc 就不会为您生成 `moc_foo.cpp`，链接阶段报的是 `undefined reference to vtable for Foo` 或者 `undefined reference to Foo::staticMetaObject`。后果是编译过了、链接挂了，新手看到 vtable 错误就一头雾水了。

解法咱们也摆出来：`Q_OBJECT` 类必须是非模板的、定义在头文件里的。模板类想要元对象能力的话，要么借 `Q_GADGET` 的部分能力，要么抽一个非模板的 `QObject` 基类放头文件里承载 `Q_OBJECT`，模板子类继承的就是这个基类。`.cpp` 里定义的 `Q_OBJECT` 类，在 `.cpp` 的末尾 `#include "foo.moc"` 让 moc 处理它，注意后缀要写的是 `.moc`，而不是 `_moc.cpp`。

## 练习项目

本篇跟前面的纯源码篇一样，练习走的是验证的路子，几道题给的是方向和核对手段，完整的工程留给您自己搭。

头一道题验的是产物结构。您写一个带 `Q_OBJECT` 的小类，构建用的命令是 `cmake -B build && cmake --build build`，跑完到 build 目录下打开生成的 `moc_xxx.cpp`，按本篇的四段顺序找：`QtMocHelpers::StringRefStorage qt_stringData` 的字符串池，`QtMocHelpers::UintData qt_methods` 的数据表，七字段的 `staticMetaObject`，最后是三个函数体的部分（`qt_metacall`/`qt_static_metacall`/`qt_metacast`）。四处都找到了，产物的段落结构您就亲眼对上了。

第二道题验的是信号索引的编排顺序。您在类里声明两个信号，构建完了留一份 `moc_xxx.cpp` 在手边，跟着把两个信号的声明顺序对调，再构建出一份新的来对比，diff 前后两份产物就能看出来了。`qt_methods` 里的条目顺序和 `qt_static_metacall` 里的 case 编号都会变，风险一节讲的索引漂移就摆在您眼前了。

第三道题验的是 `Q_GADGET` 和 `Q_OBJECT` 的分野。您把一个类的 `Q_OBJECT` 换成 `Q_GADGET` 再构建，打开产物看看少了什么，跟着写一行针对它的 `connect`，编译器会把错误直接报给您，风险一节说的后果就都对上了。

第四道题验的是 moc 看不见的类。您写一个模板类，把 `Q_OBJECT` 写在它的里头，构建到链接的阶段就会报 `undefined reference to vtable`。跟着按风险一节的解法抽一个非模板基类，链接就过了。`.cpp` 里定义的类，您补上 `#include "foo.moc"` 再对比一次，差别也是一眼能看出来的。

末了是一道对行号的题，要对的都在前文里。您挑上三四处（比如 `main.cpp:543` 的三段式，`moc.h:76` 的 `FunctionDef`，`generator.cpp:220` 的 `generateCode`，`qobjectdefs.h:600` 的七字段），带着您的行号去 `qt_src/qt6.9.1` 翻原文，一处一处地核对过去。都对上了，本篇的断言也就落了地。

## 官方文档参考链接

咱们把本篇用到的官方文档列在下面，链接都指向 doc.qt.io 的 Qt 6 文档：

- [Qt 文档 · The Meta-Object System](https://doc.qt.io/qt-6/metaobjects.html) —— 元对象系统总览，moc、`Q_OBJECT`、元调用三件套的关系都在这一页。
- [Qt 文档 · moc Tool](https://doc.qt.io/qt-6/moc.html) —— moc 工具的官方说明，命令行选项与产物说明可以对照着查。
- [Qt 文档 · Q_OBJECT & Q_GADGET](https://doc.qt.io/qt-6/qobject.html#Q_OBJECT) —— `Q_OBJECT`/`Q_GADGET` 宏的语义与适用区别，风险一节的选择题拿它当裁判。

---

咱们把 moc 这个元对象代码生成器从源码层面走了一遍。从 `runMoc` 的三段式（预处理嚼符号、parse 成 `ClassDef` 中间表示、generate 出代码），到 `generateCode` 的四段输出（字符串池、数据表、七字段聚合、三个函数体），咱们看清了 moc 怎么把咱们写的 `Q_OBJECT` 类声明，翻译成运行期能被 `QMetaObject` 查询的静态表和那些函数。moc 产物和运行期 `metacall` 的对接点咱们也串起来了，一头是 `Q_OBJECT` 宏的声明，一头是 `moc_*.cpp` 里的定义，中间的接力棒是 `QMetaObject::metacall`，三段一拼就是完整的元对象系统。回头再看 [QObject 元对象系统](./01-qobject-meta-system-expert.md) 和 [信号槽底层](./02-signal-slot-internals-expert.md) 里那些 moc 生成的细节，如今就都能落到实处的了。

本篇的行号证据按源码机制归好了类，收在 code-index 的三份子索引里。runMoc 入口、parse 和 `ClassDef` 中间表示的部分，看 [moc 三段式流水线与中间表示](../code-index/qtbase/moc-pipeline.md) 的索引。generateCode 的四段输出，看 [moc generateCode 四段输出](../code-index/qtbase/moc-codegen.md) 的。七字段的填充目标、`Q_OBJECT` 的声明端，还有运行期对接的部分，看 [moc 产物与运行期对接](../code-index/qtbase/moc-static-metaobject.md) 的，您带着行号去 `qt_src/qt6.9.1` 翻原文就能核对。
