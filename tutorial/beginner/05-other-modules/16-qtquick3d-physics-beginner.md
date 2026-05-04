# 现代Qt开发教程（新手篇）5.16——QtQuick3D Physics 物理模拟基础

## 1. 前言：让 3D 物体真正"掉下来"

上一篇我们用 QtQuick3D 搭建了一个静态 3D 场景——球体、立方体、地面，模型固定在指定位置不动。场景本身已经不错了，但总觉得缺了点什么：物体就那么悬浮在空中，没有重力、没有碰撞、没有弹跳，看起来像是美术截图而不是活的仿真。

如果你跟我一样，看到一堆 3D 物体悬浮在半空中就浑身难受，那 QtQuick3D Physics 就是来解决这个问题的。它是 Qt 6 官方提供的物理引擎模块，底层基于 PhysX（NVIDIA 开源的物理模拟引擎），直接在 QML 中以声明式语法使用——给 Model 挂上 DynamicRigidBody 组件它就会受重力影响掉下来，碰到地面会弹起，加上摩擦力参数后还会逐渐减速。整个过程不需要写一行物理计算代码，只需要在 QML 中配置几个属性。

这篇我们要做的是在上一篇的 3D 场景基础上引入物理世界：初始化 PhysicsWorld 配置重力加速度，用 StaticRigidBody 标记地面为不可移动的静态刚体，用 DynamicRigidBody 给球体和立方体赋予动态物理行为，配置 BoxShape/SphereShape/CapsuleShape 碰撞体，调整弹性系数和摩擦力参数让物体碰撞后的行为看起来自然。

## 2. 环境说明

本篇基于 Qt 6.9.1，需要引入 QtQuick3D 和 Quick 模块，物理模拟功能包含在 QtQuick3D.Physics 命名空间中。CMake 配置和上一篇一样：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Quick3D Quick)
```

QtQuick3D.Physics 从 Qt 6.4 开始提供，在 Qt Installer 中属于标准安装包的一部分。它的底层是 NVIDIA PhysX 4（静态链接，不需要额外安装 PhysX）。PhysX 是游戏行业广泛使用的物理引擎之一，支持刚体动力学、碰撞检测、关节约束等功能——QtQuick3D Physics 目前只暴露了刚体和碰撞的部分功能，但对于大多数产品展示、教育仿真、简单游戏场景来说已经足够。

工具链和前面一致：MSVC 2019+、GCC 11+、Clang 14+，C++17 标准，CMake 3.26+ 构建系统。物理模拟对 CPU 的开销不算大，PhysX 内部有 SIMD 优化；GPU 方面和普通 QtQuick3D 场景要求一样，集成显卡就能跑。

## 3. 核心概念讲解

### 3.1 PhysicsWorld——物理世界初始化

PhysicsWorld 是物理模拟的根节点，必须放在 View3D 内部，作为整个物理世界运行的容器。它控制全局物理参数，最重要的是 gravity 属性——默认值是 `(0, -981, 0)`，单位是厘米每平方秒（cm/s^2），这是因为 QtQuick3D Physics 内部使用厘米作为距离单位。地球表面的重力加速度约 9.81 m/s^2，换算成厘米就是 981 cm/s^2。

```qml
View3D {
    anchors.fill: parent

    // 物理世界——必须放在 View3D 内部
    PhysicsWorld {
        id: physicsWorld
        running: true         // 物理模拟是否运行，可暂停
        gravity: Qt.vector3d(0, -981, 0)  // 默认重力，指向 -Y 方向
    }

    PerspectiveCamera { /* ... */ }
    DirectionalLight { /* ... */ }
}
```

PhysicsWorld 的 running 属性可以随时切换——设为 false 后所有 DynamicRigidBody 停止运动，但位置不会重置。这个特性很适合做暂停/继续功能。如果你想要慢动作效果，可以调整 timeStep 属性控制物理模拟的时间步长，不过默认值通常不需要改。

这里有一个容易混淆的地方需要注意：QtQuick3D 的世界坐标单位是"未定义"的，但 QtQuick3D Physics 内部以厘米作为物理计算单位。这意味着如果你把一个 Model 的 scale 设为 `(100, 100, 100)`，Physics 会把它当作边长 100 厘米（1 米）的物体来计算碰撞和重力响应。如果不做任何缩放（scale 默认为 1），碰撞体的尺寸就只有 1 厘米，物体会非常小。所以我们在使用物理模拟时，通常会设置比较大的 scale 值，或者直接把 gravity 调小来匹配视觉效果。

### 3.2 StaticRigidBody 与 DynamicRigidBody——静态与动态刚体

刚体（Rigid Body）是物理模拟的基本单位。QtQuick3D Physics 提供两种刚体组件，它们都是 Node 的子类，需要挂载到 Model 上作为其物理代理：

StaticRigidBody 表示不可移动的刚体——它参与碰撞检测，但不受力和重力的作用，位置永远不变。典型的用法就是地面、墙壁、平台这些固定的环境物体。

DynamicRigidBody 表示可移动的刚体——它受重力影响会自由落体，碰到其他刚体会产生碰撞响应（弹开、滑动），速度受摩擦力和弹性系数影响逐渐衰减。球体、箱子、角色这些需要移动的物体都用 DynamicRigidBody。

```qml
// 地面——静态刚体，不可移动
Model {
    source: "#Rectangle"
    position: Qt.vector3d(0, 0, 0)
    scale: Qt.vector3d(50, 50, 1)
    eulerRotation: Qt.vector3d(-90, 0, 0)
    materials: PrincipledMaterial {
        baseColor: "#505050"
        roughness: 0.8
    }

    // 挂载静态刚体组件
    StaticRigidBody {
        id: groundBody
        collisionShapes: BoxShape {
            // 碰撞体尺寸跟随 Model 的 scale，通常不需要额外设置 extents
        }
    }
}

