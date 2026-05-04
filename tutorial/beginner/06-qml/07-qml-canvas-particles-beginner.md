# 现代Qt开发教程（新手篇）6.7——QML Canvas 绘图与粒子系统

## 1. 当 QML 需要自定义绘制能力

前面六篇教程我们用的都是 QML 内置的视觉元素——`Rectangle` 画方块、`Text` 显示文字、`Image` 加载图片。组合这些元素已经能搭建绝大多数常规 UI 了。但总有些场景是声明式组件覆盖不了的：你想画一个自定义的图表、实现一个涂鸦板、渲染一个动态的分形图案，或者干脆就是要实现一个炫酷的粒子烟花效果。这些需求的核心都是「逐像素控制渲染」，而这正是 `Canvas` 和 `ParticleSystem` 的领域。

QML 的 `Canvas` 类型提供了一个完整的 HTML5 Canvas 2D 绘图 API。如果你写过 Web 前端，那这套 API 你已经很熟了——`getContext('2d')`、`fillRect()`、`arc()`、`lineTo()`、`fillStyle` 这些方法和属性在 QML Canvas 里几乎一模一样。Qt 把这个 API 做了近乎完整的移植，所以你从 JavaScript Canvas 编程迁移过来的成本几乎为零。

`ParticleSystem` 则是 QML 独有的粒子特效框架。它不需要你手动计算每一帧的粒子位置——你只需要声明「粒子从哪里发射（Emitter）」「长什么样（ImageParticle）」「受什么力影响（Affector）」，引擎在 GPU 上帮你完成所有物理模拟和渲染。从雪花飘落到烟花爆炸，从火焰效果到烟雾弥漫，几行声明式代码就能实现。

---

## 2. 环境说明

本文档基于以下环境编写和验证：

- Qt 6.9.1，使用 `qt_add_executable` + `qt_add_qml_module` 构建
- CMake 3.26+，C++17 标准
- Canvas 绘图需要 `Qt6::Quick` 模块（`Canvas` 类型包含在 `QtQuick` 中）
- 粒子系统需要额外链接 `Qt6::QuickParticles` 模块
- QML 导入：Canvas 只需 `import QtQuick`，粒子系统需要 `import QtQuick.Particles`

