/// @file    scene_builder.cpp
/// @brief   SceneBuilder 类的实现，构建 Qt3D 3D 场景。
///
/// 对应教程：进阶层 05-其他模块/14-Qt3D。

// 1. 对应头文件
#include "scene_builder.h"

// 2. Qt 头文件
#include <QColor>
#include <QVector3D>

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>

#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QOrbitCameraController>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/Qt3DWindow>

#include <Qt3DRender/QCamera>
#include <Qt3DRender/QDirectionalLight>

/// @brief 构造函数。
/// @param[in] parent 父对象指针，Qt 对象树自动管理生命周期。
SceneBuilder::SceneBuilder(QObject* parent) : QObject(parent) {}

/// @brief 为 Qt3DWindow 配置相机和轨道控制器。
/// @param[in,out] window 目标 Qt3D 窗口。
/// @param[in] rootEntity 场景根实体，轨道控制器挂载到该实体下。
void SceneBuilder::setupCamera(Qt3DExtras::Qt3DWindow* window,
                               Qt3DCore::QEntity* rootEntity)
{
    // 获取窗口默认相机，设置透视投影参数
    Qt3DRender::QCamera* camera = window->camera();
    camera->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
    camera->setPosition(QVector3D(0.0f, 5.0f, 15.0f));
    camera->setViewCenter(QVector3D(0.0f, 0.0f, 0.0f));
    camera->setUpVector(QVector3D(0.0f, 1.0f, 0.0f));

    // @note OrbitCameraController 允许鼠标拖拽旋转视角、滚轮缩放
    //       其父实体设为 rootEntity，确保控制器能遍历整个场景
    auto* controller =
        new Qt3DExtras::QOrbitCameraController(rootEntity);
    controller->setCamera(camera);
    controller->setLinearSpeed(50.0f);
    controller->setLookSpeed(180.0f);
}

/// @brief 在根实体上构建完整的 3D 场景。
/// @param[in] rootEntity 场景根节点。
void SceneBuilder::buildScene(Qt3DCore::QEntity* rootEntity)
{
    // === 光源 ===
    // @note 方向光模拟平行光（如太阳光）， intensity 控制光照强度
    auto* lightEntity = new Qt3DCore::QEntity(rootEntity);
    auto* light = new Qt3DRender::QDirectionalLight(lightEntity);
    light->setWorldDirection(QVector3D(-1.0f, -1.0f, -1.0f).normalized());
    light->setColor(QColor(255, 255, 255));
    light->setIntensity(1.0f);
    lightEntity->addComponent(light);

    // === 红色球体 ===
    auto* sphereEntity = new Qt3DCore::QEntity(rootEntity);

    auto* sphereMesh = new Qt3DExtras::QSphereMesh(sphereEntity);
    sphereMesh->setRadius(2.0f);
    sphereMesh->setRings(32);
    sphereMesh->setSlices(32);

    // Phong 材质：环境光 + 漫反射 + 镜面反射的经典光照模型
    auto* sphereMaterial = new Qt3DExtras::QPhongMaterial(sphereEntity);
    sphereMaterial->setDiffuse(QColor(200, 30, 30));   // 漫反射颜色（红色）
    sphereMaterial->setSpecular(QColor(255, 200, 200)); // 镜面高光颜色
    sphereMaterial->setShininess(50.0f);                 // 镜面高光锐度

    auto* sphereTransform = new Qt3DCore::QTransform(sphereEntity);
    sphereTransform->setTranslation(QVector3D(0.0f, 2.0f, 0.0f));

    sphereEntity->addComponent(sphereMesh);
    sphereEntity->addComponent(sphereMaterial);
    sphereEntity->addComponent(sphereTransform);

    // === 蓝色圆柱体（作为球体的子实体，演示变换层次） ===
    // @note 子实体的变换在父实体变换的基础上叠加
    //       圆柱体位于球体本地坐标 (2, 0, 0) 处，世界坐标为 (2, 2, 0)
    auto* cylinderEntity = new Qt3DCore::QEntity(sphereEntity);

    auto* cylinderMesh = new Qt3DExtras::QCylinderMesh(cylinderEntity);
    cylinderMesh->setRadius(0.5f);
    cylinderMesh->setLength(3.0f);
    cylinderMesh->setRings(16);
    cylinderMesh->setSlices(16);

    auto* cylinderMaterial = new Qt3DExtras::QPhongMaterial(cylinderEntity);
    cylinderMaterial->setDiffuse(QColor(30, 30, 200));   // 漫反射颜色（蓝色）
    cylinderMaterial->setSpecular(QColor(200, 200, 255)); // 镜面高光颜色
    cylinderMaterial->setShininess(50.0f);

    auto* cylinderTransform = new Qt3DCore::QTransform(cylinderEntity);
    cylinderTransform->setTranslation(QVector3D(2.5f, 0.0f, 0.0f));
    // 绕 Z 轴旋转 90 度，让圆柱体横置
    cylinderTransform->setRotationZ(90.0f);

    cylinderEntity->addComponent(cylinderMesh);
    cylinderEntity->addComponent(cylinderMaterial);
    cylinderEntity->addComponent(cylinderTransform);
}
