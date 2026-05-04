# 现代Qt开发教程（新手篇）6.1——QML 语法基础与类型系统

## 1. 为什么我们需要 QML

如果你跟我一样，是从 QtWidgets 开始接触 Qt 的，那你大概率写过不少这样的代码：先在 C++ 里 `new` 一个 `QPushButton`，再 `new` 一个 `QVBoxLayout`，然后 `addWidget`、`setLayout`、`connect`......一套下来，一个简单的登录界面就能写掉上百行 C++。功能上没问题，但维护起来是真的头疼——尤其是当设计师跑过来跟你说「这个按钮能不能加个渐变动画」的时候，你看着 C++ 里的样式表代码，血压就开始往上走了。

QML（Qt Modeling Language）就是 Qt 给出的答案。它是一种声明式的 UI 描述语言，专门用来构建用户界面。你不需要用 C++ 一行一行地命令式描述「先创建这个、再放到那个里面」，而是直接声明「界面长什么样」，剩下的交给引擎去渲染。这种范式转换一开始可能会有点不适应，但一旦上手，你会发现写 UI 的效率直接翻了好几倍。

另外一个关键动因是，Qt 6 之后，Qt Quick（也就是 QML 的运行时框架）已经成了 Qt 官方主推的 UI 技术栈。Qt Quick Controls 从 2.0 到现在的 6.x 版本已经非常成熟，组件丰富度完全能满足实际项目需求。如果你现在还在犹豫要不要学 QML，我的建议是别犹豫了——2025 年了，QML 已经不是可选技能，而是必会技能。

---

## 2. 环境说明

本文档基于以下环境编写和验证：

- Qt 6.9.1，使用 `qt_add_executable` 构建 QML 应用
- CMake 3.26+，C++17 标准
- 操作系统为 Linux（WSL2 / 原生均可），macOS 和 Windows 同样适用
- Qt Quick 模块需要安装 `qtdeclarative` 组件

在 CMake 中构建 QML 应用时，你需要链接 `Qt6::Quick` 模块（如果使用 Qt Quick Controls 则需要 `Qt6::QuickControls2`）。一个最基础的 QML 项目 CMakeLists.txt 长这样：

```cmake
cmake_minimum_required(VERSION 3.26)
project(QmlSyntaxDemo VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_add_executable(${PROJECT_NAME}
    main.cpp
)

qt_add_qml_module(${PROJECT_NAME}
    URI QmlSyntaxDemo
    VERSION 1.0
    QML_FILES Main.qml
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Quick
)
```

这里有几个值得留意的点。首先是 `qt_add_qml_module`，这个函数是 Qt 6 引入的，它会把你的 `.qml` 文件打包成一个 QML 模块，同时生成对应的注册代码。`URI` 参数指定了模块的导入路径，`QML_FILES` 后面列出所有需要包含的 QML 文件。如果你像早期 Qt 5 那样用 `QQmlApplicationEngine::load(QUrl("qrc:/Main.qml"))` 手动加载，在现代 Qt 6 里虽然还能工作，但已经不推荐了——`qt_add_qml_module` 能在编译时检查 QML 文件的语法错误和导入依赖，这比运行时才发现某个 import 写错了要友好得多。

---

## 3. QML 文档结构——对象声明、属性赋值、层级关系

一个 QML 文件本质上就是一棵对象树。最外层是一个根对象，里面可以嵌套子对象，子对象还能继续嵌套，形成一棵完整的 UI 树。这种结构和 HTML 的 DOM 树非常相似——如果你有前端开发经验的话，理解起来会很快。

### 3.1 第一个 QML 文件

我们先看一个最基础的 QML 文件：

```qml
// Main.qml
import QtQuick

Window {
    width: 400
    height: 300
    visible: true
    title: "Hello QML"

    Text {
        anchors.centerIn: parent
        text: "Hello, World!"
        font.pixelSize: 24
        color: "#333333"
    }
}
```

逐行拆解一下。第一行 `import QtQuick` 是导入语句，QML 中的所有类型都来自模块，`QtQuick` 是最核心的模块，包含了 `Window`、`Item`、`Text`、`Rectangle` 等基础视觉类型。Qt 6 中，版本号可以省略——如果你写 `import QtQuick 2.15`，那只是历史遗留习惯，现代写法直接 `import QtQuick` 就行。

