# 现代Qt开发教程（新手篇）5.15——QtQuick3D QML 3D 场景基础

## 1. 前言：在 QML 里写 3D，真的就这么简单

上一篇我们用纯 C++ 的 Qt 3D 搭建了一个 3D 场景——创建 Entity、挂载 Component、配置 Mesh/Transform/Material、手动设置相机和光源，代码量不算少。如果你的项目本来就是 QML 前端，用 C++ API 写 3D 就显得有些割裂了——2D 界面用 QML 声明式写，3D 部分突然切到 C++ 命令式写法，来回跳转很痛苦。

QtQuick3D 就是解决这个问题的。它是 Qt 官方主推的 3D 方案，在 QML 中直接声明 3D 场景——View3D 创建 3D 视口，Model 定义几何体，PerspectiveCamera 配置相机，DirectionalLight 添加光源，PrincipledMaterial 配置 PBR 材质，全部都是 QML 声明式语法。对比上一篇 Qt 3D 的 C++ 代码，同样的场景用 QtQuick3D 写，代码量能减少一半以上，而且和 Qt Quick 2D 元素无缝混合——一个 QML 界面里同时放 2D 按钮和 3D 模型，位置、大小、动画都用同一套 QML 绑定系统管理。

这篇我们要做的是用 QML 搭建一个包含基础几何体的 3D 场景，配置相机和光照，使用 PrincipledMaterial 实现 PBR 物理材质效果，最后在 3D 场景上叠加 2D 控制面板，演示 QtQuick3D 和 Qt Quick 2D 的混合布局能力。

## 2. 环境说明

本篇基于 Qt 6.9.1，需要引入 QtQuick3D 和 Quick 模块。CMake 配置如下：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Quick3D Quick)
```

QtQuick3D 是 Qt 6 中 3D 功能的主力模块，在 Qt Installer 中默认随标准安装包提供。它的底层使用 Qt Rendering Hardware Interface (RHI) 进行渲染——这意味着同一个 QML 3D 代码可以在 OpenGL、Vulkan、Direct3D、Metal 后端上运行，不需要针对不同图形 API 写不同的代码。

QtQuick3D 支持加载外部 3D 模型文件（.mesh、.gltf、.fbx 等），也内置了几种基础几何体（球体、立方体、圆柱、圆锥、平面）可以直接在 QML 中声明使用。本篇只用内置几何体，加载外部模型是进阶话题。

工具链和前面一致：MSVC 2019+、GCC 11+、Clang 14+，C++17 标准，CMake 3.26+ 构建系统。QtQuick3D 对 GPU 有一定要求——集成显卡通常够用，但老旧显卡可能不支持某些 PBR 材质特性。

## 3. 核心概念讲解

### 3.1 View3D——QML 中的 3D 视口

View3D 是 QtQuick3D 的核心容器组件，在 QML 界面中创建一个 3D 渲染区域。它本质上是一个 Qt Quick Item，可以像 Rectangle、Image 一样放在任何 QML 布局中：

```qml
import QtQuick
import QtQuick3D

