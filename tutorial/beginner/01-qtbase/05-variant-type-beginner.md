# 现代Qt开发教程（新手篇）1.5——变体与类型系统

## 前言：为什么需要"万能类型"

说实话，第一次见到 QVariant 的时候，我心里是拒绝的。C++ 是强类型语言，搞个什么都能装的容器，这不是开历史倒车吗？但等到你要写一个能同时存储 int、double、string 甚至自定义类型的配置系统时，你会发现 QVariant 真香。

QVariant 是 Qt 的万能容器，它可以存储绝大多数 Qt 支持的类型。这在很多场景下非常有用：比如解析 JSON 时你根本不知道下一个值是什么类型、比如做一个属性编辑器要支持各种数据类型、比如写一个通用的消息传递系统。这些问题用传统 C++ 方案要么写一堆 overload，要么上模板把代码搞得像迷宫，而 QVariant 提供了一个相对优雅的解决方案。

不过，这个"万能"是有代价的。类型安全检查只能到运行时，用错了要等到程序跑起来才会炸。所以我们用 QVariant 的时候要格外小心，该检查类型的时候千万别偷懒。

## 环境说明

本文代码基于 Qt 6.5+ 版本，QVariant 在 Qt 6 中有一些行为变化（主要是隐式转换被禁用），这一点后面会专门说。如果你还在用 Qt 5，某些写法可能需要调整。

## 核心概念讲解

### QVariant 是什么

从底层看，QVariant 就是一个带了类型标签的值容器。它内部维护一个类型 ID 和一块内存，内存里存着实际的数据。你可以往里面塞 int，塞 QString，塞自定义类型，只要你告诉 QVariant 这是什么类型，它就能正确地存和取。

先看个最简单的例子：

```cpp
// 存储不同类型的值
QVariant v1 = 42;                           // 存储 int
QVariant v2 = 3.14;                         // 存储 double
QVariant v3 = QString("Hello Qt");          // 存储 QString
QVariant v4 = QColor(255, 0, 0);            // 存储 QColor

// 取出值（注意要指定类型）
int i = v1.toInt();                  // 42
double d = v2.toDouble();            // 3.14
QString s = v3.toString();           // "Hello Qt"
QColor c = v4.value<QColor>();       // QColor(255, 0, 0)
```

你会发现存值很简单，直接赋值就行。取值的时候要调用对应的转换函数，或者用 `value<T>()` 模板方法。这里有个关键点：如果类型不匹配会怎样？比如 v1 里存的是 int，你调用 `toDouble()` 会怎样？答案是 QVariant 会尝试做类型转换，int 可以转成 double，没问题。但如果 v3 存的是字符串，你调用 `toInt()` 会怎样？如果字符串内容不是数字，转换失败会返回 0。

这就是 QVariant 的坑点之一：类型转换失败时你得不到明确的错误，只能得到一个默认值。所以当你不确定类型时，一定要先检查。

### 常用类型的存储与提取

Qt 已经为大多数内置类型注册了元类型，可以直接用 QVariant 存储。基本数值类型（int、double、float 等）直接赋值，用 toInt()、toDouble()、toFloat() 提取；QString 直接赋值，toString() 提取；bool 直接赋值，toBool() 提取；QList 和 QMap 直接赋值，toList() 和 toMap() 提取；而像 QColor、QFont、QSize 这类 GUI 类型以及自定义类型，则需要用 `value<T>()` 模板方法提取，自定义类型还要配合 `QVariant::fromValue()` 来存储。

下面是一个稍微完整点的例子，展示几种典型用法：

```cpp
// 基本类型
QVariant v1 = 42;
QVariant v2 = true;
QVariant v3 = 3.14;
qDebug() << v1.toInt() << v2.toBool() << v3.toDouble();

// 字符串
QVariant v4 = QString("Hello");
qDebug() << v4.toString();

// 列表和映射
QList<QVariant> list = {1, "two", 3.0};
QVariant v5 = QVariant::fromValue(list);

QMap<QString, QVariant> map;
map["name"] = "Qt";
map["version"] = 6.5;
QVariant v6 = QVariant::fromValue(map);
```