接下来是对象声明。`Window { ... }` 声明了一个 `Window` 类型的对象，大括号内部是它的属性赋值和子对象。`width: 400` 这种写法就是属性赋值——冒号左边是属性名，右边是值。多个属性之间不需要逗号或分号，换行就行。

在这个 `Window` 内部，我们声明了一个 `Text` 子对象。这就是 QML 的层级关系——`Text` 是 `Window` 的子元素，它会自动成为 `Window` 的可视化内容。`anchors.centerIn: parent` 是锚点布局，意思是「把自己放在父对象（也就是 `Window`）的中心」。`parent` 是 QML 中的一个关键字，它总是指向当前对象的直接父对象。

### 3.2 对象声明的一般形式

QML 中声明一个对象的通用语法是：

```qml
TypeName {
    property: value
    property: expression

    ChildType {
        property: value
    }
}
```

对象声明的花括号 `{}` 定义了对象的作用域，里面的属性赋值可以按任意顺序排列，QML 引擎会在创建对象时统一处理。这意味着你不需要关心属性声明的先后顺序——`width` 写在 `height` 前面还是后面都一样。

属性赋值的右侧可以是字面量（如 `400`、`"Hello"`），也可以是 JavaScript 表达式（如 `parent.width * 0.5`），还可以是对象绑定（如 `anchors.centerIn: parent`）。QML 的属性系统是非常灵活的，我们会在下一篇文章中专门讲属性绑定。

### 3.3 层级关系与坐标系统

QML 的对象层级不仅是视觉上的嵌套，还决定了坐标系统和事件传递。每个 QML 可视对象都有一个本地坐标系，原点在左上角，x 轴向右，y 轴向下。子对象的坐标是相对于父对象的——也就是说，如果你把一个 `Rectangle` 放在 `x: 10, y: 20` 的位置，这个 `(10, 20)` 是相对于父对象左上角的偏移量。

```qml
Window {
    width: 400
    height: 300
    visible: true
    color: "#f0f0f0"

    Rectangle {
        x: 50
        y: 30
        width: 200
        height: 100
        color: "#4CAF50"

        Rectangle {
            x: 10
            y: 10
            width: 50
            height: 50
            color: "#FF5722"
        }
    }
}
```

这段代码里，绿色 `Rectangle` 相对于 `Window` 在 `(50, 30)` 的位置，而橙色 `Rectangle` 相对于绿色 `Rectangle` 在 `(10, 10)` 的位置。如果你把绿色 `Rectangle` 的 `x` 改成 `100`，橙色 `Rectangle` 会跟着移动——因为它的坐标是相对于父对象的，而父对象的位置变了。这种机制的好处是，移动一个容器对象时，所有子对象会自动跟着移动，你不需要手动更新每个子元素的坐标。

---

## 4. 基础类型系统

QML 有自己的一套类型系统，和 C++ 的类型系统独立但可以互操作。对于刚从 C++ 转过来的朋友，最容易困惑的是 QML 的类型比较松散——它是动态类型的，属性的类型在编译时不做严格检查，运行时才会报错。不过不用担心，基础类型的规则其实很简单。

### 4.1 数值类型：int 与 real

`int` 是整数类型，对应 C++ 的 `int`。`real` 是浮点数类型，对应 C++ 的 `double`。在 QML 中写 `400` 会被识别为 `int`，写 `400.0` 或 `400.5` 会被识别为 `real`。

```qml
Item {
    property int count: 42
    property real ratio: 0.618

    // 也可以在 JavaScript 表达式中使用
    width: 200 + count * 2    // int 运算，结果是 int
    height: width * ratio     // real 运算，结果是 real
}
```

这里有一个细节值得注意：QML 的 `int` 在 JavaScript 表达式中运算时，实际上会被当成 JavaScript 的 `number`（即 64 位浮点数）。所以 `200 + count * 2` 这类表达式在 JS 层面都是浮点运算，只是最终赋值给 `int` 属性时会截断小数部分。如果你在属性绑定的表达式里写了 `3 / 2`，得到的不是 `1` 而是 `1.5`——因为这是 JavaScript 的除法，不是 C++ 的整数除法。这个坑确实绊倒过不少人。

### 4.2 字符串类型：string

`string` 是 QML 的字符串类型，对应 C++ 的 `QString`。字符串用双引号或单引号包裹：

```qml
Text {
    text: "Hello, QML"          // 双引号
    // text: 'Hello, QML'       // 单引号也可以，但习惯上用双引号
}
```

