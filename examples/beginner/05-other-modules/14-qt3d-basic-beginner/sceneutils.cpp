#include "sceneutils.h"

#include <QApplication>
#include <QByteArray>
#include <QColor>
#include <QGuiApplication>
#include <QPropertyAnimation>
#include <QVector3D>
#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>

#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QOrbitCameraController>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QPlaneMesh>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/Qt3DWindow>

#include <Qt3DRender/QCamera>
#include <Qt3DRender/QDirectionalLight>

// ========================================
// 辅助函数：创建球体实体
// ========================================

/// @brief 创建一个带 Phong 材质的球体实体
/// @param parent 父实体（场景根节点）
/// @param position 球心位置
/// @param radius 球体半径
/// @param color 漫反射颜色
/// @return 创建的实体指针
Qt3DCore::QEntity *createSphere(Qt3DCore::QEntity *parent,
                                const QVector3D &position,
                                float radius,
                                const QColor &color)
{
    auto *entity = new Qt3DCore::QEntity(parent);

    // 网格组件：球体
    auto *mesh = new Qt3DExtras::QSphereMesh(entity);
    mesh->setRadius(radius);
    mesh->setSlices(32);   // 经线细分
    mesh->setRings(32);    // 纬线细分

    // 材质组件：Phong 光照
    auto *material = new Qt3DExtras::QPhongMaterial(entity);
    material->setAmbient(color.darker(300));  // 暗色环境光
    material->setDiffuse(color);               // 主色调
    material->setSpecular(QColor(255, 255, 255));  // 白色高光
    material->setShininess(80.0f);             // 高光锐度

    // 变换组件：位置
    auto *transform = new Qt3DCore::QTransform(entity);
    transform->setTranslation(position);

    // 挂载组件到实体
    entity->addComponent(mesh);
    entity->addComponent(material);
    entity->addComponent(transform);

    return entity;
}

// ========================================
// 辅助函数：创建立方体实体
// ========================================

/// @brief 创建一个带 Phong 材质的立方体实体
/// @param parent 父实体
/// @param position 中心位置
/// @param size 边长
/// @param color 漫反射颜色
/// @return 创建的实体指针
Qt3DCore::QEntity *createCube(Qt3DCore::QEntity *parent,
                              const QVector3D &position,
                              float size,
                              const QColor &color)
{
    auto *entity = new Qt3DCore::QEntity(parent);

    // 网格组件：立方体
    auto *mesh = new Qt3DExtras::QCuboidMesh(entity);
    mesh->setXExtent(size);
    mesh->setYExtent(size);
    mesh->setZExtent(size);

    // 材质组件
    auto *material = new Qt3DExtras::QPhongMaterial(entity);
    material->setAmbient(color.darker(300));
    material->setDiffuse(color);
    material->setSpecular(QColor(255, 255, 255));
    material->setShininess(50.0f);

    // 变换组件：位置 + 初始旋转
    auto *transform = new Qt3DCore::QTransform(entity);
    transform->setTranslation(position);
    // 立方体稍微倾斜一点，看起来更有立体感
    transform->setRotationX(15.0f);
    transform->setRotationY(25.0f);

    entity->addComponent(mesh);
    entity->addComponent(material);
    entity->addComponent(transform);

    return entity;
}

// ========================================
// 辅助函数：创建地面平面
// ========================================

/// @brief 创建一个地面平面实体
/// @param parent 父实体
/// @param position 中心位置
/// @param width X 方向尺寸
/// @param height Z 方向尺寸
/// @param color 漫反射颜色
/// @return 创建的实体指针
Qt3DCore::QEntity *createPlane(Qt3DCore::QEntity *parent,
                               const QVector3D &position,
                               float width,
                               float height,
                               const QColor &color)
{
    auto *entity = new Qt3DCore::QEntity(parent);

    // 网格组件：平面（默认在 XZ 平面上，法线朝 +Y）
    auto *mesh = new Qt3DExtras::QPlaneMesh(entity);
    mesh->setWidth(width);
    mesh->setHeight(height);

    // 材质组件：灰色地面
    auto *material = new Qt3DExtras::QPhongMaterial(entity);
    material->setAmbient(color.darker(200));
    material->setDiffuse(color);
    material->setSpecular(QColor(50, 50, 50));  // 地面高光弱一些
    material->setShininess(10.0f);

    // 变换组件
    auto *transform = new Qt3DCore::QTransform(entity);
    transform->setTranslation(position);

    entity->addComponent(mesh);
    entity->addComponent(material);
    entity->addComponent(transform);

    return entity;
}

// ========================================
// 辅助函数：创建方向光实体
// ========================================

