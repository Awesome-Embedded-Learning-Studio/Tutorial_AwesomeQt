/// @file    scene_builder.h
/// @brief   3D scene builder using Qt3D C++ API.
///
/// 对应教程：进阶层 05-其他模块/14-Qt3D。
/// 演示 QEntity 层级结构、QTransform 变换层次、QPhongMaterial 自定义材质
/// 以及 QDirectionalLight 光源配置。

#pragma once

#include <QObject>

namespace Qt3DCore
{
class QEntity;
} // namespace Qt3DCore

namespace Qt3DExtras
{
class Qt3DWindow;
} // namespace Qt3DExtras

/// @brief 构建 Qt3D 场景的工具类。
///
/// 负责 3D 场景中所有实体的创建与组装：相机、光源、带 Phong 材质的几何体，
/// 以及父子实体间的变换层次关系（hierarchical transform）。
class SceneBuilder : public QObject
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父对象指针，Qt 对象树自动管理生命周期。
    explicit SceneBuilder(QObject* parent = nullptr);

    /// @brief 为 Qt3DWindow 配置相机参数并创建轨道控制器。
    /// @param[in,out] window 目标 Qt3D 窗口，将设置其默认相机的投影与视口参数。
    /// @param[in] rootEntity 场景根实体，轨道控制器将挂载到该实体下。
    /// @note 相机使用透视投影，视角 45 度，近裁面 0.1，远裁面 1000。
    ///       rootEntity 必须在调用前已创建但尚未设置到窗口。
    void setupCamera(Qt3DExtras::Qt3DWindow* window,
                     Qt3DCore::QEntity* rootEntity);

    /// @brief 在给定根实体上构建完整的 3D 场景。
    /// @param[in] rootEntity 场景根节点，所有子实体挂载到该节点下。
    ///
    /// 场景包含：
    /// - 方向光源（模拟太阳光）
    /// - 红色球体（带 Phong 材质）
    /// - 蓝色圆柱体（作为球体的子实体，演示变换层次）
    void buildScene(Qt3DCore::QEntity* rootEntity);
};