字符串内可以使用转义字符，比如 `\n` 换行、`\t` 制表符、`\\` 反斜杠。如果你需要多行字符串，QML 支持模板字符串语法：

```qml
property string description: `
    This is a multi-line string.
    It spans multiple lines.
    Very convenient for long text.
`
```

模板字符串还支持插值，使用 `${expression}` 语法：

```qml
property string userName: "Charlie"
property string greeting: `Hello, ${userName}! Welcome to QML.`
```

这和 JavaScript 的模板字符串完全一致，因为 QML 的表达式求值器就是基于 JavaScript 引擎的。

### 4.3 布尔类型：bool

`bool` 就是布尔类型，取值 `true` 或 `false`。在条件判断和可见性控制中非常常用：

```qml
Item {
    property bool isLoading: false
    property bool hasError: true

    visible: !isLoading     // 当 isLoading 为 false 时，Item 可见
    enabled: !hasError      // 当 hasError 为 true 时，Item 禁用
}
```

`bool` 类型的属性可以用逻辑运算符 `&&`（与）、`||`（或）、`!`（非）组合。QML 中的条件判断和 JavaScript 完全一致，`0`、空字符串 `""`、`null`、`undefined` 在布尔上下文中都会被当作 `false`。

### 4.4 颜色类型：color

`color` 是 QML 特有的类型，C++ 里没有直接对应物（底层是 `QColor`）。颜色值可以用十六进制字符串、SVG 颜色名称或 `Qt.rgba()` 函数来指定：

```qml
Rectangle {
    // 十六进制颜色，格式：#RRGGBB 或 #AARRGGBB
    color: "#FF5722"                // 不透明，橙红色

    // 带 alpha 通道
    // color: "#80FF5722"           // 50% 透明度的橙红色

    // SVG 颜色名称
    // color: "steelblue"

    // Qt.rgba() 函数，参数范围 0.0 ~ 1.0
    // color: Qt.rgba(0.38, 0.49, 0.55, 1.0)
}
```

十六进制颜色是最常用的写法，格式是 `#RRGGBB`（6 位，不含 alpha）或 `#AARRGGBB`（8 位，含 alpha）。注意 alpha 通道在最前面，这和 CSS 的 `rgba()` 习惯不同——CSS 是 `rgba(r, g, b, a)` 把 alpha 放最后，而 QML 的十六进制颜色是 alpha 在最前面。另外还支持 `Qt.rgba()`、`Qt.hsla()` 等函数式写法，参数都是 0.0 到 1.0 的浮点数。

`color` 类型还支持一些运算，比如 `Qt.lighter()` 和 `Qt.darker()` 可以生成更亮或更暗的变体颜色，在做主题切换或者 hover 效果时特别方便。

### 4.5 URL 类型：url

`url` 类型用于表示资源路径，在加载图片、字体、其他 QML 文件时非常常见。它的值通常是一个字符串形式的路径，支持 Qt 资源系统的 `qrc:/` 前缀、文件系统的 `file:///` 前缀，以及网络 URL：

```qml
Image {
    // 相对路径（相对于当前 QML 文件所在目录）
    source: "images/logo.png"

    // Qt 资源系统路径
    // source: "qrc:/images/logo.png"

    // 网络图片
    // source: "https://example.com/image.png"
}
```

相对路径是最常用的写法。当 QML 引擎解析 `url` 类型的属性时，它会自动把相对路径解析为绝对路径，解析的基准目录是当前 QML 文件所在的目录。如果你在 CMake 中使用了 `qt_add_qml_module`，相对路径会正确地指向资源系统中的文件位置。

### 4.6 枚举与列表

除了上面这些基础类型，QML 中还有两种常用的复合类型值得一提。

枚举类型通过自定义的 `enum` 关键字在 QML 对象中定义：

```qml
Item {
    enum Status {
        Offline,
        Connecting,
        Online,
        Error
    }

    property int status: Item.Status.Online
}
```

列表类型用方括号 `[]` 表示，可以存储多个同类型的对象：

```qml
Item {
    property var items: [1, 2, 3, 4, 5]
    property var names: ["Alice", "Bob", "Charlie"]

    // 对象列表
    children: [
        Rectangle { width: 50; height: 50; color: "red" },
        Rectangle { width: 50; height: 50; color: "blue" }
    ]
}
```