注意这里的 `QVariant::fromValue()`，对于像 QList、QMap 这样的模板类型，直接赋值可能有问题，用 `fromValue()` 更保险。Qt 6 里这个问题更明显，因为它禁用了一些隐式转换。

### 类型检查的重要性

刚才说到类型转换失败会返回默认值，这可能导致你得到错误的结果而不自知。所以每次取值前，最好先检查一下类型：

```cpp
QVariant v = "123";

// 不安全的做法：直接转换
int num = v.toInt();  // 可能得到 123，也可能得到别的

// 安全的做法：先检查类型
if (v.type() == QVariant::Int) {
    int num = v.toInt();
    // 使用 num
} else if (v.type() == QVariant::String) {
    QString s = v.toString();
    bool ok;
    int num = s.toInt(&ok);
    if (ok) {
        // 使用 num
    }
}

// 或者用 typeId（Qt 6 推荐）
if (v.typeId() == QMetaType::Int) {
    // ...
}
```

`type()` 返回的是 `QVariant::Type` 枚举，这个在 Qt 6 里已经被标记为废弃，推荐用 `typeId()` 配合 `QMetaType`。但为了兼容性，很多代码还在用 `type()`。

还有一个 `canConvert()` 方法，它可以告诉你 QVariant 能否转换成目标类型：

```cpp
QVariant v = "123";

if (v.canConvert<int>()) {
    int num = v.toInt();  // 转换成功
}

QVariant v2 = "hello";
if (v2.canConvert<int>()) {
    int num = v2.toInt();  // 返回 0，但 canConvert 可能说可以
}
```

这里有个坑：`canConvert<int>()` 对字符串来说总是返回 true，因为字符串理论上可以转成 int（即使是 "hello"）。所以 `canConvert` 只是一个粗略的检查，真正关键还是要看转换后的值是否合理。

### QMetaType 类型注册

如果你想用 QVariant 存储自定义类型，需要先把这个类型注册到 Qt 的元对象系统中。注册很简单，用 `Q_DECLARE_METATYPE()` 宏：

```cpp
// 在头文件中声明自定义类型
struct Person {
    QString name;
    int age;
};

// 注册类型（放在头文件中，类定义之后）
Q_DECLARE_METATYPE(Person)

// 现在可以用 QVariant 存储 Person
Person p{"Alice", 30};
QVariant v = QVariant::fromValue(p);

// 取出时也要用 value<T>()
Person p2 = v.value<Person>();
```

注册要放在头文件中，因为所有用到这个类型的地方都需要看到这个声明。如果你的类型在 cpp 文件中定义，那就在 cpp 文件中注册。

## 踩坑预警

用 QVariant 最容易踩的三个坑，我们逐个过一遍。

第一个是忘记检查类型就转换。你拿到一个不知道是什么类型的 QVariant，直接调 toInt() 就用，如果里面存的是字符串 "hello"，toInt() 会返回 0，你可能以为用户输入的就是 0，实际上是转换失败的默认值。所以取出 QVariant 的值前，先问自己一句"这真的对吗？"，然后检查类型。

```cpp
// 危险：不知道类型就直接转换
QVariant v = getUserInput();
int num = v.toInt();  // 如果 v 里是字符串 "hello"，你会拿到 0

// 正确：先检查类型
QVariant v2 = getUserInput();
if (v2.type() == QVariant::Int || v2.type() == QVariant::Double) {
    int num = v2.toInt();
    // 使用 num...
} else {
    // 处理类型不匹配的情况
}
```

第二个坑是 Qt 6 的隐式转换问题。Qt 5 的时候你可以用 `<<` 运算符往 QList<QVariant> 里塞不同类型的值，Qt 6 禁用了某些隐式转换，这种写法可能直接编译失败。Qt 6 里类型转换要显式一点，别指望编译器帮你猜。