// 球体——动态刚体，会掉落
Model {
    id: sphereModel
    source: "#Sphere"
    position: Qt.vector3d(0, 300, 0)  // 高处初始位置
    scale: Qt.vector3d(100, 100, 100)
    materials: PrincipledMaterial {
        baseColor: "#4080ff"
        metalness: 0.1
        roughness: 0.4
    }

    // 挂载动态刚体组件
    DynamicRigidBody {
        id: sphereBody
        mass: 1.0                    // 质量（千克）
        restitution: 0.6             // 弹性系数（0~1）
        friction: 0.3                // 摩擦系数
        collisionShapes: SphereShape {}
    }
}
```

DynamicRigidBody 的 mass 属性控制物体的质量，单位是千克。质量影响碰撞时力的大小——轻的物体被撞开得更远，重的物体纹丝不动。但有意思的是，质量不影响自由落体的速度——在真空中一根羽毛和一个铁球下落速度是一样的，PhysX 也不会在自由落体中区分质量。质量只在碰撞响应时起作用。

DynamicRigidBody 还有一个 useful 属性 isKinematic——设为 true 后，物体不受物理引擎控制运动，而是由你通过代码手动设置它的位置。其他动态刚体仍然会和它碰撞，但它自己像静态刚体一样不受力的影响。这个特性很适合做移动平台、电梯之类的物体——它们的位置由游戏逻辑控制，但其他物体可以站在上面被带动。

### 3.3 碰撞体形状——BoxShape、SphereShape、CapsuleShape

碰撞体（Collision Shape）定义了刚体的物理边界。它和视觉模型是分开的——碰撞体用于物理引擎的碰撞检测计算，而 Model 的 source 和 scale 只影响渲染。理想情况下碰撞体的形状应该和视觉模型匹配，但为了性能，碰撞体通常使用简化的几何形状。

QtQuick3D Physics 提供三种基础碰撞体：

BoxShape 是轴对齐包围盒，最适合方块形状的物体。它的 extents 属性控制三个轴的尺寸，默认值是 `(100, 100, 100)` 厘米——也就是 1 米的立方体。如果你给一个立方体 Model 用了 `scale: (100, 100, 100)`，那 BoxShape 的默认 extents 恰好匹配。

SphereShape 是球形碰撞体，没有额外参数——半径自动根据 Model 的 scale 推导。球体碰撞体的碰撞检测效率最高，因为球与球、球与平面的碰撞判定只需要计算距离和半径之和的比较。

CapsuleShape 是胶囊碰撞体（两端半球中间圆柱），非常适合角色控制器和圆柱形物体。它有 diameter（直径）和 height（总高度）两个属性。人形角色通常用 CapsuleShape 做碰撞体——比 BoxShape 更贴合身体轮廓，碰撞时不容易卡住边角。

```qml
// 箱子用 BoxShape
Model {
    source: "#Cube"
    scale: Qt.vector3d(80, 80, 80)
    materials: PrincipledMaterial { baseColor: "#cc6633" }
    DynamicRigidBody {
        mass: 2.0
        restitution: 0.3
        friction: 0.5
        collisionShapes: BoxShape {
            extents: Qt.vector3d(80, 80, 80)
        }
    }
}