`var` 类型是 QML 中的万能类型，类似于 JavaScript 的 `var`，可以存储任何值。当你不确定类型或者需要存储混合类型的数据时，`var` 是最方便的选择。

---

## 5. id 机制——在文档内引用对象

在 QML 中，每个对象都可以有一个 `id`，这是在当前 QML 文档内唯一标识该对象的标识符。通过 `id`，你可以在任何位置引用到这个对象的属性和方法。

```qml
Window {
    width: 400
    height: 300
    visible: true

    Rectangle {
        id: backgroundRect
        anchors.fill: parent
        color: "#f5f5f5"

        Text {
            id: statusLabel
            anchors.centerIn: parent
            text: "Width: " + backgroundRect.width
            font.pixelSize: 18
        }
    }

    // 在任意位置通过 id 引用
    MouseArea {
        anchors.fill: parent
        onClicked: {
            statusLabel.text = "Clicked at " + mouseX + ", " + mouseY
            backgroundRect.color = "#e0e0e0"
        }
    }
}
```

这里 `backgroundRect` 和 `statusLabel` 就是 `id`。在 `MouseArea` 的 `onClicked` 处理器中，我们通过 `statusLabel.text` 和 `backgroundRect.color` 直接修改了这些对象的属性。

`id` 有几个重要的使用规则。第一，`id` 的值必须以小写字母或下划线开头，只能包含字母、数字和下划线。第二，`id` 在当前 QML 文档内必须唯一——如果两个对象有相同的 `id`，QML 引擎会在运行时报错。第三，`id` 不是字符串，不需要加引号——`id: myButton` 是正确的，`id: "myButton"` 也能工作但不推荐。

你可能会问：`id` 和 `property` 有什么区别？`property` 声明的是对象的属性，可以通过外部访问，也可以绑定到其他属性；而 `id` 是对象的标识符，用来引用对象本身。简单来说，`id` 指向「这个对象是谁」，`property` 描述「这个对象有什么特征」。

还有一个容易踩的坑：`id` 只在当前 QML 文档内可见。如果你在 `Main.qml` 里定义了 `id: myLabel`，在 `Other.qml` 里是访问不到的。跨文档的数据传递需要通过属性（property）或者 C++ 后端来实现。

---

## 6. JavaScript 表达式在属性值中的使用

QML 的属性值不只是简单的字面量，它完整支持 JavaScript 表达式。这意味着你可以在属性绑定中使用变量、函数调用、三元运算符、数学运算等任何合法的 JavaScript 表达式。

### 6.1 内联表达式

最常见的用法是内联表达式，直接在属性值中写短小的计算逻辑：

```qml
Item {
    width: 400
    height: 300

    Rectangle {
        width: parent.width * 0.5       // 数学运算
        height: parent.height - 20      // 减法
        color: width > 200 ? "#4CAF50" : "#F44336"   // 三元运算符
    }
}
```

`parent.width * 0.5` 是一个 JavaScript 表达式，它会计算父对象宽度的一半并赋值给 `width`。`width > 200 ? "#4CAF50" : "#F44336"` 使用三元运算符根据宽度选择不同的颜色——当宽度大于 200 时为绿色，否则为红色。

### 6.2 代码块与多行逻辑

当表达式比较复杂时，可以把逻辑放到花括号 `{}` 中，形成多行代码块。在代码块内可以使用常规的 JavaScript 语句：

```qml
Item {
    width: 400
    height: 300

    property int baseSize: 100

    Rectangle {
        width: {
            var calculated = parent.width * 0.5;
            if (calculated < baseSize) {
                calculated = baseSize;
            }
            return calculated;
        }
        height: width * 0.6
        color: "steelblue"
    }
}
```

这种多行代码块的写法虽然灵活，但我建议尽量控制使用频率。如果一个属性的计算逻辑复杂到需要 `if-else` 甚至循环，通常更好的做法是把逻辑封装到一个自定义函数或者 JavaScript 文件中，保持 QML 文件的声明式风格不被过度冲淡。

### 6.3 信号处理器中的 JavaScript

QML 的信号处理器（如 `onClicked`、`onPressed`）本质上就是 JavaScript 函数体。你可以在里面写任意复杂的逻辑：