```cpp
// Qt 5 可以这样写，Qt 6 可能编译不过
QList<QVariant> list;
list << 1 << "two" << 3.0;

// Qt 6 推荐写法
QList<QVariant> list2 = {1, "two", 3.0};
// 或者显式转换
QList<QVariant> list3;
list3.append(QVariant(1));
list3.append(QVariant("two"));
list3.append(QVariant(3.0));
```

第三个坑是自定义类型没注册就往 QVariant 里塞。没注册的类型存进 QVariant 会得到一个无效的 QVariant，取出来也是垃圾值，而且这种错误编译器不会帮你拦住，只有运行时才会暴露。所以自定义类型想进 QVariant，先去 `Q_DECLARE_METATYPE` 报个到。

```cpp
// 忘记注册
struct Point { int x, y; };
// Q_DECLARE_METATYPE(Point)  <-- 漏了这行

Point p{1, 2};
QVariant v = QVariant::fromValue(p);  // 返回无效的 QVariant

// 正确：注册后再用
struct Point2 { int x, y; };
Q_DECLARE_METATYPE(Point2)

Point2 p2{1, 2};
QVariant v2 = QVariant::fromValue(p2);  // 正常工作
Point2 p3 = v2.value<Point2>();
```

## 随堂测验

先想一个问题：QVariant 和 C++ 的 std::any 有什么区别？为什么 Qt 要搞自己的万能类型而不是用标准库的？简单来说，QVariant 在 C++17 的 std::any 出现之前就存在了，而且 QVariant 和 Qt 的元对象系统深度集成——信号槽可以传 QVariant、JSON 解析返回 QVariant、属性系统也基于 QVariant，这是 std::any 做不到的。

再来一个代码填空题：下面这段代码想把一个 QVariant 里的值转换成字符串输出，但有时转换会失败，补全错误处理部分。

```cpp
QVariant v = getSomeValue();
QString str;

if (v.type() == QVariant::String) {
    str = v.toString();
} else if (v.canConvert<QString>()) {
    str = v.toString();
} else {
    str = "(unknown type)";
}

qDebug() << "Value:" << str;
```

最后是一个调试题：这段代码想从用户输入读取一个数字，但有一个 bug，问题在哪里？

```cpp
QVariant input = QLineEdit::text();  // 假设这是用户输入
int number = input.toInt();  // 转换成数字
if (number > 100) {
    qDebug() << "数字太大";
}
```

问题在于 `QLineEdit::text()` 返回的是 QString，不是 QVariant。即使你把它塞进了 QVariant，如果用户输入的是非数字字符串，`toInt()` 会返回 0 而不是报错，你可能把无效输入当成合法的 0 来处理了。正确做法是用 `QString::toInt(&ok)` 检查转换是否成功。

## 练习项目

练习项目：简易配置编辑器。

实现一个配置文件编辑器，可以读取、修改、保存配置项。每个配置项都有一个名称和一个 QVariant 值，支持 int、double、bool、string 等类型。

完成标准是定义一个 `Config` 类，内部用 `QMap<QString, QVariant>` 存储配置项，实现 `set()`、`get()`、`save()`、`load()` 方法。`get()` 方法要支持默认值，如果配置项不存在返回默认值；`save()` 把配置保存到 JSON 文件；`load()` 从 JSON 文件加载配置；最后写一个 main 函数演示如何使用这个 Config 类。

两个思路：用 QJsonDocument 处理 JSON 读写，从 JSON 读值时得到的直接就是 QVariant，存取很方便；保存时注意处理不同类型，QJsonObject 支持大多数 QVariant 类型。

## 官方文档参考链接

[Qt 文档 - QVariant](https://doc.qt.io/qt-6/qvariant.html) - QVariant 完整 API 参考，必查文档

[Qt 文档 - QMetaType](https://doc.qt.io/qt-6/qmetatype.html) - 了解 Qt 的类型系统和元类型注册机制

[Qt 文档 - Container Classes](https://doc.qt.io/qt-6/containers.html) - Qt 容器类与 QVariant 配合使用的说明
