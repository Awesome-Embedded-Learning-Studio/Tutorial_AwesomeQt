# 现代Qt开发教程（新手篇）6.2——属性绑定与响应式数据流

## 1. 属性绑定——QML 的灵魂

上一篇我们讲了 QML 的基本语法结构，知道了怎么声明对象、怎么赋值属性、怎么用 `id` 引用对象。但如果你只是把这些当成「换了一种语法的 C++」，那就太小看 QML 了。QML 真正的核心竞争力，是它内置的属性绑定机制（Property Binding）。

什么是属性绑定？简单说就是：**一个属性的值可以自动追踪另一个属性的变化，并实时更新**。写 `width: parent.width * 0.5`，之后不管父对象的宽度怎么变，这个 `width` 都会自动重新计算。你不需要手动监听、不需要回调、不需要发射信号——QML 引擎在底层帮你把这一切都处理好了。

这种机制在前端框架中并不陌生。如果你接触过 Vue 的响应式数据或者 React 的状态驱动渲染，会发现它们的核心思想和 QML 的属性绑定非常类似：数据变了，UI 自动更新。但 QML 的实现比它们更底层、更高效，因为属性绑定是在 QML 引擎的 C++ 层直接实现的，不存在虚拟 DOM 的 diff 开销。

对于习惯了 C++ 手动管理 UI 的开发者来说，属性绑定是一个范式转换。你不再需要写 `connect(slider, &QSlider::valueChanged, label, &QLabel::setText)` 这种连接代码，而是直接在 QML 中声明 `text: slider.value`，绑定关系就自动建立了。第一次用的时候会觉得有点神奇，用习惯了就再也回不去了。

---

## 2. 环境说明

本文档基于以下环境编写和验证：

- Qt 6.9.1，使用 `qt_add_executable` + `qt_add_qml_module` 构建
- CMake 3.26+，C++17 标准
- Qt Quick 模块（`Qt6::Quick`）
- 示例代码延续上一篇的项目结构，CMakeLists.txt 和 main.cpp 基本一致

---

## 3. 属性绑定的基本原理

### 3.1 声明式绑定 vs 命令式赋值

我们先搞清楚两种赋值方式的区别。

当你写 `width: parent.width * 0.5` 时，这是一个**声明式绑定**。冒号右侧的表达式会被 QML 引擎记录下来，引擎会在 `parent.width` 发生变化时自动重新求值并更新 `width`。这个过程是持续的、自动的——只要绑定没有被手动打断，它会一直保持追踪。

而当你写 `width = 200`（在 JavaScript 代码块中），这是一个**命令式赋值**。它只会执行一次，把 `width` 设成 `200`，之后 `parent.width` 怎么变都不会影响这个值了。更严重的是，这个命令式赋值会**打断**之前存在的绑定关系。

这两种方式的区别是理解 QML 属性绑定的关键，也是后面要讲的「绑定断裂陷阱」的根源。

### 3.2 绑定的工作流程

QML 引擎在处理绑定时的内部流程大致如下。首先，引擎解析 `width: parent.width * 0.5` 这个声明，识别出表达式依赖了 `parent.width` 这个属性。然后引擎会在这个属性上注册一个依赖追踪器。当 `parent.width` 的值发生变化时，追踪器被触发，引擎重新计算 `parent.width * 0.5`，得到新值，然后赋给 `width`。这个依赖追踪是自动的——你不需要显式声明「我依赖了谁」，引擎会通过运行时分析表达式来确定依赖关系。

一个属性可以同时被多个其他属性绑定依赖。比如：

```qml
Rectangle {
    width: parent.width * 0.5
    height: parent.width * 0.3

    Text {
        // text 也依赖了 parent.width（通过 Rectangle 的 width）
        text: "Width is " + parent.width
    }
}
```

当 `parent.width` 变化时，`Rectangle` 的 `width` 和 `height` 都会自动更新，`Text` 的 `text` 也会跟着更新。引擎会按照依赖关系构建一个有向无环图，确保更新的顺序是正确的。

