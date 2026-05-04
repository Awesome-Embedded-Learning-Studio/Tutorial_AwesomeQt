# 现代Qt开发教程（新手篇）5.14——Qt 3D 基础场景搭建

## 1. 前言：Qt 的 C++ 3D 渲染框架

说到 Qt 里的 3D 渲染，大部分人的第一反应是 QtQuick3D —— 那个在 QML 里写 View3D + Model 的方案。但 Qt 其实还有另一套纯 C++ 的 3D 框架叫 Qt 3D，它比 QtQuick3D 出现得更早，设计理念也更底层。Qt 3D 的核心是 ECS（Entity-Component-System）架构——场景中的每个对象是一个 Entity（实体），每个实体的外观和行为由挂载的 Component（组件）决定，System（系统）负责遍历所有实体执行对应的处理逻辑。

这种架构在游戏引擎里很常见（Unity 和 Unreal 都用类似模式），好处是组合灵活：同一个实体可以同时挂载 Mesh（网格形状）、Material（材质）、Transform（变换）、Light（光源）等多个组件，组件之间松耦合，增删组件不影响其他部分。对比传统的继承体系（比如一个 GameObject 基类派生出 StaticMesh、SkeletalMesh、LightObject 等子类），ECS 在处理"一个既是光源又带有网格的实体"这类交叉场景时优雅得多。

这篇我们要做的是用 Qt3DExtras::Qt3DWindow 创建一个独立的 3D 窗口，搭建 Entity + Component 的基础场景，往里面塞球体、立方体、平面三种基础几何体，配上相机、光源和材质，让整个场景真正渲染出有光照效果的三维画面。

## 2. 环境说明

本篇基于 Qt 6.9.1，需要引入 Qt 3D 的三个核心模块。CMake 配置如下：

```cmake
find_package(Qt6 REQUIRED COMPONENTS
    3DCore 3DExtras 3DInput Widgets)
```

Qt3DCore 是 ECS 架构的基础类（QEntity、QComponent），Qt3DExtras 提供了预置的几何体（球体、立方体、平面等）、材质和 Qt3DWindow 便利类，Qt3DInput 处理鼠标/键盘的 3D 场景交互（轨道控制器等）。

在 Qt Installer 中需要确保勾选 Qt 3D 组件。部分发行版中包名可能是 qt6-3d，Debian/Ubuntu 系列可以尝试 apt install qt6-3d-dev。值得注意的是 Qt 3D 在 Qt 6 中的维护状态——它仍然可用，但 Qt 官方的重心已经向 QtQuick3D 倾斜。如果你的项目不需要 C++ API 层面的 3D 控制，直接用下一篇讲的 QtQuick3D 可能是更好的选择。不过理解 Qt 3D 的 ECS 架构对掌握 QtQuick3D 的底层机制也很有帮助，因为 QtQuick3D 内部也是类似的场景图模型。

工具链方面：MSVC 2019+、GCC 11+、Clang 14+，C++17 标准，CMake 3.26+ 构建系统。Qt 3D 底层使用 OpenGL 或 Vulkan 进行渲染，需要确保系统有对应的图形驱动。

## 3. 核心概念讲解

### 3.1 Qt3DWindow——3D 渲染窗口的创建

Qt3DExtras::Qt3DWindow 是 Qt 3D 提供的一个便利窗口类，它内部封装了 QWindow + QRenderAspect + QInputAspect 的组合，开箱即用。创建一个 3D 窗口只需要几行代码：

```cpp
#include <Qt3DExtras/Qt3DWindow>

auto *view = new Qt3DWindow();
view->setTitle("Qt 3D 基础场景");
view->resize(800, 600);
view->show();
```

Qt3DWindow 继承自 QWindow（不是 QWidget），这意味着它不能直接嵌入到 QWidget 的布局中。如果你需要把 3D 视图嵌入 QWidget 界面，需要使用 QWidget::createWindowContainer 来包装：

```cpp
auto *container = QWidget::createWindowContainer(view, parent_widget);
layout->addWidget(container);
```

这种方式可行但有一些限制——嵌入后的 3D 窗口在某些平台上可能有输入事件处理的问题。如果你的界面本来就是纯 QML 的，直接用 Qt3DWindow 作为顶层窗口是最干净的做法。

Qt3DWindow 的核心概念是"根实体"（Root Entity）。每个 3D 窗口都需要一个根实体作为场景图的入口，所有的 3D 对象都是根实体的子实体。设置根实体的方法是：

```cpp
auto *root_entity = new Qt3DCore::QEntity();
view->setRootEntity(root_entity);
```

根实体本身不渲染任何东西——它只是场景图的容器。你需要在它下面创建子实体并挂载组件，才能看到画面。

