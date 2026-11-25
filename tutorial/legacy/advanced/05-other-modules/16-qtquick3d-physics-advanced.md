---
title: "5.16 物理进阶：关节约束、力与冲量、射线检测"
description: "入门篇我们用 QtQuick3D Physics 做了简单的重力下落和碰撞。进阶篇要把物理引擎的高级能力拆开——关节约束、手动施加力与冲量、射线检测。这些是构建物理交互场景的核心工具：门的铰链、活塞运动、子弹命中检测，全靠它们。"
---

# 现代Qt开发教程（进阶篇）5.16——物理进阶：关节约束、力与冲量、射线检测

## 1. 前言

入门篇我们把 QtQuick3D Physics 跑通了——DynamicRigidBody 掉到地上弹几下，StaticRigidBody 当地面，Collision 碰撞触发信号。但这些只是物理引擎最基本的能力。真正做物理交互场景的时候，我们需要更精细的控制手段。

你想想看：做一扇可以推开的门，需要铰链约束；做液压升降台，需要滑轨约束；做枪击命中检测，不能给子弹放一个碰撞体（太慢、太不精确），得用射线检测；想让某个物体被爆炸「炸飞」，不能靠碰碰运气，得手动施加冲量。这些就是进阶篇要讲的内容。

QtQuick3D Physics 的底层是 PhysX，NVIDIA 的物理引擎。Qt 对 PhysX 做了一层 QML 友好的封装。我们在 QML 层面操作的 `PhysicsBody`、`Joint`、`PhysicsWorld`，最终都会映射到 PhysX 的 `PxRigidDynamic`、`PxJoint`、`PxScene::raycast`。理解了这个映射关系，查阅 PhysX 文档也会更顺畅。

## 2. 环境说明

本文档基于 Qt 6.4+ 编写，需要 Qt6::Quick3DPhysics 模块。CMake 中通过 `find_package(Qt6 REQUIRED COMPONENTS Quick3DPhysics)` 引入。QtQuick3D Physics 依赖 RHI 后端，支持 OpenGL、Vulkan、Metal、Direct3D。注意 Qt 6.4 之前的版本没有 Quick3DPhysics 模块，如果你用的是 Qt 6.2/6.3，需要升级。

## 3. 核心概念讲解

### 3.1 动态物体与静态物体——PhysicsBody 配置进阶

入门篇我们用过 `DynamicRigidBody` 和 `StaticRigidBody`，但没深入讲它们的配置参数。进阶篇必须把这些参数吃透，因为后续讲关节和力的时候，物体的质量、质心、阻尼参数直接影响物理行为。

`DynamicRigidBody` 是受物理引擎驱动的物体，它有质量、速度、角速度、阻尼等属性。`StaticRigidBody` 则是完全不动的物体，参与碰撞检测但不受力的影响，通常用作地面、墙壁。还有一种 `KinematicRigidBody`，它不受物理引擎驱动，但可以由代码控制位置，同时能推开动态物体——想象一个由动画控制的移动平台。

```qml
DynamicRigidBody {
    id: kBoxBody
    mass: 5.0                    // 质量（千克）
    centerOfMassPosition: Qt.vector3d(0, 0, 0)  // 质心偏移
    linearDamping: 0.1           // 线性阻尼，模拟空气阻力
    angularDamping: 0.3          // 角阻尼，让旋转逐渐停止
    isKinematic: false           // true 则变为运动学物体

    // 碰撞形状
    collisionShapes: BoxShape {
        extents: Qt.vector3d(1, 1, 1)
    }

    Model {
        source: "#Cube"
        materials: PrincipledMaterial {
            baseColor: "#cc4444"
            roughness: 0.5
        }
    }
}
```

这里有个关键参数是 `mass`。关节约束和力的计算都依赖质量。质量为零的 `DynamicRigidBody` 在 PhysX 中会导致未定义行为（某些版本直接 crash）。如果你发现物体对施加的力完全没反应，第一件事就是检查质量是否为零。阻尼参数也很重要——`linearDamping` 让物体速度随时间衰减（值越大停得越快），`angularDamping` 控制旋转衰减。真实世界中一切都有阻尼，不给阻尼的话物体会永远运动下去。

现在有一个思考题给大家：如果我们想让一个物体变成「不可被推动的」，但又需要用代码手动控制它的位置（比如电梯），应该用 `StaticRigidBody` 还是 `DynamicRigidBody { isKinematic: true }`？