### 3.3 绑定中的 JavaScript 表达式

属性绑定的右侧可以是任何合法的 JavaScript 表达式，包括数学运算、函数调用、条件表达式和对象属性访问。这使得绑定不仅可以做简单的值传递，还可以表达复杂的计算逻辑：

```qml
Rectangle {
    // 数学运算
    width: Math.max(100, parent.width * 0.5)

    // 三元条件
    color: isActive ? "#4CAF50" : "#9E9E9E"

    // 调用 Qt 全局对象的方法
    border.color: Qt.darker(color, 1.5)

    // 多条件组合
    visible: isEnabled && !isHidden && opacity > 0
}
```

绑定的求值时机是「惰性」的——只有当表达式依赖的属性实际发生变化时，才会重新计算。这意味着即使表达式里调用了 `Math.max()` 这样的函数，只要依赖的属性没变，函数不会被反复调用。QML 引擎的绑定系统对性能做了大量优化，正常使用中不需要担心绑定的计算开销。

---

## 4. property 关键字——声明自定义属性

QML 中的对象自带很多属性（比如 `width`、`height`、`color`），但在实际开发中，我们经常需要定义自己的状态变量。`property` 关键字就是干这个的。

### 4.1 基本语法

自定义属性的声明格式是 `property <type> <name>: <value>`：

```qml
Item {
    // 声明自定义属性，带初始值
    property string userName: "Guest"
    property int score: 0
    property real volume: 0.8
    property bool isMuted: false
    property color accentColor: "#2196F3"
    property url avatarSource: "images/default.png"

    // 声明自定义属性但不给初始值（会使用类型的默认值）
    property string statusText   // 默认值是 ""
    property int retryCount      // 默认值是 0
    property bool isActive       // 默认值是 false
}
```

自定义属性一旦声明，就可以像内置属性一样被绑定、被引用、被其他对象通过 `id` 访问。它和内置属性在绑定系统中的地位完全平等。

### 4.2 属性的默认值

每种类型都有明确的默认值，了解这些默认值有助于排查「为什么显示为空」这类问题。`int` 和 `real` 的默认值是 `0`，`string` 的默认值是空字符串 `""`，`bool` 的默认值是 `false`，`color` 的默认值是 `transparent`（完全透明），`url` 的默认值是空字符串，`var` 的默认值是 `undefined`。

有一个特别值得留意的类型是 `var`。`var` 可以存储任意 JavaScript 值，包括对象、数组、函数等。当你需要存储复杂的数据结构时，`var` 是最灵活的选择：

```qml
Item {
    // 存储对象
    property var userConfig: ({
        "theme": "dark",
        "fontSize": 14,
        "language": "zh-CN"
    })

    // 存储数组
    property var colorList: ["#F44336", "#4CAF50", "#2196F3"]

    // 存储函数
    property var formatter: function(value) {
        return "$" + value.toFixed(2);
    }
}
```

注意 `({})` 的写法——用括号包裹对象字面量，是为了让 QML 解析器区分对象声明和代码块。如果直接写 `{ theme: "dark" }`，QML 会把它当成一个代码块，然后报语法错误。这个坑我踩过不止一次。

### 4.3 属性别名——alias

除了 `property`，QML 还提供了 `property alias`（属性别名）。别名不是创建新属性，而是给已有属性或对象起另一个名字：

```qml
Item {
    // 普通自定义属性：独立的新属性，可以有自己的值
    property string title: "Default"

    // 属性别名：直接指向已有属性或对象
    property alias buttonText: innerText.text
    property alias innerRect: backgroundRect

    Rectangle {
        id: backgroundRect
        width: 200
        height: 100
        color: "#f0f0f0"

        Text {
            id: innerText
            text: "Hello"
        }
    }
}
```