```qml
Window {
    width: 400
    height: 300
    visible: true
    title: "QML Expression Demo"

    property int clickCount: 0

    Rectangle {
        id: background
        anchors.fill: parent
        color: "#f5f5f5"

        Text {
            id: counterLabel
            anchors.centerIn: parent
            text: "Clicks: 0"
            font.pixelSize: 24
        }

        MouseArea {
            anchors.fill: parent
            onClicked: function(mouse) {
                // 更新计数器
                clickCount = clickCount + 1;
                counterLabel.text = "Clicks: " + clickCount;

                // 根据点击次数改变背景色
                if (clickCount % 3 === 0) {
                    background.color = "#4CAF50";   // 绿色
                } else if (clickCount % 3 === 1) {
                    background.color = "#2196F3";   // 蓝色
                } else {
                    background.color = "#FF9800";   // 橙色
                }

                // 使用 console.log 调试输出
                console.log("Clicked at:", mouse.x, mouse.y,
                           "Count:", clickCount);
            }
        }
    }
}
```

这段示例中，`onClicked` 处理器接收一个 `mouse` 参数，包含了点击位置的坐标信息。处理器内部更新了自定义属性 `clickCount`，修改了 `Text` 的 `text` 属性和 `Rectangle` 的 `color` 属性，还通过 `console.log` 输出了调试信息。`console.log` 在 QML 中的输出会打印到应用程序的标准输出（或者 Qt Creator 的「应用程序输出」面板），调试时非常好用。

有一点要特别注意：在信号处理器中直接给属性赋值（如 `counterLabel.text = "Clicks: " + clickCount`），会**打断**属性绑定。这是 QML 新手最容易踩的坑之一，我们会在下一篇文章中详细讨论。

---

## 7. C++ 端如何加载 QML

QML 文件需要一个 C++ 入口来启动。这个入口通常就是一个简单的 `main.cpp`：

```cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    // 由 qt_add_qml_module 自动生成，从资源加载 QML
    const QUrl url(u"qrc:/QmlSyntaxDemo/Main.qml"_qs);

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
```

这段代码做的事情很直观：创建一个 `QGuiApplication`（注意这里用的是 `QGuiApplication` 而不是 `QApplication`，因为 QML 应用不需要 QtWidgets 模块），然后创建一个 `QQmlApplicationEngine` 来加载和运行 QML 文件。`engine.load(url)` 会加载 QML 文件并创建对象树，如果加载失败（比如文件不存在或语法错误），就会通过 `objectCreated` 信号通知，然后退出程序。

`const QUrl url(u"qrc:/QmlSyntaxDemo/Main.qml"_qs)` 这行里的路径取决于你在 CMake 中 `qt_add_qml_module` 的 `URI` 参数。URI 是 `QmlSyntaxDemo`，那么资源路径就是 `qrc:/QmlSyntaxDemo/` 前缀加上文件名。`u"...""_qs` 是 Qt 6 的 Unicode 字符串字面量写法，等价于 `QStringLiteral("...")`。

---

## 8. 完整示例

下面是本篇文章配套的完整示例代码，包含一个 CMakeLists.txt、一个 main.cpp 和一个 Main.qml 文件。这个示例综合演示了对象声明、基础类型、id 引用和 JavaScript 表达式的使用。

项目结构：

```
01-qml-syntax-basics-beginner/
  CMakeLists.txt
  main.cpp
  Main.qml
```

**CMakeLists.txt**：

```cmake
# Qt 6 QML 语法基础示例 CMake 配置
cmake_minimum_required(VERSION 3.26)
project(QmlSyntaxDemo VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_add_executable(${PROJECT_NAME}
    main.cpp
)

qt_add_qml_module(${PROJECT_NAME}
    URI QmlSyntaxDemo
    VERSION 1.0
    QML_FILES Main.qml
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Quick
)
```

**main.cpp**：

```cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/QmlSyntaxDemo/Main.qml"_qs);

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
```

**Main.qml**：