### 3.2 Entity + Component——ECS 架构基础

Qt 3D 的 ECS 架构中，QEntity 是场景中的节点（可以理解为场景图中的一个位置），QComponent 是挂载到实体上的功能模块。一个实体可以挂载多个组件，每个组件赋予实体一种能力。

一个可见的 3D 物体通常需要三种组件：网格（Mesh）定义形状、材质（Material）定义外观、变换（Transform）定义位置/旋转/缩放。

```cpp
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QPhongMaterial>

auto *entity = new Qt3DCore::QEntity(root_entity);

// 网格组件：球体
auto *mesh = new Qt3DExtras::QSphereMesh();
mesh->setRadius(2.0f);
mesh->setSlices(32);
mesh->setRings(32);

// 材质组件：Phong 光照模型
auto *material = new Qt3DExtras::QPhongMaterial();
material->setDiffuse(QColor(100, 150, 255));

// 变换组件：位置
auto *transform = new Qt3DCore::QTransform();
transform->setTranslation(QVector3D(0, 0, 0));

// 把组件挂载到实体上
entity->addComponent(mesh);
entity->addComponent(material);
entity->addComponent(transform);
```

addComponent 把组件注册到实体上。Qt 3D 内部的各个 System 会在每帧遍历场景图，找到挂载了对应组件的实体并执行处理——渲染系统处理有 Mesh + Material 的实体，变换系统处理有 Transform 的实体，光照系统处理有 Light 的实体。组件的添加顺序不影响渲染结果。

组件的生命周期由 Qt 的父子关系管理。创建组件时如果不指定 parent，addComponent 不会自动接管所有权——你需要手动设置 parent 或者把实体设为 parent。比较安全的做法是在创建组件时就指定 parent：

```cpp
auto *mesh = new Qt3DExtras::QSphereMesh(entity);
```

这样当 entity 被销毁时，组件也会自动释放。

### 3.3 基础几何体——球体、立方体、平面

Qt3DExtras 模块预置了几种常用的基础几何体组件，可以直接使用而不需要加载外部模型文件。

QSphereMesh 生成球体。setRadius 设置半径，setSlices 和 setRings 控制经纬方向的细分数——细分数越高球面越光滑，但顶点数也越多。32x32 对大多数场景来说已经足够平滑：

```cpp
auto *sphere = new Qt3DExtras::QSphereMesh();
sphere->setRadius(1.5f);
sphere->setSlices(32);
sphere->setRings(32);
```

QCuboidMesh 生成长方体（包括立方体）。setXExtents、setYExtents、setZExtents 分别设置三个轴的尺寸，默认是 1x1x1 的单位立方体：

```cpp
auto *cube = new Qt3DExtras::QCuboidMesh();
cube->setXExtents(2.0f);
cube->setYExtents(2.0f);
cube->setZExtents(2.0f);
```

QPlaneMesh 生成平面矩形。setWidth 和 setHeight 设置尺寸。平面默认在 XZ 平面上（面朝 Y 轴正方向），常用来做地面：

```cpp
auto *plane = new Qt3DExtras::QPlaneMesh();
plane->setWidth(20.0f);
plane->setHeight(20.0f);
```

这些基础几何体在原型开发阶段非常方便——你不需要准备任何 .obj 或 .fbx 模型文件就能快速搭建一个 3D 场景。正式项目中当然会用加载外部模型的方式，不过那就是进阶话题了。

### 3.4 相机——视角控制

Qt3DWindow 内部自带了一个 QCamera 实例，通过 camera() 方法获取。相机定义了观察者的位置、朝向和视野参数：

```cpp
auto *camera = view->camera();

// 设置相机位置
camera->setPosition(QVector3D(0, 5, 15));

// 设置相机朝向的目标点（lookAt）
camera->setViewCenter(QVector3D(0, 0, 0));

// 设置相机的上方向
camera->setUpVector(QVector3D(0, 1, 0));

// 设置透视投影参数
camera->setFieldOfView(45.0f);    // 视场角
camera->setNearPlane(0.1f);       // 近裁剪面
camera->setFarPlane(1000.0f);     // 远裁剪面
camera->setAspectRatio(           // 宽高比
    static_cast<float>(view->width()) / view->height());
```

setPosition 是相机的世界坐标位置，setViewCenter 是相机看向的点（不是方向向量，是目标位置），setUpVector 定义哪个方向是"上"——这三个参数完全决定了相机的朝向，原理和 OpenGL 的 gluLookAt 一样。

setFieldOfView 设置垂直视场角（单位是度），45 度是比较常用的值。setNearPlane 和 setFarPlane 定义了可见深度范围——距离相机小于 nearPlane 或大于 farPlane 的物体不会被渲染。宽高比不匹配会导致画面拉伸或压缩，通常设置为窗口宽度除以高度。

