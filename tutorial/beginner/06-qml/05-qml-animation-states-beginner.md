# 现代Qt开发教程（新手篇）6.5——QML 动画与状态机基础

## 1. 为什么要学 QML 动画

静态界面能用，但不好用。一个按钮从灰色变成蓝色，如果是一瞬间切换，用户甚至可能注意不到自己按了它。但如果有一个 150 毫秒的渐变过渡，视觉反馈就完全不一样了——用户明确地感知到「我触发了什么」。动画不是花架子，它是人机交互反馈链路中不可或缺的一环。

在传统的 QtWidgets 里做动画，要么用 `QPropertyAnimation` 配合属性系统，要么手动写定时器在 `paintEvent` 里插帧。功能上都能实现，但代码量大、维护成本高，而且一旦动画逻辑复杂起来（多个属性同时变、串行等待、条件分支），代码结构就会变得非常混乱。QML 则把动画做成了语言级别的一等公民。你不需要额外创建动画对象、不需要手动管理时间线、不需要写回调——直接在属性声明旁边写一行 `Behavior on width { NumberAnimation { duration: 200 } }`，这个属性的所有变化就自动带上了平滑过渡。

这篇文章我们要把 QML 动画体系从底层到上层全部走一遍。先看最基础的 `PropertyAnimation` 家族，它们能对任何 QML 属性做插值动画。然后学习 `Behavior on`，这是一种声明式的自动过渡方式，让属性变化自带动画而不需要显式触发。最后进入 `State` + `Transition` 的组合，这是 QML 状态机的核心——通过定义离散的界面状态和状态之间的切换动画，来组织复杂的界面变化。

---

## 2. 环境说明

本文档基于以下环境编写和验证：

- Qt 6.9.1，使用 `qt_add_executable` + `qt_add_qml_module` 构建
- CMake 3.26+，C++17 标准
- 仅需 `Qt6::Quick` 模块，动画类型全部内置在 `QtQuick` 中
- 本篇示例不涉及 C++ 代码（纯 QML 动画），main.cpp 和 CMakeLists.txt 延续基础模板

---

## 3. PropertyAnimation 家族——数值插值动画

### 3.1 动画的基本原理

QML 动画的核心机制是「属性插值」。你指定一个起始值、一个目标值和一个持续时间，动画引擎会在持续时间内按时间比例计算出中间值，逐帧赋给目标属性。比如从 `x: 0` 动画到 `x: 300`，持续 1000 毫秒，引擎在 500 毫秒时会把 `x` 设成 150，在 750 毫秒时设成 225，以此类推。

`PropertyAnimation` 是所有属性动画的基类。它有一个 `target` 属性指定动画作用的对象，一个 `property` 指定要动画的属性名，以及 `from`、`to`、`duration` 分别指定起始值、目标值和持续时间。但在实际使用中，我们更多用的是它的子类——`NumberAnimation`、`ColorAnimation`、`RotationAnimation` 等——它们针对特定类型做了简化。

### 3.2 NumberAnimation——数值属性动画

`NumberAnimation` 专门用于 `real`、`int` 等数值类型的属性动画。它的 `from` 和 `to` 直接接受数字，不需要额外的类型转换。

我们先看一个使用 `NumberAnimation` 做位移动画的例子。下面的代码让一个矩形在点击时平滑地左右移动：

```qml
Rectangle {
    id: movingBox
    width: 80
    height: 80
    color: "#3498db"
    radius: 8

    // 点击触发位移动画
    MouseArea {
        anchors.fill: parent
        onClicked: moveAnimation.running = !moveAnimation.running
    }

    // 显式动画：通过 running 属性控制启停
    NumberAnimation on x {
        id: moveAnimation
        from: 50
        to: 400
        duration: 800
        // 缓动函数：先快后慢
        easing.type: Easing.OutCubic
        running: false
        loops: Animation.Infinite
        // 每次循环来回往返
        onFinished: {
            // 切换方向
            var tmp = from;
            from = to;
            to = tmp;
            running = true;
        }
    }
}
```

这里有个比较重要的属性是 `easing`。`Easing.OutCubic` 表示「先快后慢」的减速曲线。QML 提供了几十种缓动曲线，从最简单的线性（`Easing.Linear`，匀速）到弹性效果（`Easing.OutElastic`，像弹簧一样回弹）。缓动函数的选择对动画的「手感」影响极大，同样的距离和时长，`Linear` 和 `OutCubic` 给人的感觉完全不同。

### 3.3 ColorAnimation——颜色渐变动画

`ColorAnimation` 是另一个常用的子类，专门处理 `color` 类型属性的动画。它的 `from` 和 `to` 接受颜色值，引擎会在 RGB 空间中做插值：