```qml
// Main.qml — QML 语法基础综合演示
// 展示：对象声明、基础类型、id 引用、JavaScript 表达式

import QtQuick

Window {
    id: root
    width: 640
    height: 480
    visible: true
    title: "QML Syntax Basics"

    // --- 自定义属性（基础类型演示） ---
    property int clickCount: 0
    property real fillRatio: 0.0
    property string statusMessage: "Ready"
    property bool isHighlighted: false
    property color normalColor: "#f5f5f5"
    property color highlightColor: "#E3F2FD"

    // --- 背景矩形 ---
    Rectangle {
        id: background
        anchors.fill: parent
        color: isHighlighted ? highlightColor : normalColor

        // 左侧信息面板
        Rectangle {
            id: infoPanel
            width: parent.width * 0.4
            height: parent.height - 40
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.verticalCenter: parent.verticalCenter
            color: "#ffffff"
            radius: 8

            // 边框用 border 属性
            border.color: "#e0e0e0"
            border.width: 1

            Column {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 16

                Text {
                    text: "QML Basic Types"
                    font.pixelSize: 20
                    font.bold: true
                    color: "#333333"
                }

                Text {
                    text: "int (clickCount): " + clickCount
                    font.pixelSize: 14
                    color: "#666666"
                }

                Text {
                    text: "real (fillRatio): " + fillRatio.toFixed(2)
                    font.pixelSize: 14
                    color: "#666666"
                }

                Text {
                    text: "string: " + statusMessage
                    font.pixelSize: 14
                    color: "#666666"
                }

                Text {
                    text: "bool (highlighted): " + isHighlighted
                    font.pixelSize: 14
                    color: "#666666"
                }

                Text {
                    text: "color: " + (isHighlighted ? highlightColor : normalColor)
                    font.pixelSize: 14
                    color: "#666666"
                }
            }
        }

        // 右侧进度可视化
        Rectangle {
            id: progressPanel
            width: parent.width * 0.5
            height: parent.height - 40
            anchors.right: parent.right
            anchors.rightMargin: 20
            anchors.verticalCenter: parent.verticalCenter
            color: "#ffffff"
            radius: 8
            border.color: "#e0e0e0"
            border.width: 1

            // 进度条（fillRatio 的可视化）
            Rectangle {
                id: progressBar
                anchors.fill: parent
                anchors.margins: 20
                color: "transparent"

                Rectangle {
                    id: progressFill
                    width: parent.width * fillRatio
                    height: 40
                    anchors.bottom: parent.bottom
                    color: {
                        // JavaScript 表达式：根据进度选择颜色
                        if (fillRatio < 0.33) return "#F44336";
                        if (fillRatio < 0.66) return "#FF9800";
                        return "#4CAF50";
                    }
                    radius: 4
                }

                Text {
                    anchors.centerIn: parent
                    text: {
                        if (fillRatio >= 1.0) return "Full!";
                        return "Fill: " + Math.round(fillRatio * 100) + "%";
                    }
                    font.pixelSize: 24
                    font.bold: true
                    color: "#333333"
                }
            }
        }
    }

    // --- 全局鼠标交互 ---
    MouseArea {
        anchors.fill: parent
        onClicked: function(mouse) {
            clickCount = clickCount + 1;
            isHighlighted = !isHighlighted;

            // 计算填充比例：点击位置 x 占窗口宽度的百分比
            fillRatio = Math.min(mouse.x / root.width, 1.0);

            statusMessage = "Click #" + clickCount
                + " at (" + Math.round(mouse.x) + ", " + Math.round(mouse.y) + ")";

            console.log("Click event:", JSON.stringify({
                x: mouse.x,
                y: mouse.y,
                count: clickCount,
                ratio: fillRatio.toFixed(2)
            }));
        }
    }
}
```

编译运行后，你会看到一个窗口，左侧是各种基础类型的实时值展示，右侧是一个可以根据鼠标点击位置变化的进度条。每次点击窗口，`clickCount` 会递增，`fillRatio` 会根据点击的 x 坐标更新，`isHighlighted` 会在 `true`/`false` 之间切换，同时背景色也会跟着变化。这个示例虽然简单，但完整地覆盖了本篇涉及的所有核心概念。

---

## 9. 小结

到这里，我们已经把 QML 的语法骨架梳理了一遍。QML 文档是由对象声明组成的树结构，属性赋值驱动了 UI 的配置，基础类型覆盖了数值、字符串、布尔、颜色和路径等常见场景，`id` 机制让我们可以在文档内灵活引用对象，而 JavaScript 表达式的全面支持又给了我们在属性值中嵌入逻辑的能力。

这些知识点单独看都不复杂，但组合在一起就是 QML 开发的基础功。下一篇我们会深入 QML 最核心的特性——属性绑定，搞清楚为什么 QML 的 `width: parent.width * 0.5` 能在父窗口大小变化时自动更新，以及什么情况下这种绑定会意外断裂。