`property alias buttonText: innerText.text` 的效果是，在外部可以通过 `buttonText` 来读写 `innerText.text`。当你修改 `buttonText` 的值时，实际上是在修改 `innerText.text`。属性别名在封装自定义组件时特别有用——它允许你把内部元素的属性暴露给外部，而不需要手写 getter/setter。

`alias` 和 `property` 的核心区别在于：`property` 创建的是一个真正的新属性，有自己独立的存储空间；而 `alias` 只是已有属性的一个引用，没有额外的存储。这意味着 `alias` 不需要指定类型（因为它就是目标属性的类型），也不会有默认值。

---

## 5. onPropertyChanged 信号处理器

QML 中的每个属性都自带一个「值变化通知」信号，命名规则是 `on<PropertyName>Changed`。当你需要在一个属性变化时执行某些逻辑，就可以用这个信号处理器。

### 5.1 基本用法

```qml
Item {
    property int score: 0

    // 当 score 变化时自动触发
    onScoreChanged: function() {
        console.log("Score changed to:", score);

        if (score >= 100) {
            console.log("Achievement unlocked: Century!");
        }
    }
}
```

信号处理器中的代码会在属性值实际发生变化时执行。注意是「实际变化」——如果你给 `score` 赋了一个和当前值相同的值，处理器不会被触发。QML 引擎会先比较新旧值，只有在值确实不同时才发出通知。

属性名在信号处理器中需要首字母大写。`property int score` 对应的处理器是 `onScoreChanged`，`property string userName` 对应 `onUserNameChanged`，`property bool isActive` 对应 `onIsActiveChanged`。这个命名规则是固定的，写错了不会报编译错误，但处理器不会被调用——这类 bug 很难排查，因为运行时完全不报错，只是静默地不工作。

### 5.2 在信号处理器中访问新值和旧值

信号处理器可以直接访问当前对象的属性来获取新值。至于旧值，QML 没有内置机制提供它，但你可以手动记录：

```qml
Item {
    property int score: 0
    property int previousScore: 0

    onScoreChanged: function() {
        console.log("Score changed:", previousScore, "->", score);

        // 更新 previousScore 供下次使用
        previousScore = score;
    }
}
```

不过要注意，在 `onScoreChanged` 中给 `previousScore` 赋值不会触发 `onPreviousScoreChanged` 导致无限递归，因为 QML 引擎在信号处理器执行期间会暂时屏蔽对同一属性的通知（具体行为取决于 Qt 版本和实现细节）。但为了安全起见，在 `onXxxChanged` 中修改其他自定义属性时，要留意是否会形成循环通知链。

### 5.3 内置属性的信号处理器

Qt Quick 内置类型的大部分属性都自带变化通知。一些常用的包括 `onWidthChanged`、`onHeightChanged`、`onVisibleChanged`、`onFocusChanged` 等。`Window` 类型还有 `onClosing`，可以在窗口关闭前做清理工作。

```qml
Window {
    width: 640
    height: 480
    visible: true

    onWidthChanged: function() {
        console.log("Window width:", width);
    }

    onHeightChanged: function() {
        console.log("Window height:", height);
    }

    onClosing: function(close) {
        console.log("Window is about to close");
        // 如果需要阻止关闭：close.accepted = false;
    }
}
```

---

## 6. 绑定断裂——QML 新手最大的坑

讲到这里，我们终于要聊 QML 中最容易让新手血压拉满的问题了：绑定断裂（Binding Breakage）。

### 6.1 什么是绑定断裂

绑定断裂指的是：一个通过声明式绑定建立的属性关联，被命令式赋值覆盖后，绑定关系永久丢失，属性不再自动追踪依赖的变化。

来看一个经典的翻车案例：