```qml
Rectangle {
    id: colorBox
    width: 120
    height: 120
    color: "#e74c3c"

    ColorAnimation on color {
        id: colorAnim
        from: "#e74c3c"
        to: "#2ecc71"
        duration: 600
        easing.type: Easing.InOutQuad
        running: false
    }

    MouseArea {
        anchors.fill: parent
        onClicked: colorAnim.running = true
    }
}
```

颜色动画的插值在 RGB 空间中进行，这意味着如果从红色 `(255, 0, 0)` 过渡到绿色 `(0, 255, 0)`，中间会经过一些灰暗的色调（因为两个通道同时变化）。如果你想要更自然的颜色过渡（比如走 HSL 色彩空间的色相环），需要自己写 JavaScript 插值逻辑。不过对于大多数 UI 场景，RGB 插值已经够用了。

### 3.4 PropertyAnimation——通用属性动画

当你的动画目标不是一个简单的数值或颜色，而是其他类型的属性（比如 `font.pixelSize`、`opacity`、`scale`），就需要回退到通用的 `PropertyAnimation`。它的用法和子类类似，只是没有预设的类型约束：

```qml
PropertyAnimation {
    target: someText
    property: "opacity"
    from: 0.0
    to: 1.0
    duration: 500
}
```

---

## 4. Behavior on——声明式自动过渡

### 4.1 Behavior 的核心思想

前面讲的 `NumberAnimation`、`ColorAnimation` 都是显式触发的——你需要在合适的时机把 `running` 设成 `true`，或者通过 `start()` 方法启动。这种方式的控制粒度很细，但代码量也多。

`Behavior on` 提供了一种完全不同的思路：你不主动触发动画，而是告诉 QML 引擎「这个属性的任何变化都自动带上动画过渡」。之后不管属性是怎么变的——用户点击、绑定更新、JavaScript 赋值——引擎都会自动插入平滑过渡，而不是瞬间跳变。

```qml
Rectangle {
    width: 200
    height: 100
    color: mouseArea.containsPress ? "#e74c3c" : "#3498db"
    radius: mouseArea.containsPress ? 20 : 8

    // color 变化时自动过渡
    Behavior on color {
        ColorAnimation { duration: 200 }
    }

    // radius 变化时自动过渡
    Behavior on radius {
        NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
    }
}
```

这段代码没有任何显式的动画启动逻辑。`color` 和 `radius` 的值由 `containsPress` 这个布尔表达式决定，当鼠标按下或释放时，三元表达式会产生新的值，`Behavior on` 捕获到变化后自动启动过渡动画。

`Behavior on` 最大的优势是声明式——你不需要关心「什么时候触发」，只需要声明「怎么过渡」。这和 QML 的属性绑定哲学高度一致：描述结果，而不是描述过程。在按钮悬停效果、面板展开收起、颜色主题切换这些场景下，`Behavior on` 是最高效的写法。

### 4.2 多属性联动动画

当多个属性同时有 `Behavior` 时，它们会并行执行。这使得我们可以非常简洁地实现多属性联动的视觉效果：

```qml
Rectangle {
    id: card
    width: 160
    height: 120
    color: "#ffffff"
    radius: 12
    border.color: "#e0e0e0"

    scale: mouseArea.containsMouse ? 1.05 : 1.0
    opacity: mouseArea.containsMouse ? 1.0 : 0.85

    Behavior on scale {
        NumberAnimation { duration: 150; easing.type: Easing.OutCubic }
    }
    Behavior on opacity {
        NumberAnimation { duration: 150 }
    }

    // 阴影效果通过额外的 Rectangle 模拟
    Rectangle {
        anchors.fill: parent
        anchors.margins: -4
        radius: parent.radius + 2
        color: "transparent"
        border.color: "#00000010"
        z: -1

        Behavior on anchors.margins {
            NumberAnimation { duration: 150 }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
    }
}
```

这段代码模拟了一个 Material Design 风格的卡片悬停效果。鼠标进入时卡片微微放大、透明度提升，离开时恢复。所有过渡都通过 `Behavior on` 自动完成，代码极其简洁。

---

## 5. State + Transition——状态驱动的界面切换

### 5.1 为什么需要状态机

`Behavior on` 适合单个属性的简单过渡，但当界面的变化涉及大量属性的联动，而且有多种不同的「模式」或「布局」需要切换时，`Behavior on` 就不够用了。你需要在 JavaScript 里维护一堆标志位来判断当前是哪种状态，然后分别设置各个属性——代码很快就变成一团面条。

QML 的 `State` 和 `Transition` 就是为了解决这个问题而生的。你可以把界面定义成若干个具名状态，每个状态包含一组属性修改（`PropertyChanges`），然后声明状态之间的切换动画（`Transition`）。切换状态只需要一行 `state = "expanded"`，QML 引擎会自动应用该状态的所有属性修改，并通过 `Transition` 插入动画。