答案是后者。`StaticRigidBody` 完全不参与动态碰撞响应，即使你移动了它的位置，碰撞检测也不会更新。而 `isKinematic: true` 的 `DynamicRigidBody` 会正确地推开其他动态物体。

### 3.2 关节约束——HingeJoint、SliderJoint、FixedJoint

关节（Joint）是物理引擎中约束两个物体相对运动的机制。QtQuick3D Physics 提供了三种关节：铰链关节 `HingeJoint`（绕一个轴旋转）、滑轨关节 `SliderJoint`（沿一个轴滑动）、固定关节 `FixedJoint`（保持相对位置不变）。

先看铰链关节。一扇门就是最典型的铰链——门绕着门轴旋转，但不能平移、不能绕其他轴旋转。

```qml
// 门框——静态物体
StaticRigidBody {
    id: kDoorFrame
    position: Qt.vector3d(0, 1, 0)
    collisionShapes: BoxShape { extents: Qt.vector3d(0.1, 2, 1) }
    Model {
        source: "#Cube"
        materials: PrincipledMaterial { baseColor: "#888888" }
    }
}

// 门——动态物体，受铰链约束
DynamicRigidBody {
    id: kDoor
    position: Qt.vector3d(0.5, 1, 0)
    mass: 3.0
    collisionShapes: BoxShape { extents: Qt.vector3d(0.05, 2, 1) }
    Model {
        source: "#Cube"
        materials: PrincipledMaterial { baseColor: "#aa8844" }
    }
}

// 铰链关节
HingeJoint {
    bodyA: kDoorFrame
    bodyB: kDoor
    // 关节轴——绕 Y 轴旋转
    axis: Joint.Axis.Y
    // 关节锚点——在门轴位置
    anchorPosition: Qt.vector3d(0, 1, 0)
    // 旋转范围限制（弧度）
    limitMin: -Math.PI * 0.5
    limitMax: Math.PI * 0.5
}
```

`bodyA` 和 `bodyB` 是关节连接的两个物体。`axis` 指定旋转轴方向。`anchorPosition` 是关节在世界空间中的锚点。`limitMin` 和 `limitMax` 限制了旋转角度范围——门不能转 360 度是合理的。如果不设限制，铰链可以无限制旋转。

滑轨关节 `SliderJoint` 用于沿一个轴滑动的场景，比如活塞、抽屉、升降台：

```qml
SliderJoint {
    bodyA: kFixedBase
    bodyB: kMovingPlatform
    axis: Joint.Axis.Y
    anchorPosition: Qt.vector3d(0, 2, 0)
    limitMin: 0.0         // 最低位置
    limitMax: 3.0         // 最高位置
}
```

滑轨关节的核心参数就是 `axis`（滑动方向）和 `limitMin`/`limitMax`（滑动范围）。物理引擎会确保 `bodyB` 只能沿指定轴在限定范围内运动。

固定关节 `FixedJoint` 看起来最简单——把两个物体「焊死」。但它的用途比你想的广：比如角色捡起物品时，把物品固定到角色手上；或者把多个小物体拼成一个大物体。

```qml
FixedJoint {
    bodyA: kCharacterHand
    bodyB: kPickedUpItem
    anchorPosition: kCharacterHand.position
}
```

关节的一个关键注意事项是：`bodyA` 可以是 `StaticRigidBody`，这种情况下相当于把关节的一端固定在世界坐标系中。门的铰链就是这种——门框不动，门绕门框转。

### 3.3 施加力与冲量——applyForce、applyImpulse、applyTorque

关节约束是「被动约束」，让物体按照规则运动。但有时候我们需要「主动」给物体施加力。比如：发射炮弹的推力、爆炸的冲击波、旋翼的升力。

QtQuick3D Physics 在 `DynamicRigidBody` 上提供了三个方法：

`applyForce(force, position)` 施加持续力。力在每个物理模拟步骤中都作用在物体上，直到你停止施加。比如推进器、重力补偿。`force` 是力的向量，`position` 是力作用点（局部坐标），如果不在质心上还会产生力矩。

`applyImpulse(impulse, position)` 施加冲量。冲量是一次性的，瞬间改变物体速度。比如发射子弹的初始推力、爆炸冲击。冲量和力的关系是 `impulse = force * deltaTime`，但 `applyImpulse` 不需要你关心 deltaTime，直接给定速度变化量。