Window {
    width: 800
    height: 600
    visible: true

    View3D {
        anchors.fill: parent

        // 场景内容（相机、光源、模型）都放在 View3D 内部
        PerspectiveCamera {
            position: Qt.vector3d(0, 5, 15)
            eulerRotation: Qt.vector3d(-15, 0, 0)
        }

        DirectionalLight {
            eulerRotation: Qt.vector3d(-45, 45, 0)
        }

        Model {
            source: "#Sphere"
            materials: PrincipledMaterial {
                baseColor: "#4080ff"
            }
        }
    }
}
```

View3D 内部的坐标系是标准的 3D 右手坐标系：X 轴朝右，Y 轴朝上，Z 轴朝屏幕外（朝观察者）。所有 3D 对象的 position、rotation、scale 属性都在这个坐标系中定义。

View3D 作为 Qt Quick Item，可以设置 anchors、width、height 等属性控制其在 2D 界面中的位置和大小。这意味着你可以轻松实现"左边 2D 控制面板、右边 3D 视图"或者"3D 视图上方叠加 2D 按钮"这样的布局——不需要任何窗口容器包装，直接用 QML 的布局系统就行。

环境贴图方面，View3D 默认提供一个深灰色背景。如果想要更丰富的环境效果（天空盒、环境光照贴图），可以使用 SceneEnvironment 组件配置：

```qml
environment: SceneEnvironment {
    clearColor: "#1a1a2e"
    backgroundMode: SceneEnvironment.Color
}
```

### 3.2 PerspectiveCamera——透视相机

PerspectiveCamera 定义了观察 3D 场景的视角。它的核心属性是 position（相机位置）和 eulerRotation（欧拉角旋转，控制朝向），此外还有 clipNear、clipFar（裁剪范围）和 fieldOfView（视场角）：

```qml
PerspectiveCamera {
    id: camera
    position: Qt.vector3d(0, 8, 20)      // 相机位于高处偏后
    eulerRotation: Qt.vector3d(-20, 0, 0) // 稍微俯视
    fieldOfView: 45                        // 视场角 45 度
    clipNear: 0.1                          // 近裁剪面
    clipFar: 1000                          // 远裁剪面
}
```

eulerRotation 使用欧拉角表示旋转，三个分量分别是绕 X、Y、Z 轴的旋转角度（度数）。把 X 旋转设为负值会让相机朝下看（俯视），设为正值朝上看（仰视）。这种表示方式比 lookAt 直觉上不够直观，但更灵活——你可以独立控制俯仰角和偏航角。

如果你想要和上一篇 QOrbitCameraController 类似的鼠标旋转/缩放体验，QtQuick3D 提供了 WasdController 组件（键盘 WASD + 鼠标控制）：

```qml
WasdController {
    controlledObject: camera
    speed: 0.1
}
```

不过 WasdController 的行为更像第一人称游戏控制（WASD 移动 + 鼠标转向），不是轨道旋转。如果你需要轨道控制，需要自己用 MouseArea + 属性绑定实现，或者等 Qt 后续提供更丰富的控制器。

### 3.3 Model 节点——几何体和模型加载

Model 是 QtQuick3D 中表示 3D 物体的组件。source 属性指定几何体来源——加载外部模型文件用文件路径，使用内置几何体用 "#" 前缀的预定义名称。

QtQuick3D 内置了几种基础几何体，用 source 属性的魔法字符串引用：

```qml
// 球体
Model {
    source: "#Sphere"
    scale: Qt.vector3d(1.5, 1.5, 1.5)
    position: Qt.vector3d(-4, 1.5, 0)
}

// 立方体
Model {
    source: "#Cube"
    scale: Qt.vector3d(2, 2, 2)
    position: Qt.vector3d(3, 1, 0)
}

// 平面（XZ 平面，适合做地面）
Model {
    source: "#Rectangle"
    scale: Qt.vector3d(20, 20, 1)
    eulerRotation: Qt.vector3d(-90, 0, 0)
    position: Qt.vector3d(0, 0, 0)
}
```

内置几何体都是单位尺寸（半径/边长为 1），通过 scale 属性缩放到需要的大小。position 控制世界坐标位置。eulerRotation 控制旋转角度。

加载外部 3D 模型文件时，source 直接设为文件路径：

```qml
Model {
    source: "models/character.mesh"
    position: Qt.vector3d(0, 0, 0)
}
```

QtQuick3D 支持的模型格式包括 Qt 自有的 .mesh 格式（需要用 Qt 的 balsam 工具从 .fbx/.gltf 等格式转换）、以及直接加载 .gltf/.glb 文件。不过模型加载涉及到材质映射、骨骼动画、纹理路径等问题，是进阶话题，本篇只用内置几何体。

### 3.4 PrincipledMaterial——PBR 物理材质

QtQuick3D 推荐使用 PrincipledMaterial——一种基于物理的渲染（PBR）材质。它比 Phong 模型更真实，通过少量参数就能模拟金属、塑料、玻璃、布料等各种材质的外观。

PrincipledMaterial 的核心属性是 baseColor（基础颜色）和 metalness（金属度）、roughness（粗糙度）：

```qml
// 蓝色塑料球
materials: PrincipledMaterial {
    baseColor: "#4080ff"
    metalness: 0.0     // 非金属
    roughness: 0.5     // 半粗糙
}