### 5.2 定义状态

`State` 通过 `states` 属性定义，它是一个 `State` 对象的列表。每个 `State` 有一个 `name` 标识，以及一系列 `PropertyChanges` 描述在该状态下哪些属性要变成什么值：

```qml
Rectangle {
    id: panel
    width: 300
    height: 60
    color: "#2c3e50"
    radius: 8

    // 默认状态下的属性值已经在上面声明
    // 以下是具名状态
    states: [
        State {
            name: "collapsed"
            PropertyChanges {
                target: panel
                height: 60
            }
            PropertyChanges {
                target: contentText
                opacity: 0
            }
            PropertyChanges {
                target: expandIcon
                rotation: 0
            }
        },
        State {
            name: "expanded"
            PropertyChanges {
                target: panel
                height: 200
            }
            PropertyChanges {
                target: contentText
                opacity: 1.0
            }
            PropertyChanges {
                target: expandIcon
                rotation: 180
            }
        }
    ]

    // 默认状态为 collapsed
    state: "collapsed"

    Text {
        id: contentText
        anchors.fill: parent
        anchors.margins: 16
        anchors.topMargin: 40
        text: "Here is the expanded content. It can be anything — text, images, or other components."
        color: "#ecf0f1"
        font.pixelSize: 14
        wrapMode: Text.WordWrap
    }

    Text {
        id: expandIcon
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 16
        text: "\u25BC"  // 下三角
        color: "#ecf0f1"
        font.pixelSize: 16
    }

    MouseArea {
        anchors.fill: parent
        onClicked: panel.state = (panel.state === "collapsed") ? "expanded" : "collapsed"
    }
}
```

`PropertyChanges` 的 `target` 指向要修改的对象，然后列出该对象在新状态下应该具有的属性值。这些修改是相对默认状态的覆盖——切换到某个状态时，被 `PropertyChanges` 提到的属性会被设成新值，没提到的属性保持默认值不变。切换回默认状态（空字符串 `""`）时，所有覆盖都会被撤销，属性恢复到初始声明。

### 5.3 Transition——状态切换的动画

光有状态切换还不够——不加动画的话，面板高度会瞬间从 60 跳到 200，内容文本突然出现，体验非常生硬。`Transition` 就是在状态切换过程中插入的动画：

```qml
transitions: [
    Transition {
        // 从 collapsed 到 expanded 的切换动画
        from: "collapsed"
        to: "expanded"

        // 并行动画组：所有动画同时播放
        ParallelAnimation {
            NumberAnimation {
                target: panel
                property: "height"
                duration: 300
                easing.type: Easing.OutCubic
            }
            NumberAnimation {
                target: contentText
                property: "opacity"
                duration: 300
            }
            NumberAnimation {
                target: expandIcon
                property: "rotation"
                duration: 300
            }
        }
    },
    Transition {
        // 反向：从 expanded 回到 collapsed
        from: "expanded"
        to: "collapsed"

        ParallelAnimation {
            NumberAnimation {
                target: panel
                property: "height"
                duration: 250
                easing.type: Easing.InCubic
            }
            NumberAnimation {
                target: contentText
                property: "opacity"
                duration: 150
            }
            NumberAnimation {
                target: expandIcon
                property: "rotation"
                duration: 250
            }
        }
    }
]
```

`from` 和 `to` 指定了这个 Transition 适用的状态切换方向。展开和收起可以使用不同的动画参数——展开时用 `OutCubic`（先快后慢），收起时用 `InCubic`（先慢后快），这在视觉上更符合物理直觉（物体展开时先加速后减速，折叠时反过来）。

`ParallelAnimation` 表示内部的所有动画同时播放。如果你需要串行（一个播完再播下一个），可以用 `SequentialAnimation`。这两种组合动画也可以嵌套使用——比如前 300 毫秒展开高度，然后 200 毫秒淡入内容，这种「先移后显」的效果就需要 `SequentialAnimation` 包裹两个阶段。

### 5.4 reversible 与 Animation. reversible

如果你发现展开和收起的动画参数基本一样，只是方向相反，可以用一个 Transition 搞定，加一个 `reversible: true` 属性：

```qml
transitions: Transition {
    ParallelAnimation {
        NumberAnimation {
            property: "height"
            duration: 300
            easing.type: Easing.OutCubic
        }
        NumberAnimation {
            property: "opacity"
            duration: 300
        }
        NumberAnimation {
            property: "rotation"
            duration: 300
        }
    }
}
```

当 `from` 和 `to` 都不指定时，这个 Transition 会匹配所有状态切换。QML 引擎会自动在正向和反向播放时反转动画方向。

---

## 6. SequentialAnimation 与 ParallelAnimation——组合动画

### 6.1 串行动画