粒子系统的 CMake 配置需要额外注意：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Quick QuickParticles)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Quick Qt6::QuickParticles
)
```

`QuickParticles` 是一个独立模块，不是 `QtQuick` 的一部分。如果忘记链接这个模块，编译不会报错（QML 是运行时解析的），但运行时会提示 `module "QtQuick.Particles" is not installed`。

---

## 3. Canvas 2D 绘图 API

### 3.1 Canvas 的基本结构

`Canvas` 是 QML 中的一个视觉元素，它和 `Rectangle` 一样占据一块矩形区域，但它的内容不是通过属性描述的，而是通过 JavaScript 绘图命令逐帧渲染的。基本结构如下：

```qml
Canvas {
    id: canvas
    width: 400
    height: 300

    onPaint: {
        var ctx = getContext('2d')
        // 在这里写绘图命令
        ctx.fillStyle = '#3498db'
        ctx.fillRect(0, 0, width, height)
    }
}
```

`onPaint` 是 Canvas 的核心信号处理器。每当 Canvas 需要重绘时（初次显示、调用 `requestPaint()`、窗口大小变化等），`onPaint` 里的代码就会执行一遍。`getContext('2d')` 返回一个 2D 绘图上下文对象，它拥有和 HTML5 Canvas 一致的 API。

### 3.2 基本绘图操作

2D 绘图上下文的核心操作可以分为几类。路径绘制通过 `beginPath()`、`moveTo()`、`lineTo()`、`arc()` 等方法构建几何路径，然后用 `fill()` 填充或 `stroke()` 描边。矩形可以直接用 `fillRect()` 和 `strokeRect()` 快捷绘制，不需要路径。文本绘制通过 `fillText()` 和 `strokeText()` 实现。颜色和样式通过 `fillStyle`、`strokeStyle`、`lineWidth`、`font` 等属性设置。

我们来看一个稍微完整的例子，画一个带背景、网格线和彩色圆形的场景：

```qml
Canvas {
    id: drawingCanvas
    width: 400
    height: 300

    onPaint: {
        var ctx = getContext('2d')

        // 清空画布（用白色填充）
        ctx.fillStyle = '#ffffff'
        ctx.fillRect(0, 0, width, height)

        // 画网格线
        ctx.strokeStyle = '#f0f0f0'
        ctx.lineWidth = 1
        for (var x = 0; x < width; x += 20) {
            ctx.beginPath()
            ctx.moveTo(x, 0)
            ctx.lineTo(x, height)
            ctx.stroke()
        }
        for (var y = 0; y < height; y += 20) {
            ctx.beginPath()
            ctx.moveTo(0, y)
            ctx.lineTo(width, y)
            ctx.stroke()
        }

        // 画几个彩色圆形
        var colors = ['#e74c3c', '#3498db', '#2ecc71', '#f39c12']
        for (var i = 0; i < colors.length; i++) {
            ctx.beginPath()
            ctx.arc(80 + i * 90, height / 2, 30, 0, Math.PI * 2)
            ctx.fillStyle = colors[i]
            ctx.fill()
            ctx.strokeStyle = '#333333'
            ctx.lineWidth = 2
            ctx.stroke()
        }
    }
}
```

`arc(x, y, radius, startAngle, endAngle)` 画圆弧，参数依次是圆心坐标、半径、起始角度和终止角度。角度用弧度制，`Math.PI * 2` 就是完整的圆。这个 API 和浏览器里的 Canvas 完全一致，如果你之前有 HTML5 Canvas 经验，这里基本不需要额外学习。

### 3.3 渐变与阴影

Canvas 支持线性渐变和径向渐变，通过 `createLinearGradient()` 和 `createRadialGradient()` 创建渐变对象，然后赋给 `fillStyle`：

```qml
onPaint: {
    var ctx = getContext('2d')

    // 线性渐变：从上到下
    var gradient = ctx.createLinearGradient(0, 0, 0, height)
    gradient.addColorStop(0.0, '#3498db')
    gradient.addColorStop(1.0, '#2c3e50')

    ctx.fillStyle = gradient
    ctx.fillRect(0, 0, width, height)

    // 带阴影的矩形
    ctx.shadowColor = 'rgba(0, 0, 0, 0.3)'
    ctx.shadowBlur = 15
    ctx.shadowOffsetX = 5
    ctx.shadowOffsetY = 5
    ctx.fillStyle = '#ffffff'
    ctx.fillRect(50, 50, 200, 120)

    // 重置阴影（后续绘制不受影响）
    ctx.shadowColor = 'transparent'
    ctx.shadowBlur = 0
    ctx.shadowOffsetX = 0
    ctx.shadowOffsetY = 0
}
```

`addColorStop(offset, color)` 定义渐变上的颜色节点。`offset` 是 0 到 1 之间的值，表示这个颜色在渐变线上的位置。你可以在渐变上添加任意多个颜色节点，实现多色渐变效果。

阴影效果通过 `shadowColor`、`shadowBlur`、`shadowOffsetX`、`shadowOffsetY` 四个属性控制。阴影会应用到后续所有填充和描边操作上，所以用完后需要重置，否则后面的绘制都会带阴影。

---

## 4. requestAnimationFrame 动画循环

### 4.1 为什么需要动画循环

静态绘制只是 Canvas 的基础能力。真正有意思的是动画——每帧重绘 Canvas 内容，让它动起来。在 Canvas 上做动画需要一个循环机制：每隔大约 16 毫秒（对应 60 FPS）重绘一次画面。

QML Canvas 没有内置的 `requestAnimationFrame`，但我们可以用 `Timer` 配合 `requestPaint()` 实现相同的效果：

```qml
Canvas {
    id: animCanvas
    width: 400
    height: 300

    property real time: 0

    onPaint: {
        var ctx = getContext('2d')
        ctx.clearRect(0, 0, width, height)

        // 背景
        ctx.fillStyle = '#1a1a2e'
        ctx.fillRect(0, 0, width, height)

        // 画一组随时间变化的圆
        for (var i = 0; i < 8; i++) {
            var angle = time * 2 + i * (Math.PI * 2 / 8)
            var cx = width / 2 + Math.cos(angle) * 80
            var cy = height / 2 + Math.sin(angle) * 60
            var radius = 15 + Math.sin(time * 3 + i) * 5

            ctx.beginPath()
            ctx.arc(cx, cy, radius, 0, Math.PI * 2)
            ctx.fillStyle = Qt.hsla(i / 8, 0.8, 0.6, 0.9)
            ctx.fill()
        }
    }

    Timer {
        interval: 16  // 约 60 FPS
        running: true
        repeat: true
        onTriggered: {
            animCanvas.time += 0.016
            animCanvas.requestPaint()
        }
    }
}
```

`requestPaint()` 标记 Canvas 为需要重绘，QML 引擎会在下一个渲染帧调用 `onPaint`。`Timer` 每 16 毫秒触发一次，更新 `time` 属性并请求重绘。`time` 是一个持续递增的时间变量，所有动画的计算都基于它——圆的位置用 `cos` 和 `sin` 基于 `time` 做周期运动，圆的半径用 `sin(time * 3)` 做脉冲。

这种基于时间的动画计算方式有一个重要优势：它是帧率无关的。不管实际渲染帧率是多少，同样的 `time` 值总是对应同样的画面。这意味着如果某一帧因为性能原因被跳过了，动画不会「卡住」，而是直接跳到正确的位置。

### 4.2 Canvas 性能注意事项

Canvas 的 `onPaint` 在 UI 线程上执行 JavaScript 代码，这意味着如果绘制逻辑太复杂，会阻塞 UI 线程导致界面卡顿。对于简单的几何图形和少量元素，60 FPS 没有问题。但如果你的 Canvas 每帧要绘制几千个对象（比如一个包含上万条数据的实时折线图），单帧耗时可能会超过 16 毫秒。

应对这个问题的思路有几个。最直接的是减少绘制量——只重绘变化的区域而不是整个画布。Canvas 支持 `save()` 和 `restore()` 来保存和恢复绘图状态，配合 `clip()` 可以限制绘制区域。第二个思路是用 `renderTarget: Canvas.FramebufferObject` 把 Canvas 的渲染目标设为 FBO（帧缓冲对象），这样绘制操作可以在 GPU 上完成，比默认的软件渲染快很多。第三个思路是对于真正复杂的场景，放弃 Canvas 改用 `QQuickPaintedItem`（C++ 端用 `QPainter` 绘制）或 `QQuickFramebufferObject`（C++ 端用 OpenGL 渲染），把计算密集型的工作移到 C++ 层。

---

## 5. ParticleSystem 粒子系统

### 5.1 粒子系统的基本组成

QML 的粒子系统由三个核心组件构成。`ParticleSystem` 是容器，管理所有粒子并驱动模拟循环。`Emitter` 是发射器，定义粒子从哪里产生、以什么速度和方向发射、产生频率是多少。`ImageParticle`（或 `ItemParticle`）定义粒子的视觉外观——用什么图片渲染、有多大、怎么旋转。这三个组件组合在一起，就能产生各种粒子效果。

```qml
import QtQuick
import QtQuick.Particles