// 角色用 CapsuleShape
Model {
    source: "#Cylinder"
    scale: Qt.vector3d(60, 120, 60)
    materials: PrincipledMaterial { baseColor: "#44aa44" }
    DynamicRigidBody {
        mass: 5.0
        friction: 0.8
        collisionShapes: CapsuleShape {
            diameter: 60
            height: 120
        }
    }
}
```

碰撞体的选择是个性能和精度权衡的问题。精确匹配视觉模型的碰撞体需要用 TriangleMeshShape（从网格提取碰撞面），但计算量巨大。实际项目中，绝大多数物体用 BoxShape 或 SphereShape 就够了——游戏行业把这个技巧叫作"碰撞代理"，用简单形状近似复杂几何体，视觉上玩家根本注意不到碰撞体和实际模型的差异。

### 3.4 弹性系数与摩擦力

DynamicRigidBody 的 restitution 属性控制弹性——值域 0 到 1。0 表示完全不反弹（碰撞后直接贴住），1 表示完美弹性碰撞（反弹后速度不损失，理论上永远弹跳）。现实中的橡胶球大约 0.8，网球 0.7，木块 0.3，混凝土块接近 0。

friction 属性控制摩擦力——值域 0 到 1。0 表示完全光滑无摩擦（碰撞后物体会在表面永远滑动），1 表示最大摩擦。现实中冰面大约 0.03，干燥混凝土 0.6~0.8，橡胶轮胎在柏油路上 0.7~0.9。

两个物体碰撞时的实际弹性系数取两者 restitution 的乘积（或者最小值，取决于引擎实现——PhysX 默认使用乘法）。同样，摩擦力也是两个物体摩擦系数的组合。所以如果你希望球在地面上弹跳高度适中，不仅需要调整球的 restitution，地面的 StaticRigidBody 上的 restitution 也会参与计算。

```qml
// 地面——中等摩擦，低弹性
StaticRigidBody {
    restitution: 0.2
    friction: 0.7
    collisionShapes: BoxShape {}
}

// 橡胶球——高弹性
DynamicRigidBody {
    mass: 0.5
    restitution: 0.85   // 配合地面的 0.2，实际弹性 ≈ 0.17
    friction: 0.9
    collisionShapes: SphereShape {}
}
```

实际调试物理参数的时候，建议先固定其他参数只改一个，观察效果后再调下一个。弹性系数和摩擦力组合起来效果会非常复杂——高摩擦 + 高弹性的物体碰撞后可能会产生不稳定的抖动，这时候需要降低其中一个参数。

## 4. 综合示例：物理模拟场景——落体与碰撞

把前面的知识串起来，我们搭建一个包含动态物理行为的 3D 场景：一个球体从高处掉落弹跳，一个立方体从另一侧掉落，一个胶囊体斜向抛出。地面是静态刚体，两侧各有一面静态墙壁，防止物体滚出视野。右上角的 2D 控制面板提供"重置"按钮，把所有物体拉回初始位置重新掉落。

因为这个项目也是以 QML 为主，C++ 的 main.cpp 只需要加载 QML 文件。

CMake 配置：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Quick Quick3D)

qt_add_executable(${PROJECT_NAME} main.cpp)

qt_add_qml_module(${PROJECT_NAME}
    URI PhysicsScene
    VERSION 1.0
    QML_FILES Main.qml
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Quick Qt6::Quick3D)
```

QML 场景文件 Main.qml 的完整代码：