// 金属质感的红色立方体
materials: PrincipledMaterial {
    baseColor: "#cc3333"
    metalness: 0.9     // 高金属度
    roughness: 0.2     // 光滑金属表面
}
```

baseColor 是物体在白光下的颜色。metalness 范围 0 到 1——0 表示非金属（电介质），1 表示纯金属。金属表面会反射环境色，而非金属表面主要显示自身的 baseColor。roughness 范围 0 到 1——0 表示完全光滑（镜面反射），1 表示完全粗糙（漫反射）。

对比上一篇的 QPhongMaterial（ambient + diffuse + specular + shininess），PrincipledMaterial 用 metalness + roughness 两个参数替代了 specular + shininess 的组合，参数更少但效果更真实。PBR 的物理基础确保了同一组参数在不同光照条件下看起来都自然——Phong 模型在某些角度和光照组合下会出现不真实的高光或暗部，PBR 基本不会。

如果你想添加环境反射效果（让金属物体反射周围环境），可以配合 SceneEnvironment 的 lightProbe 属性加载 HDR 环境贴图。不过没有环境贴图的情况下 PrincipledMaterial 也能正常工作，只是反射效果会弱一些。

### 3.5 与 Qt Quick 2D 混合——3D 场景上的 2D 叠加

QtQuick3D 最大的优势之一是和 Qt Quick 2D 元素无缝混合。View3D 本身就是一个标准的 Qt Quick Item，你可以用 Z 轴叠加、anchors 锚点、Row/Column 布局等常规 QML 布局手段把 2D 元素放在 3D 视图上面、下面、旁边：

```qml
Window {
    View3D {
        id: view3d
        anchors.fill: parent

        // 3D 场景内容...

        // 在 3D 场景内部叠加 2D 元素
        Text {
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: 20
            text: "QtQuick3D 场景"
            font.pixelSize: 24
            color: "white"
            style: Text.Outline
            styleColor: "black"
        }
    }

    // 2D 控制面板浮在 3D 视图上
    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 10
        width: 200
        height: 150
        color: "#80000000"  // 半透明黑色背景
        radius: 10

        Column {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 8

            Button {
                text: "切换颜色"
                onClicked: {
                    // 修改 3D 模型的材质属性
                    sphereMaterial.baseColor = "#ff4040"
                }
            }
        }
    }
}
```

这种混合方式在实现 3D 产品展示页面、游戏 HUD、数据可视化面板等场景时非常实用——不需要额外的窗口管理，2D 和 3D 用同一套 QML 语法，属性绑定和信号/槽机制完全互通。

需要注意的是，2D 元素叠在 View3D 上方时，鼠标事件会被 2D 元素拦截——如果你需要 3D 场景接收鼠标事件，确保叠加的 2D 元素设置了 `enabled: false` 或者只占屏幕的部分区域。

## 4. 综合示例：QtQuick3D 基础场景 + 2D 控制面板

把前面的知识串起来，我们用 QML 搭建一个包含球体和立方体的 3D 场景，右侧叠加一个 2D 控制面板，通过按钮切换球体和立方体的材质颜色。

因为 QtQuick3D 项目主要是 QML 代码，C++ 的 main.cpp 只需要很少的启动代码——创建 QQmlApplicationEngine 加载 QML 文件即可。CMake 用 qt_add_qml_module 注册 QML 模块。

CMake 配置：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Quick Quick3D)

qt_add_executable(${PROJECT_NAME} main.cpp)

qt_add_qml_module(${PROJECT_NAME}
    URI Quick3DScene
    VERSION 1.0
    QML_FILES Main.qml
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Quick Qt6::Quick3D)
```

QML 场景文件 Main.qml 的核心结构。场景包含 PerspectiveCamera、两盏 DirectionalLight（主光 + 补光）、一个蓝色球体、一个橙色立方体、一个灰色地面平面，以及右上角浮动的 2D 控制面板：

```qml
import QtQuick
import QtQuick.Controls
import QtQuick3D

Window {
    width: 1000
    height: 700
    visible: true
    title: "QtQuick3D 基础场景"

    View3D {
        id: view3d
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#1a1a2e"
            backgroundMode: SceneEnvironment.Color
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 10, 25)
            eulerRotation: Qt.vector3d(-20, 0, 0)
            fieldOfView: 45
        }

        DirectionalLight {
            eulerRotation: Qt.vector3d(-60, 45, 0)
            brightness: 1.0
        }

        DirectionalLight {
            eulerRotation: Qt.vector3d(-30, -45, 0)
            brightness: 0.3
        }

        // 球体
        Model {
            id: sphereModel
            source: "#Sphere"
            position: Qt.vector3d(-4, 1.5, 0)
            scale: Qt.vector3d(1.5, 1.5, 1.5)
            materials: PrincipledMaterial {
                id: sphereMaterial
                baseColor: "#4080ff"
                metalness: 0.1
                roughness: 0.4
            }
        }

        // 立方体
        Model {
            id: cubeModel
            source: "#Cube"
            position: Qt.vector3d(4, 1, 0)
            scale: Qt.vector3d(2, 2, 2)
            materials: PrincipledMaterial {
                id: cubeMaterial
                baseColor: "#e68a30"
                metalness: 0.5
                roughness: 0.3
            }
        }

        // 地面
        Model {
            source: "#Rectangle"
            position: Qt.vector3d(0, 0, 0)
            scale: Qt.vector3d(30, 30, 1)
            eulerRotation: Qt.vector3d(-90, 0, 0)
            materials: PrincipledMaterial {
                baseColor: "#808080"
                roughness: 0.8
            }
        }
    }

    // 2D 控制面板
    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 15
        width: 220
        height: 200
        color: "#c01a1a2e"
        radius: 12
        border.color: "#ffffff40"
        border.width: 1

        Column {
            anchors.fill: parent
            anchors.margins: 15
            spacing: 12

            Label {
                text: "控制面板"
                font.bold: true
                font.pixelSize: 16
                color: "white"
            }

            Button {
                text: "球体变红"
                onClicked: sphereMaterial.baseColor = "#ff4444"
            }

            Button {
                text: "立方体变绿"
                onClicked: cubeMaterial.baseColor = "#44cc44"
            }

            Button {
                text: "重置颜色"
                onClicked: {
                    sphereMaterial.baseColor = "#4080ff"
                    cubeMaterial.baseColor = "#e68a30"
                }
            }
        }
    }
}
```