想要鼠标拖拽旋转、滚轮缩放的相机控制，Qt 3D 提供了 QOrbitCameraController（轨道相机控制器）：

```cpp
#include <Qt3DExtras/QOrbitCameraController>

auto *controller = new Qt3DExtras::QOrbitCameraController(root_entity);
controller->setCamera(camera);
controller->setLinearSpeed(50.0f);
controller->setLookSpeed(180.0f);
```

QOrbitCameraController 让相机围绕 viewCenter 旋转。左键拖拽旋转视角，右键拖拽平移，滚轮缩放。setLinearSpeed 控制平移速度，setLookSpeed 控制旋转速度。这个控制器在 3D 场景浏览中非常实用，原型阶段几乎必用。

### 3.5 光源与材质

没有光源的 3D 场景是全黑的——除非你用无光照材质。Qt 3D 提供了多种光源组件，最常用的是方向光（QDirectionalLight）和点光源（QPointLight）。

方向光模拟来自无限远处的平行光线（比如太阳光），所有光线方向一致，不随距离衰减：

```cpp
#include <Qt3DRender/QDirectionalLight>

auto *light_entity = new Qt3DCore::QEntity(root_entity);
auto *light = new Qt3DRender::QDirectionalLight();
light->setWorldDirection(QVector3D(-1, -1, -1).normalized());
light->setColor(QColor(255, 255, 255));
light->setIntensity(1.0f);

light_entity->addComponent(light);
```

方向光的 worldDirection 是光线行进的方向向量（不是光源位置），归一化后传入。intensity 控制亮度，1.0 是默认值。

材质方面，QPhongMaterial 实现了经典的 Phong 光照模型，支持环境光（ambient）、漫反射（diffuse）、高光（specular）三个分量：

```cpp
auto *material = new Qt3DExtras::QPhongMaterial();
material->setAmbient(QColor(20, 20, 20));       // 环境光（不受光源影响的底色）
material->setDiffuse(QColor(100, 150, 255));     // 漫反射颜色（主色调）
material->setSpecular(QColor(255, 255, 255));    // 高光颜色
material->setShininess(50.0f);                   // 高光锐度（越大越亮越集中）
```

环境光是物体在没有直接光照时的颜色，通常设为很暗的版本。漫反射是物体在光照下的主色调，这是我们最常调整的属性。高光是反射光在视角方向的集中亮点，shininess 控制高光斑的大小——值越大高光越集中越亮。

Phong 模型在大多数场景下效果都不错，但如果你想追求更物理真实的渲染效果，Qt 3D 还提供了 QMetalRoughMaterial（基于 PBR 物理渲染），不过那属于进阶话题了。

## 4. 综合示例：基础 3D 场景搭建

把前面的知识串起来，我们搭建一个包含球体、立方体、地面的 3D 场景。场景使用轨道相机控制器支持鼠标旋转和缩放，配有方向光照亮所有物体，每个几何体使用不同的 Phong 材质。

CMake 配置：

```cmake
find_package(Qt6 REQUIRED COMPONENTS 3DCore 3DExtras 3DInput Widgets)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::3DCore Qt6::3DExtras Qt6::3DInput Qt6::Widgets)
```

场景构建的核心代码。先创建根实体和相机配置，然后逐个添加几何体实体：

```cpp
void setupScene(Qt3DExtras::Qt3DWindow *view)
{
    auto *root = new Qt3DCore::QEntity();
    view->setRootEntity(root);

    // 相机配置
    auto *camera = view->camera();
    camera->setPosition(QVector3D(0, 8, 20));
    camera->setViewCenter(QVector3D(0, 0, 0));
    camera->setUpVector(QVector3D(0, 1, 0));
    camera->setFieldOfView(45.0f);
    camera->setNearPlane(0.1f);
    camera->setFarPlane(100.0f);

    // 轨道相机控制器（鼠标旋转/缩放）
    auto *orbit = new Qt3DExtras::QOrbitCameraController(root);
    orbit->setCamera(camera);

    // 方向光
    auto *light_entity = new Qt3DCore::QEntity(root);
    auto *dir_light = new Qt3DRender::QDirectionalLight();
    dir_light->setWorldDirection(QVector3D(-1, -1, -1).normalized());
    dir_light->setColor(QColor(255, 255, 255));
    dir_light->setIntensity(0.8f);
    light_entity->addComponent(dir_light);

    // 球体（蓝色，偏左）
    createSphere(root, QVector3D(-4, 1.5f, 0), 1.5f,
                 QColor(70, 130, 230));

    // 立方体（橙色，居中偏右）
    createCube(root, QVector3D(3, 1.0f, 0), 2.0f,
               QColor(230, 140, 50));

    // 地面平面（灰色）
    createPlane(root, QVector3D(0, 0, 0), 30.0f, 30.0f,
                QColor(180, 180, 180));
}
```