```qml
import QtQuick
import QtQuick.Controls
import QtQuick3D
import QtQuick3D.Physics

Window {
    width: 1000
    height: 700
    visible: true
    title: "QtQuick3D Physics 物理模拟"

    View3D {
        id: view3d
        anchors.fill: parent

        environment: SceneEnvironment {
            clearColor: "#1a1a2e"
            backgroundMode: SceneEnvironment.Color
        }

        // 物理世界
        PhysicsWorld {
            id: physicsWorld
            running: true
            gravity: Qt.vector3d(0, -500, 0)
        }

        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 300, 800)
            eulerRotation: Qt.vector3d(-20, 0, 0)
            fieldOfView: 50
        }

        DirectionalLight {
            eulerRotation: Qt.vector3d(-60, 30, 0)
            brightness: 1.0
        }

        DirectionalLight {
            eulerRotation: Qt.vector3d(-30, -45, 0)
            brightness: 0.3
        }

        // 地面
        Model {
            source: "#Rectangle"
            position: Qt.vector3d(0, 0, 0)
            scale: Qt.vector3d(100, 100, 1)
            eulerRotation: Qt.vector3d(-90, 0, 0)
            materials: PrincipledMaterial {
                baseColor: "#505050"
                roughness: 0.85
            }
            StaticRigidBody {
                restitution: 0.3
                friction: 0.6
                collisionShapes: BoxShape {
                    extents: Qt.vector3d(10000, 100, 10000)
                }
            }
        }

        // 左墙
        Model {
            source: "#Cube"
            position: Qt.vector3d(-500, 250, 0)
            scale: Qt.vector3d(1, 5, 10)
            materials: PrincipledMaterial {
                baseColor: "#404060"
                roughness: 0.7
            }
            StaticRigidBody {
                restitution: 0.2
                friction: 0.5
                collisionShapes: BoxShape {}
            }
        }

        // 右墙
        Model {
            source: "#Cube"
            position: Qt.vector3d(500, 250, 0)
            scale: Qt.vector3d(1, 5, 10)
            materials: PrincipledMaterial {
                baseColor: "#404060"
                roughness: 0.7
            }
            StaticRigidBody {
                restitution: 0.2
                friction: 0.5
                collisionShapes: BoxShape {}
            }
        }

        // 动态球体——从高处掉落
        Model {
            id: sphereModel
            source: "#Sphere"
            position: Qt.vector3d(-100, 500, 0)
            scale: Qt.vector3d(50, 50, 50)
            materials: PrincipledMaterial {
                id: sphereMaterial
                baseColor: "#4488ff"
                metalness: 0.1
                roughness: 0.3
            }
            DynamicRigidBody {
                id: sphereBody
                mass: 1.0
                restitution: 0.75
                friction: 0.4
                collisionShapes: SphereShape {}
            }
        }

        // 动态立方体——从另一侧掉落
        Model {
            id: cubeModel
            source: "#Cube"
            position: Qt.vector3d(100, 600, 50)
            scale: Qt.vector3d(50, 50, 50)
            materials: PrincipledMaterial {
                id: cubeMaterial
                baseColor: "#e68a30"
                metalness: 0.3
                roughness: 0.5
            }
            DynamicRigidBody {
                id: cubeBody
                mass: 2.0
                restitution: 0.4
                friction: 0.5
                collisionShapes: BoxShape {
                    extents: Qt.vector3d(50, 50, 50)
                }
            }
        }

        // 动态胶囊体——斜向抛出
        Model {
            id: capsuleModel
            source: "#Cylinder"
            position: Qt.vector3d(0, 400, -100)
            scale: Qt.vector3d(40, 80, 40)
            materials: PrincipledMaterial {
                id: capsuleMaterial
                baseColor: "#44cc66"
                metalness: 0.0
                roughness: 0.6
            }
            DynamicRigidBody {
                id: capsuleBody
                mass: 1.5
                restitution: 0.6
                friction: 0.7
                collisionShapes: CapsuleShape {
                    diameter: 40
                    height: 80
                }
                // 给一个初始线速度，模拟斜向抛出
                linearVelocity: Qt.vector3d(80, 200, 30)
            }
        }
    }

    // 2D 控制面板
    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 15
        width: 220
        height: 180
        color: "#c01a1a2e"
        radius: 12
        border.color: "#ffffff40"
        border.width: 1

        Column {
            anchors.fill: parent
            anchors.margins: 15
            spacing: 10

            Label {
                text: "物理控制面板"
                font.bold: true
                font.pixelSize: 16
                color: "white"
            }

            Button {
                text: "重置所有物体"
                onClicked: {
                    sphereModel.position = Qt.vector3d(-100, 500, 0);
                    sphereBody.linearVelocity = Qt.vector3d(0, 0, 0);
                    sphereBody.angularVelocity = Qt.vector3d(0, 0, 0);

                    cubeModel.position = Qt.vector3d(100, 600, 50);
                    cubeBody.linearVelocity = Qt.vector3d(0, 0, 0);
                    cubeBody.angularVelocity = Qt.vector3d(0, 0, 0);

                    capsuleModel.position = Qt.vector3d(0, 400, -100);
                    capsuleBody.linearVelocity = Qt.vector3d(80, 200, 30);
                    capsuleBody.angularVelocity = Qt.vector3d(0, 0, 0);
                }
            }

            Button {
                text: physicsWorld.running ? "暂停模拟" : "继续模拟"
                onClicked: physicsWorld.running = !physicsWorld.running
            }

            Label {
                text: "重力: " + physicsWorld.gravity.y.toFixed(0) + " cm/s\u00B2"
                color: "#aaa"
                font.pixelSize: 12
            }
        }
    }
}
```