`applyTorque(torque)` 施加力矩（旋转力）。只改变角速度，不影响线性速度。比如让陀螺旋转、让风扇转动。

```qml
DynamicRigidBody {
    id: kProjectile
    mass: 0.1

    function launch_in_direction(dir) {
        // 施加冲量让炮弹飞出——dir 是单位方向向量
        // 冲量大小 50 N·s，方向由参数决定
        var kImpulse = Qt.vector3d(dir.x * 50, dir.y * 50, dir.z * 50)
        applyImpulse(kImpulse)
    }
}

DynamicRigidBody {
    id: kSpinningTop
    mass: 0.5

    function spin_up() {
        // 施加 Y 轴力矩让陀螺旋转
        applyTorque(Qt.vector3d(0, 10, 0))
    }
}
```

力和冲量的单位都是国际单位制（牛顿、牛顿·秒）。质量用千克。这意味着如果你想让一个 1 千克的物体获得 10 m/s 的速度，需要的冲量是 10 N·s。理解这个公式 `deltaVelocity = impulse / mass` 对于调参非常关键。

还有一个容易忽略的参数——`position`。当你不指定 `position` 时，力施加在质心上，只产生线性加速度。如果力施加在偏离质心的位置上，会产生力矩，让物体同时旋转。这就是为什么你推门的时候推门把手比推门轴容易——力臂更长。

### 3.4 射线检测——PhysicsWorld::rayCast

射线检测是物理引擎中非常重要的查询工具。它从空间中一点沿某个方向发射一条不可见的射线，检测射线与碰撞体的交点。用途极广：射击检测（子弹命中）、鼠标拾取（点击 3D 物体）、视线检测（AI 是否能看到玩家）、地面检测（角色是否站在地面上）。

QtQuick3D Physics 的射线检测通过 `PhysicsWorld` 提供。`PhysicsWorld` 是物理世界的全局管理器，通常你在场景中放一个就行：

```qml
PhysicsWorld {
    id: kPhysicsWorld
    running: true
    force: Qt.vector3d(0, -9.81, 0)  // 重力
}

// 在某个函数中执行射线检测
function shoot_ray(origin, direction) {
    // 从 origin 出发，沿 direction 方向发射射线
    // maxDistance 限制检测范围
    // results 是命中结果数组
    var kResults = kPhysicsWorld.rayCast(
        origin,
        direction,
        100.0  // 最大检测距离
    )

    for (var i = 0; i < kResults.length; ++i) {
        var kHit = kResults[i]
        console.log("命中物体:", kHit.object)
        console.log("命中点:", kHit.position)
        console.log("命中法线:", kHit.normal)
        console.log("命中距离:", kHit.distance)
    }
}
```

`rayCast` 返回的结果数组中，每个元素包含 `object`（命中的物体）、`position`（世界空间命中点）、`normal`（命中面的法线）、`distance`（射线起点到命中点的距离）。如果射线没有命中任何物体，返回空数组。

射线检测的一个常见用法是鼠标拾取——把屏幕上的鼠标位置转换为 3D 空间中的射线：

```qml
View3D {
    id: kView

    MouseArea {
        anchors.fill: parent
        onClicked: function(mouse) {
            // 将屏幕坐标转换为 3D 射线
            var kRayOrigin = kView.camera.mapToPosition(
                Qt.vector3d(0, 0, 0),
                Qt.vector3d(mouse.x, mouse.y, 0)
            )
            var kRayDirection = kView.camera.mapToPosition(
                Qt.vector3d(0, 0, -1),
                Qt.vector3d(mouse.x, mouse.y, 0)
            ).minus(kRayOrigin).normalized()

            // 执行射线检测
            var kHits = kPhysicsWorld.rayCast(kRayOrigin, kRayDirection, 1000)
            // ... 处理命中结果
        }
    }
}
```

实际上 QtQuick3D 的 `View3D` 提供了更方便的 `pick` 方法来做鼠标拾取，但 `rayCast` 更通用——你可以在任意位置、任意方向发射射线，不限于鼠标点击。

现在有一道调试题给大家。下面这段射线检测代码为什么永远返回空数组？

```qml
PhysicsWorld {
    id: kWorld
    running: true
    force: Qt.vector3d(0, -98.1, 0)  // 疑似重力值有误
}

function test_cast() {
    var kOrigin = Qt.vector3d(0, 10, 0)
    var kDir = Qt.vector3d(0, -1, 0)
    var kHits = kWorld.rayCast(kOrigin, kDir, 20)
    console.log("hits:", kHits.length)  // 始终输出 0
}
```