Item {
    width: 400
    height: 300

    ParticleSystem {
        id: particleSystem
    }

    Emitter {
        id: emitter
        system: particleSystem
        anchors.centerIn: parent

        // 每秒发射 100 个粒子
        emitRate: 100
        // 每个粒子最大存活 3 秒
        lifeSpan: 3000
        // 粒子初始大小
        size: 16
        sizeVariation: 8
        // 发射速度
        velocity: AngleDirection {
            angle: 0
            angleVariation: 360
            magnitude: 60
            magnitudeVariation: 30
        }
    }

    ImageParticle {
        system: particleSystem
        source: "qrc:///particleresources/glowdot.png"
        color: "#3498db"
        colorVariation: 0.3
        // 粒子随时间变透明
        opacity: 1.0
        // 混合模式：叠加效果
        alpha: 0.8
    }
}
```

`Emitter` 的 `velocity` 属性指定了粒子的初始速度。`AngleDirection` 是一种基于角度的速度模式——`angle` 指定主方向（0 度是向右），`angleVariation` 是方向的随机偏差范围（360 表示全方向随机），`magnitude` 是速度大小，`magnitudeVariation` 是速度的随机偏差。这个配置产生的是一种「从中心向四周扩散」的效果。

`ImageParticle` 的 `source` 指定了粒子的纹理图片。Qt 自带了一些粒子资源（通过 `qrc:///particleresources/` 路径访问），你也可以用自己的图片。`color` 给粒子染色，`colorVariation` 控制颜色随机偏差的范围（0 到 1，0 表示全部同色，1 表示完全随机）。

### 5.2 多种发射器组合

一个 `ParticleSystem` 可以包含多个 `Emitter`，实现更复杂的粒子效果。比如一个「火箭喷射」效果：一个主发射器向下喷射火焰粒子，一个次发射器在火焰尖端产生烟雾粒子：