运行程序后你会看到：蓝色球体从左侧高处自由落体，碰到地面后弹起数次逐渐停止；橙色立方体从右侧掉落，弹性较低，弹两下就停了；绿色胶囊体斜向飞出，撞到地面后弹起再撞到右墙，最后在地面滚动一段距离后静止。整个过程完全由 PhysX 物理引擎驱动——重力、碰撞检测、弹性响应、摩擦减速全部自动计算。

控制面板上的"重置所有物体"按钮会把三个物体拉回初始位置并清零速度，重新开始掉落。"暂停模拟"按钮冻结所有物体的运动状态，再点一次恢复。

几个值得注意的细节：地面的 StaticRigidBody 碰撞体 extents 设为 `(10000, 100, 10000)`——比视觉上的地面大得多，这是为了确保物体不会从地面边缘"漏"出去；胶囊体的 linearVelocity 初始值设为 `(80, 200, 30)`，这是一个斜向上的初始速度，模拟被抛出的效果；重力值设为 `-500` 而不是默认的 `-981`，是因为在我们的场景尺度下 -981 会让物体掉得太快看不清弹跳过程。

## 5. 练习项目

练习项目：多米诺骨牌物理模拟。

在 3D 场景中用一排立方体摆出多米诺骨牌阵列，用 DynamicRigidBody 给每块骨牌赋予物理行为，用 StaticRigidBody 做地面和挡板。第一块骨牌用代码施加一个初始冲量（DynamicRigidBody 的 linearVelocity 或 kinematicPoke），触发连锁倒下。骨牌之间保持合适的间距，确保碰撞能传递到下一块。

完成标准是这样的：至少摆 10 块骨牌，每块是细长的 BoxShape（高度远大于宽度），间距约为骨牌高度的 1/3，连锁倒下过程中每块骨牌都能成功碰倒下一块，最终全部倒下。2D 面板上提供"重新摆牌"按钮重置所有骨牌的位置和旋转，以及一个 Slider 控制"推力大小"来调整第一块骨牌的初始冲量。

几个实现提示：骨牌用 Model { source: "#Cube" } 加 scale 缩放到细长形状，比如 scale: Qt.vector3d(15, 100, 30) 大约是 15cm 宽、100cm 高、30cm 厚的骨牌；间距用 JavaScript 循环或者手动排列 position，注意 DynamicRigidBody 的碰撞体要匹配 Model 的视觉尺寸；初始冲量可以通过设置第一块骨牌的 linearVelocity.x 为正值来实现，方向朝骨牌阵列的延伸方向。

## 6. 官方文档参考

[Qt 文档 · QtQuick3D Physics 模块](https://doc.qt.io/qt-6/qtquick3d-physics-index.html) -- QtQuick3D Physics 模块总览

[Qt 文档 · PhysicsWorld](https://doc.qt.io/qt-6/qml-qtquick3d-physics-physicsworld.html) -- 物理世界组件

[Qt 文档 · DynamicRigidBody](https://doc.qt.io/qt-6/qml-qtquick3d-physics-dynamicrigidbody.html) -- 动态刚体

[Qt 文档 · StaticRigidBody](https://doc.qt.io/qt-6/qml-qtquick3d-physics-staticrigidbody.html) -- 静态刚体

[Qt 文档 · BoxShape](https://doc.qt.io/qt-6/qml-qtquick3d-physics-boxshape.html) -- 盒状碰撞体

[Qt 文档 · SphereShape](https://doc.qt.io/qt-6/qml-qtquick3d-physics-sphereshape.html) -- 球形碰撞体

[Qt 文档 · CapsuleShape](https://doc.qt.io/qt-6/qml-qtquick3d-physics-capsuleshape.html) -- 胶囊碰撞体

*(链接已验证，2026-04-23 可访问)*

---

到这里就大功告成了。QtQuick3D Physics 把 PhysX 的刚体模拟能力封装成了简洁的 QML 组件——PhysicsWorld 开启物理世界，StaticRigidBody 锚定地面和墙壁，DynamicRigidBody 赋予物体质量和运动能力，BoxShape/SphereShape/CapsuleShape 定义碰撞边界，restitution 和 friction 控制碰撞后的弹性和摩擦行为。整个过程中我们没有写一行数学公式或物理计算代码，PhysX 引擎在幕后处理了重力积分、碰撞检测、穿透修正、摩擦力计算等所有脏活。对于产品展示、教育仿真、简单物理小游戏这类场景，QtQuick3D Physics 的开箱体验已经足够好。下一篇我们换一个完全不同的方向，看 QtPdf 模块如何加载和渲染 PDF 文档。