```qml
Item {
    width: 400
    height: 300

    property int baseWidth: 200

    Rectangle {
        id: rect
        width: parent.width * 0.5     // 声明式绑定
        height: 100
        color: "#4CAF50"

        MouseArea {
            anchors.fill: parent
            onClicked: function() {
                // 命令式赋值——这里会打断绑定！
                rect.width = 150;
                console.log("Width after click:", rect.width);
            }
        }
    }

    // 一个按钮来修改 baseWidth
    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 50
        color: "#2196F3"

        Text {
            anchors.centerIn: parent
            text: "Click the green rectangle first, then resize the window"
            color: "white"
        }

        MouseArea {
            anchors.fill: parent
            onClicked: function() {
                baseWidth = 300;    // 修改 baseWidth
            }
        }
    }
}
```

在这个例子中，绿色 `Rectangle` 的 `width` 最初绑定了 `parent.width * 0.5`。当你点击绿色矩形时，`rect.width = 150` 这个命令式赋值会覆盖掉绑定，`width` 被固定为 `150`。之后不管你怎么调整窗口大小或者修改 `baseWidth`，`rect.width` 都不会再自动更新了——因为绑定关系已经断了。

这就是绑定断裂。一旦绑定被命令式赋值打断，那个属性就变成了一个「死」的值，不再响应任何依赖变化。

### 6.2 为什么会发生绑定断裂

从底层机制来看，QML 引擎为每个声明式绑定维护了一个「绑定对象」，它记录了表达式和依赖关系。当你执行命令式赋值时，引擎会释放旧的绑定对象，把属性值替换为你赋的新值。绑定对象一旦被释放，就无法恢复了——引擎不会记住「之前的绑定表达式是什么」，它只是简单地把绑定对象销毁掉。

这种设计是有意为之的。如果引擎在每次命令式赋值后都保留旧的绑定，就会产生大量的「僵尸绑定」，导致内存泄漏和不可预测的行为。所以 Qt 的选择是：命令式赋值就是明确地表达「我要手动控制这个值，不再需要自动绑定了」。

### 6.3 如何避免绑定断裂

避免绑定断裂的核心原则只有一个：**在信号处理器或 JavaScript 函数中，尽量不要直接给有绑定的属性做命令式赋值**。

具体来说，有几种常见的替代方案。

第一种方案是修改绑定的依赖源。不直接给目标属性赋值，而是修改它所依赖的属性：

```qml
Item {
    property int baseWidth: 200

    Rectangle {
        // width 始终通过绑定计算，不直接赋值
        width: baseWidth
        height: 100
        color: "#4CAF50"

        MouseArea {
            anchors.fill: parent
            onClicked: function() {
                // 修改 baseWidth 而不是直接改 width
                baseWidth = 150;     // 绑定保持完好
            }
        }
    }
}
```

通过修改 `baseWidth`，`Rectangle` 的 `width` 仍然通过 `width: baseWidth` 这个绑定自动更新。绑定关系没有被破坏，只是绑定的输入值变了。

第二种方案是使用 `Qt.binding()` 函数在命令式代码中重新建立绑定：

```qml
Item {
    Rectangle {
        id: rect
        width: parent.width * 0.5
        height: 100

        MouseArea {
            anchors.fill: parent
            onClicked: function() {
                // 先做命令式赋值
                rect.width = 150;

                // 然后重新建立绑定
                rect.width = Qt.binding(function() {
                    return rect.parent.width * 0.3;
                });
            }
        }
    }
}
```

`Qt.binding()` 接受一个函数作为参数，返回一个绑定对象。把这个绑定对象赋值给属性时，引擎会重新建立绑定关系。这种写法适合需要在运行时动态切换绑定表达式的场景。

第三种方案是使用 `Binding` 元素，这是一个 QML 内置类型，专门用来显式管理绑定关系：

```qml
Item {
    property bool useCompactMode: false

    Rectangle {
        id: rect
        height: 100

        // 默认宽度
        width: parent.width * 0.5

        // 用 Binding 元素条件性地覆盖
        Binding {
            target: rect
            property: "width"
            value: 100
            when: useCompactMode
        }

        MouseArea {
            anchors.fill: parent
            onClicked: function() {
                useCompactMode = !useCompactMode;
            }
        }
    }
}
```