/// @brief 创建一个方向光（模拟太阳光）
/// @param parent 父实体
/// @param direction 光线方向（归一化向量）
/// @param color 光源颜色
/// @param intensity 光源强度
/// @return 创建的光源实体指针
Qt3DCore::QEntity *createDirectionalLight(
    Qt3DCore::QEntity *parent,
    const QVector3D &direction,
    const QColor &color,
    float intensity)
{
    auto *entity = new Qt3DCore::QEntity(parent);

    auto *light = new Qt3DRender::QDirectionalLight(entity);
    light->setWorldDirection(direction);
    light->setColor(color);
    light->setIntensity(intensity);

    entity->addComponent(light);

    return entity;
}

// ========================================
// 辅助函数：添加旋转动画
// ========================================

/// @brief 给实体的 Transform 组件添加 Y 轴旋转动画
/// @param entity 目标实体（必须已挂载 QTransform）
/// @param duration_ms 旋转一圈的毫秒数
void addRotationAnimation(Qt3DCore::QEntity *entity, int duration_ms)
{
    // 查找实体上的 Transform 组件
    auto transforms = entity->componentsOfType<Qt3DCore::QTransform>();
    if (transforms.isEmpty()) {
        qWarning() << "实体没有 Transform 组件，无法添加旋转动画";
        return;
    }

    auto *transform = transforms.first();
    auto *animation = new QPropertyAnimation(transform, QByteArray("rotationY"));
    animation->setStartValue(0.0f);
    animation->setEndValue(360.0f);
    animation->setDuration(duration_ms);
    animation->setLoopCount(-1);  // 无限循环
    animation->start();

    // 防止动画被垃圾回收（附着到实体上延长生命周期）
    animation->setParent(entity);
}

// ========================================
// 场景构建：组装所有 3D 对象
// ========================================

/// @brief 构建 3D 场景（相机、光源、几何体）
/// @param view Qt3DWindow 窗口实例
void setupScene(Qt3DExtras::Qt3DWindow *view)
{
    // 根实体：场景图的顶层容器
    auto *root = new Qt3DCore::QEntity();
    view->setRootEntity(root);

    // ========================================
    // 相机配置
    // ========================================

    auto *camera = view->camera();
    camera->setPosition(QVector3D(0, 10, 25));     // 相机位置：高处偏后
    camera->setViewCenter(QVector3D(0, 1, 0));     // 看向场景中心偏上
    camera->setUpVector(QVector3D(0, 1, 0));       // Y 轴朝上
    camera->setFieldOfView(45.0f);                 // 透视视场角
    camera->setNearPlane(0.1f);                    // 近裁剪面
    camera->setFarPlane(1000.0f);                  // 远裁剪面
    camera->setAspectRatio(
        static_cast<float>(view->width()) / view->height());

    // 轨道相机控制器：鼠标左键旋转、右键平移、滚轮缩放
    auto *orbit_ctrl =
        new Qt3DExtras::QOrbitCameraController(root);
    orbit_ctrl->setCamera(camera);
    orbit_ctrl->setLinearSpeed(50.0f);   // 平移速度
    orbit_ctrl->setLookSpeed(180.0f);    // 旋转速度

    // ========================================
    // 光源
    // ========================================

    // 主方向光：从左上方照射（模拟太阳光）
    createDirectionalLight(
        root,
        QVector3D(-1, -1, -0.5f).normalized(),
        QColor(255, 255, 255),
        0.9f);

    // 补光：从右侧补光，减弱阴影面
    createDirectionalLight(
        root,
        QVector3D(1, -0.5f, 1).normalized(),
        QColor(200, 200, 230),
        0.3f);

    // ========================================
    // 几何体
    // ========================================

    // 球体（蓝色，偏左）
    auto *sphere = createSphere(
        root,
        QVector3D(-5, 1.5f, 0),   // 位置
        1.5f,                      // 半径
        QColor(70, 130, 230));     // 蓝色

    // 立方体（橙色，居中偏右）
    auto *cube = createCube(
        root,
        QVector3D(3, 1.0f, 0),    // 位置
        2.0f,                      // 边长
        QColor(230, 140, 50));     // 橙色

    // 地面（灰色）
    createPlane(
        root,
        QVector3D(0, 0, 0),       // 地面位置（Y=0 平面）
        40.0f,                     // 宽度
        40.0f,                     // 深度
        QColor(160, 160, 160));    // 灰色

    // ========================================
    // 旋转动画
    // ========================================

    // 球体缓慢旋转（6 秒一圈）
    addRotationAnimation(sphere, 6000);

    // 立方体快速旋转（3 秒一圈）
    addRotationAnimation(cube, 3000);

    qDebug() << "3D 场景构建完成";
    qDebug() << "操作提示：鼠标左键拖拽旋转 | 右键拖拽平移 | 滚轮缩放";
}