运行程序后你会看到一个深色背景的 3D 场景：左侧蓝色球体、右侧橙色立方体、底部灰色地面，右上角浮动着半透明的 2D 控制面板。点击面板上的按钮可以实时切换球体和立方体的颜色——QML 的属性绑定机制让材质颜色的切换即时生效，不需要重新创建对象或手动刷新。

对比上一篇用纯 C++ 写的 Qt 3D 场景，你会发现 QML 版本的代码可读性强得多——3D 对象的属性（位置、缩放、材质参数）全部用声明式的 QML 语法写出来，一目了然。而且 2D 控制面板和 3D 场景的混合如此自然，完全不需要额外的桥接代码。

## 5. 练习项目

练习项目：带旋转动画和颜色选择器的 3D 展示台。

在基础场景上增加旋转动画和交互式颜色控制。球体和立方体各自绕 Y 轴持续旋转（速度不同），2D 面板上增加两个 ColorDialog 颜色选择器分别控制球体和立方体的 baseColor，再增加一个 Slider 滑块控制立方体的 metalness 从 0 到 1 变化。

完成标准是这样的：球体旋转一圈用时 4 秒，立方体旋转一圈用时 6 秒，使用 NumberAnimation 对 Model 的 eulerRotation.y 属性做动画（from: 0, to: 360, duration: 毫秒数, loops: Animation.Infinite）；颜色选择器点击后弹出系统颜色对话框，选中颜色后更新对应材质的 baseColor；metalness 滑块实时显示当前值（保留一位小数），拖动时实时更新立方体材质。

几个实现提示：QML 的 NumberAnimation 可以直接嵌入到 Model 的属性声明中，语法是 `Behavior on eulerRotation.y { NumberAnimation { ... } }`，或者用独立的 NumberAnimation 组件设置 target 和 property；颜色选择器可以用 Qt Labs Platform 的 ColorDialog（需要 import Qt.labs.platform），或者简单地用几个预设颜色的按钮代替；Slider 的 value 属性直接绑定到 cubeMaterial.metalness 即可，QML 的属性绑定是双向的——Slider 拖动会自动更新材质。

## 6. 官方文档参考

[Qt 文档 · QtQuick3D 模块](https://doc.qt.io/qt-6/qtquick3d-index.html) -- QtQuick3D 模块总览

[Qt 文档 · View3D](https://doc.qt.io/qt-6/qml-qtquick3d-view3d.html) -- 3D 视口组件

[Qt 文档 · PerspectiveCamera](https://doc.qt.io/qt-6/qml-qtquick3d-perspectivecamera.html) -- 透视相机

[Qt 文档 · Model](https://doc.qt.io/qt-6/qml-qtquick3d-model.html) -- 模型节点

[Qt 文档 · PrincipledMaterial](https://doc.qt.io/qt-6/qml-qtquick3d-principledmaterial.html) -- PBR 材质

[Qt 文档 · DirectionalLight](https://doc.qt.io/qt-6/qml-qtquick3d-directionallight.html) -- 方向光

[Qt 文档 · SceneEnvironment](https://doc.qt.io/qt-6/qml-qtquick3d-sceneenvironment.html) -- 场景环境配置

*(链接已验证，2026-04-23 可访问)*

---

到这里就大功告成了。QtQuick3D 用 QML 声明式语法把 3D 场景搭建简化到了极致——View3D 开视口，Model 塞几何体，PrincipledMaterial 配材质，PerspectiveCamera 定视角，DirectionalLight 打光，和 2D Qt Quick 元素混合时用同一套布局系统。对比上一篇 Qt 3D 的 C++ ECS 架构，QtQuick3D 在代码量、可读性、2D/3D 混合能力上都更有优势，是 Qt 6 中 3D 开发的首选方案。下一篇我们会在这个基础上继续深入，看 QtQuick3D Physics 物理模拟——让 3D 物体在重力下掉落、碰撞、弹跳。