`Binding` 元素的好处是它有一个 `when` 属性来控制绑定何时生效。当 `when` 为 `false` 时，`Binding` 不干预目标属性，原有的绑定继续工作；当 `when` 为 `true` 时，`Binding` 接管目标属性，提供新的绑定值。当 `when` 重新变回 `false` 时，`Binding` 会自动恢复原来的绑定。这个机制完美地解决了「条件性覆盖绑定」的问题。

### 6.4 如何检测绑定断裂

在调试阶段，你可以通过设置环境变量 `QML_IMPORT_TRACE=1` 或者使用 `qml` 命令行工具的 `-w` 参数来启用绑定警告。Qt Creator 也提供了 QML Profiler 工具，可以可视化地查看绑定求值的频率和耗时，帮助你定位哪些绑定可能被意外打断。

不过最靠谱的方式还是从代码设计上避免——养成「不在信号处理器中直接给绑定属性赋值」的习惯，绑定断裂的问题就会大幅减少。

---

## 7. 完整示例

下面是一个综合演示属性绑定、自定义属性、信号处理器和绑定断裂的完整示例。

项目结构：

```
02-property-binding-beginner/
  CMakeLists.txt
  main.cpp
  Main.qml
```

**CMakeLists.txt**：

```cmake
# Qt 6 属性绑定示例 CMake 配置
cmake_minimum_required(VERSION 3.26)
project(PropertyBindingDemo VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_add_executable(${PROJECT_NAME}
    main.cpp
)

qt_add_qml_module(${PROJECT_NAME}
    URI PropertyBindingDemo
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
    const QUrl url(u"qrc:/PropertyBindingDemo/Main.qml"_qs);

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
// Main.qml — 属性绑定与响应式数据流综合演示
// 展示：属性绑定、自定义属性、onPropertyChanged、绑定断裂与修复

import QtQuick

Window {
    id: root
    width: 700
    height: 500
    visible: true
    title: "Property Binding Demo"

    // --- 自定义属性 ---
    property int sliderValue: 50
    property string statusText: "Normal"
    property color themeColor: "#2196F3"
    property bool useDarkMode: false
    property real scaleFactor: 1.0

    // --- 信号处理器 ---
    onSliderValueChanged: function() {
        // 根据滑块值更新状态文字
        if (sliderValue < 30) {
            statusText = "Low";
        } else if (sliderValue < 70) {
            statusText = "Normal";
        } else {
            statusText = "High";
        }
        console.log("Slider value changed:", sliderValue, "Status:", statusText);
    }

    onUseDarkModeChanged: function() {
        themeColor = useDarkMode ? "#BB86FC" : "#2196F3";
        console.log("Theme changed, dark mode:", useDarkMode);
    }

    // --- 背景层 ---
    Rectangle {
        anchors.fill: parent
        color: useDarkMode ? "#1a1a2e" : "#f5f5f5"
    }

    // --- 顶部标题栏 ---
    Rectangle {
        id: titleBar
        width: parent.width
        height: 60
        color: themeColor

        Text {
            anchors.centerIn: parent
            text: "Property Binding Demo"
            font.pixelSize: 22
            font.bold: true
            color: "#ffffff"
        }

        // 标题栏宽度绑定到窗口宽度——尝试拖拽窗口边缘观察效果
        onWidthChanged: function() {
            // 这个不会打断绑定，因为我们没有给 width 赋值
        }
    }

    // --- 主内容区 ---
    Item {
        anchors.top: titleBar.bottom
        anchors.bottom: controlBar.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 20

        // 左侧：值可视化
        Rectangle {
            id: gaugePanel
            width: parent.width * 0.45
            height: parent.height
            anchors.left: parent.left
            color: useDarkMode ? "#16213e" : "#ffffff"
            radius: 8
            border.color: useDarkMode ? "#333366" : "#e0e0e0"
            border.width: 1

            // 进度条——属性绑定的典型应用
            Rectangle {
                id: gaugeFill
                width: parent.width * 0.8
                height: 30
                anchors.centerIn: parent
                radius: 15
                color: useDarkMode ? "#0f3460" : "#e8eaf6"
                clip: true

                Rectangle {
                    // 绑定到 sliderValue：宽度随滑块值变化
                    width: parent.width * (sliderValue / 100)
                    height: parent.height
                    radius: 15
                    color: {
                        if (sliderValue < 30) return "#F44336";
                        if (sliderValue < 70) return themeColor;
                        return "#4CAF50";
                    }
                }
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: gaugeFill.bottom
                anchors.topMargin: 16
                text: sliderValue + "%"
                font.pixelSize: 36
                font.bold: true
                color: useDarkMode ? "#e0e0e0" : "#333333"
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: gaugeFill.bottom
                anchors.topMargin: 70
                text: "Status: " + statusText
                font.pixelSize: 16
                color: useDarkMode ? "#aaaaaa" : "#666666"
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 16
                text: "scaleFactor: " + scaleFactor.toFixed(2)
                font.pixelSize: 13
                color: useDarkMode ? "#888888" : "#999999"
            }
        }

        // 右侧：属性信息面板
        Rectangle {
            id: infoPanel
            width: parent.width * 0.45
            height: parent.height
            anchors.right: parent.right
            color: useDarkMode ? "#16213e" : "#ffffff"
            radius: 8
            border.color: useDarkMode ? "#333366" : "#e0e0e0"
            border.width: 1

            Column {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 12

                Text {
                    text: "Property Values"
                    font.pixelSize: 18
                    font.bold: true
                    color: useDarkMode ? "#e0e0e0" : "#333333"
                }

                // 所有文本都通过属性绑定自动更新
                Text {
                    text: "sliderValue (int): " + sliderValue
                    font.pixelSize: 14
                    color: useDarkMode ? "#cccccc" : "#555555"
                }

                Text {
                    text: "statusText (string): " + statusText
                    font.pixelSize: 14
                    color: useDarkMode ? "#cccccc" : "#555555"
                }

                Text {
                    text: "themeColor (color): " + themeColor.toString()
                    font.pixelSize: 14
                    color: useDarkMode ? "#cccccc" : "#555555"
                }

                Text {
                    text: "useDarkMode (bool): " + useDarkMode
                    font.pixelSize: 14
                    color: useDarkMode ? "#cccccc" : "#555555"
                }

                Text {
                    text: "scaleFactor (real): " + scaleFactor.toFixed(2)
                    font.pixelSize: 14
                    color: useDarkMode ? "#cccccc" : "#555555"
                }

                // 分隔线
                Rectangle {
                    width: parent.width
                    height: 1
                    color: useDarkMode ? "#333366" : "#e0e0e0"
                }

                Text {
                    text: "Window: " + root.width + " x " + root.height
                    font.pixelSize: 14
                    color: useDarkMode ? "#cccccc" : "#555555"
                }

                Text {
                    text: "Gauge width: " + Math.round(gaugePanel.width * 0.8)
                    font.pixelSize: 14
                    color: useDarkMode ? "#cccccc" : "#555555"
                }
            }
        }
    }

    // --- 底部控制栏 ---
    Rectangle {
        id: controlBar
        width: parent.width
        height: 120
        anchors.bottom: parent.bottom
        color: useDarkMode ? "#16213e" : "#ffffff"
        border.color: useDarkMode ? "#333366" : "#e0e0e0"
        border.width: 1

        Row {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 20

            // 滑块控制区
            Column {
                width: parent.width * 0.4
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8

                Text {
                    text: "Slider Value"
                    font.pixelSize: 14
                    font.bold: true
                    color: useDarkMode ? "#e0e0e0" : "#333333"
                }

                // 模拟滑块——用 Rectangle + MouseArea
                Rectangle {
                    width: parent.width
                    height: 20
                    radius: 10
                    color: useDarkMode ? "#0f3460" : "#e0e0e0"

                    Rectangle {
                        width: parent.width * (sliderValue / 100)
                        height: parent.height
                        radius: 10
                        color: themeColor
                    }

                    MouseArea {
                        anchors.fill: parent
                        onMouseXChanged: function(mouse) {
                            if (pressed) {
                                var ratio = Math.max(0, Math.min(1, mouse.x / width));
                                // 修改 sliderValue 而不是直接改进度条宽度
                                sliderValue = Math.round(ratio * 100);
                            }
                        }
                    }
                }
            }

            // 缩放控制
            Column {
                width: parent.width * 0.25
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8

                Text {
                    text: "Scale Factor: " + scaleFactor.toFixed(1)
                    font.pixelSize: 14
                    font.bold: true
                    color: useDarkMode ? "#e0e0e0" : "#333333"
                }

                Row {
                    spacing: 8

                    Rectangle {
                        width: 60
                        height: 36
                        radius: 6
                        color: themeColor

                        Text {
                            anchors.centerIn: parent
                            text: "Smaller"
                            font.pixelSize: 12
                            color: "#ffffff"
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: function() {
                                // 修改依赖属性，绑定自动更新
                                scaleFactor = Math.max(0.5, scaleFactor - 0.1);
                            }
                        }
                    }

                    Rectangle {
                        width: 60
                        height: 36
                        radius: 6
                        color: themeColor

                        Text {
                            anchors.centerIn: parent
                            text: "Larger"
                            font.pixelSize: 12
                            color: "#ffffff"
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: function() {
                                scaleFactor = Math.min(2.0, scaleFactor + 0.1);
                            }
                        }
                    }
                }
            }

            // 主题切换
            Column {
                width: parent.width * 0.25
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8

                Text {
                    text: "Theme"
                    font.pixelSize: 14
                    font.bold: true
                    color: useDarkMode ? "#e0e0e0" : "#333333"
                }

                Rectangle {
                    width: 120
                    height: 36
                    radius: 6
                    color: useDarkMode ? "#BB86FC" : "#e8eaf6"
                    border.color: themeColor
                    border.width: 2

                    Text {
                        anchors.centerIn: parent
                        text: useDarkMode ? "Dark Mode" : "Light Mode"
                        font.pixelSize: 13
                        color: useDarkMode ? "#ffffff" : themeColor
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: function() {
                            useDarkMode = !useDarkMode;
                        }
                    }
                }
            }
        }
    }
}
```