问题可能出在几个地方。首先检查场景中是否有碰撞体——射线只和有 `collisionShapes` 的物体碰撞。其次检查物体的 `collisionShapes` 是否正确设置了 `extents` 或 `radius`。最后检查物体是否启用了碰撞（`sendTriggerReports: false` 的物体不参与射线检测）。最常见的坑是忘了给物体加 `collisionShapes`。

## 4. 踩坑预防

第一个坑是关节连接的两个物体必须有碰撞形状。这是很多人容易忽略的。`HingeJoint` 连接 `bodyA` 和 `bodyB` 时，两个物体都必须有 `collisionShapes` 属性。如果其中一个物体没有碰撞形状，PhysX 会报错或直接忽略关节。有些版本的 Qt 不会在控制台打印明确错误，只会看到关节不生效。解决方案是确保所有参与关节的物体都有碰撞形状，即使是「不可见」的物体也必须加一个小的 `SphereShape` 或 `BoxShape`。

第二个坑是 `applyImpulse` 的方向向量没有归一化。很多人直接把方向向量传进去，但如果方向向量长度不是 1，冲量的大小就不符合预期。比如你想施加 10 N·s 的冲量，但方向向量长度是 5，实际施加的冲量就是 50 N·s。解决方案是始终用 `normalized()` 归一化方向向量，然后再乘以你想要的冲量大小：`dir.normalized().times(kImpulseMagnitude)`。

第三个坑是射线检测在 QML 中是同步执行的，高频调用会卡主线程。如果你在每帧的 `onFrameDone` 中都做射线检测，物理模拟帧率会显著下降。射线检测本身很快（PhysX 的空间索引优化得很好），但如果你同时对大量射线做检测，开销会累积。建议只在确实需要的时候（比如鼠标点击事件）做射线检测，不要在每帧的更新循环中滥用。

## 5. 练习项目

练习项目是一个简易物理沙盒。我们要做一个包含以下交互的 3D 场景。

场景中有一个地面（StaticRigidBody），地面中央有一扇可以用鼠标推开的门（DynamicRigidBody + HingeJoint）。门的旁边有一个滑轨升降台（SliderJoint），点击按钮可以让升降台上下移动。场景上方有一个可以发射炮弹的发射器，点击 3D 场景中的任意位置，从发射器向点击位置发射一颗小球（applyImpulse），小球落地后与其他物体碰撞。还需要实现射线检测来高亮鼠标指向的物体——当鼠标悬停在某个可交互物体上时，物体发光提示。

完成标准是门可以被推开且自动回弹、升降台可以在限定范围内上下运动、炮弹能正确飞向目标位置并在命中时产生碰撞效果、鼠标悬停能正确高亮物体。代码结构要清晰，物理参数要合理，不能出现穿透或抖动现象。

提示几个关键点：门的铰链用 HingeJoint 连接到门框上，设置合理的角度限制；炮弹发射需要将屏幕坐标转换为 3D 射线方向，再用 applyImpulse 施加冲量；鼠标悬停高亮用每帧做一次射线检测（但注意性能，不要检测过多射线）。

## 6. 官方文档参考链接

[Qt 文档 · DynamicRigidBody](https://doc.qt.io/qt-6/qml-qtquick3d-physics-dynamicrigidbody.html) -- 动态刚体的完整属性和方法列表

[Qt 文档 · StaticRigidBody](https://doc.qt.io/qt-6/qml-qtquick3d-physics-staticrigidbody.html) -- 静态刚体配置

[Qt 文档 · HingeJoint](https://doc.qt.io/qt-6/qml-qtquick3d-physics-hingejoint.html) -- 铰链关节参数说明

[Qt 文档 · SliderJoint](https://doc.qt.io/qt-6/qml-qtquick3d-physics-sliderjoint.html) -- 滑轨关节参数说明

[Qt 文档 · PhysicsWorld](https://doc.qt.io/qt-6/qml-qtquick3d-physics-physicsworld.html) -- 物理世界管理器，包含 rayCast 方法

---

到这里我们就把 QtQuick3D Physics 的进阶能力拆干净了。关节约束、力与冲量、射线检测——这三个工具组合起来，足以构建绝大多数物理交互场景。后面如果遇到更复杂的物理需求（布料模拟、车辆物理），可以直接查阅 PhysX 的文档，Qt 的封装基本一一对应。