有些效果需要严格按顺序执行。比如一个「脉冲」效果——先放大、再缩回、再放大、最后恢复。这种多步骤的串联需要 `SequentialAnimation`：

```qml
SequentialAnimation {
    id: pulseAnimation
    running: false

    NumberAnimation {
        target: pulseBox
        property: "scale"
        to: 1.3
        duration: 150
        easing.type: Easing.OutCubic
    }
    NumberAnimation {
        target: pulseBox
        property: "scale"
        to: 0.95
        duration: 100
        easing.type: Easing.InOutCubic
    }
    NumberAnimation {
        target: pulseBox
        property: "scale"
        to: 1.0
        duration: 80
        easing.type: Easing.OutCubic
    }
}
```

`SequentialAnimation` 内部的子动画会按顺序逐个播放，一个结束后下一个才开始。总时长是所有子动画时长之和。

### 6.2 并行动画

`ParallelAnimation` 则让所有子动画同时开始播放，总时长等于最长的那个子动画的时长。这在需要多个属性同时变化的场景下非常常用：

```qml
ParallelAnimation {
    id: appearAnimation
    running: false

    NumberAnimation {
        target: someItem
        property: "opacity"
        from: 0
        to: 1
        duration: 500
    }
    NumberAnimation {
        target: someItem
        property: "y"
        from: 30
        to: 0
        duration: 400
        easing.type: Easing.OutCubic
    }
}
```

两种组合动画可以自由嵌套。一个 `SequentialAnimation` 里可以包含 `ParallelAnimation`，反之亦然。这使得你能表达非常复杂的动画编排逻辑——比如「先并行做 A 和 B，等它们都完成后串行做 C，C 完成后再并行做 D 和 E」。

---

## 7. 完整示例——动画与状态机综合演示

现在我们把前面讲的所有动画机制整合到一个完整的项目里。这个示例包含了 `NumberAnimation`、`ColorAnimation`、`Behavior on`、`State` + `Transition`，以及 `SequentialAnimation` / `ParallelAnimation` 的使用。

### 7.1 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.26)
project(QmlAnimationDemo VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_add_executable(${PROJECT_NAME}
    main.cpp
)

qt_add_qml_module(${PROJECT_NAME}
    URI QmlAnimationDemo
    VERSION 1.0
    QML_FILES Main.qml
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Quick
)
```

### 7.2 main.cpp

```cpp
/*
 *  Qt 6 入门教程 - 示例 6.5
 *  主题：QML 动画与状态机基础
 *
 * 本示例为纯 QML 动画演示，C++ 端仅负责加载 QML 引擎。
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/QmlAnimationDemo/Main.qml"_qs);

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

### 7.3 Main.qml

完整的 QML 文件请参考示例代码目录中的 `Main.qml`，这里不再全部贴出。它会整合本文讲到的所有动画类型，包括：

- 一个用 `NumberAnimation` 做来回平移的小球
- 一个用 `ColorAnimation` 做颜色循环的矩形
- 一组用 `Behavior on` 实现悬停效果的卡片
- 一个用 `State` + `Transition` 控制展开/折叠的面板
- 一个用 `SequentialAnimation` 实现脉冲效果的按钮

---

## 8. 动画性能与常见陷阱

动画写起来很爽，但有几个性能陷阱新手容易踩到。

第一个是动画属性的选择。QML 的渲染架构分为 UI 线程和渲染线程。`x`、`y`、`width`、`height` 这些属性的动画会触发布局重算，开销较大。而 `opacity`、`scale`、`rotation` 这些属性的动画可以完全在 GPU 上完成（通过 transform 矩阵），不需要重新走布局流程。如果你的动画只涉及位置和大小变化，考虑用 `transform` 替代——把元素放在 `Item` 里，用 `translate` 做 GPU 加速的位移动画。

第二个是 `Behavior on` 的滥用。`Behavior on` 会对属性的每一次变化都启动动画，包括初始化时的属性赋值。如果你一个 `ListView` 的 delegate 里有 `Behavior on height`，每次列表滚动复用 delegate 时都会播放高度动画，这绝对不是你想要的效果。解决方案是在 `Component.onCompleted` 之后再启用 Behavior，或者改用显式的 `Transition`。

第三个是动画与绑定的冲突。如果一个属性同时有绑定和动画，动画运行时会临时覆盖绑定。动画结束后，绑定会恢复——前提是你没有在动画过程中用命令式赋值打断了绑定。这个坑在第六章（属性绑定）里已经提过，但在动画场景下更容易触发。

到这里就大功告成了。QML 的动画体系从简单的 `Behavior on` 到复杂的 `State` + `Transition` 编排，覆盖了从「让按钮有个过渡」到「实现完整界面切换动效」的全部需求。下一篇我们进入 Model/Delegate 数据驱动视图，把数据和 UI 接起来。