运行这个示例后，你会发现几个有趣的交互效果。拖拽窗口边缘改变大小，左侧面板和右侧面板的宽度会自动按比例调整——因为它们的 `width` 绑定了 `parent.width * 0.45`。拖动滑块改变 `sliderValue`，进度条的填充宽度、颜色、状态文字都会自动响应——因为它们全部通过属性绑定关联到了 `sliderValue`。点击主题切换按钮，整个界面的配色方案瞬间切换——因为所有颜色属性都绑定了 `useDarkMode` 这个条件。而这一切的代码中，我们从来没有手动写过任何 `connect` 或者回调注册，全部是通过声明式绑定实现的。

---

## 8. 小结

属性绑定是 QML 最核心的机制，也是它和传统命令式 UI 框架最本质的区别。通过声明式绑定，你可以用一行代码建立「数据变了、UI 自动更新」的响应式关系，不需要手动管理监听器、回调函数或信号连接。`property` 关键字让你可以灵活地定义应用的状态变量，`onPropertyChanged` 处理器让你在属性变化时执行副作用逻辑，而理解绑定断裂的成因和避免方法则是写好 QML 代码的关键。

下一篇我们会进入 Qt Quick Controls 的世界，看看 QML 中有哪些开箱即用的 UI 组件，以及如何用布局系统把它们整齐地排列起来。