创建单个几何体实体的辅助方法：

```cpp
void createSphere(Qt3DCore::QEntity *parent,
                  const QVector3D &pos, float radius,
                  const QColor &color)
{
    auto *entity = new Qt3DCore::QEntity(parent);

    auto *mesh = new Qt3DExtras::QSphereMesh(entity);
    mesh->setRadius(radius);
    mesh->setSlices(32);
    mesh->setRings(32);

    auto *material = new Qt3DExtras::QPhongMaterial(entity);
    material->setDiffuse(color);
    material->setSpecular(QColor(255, 255, 255));
    material->setShininess(80.0f);

    auto *transform = new Qt3DCore::QTransform(entity);
    transform->setTranslation(pos);

    entity->addComponent(mesh);
    entity->addComponent(material);
    entity->addComponent(transform);
}
```

运行程序后你会看到一个 3D 场景：左侧一个蓝色球体，中间偏右一个橙色立方体，底部一块灰色地面，方向光从左上方照下来，两个物体表面都有明显的光照渐变和高光效果。用鼠标左键拖拽可以旋转视角，滚轮缩放，右键拖拽平移。

你会注意到 Phong 材质在光照下表现出的明暗变化让物体看起来有立体感——朝向光源的面亮，背向光源的面暗（由 ambient 底色提供最低亮度），高光在球体上形成一个小亮点。这就是经典光照模型的魅力，几行代码就能让纯色几何体看起来像真正的 3D 物体。

## 5. 练习项目

练习项目：带动画旋转的几何体展示台。

在基础场景上增加旋转动画，让球体绕 Y 轴持续旋转，立方体绕 X 和 Y 轴同时旋转（产生翻滚效果）。同时在场景中新增一个圆环体（使用 Qt3DExtras::QTorusMesh），给每个几何体不同的旋转速度和方向。

完成标准是这样的：使用 QPropertyAnimation 对 QTransform 的 rotation 属性做动画，QTransform 的 rotationY 或 rotationX 属性从 0 到 360 循环；圆环体的参数设置：半径 2.0、管道半径 0.5、细分数 32x32；所有动画设置为无限循环（loopCount = -1）；三个几何体呈三角形排列，各自绕自己的中心旋转。

几个实现提示：QPropertyAnimation 需要目标对象有 Q_PROPERTY 定义的属性——QTransform 已经暴露了 rotationX、rotationY、rotationZ 和 translation 等属性，可以直接用。QPropertyAnimation 的 targetObject 设为 transform 对象，propertyName 设为 "rotationY"（注意是 QByteArray 形式），setStartValue(0)，setEndValue(360)，setLoopCount(-1) 表示无限循环，setDuration 设为毫秒数控制旋转一圈的速度。QTorusMesh 的 setRadius 和 setMinorRadius 分别设置环面半径和管道半径。

## 6. 官方文档参考

[Qt 文档 · Qt 3D 模块](https://doc.qt.io/qt-6/qt3d-index.html) -- Qt 3D 模块总览

[Qt 文档 · Qt3DCore](https://doc.qt.io/qt-6/qt3dcore-index.html) -- ECS 核心类

[Qt 文档 · Qt3DExtras](https://doc.qt.io/qt-6/qt3dextras-index.html) -- 便利类（几何体、材质、窗口）

[Qt 文档 · QEntity](https://doc.qt.io/qt-6/qt3dcore-qentity.html) -- 实体类

[Qt 文档 · QPhongMaterial](https://doc.qt.io/qt-6/qt3dextras-qphongmaterial.html) -- Phong 材质

[Qt 文档 · QOrbitCameraController](https://doc.qt.io/qt-6/qt3dextras-qorbitcameracontroller.html) -- 轨道相机控制器

*(链接已验证，2026-04-23 可访问)*

---

到这里就大功告成了。Qt 3D 的 ECS 架构用 Entity 做容器、Component 做功能挂载、System 做后台处理，把"场景中有什么"和"每个东西怎么表现"干净地解耦了。Qt3DWindow 创建窗口，QEntity + addComponent 组装物体，Mesh/Transform/Material 三件套让几何体可见，Camera + Light 让场景有透视和光照——这套流程跑通之后，后续加骨骼动画、自定义 shader、后处理效果都是在同一个架构上叠加组件。虽然 Qt 3D 在 Qt 6 中的优先级不如 QtQuick3D，但它的 C++ API 设计思路对理解 3D 引擎架构很有帮助。下一篇我们看 QtQuick3D——在 QML 里做同样的事情，代码量会少很多。