```qml
ParticleSystem {
    id: rocketSystem
}

// 火焰粒子
Emitter {
    system: rocketSystem
    x: parent.width / 2
    y: parent.height - 20

    emitRate: 200
    lifeSpan: 800
    size: 12
    sizeVariation: 4

    velocity: AngleDirection {
        angle: 270         // 向上
        angleVariation: 15
        magnitude: 120
    }

    acceleration: AngleDirection {
        angle: 90           // 重力向下拉
        magnitude: 30
    }
}

ImageParticle {
    system: rocketSystem
    source: "qrc:///particleresources/glowdot.png"
    color: "#f39c12"
    colorVariation: 0.2
    alpha: 0.7
}
```

`acceleration` 属性给粒子施加一个持续加速度。上面的例子中，火焰粒子以 120 的初速度向上发射（angle: 270 是正上方），但受到一个向下的加速度（angle: 90 是正下方）影响，粒子会先上升后下落，模拟出被重力拉回的效果。

### 5.3 影响器 Affector

`Affector` 是粒子系统中用来对已发射的粒子施加额外影响的组件。比如 `Gravity` 给粒子加重力、`Attractor` 把粒子吸向某个点、`Wander` 让粒子随机游走、`Turbulence` 给粒子加湍流噪声：

```qml
// 让粒子受湍流影响，产生烟雾般的随机飘动
Turbulence {
    system: particleSystem
    strength: 30
    // 使用 Simplex 噪声算法
    noiseSource: "qrc:///particleresources/noise.png"
}
```

影响器可以叠加使用，多个影响器同时作用于同一组粒子。你可以通过 `groups` 属性控制影响器只作用于特定的粒子组，实现「火焰粒子不受风的影响，但烟雾粒子受风的影响」这种分层控制。

---

## 6. 完整示例——Canvas 绘图与粒子系统

下面是一个整合了 Canvas 绘图和粒子系统的完整示例。上半部分是一个交互式的 Canvas 涂鸦板（鼠标按下拖动绘制线条），下半部分是一个粒子烟花效果。

### 6.1 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.26)
project(QmlCanvasParticlesDemo VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickParticles)

qt_add_executable(${PROJECT_NAME}
    main.cpp
)

qt_add_qml_module(${PROJECT_NAME}
    URI QmlCanvasParticlesDemo
    VERSION 1.0
    QML_FILES Main.qml
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Quick Qt6::QuickParticles
)
```

### 6.2 main.cpp

```cpp
/*
 *  Qt 6 入门教程 - 示例 6.7
 *  主题：QML Canvas 绘图与粒子系统
 *
 * 本示例为纯 QML 演示（Canvas + Particles），C++ 端仅加载引擎。
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/QmlCanvasParticlesDemo/Main.qml"_qs);

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

### 6.3 Main.qml

完整的 QML 文件请参考示例代码目录。它包含三个区域：

- 一个交互式 Canvas 涂鸦板，支持鼠标绘制、颜色选择和清空
- 一个基于 `requestPaint` + `Timer` 的 Canvas 动画，展示轨道运动效果
- 一个粒子系统，使用 `ParticleSystem` + `Emitter` + `ImageParticle` + `Gravity` 实现烟花效果

---

## 7. Canvas vs ParticleSystem 的选择

Canvas 和 ParticleSystem 都能做「动态视觉效果」，但它们的适用场景完全不同。

Canvas 适合需要精确控制每一像素的场景：自定义图表、涂鸦板、游戏地图、数据可视化。你拥有对每一帧画面的完全控制权，但代价是需要手动管理所有绘制逻辑，包括脏区域检测和重绘优化。

ParticleSystem 适合需要大量相似元素做物理运动的场景：雪花、烟雾、火焰、爆炸、星尘。你只需要声明行为参数，引擎帮你完成物理模拟和渲染。它用 GPU 加速，能同时处理成千上万个粒子而不影响帧率。但它的灵活性有限——粒子不是通用的视觉元素，不能用它来做复杂的自定义绘制。

在实际项目中，这两种技术经常配合使用。比如一个天气应用中，天气图标用 Canvas 绘制（精确的太阳、云朵形状），背景的雨雪效果用 ParticleSystem（大量粒子的物理运动）。分工明确，各取所长。

到这里就大功告成了。这也是我们入门层 QML 独立教程的最后一篇，同时也是整个入门层教程的收官。从 Qt 的元对象系统到信号槽，从 QtWidgets 到 QML，从属性绑定到动画状态机，从数据驱动视图到自定义绘制和粒子系统——走到这里，你已经具备了使用 Qt 6 进行实际项目开发的全部基础知识。进阶层见。
