/**
 * Qt 3D 基础场景搭建示例
 *
 * 本示例演示 Qt 3D 模块的核心功能：
 * 1. Qt3DExtras::Qt3DWindow 创建独立 3D 渲染窗口
 * 2. QEntity + QComponent 的 ECS 架构组装 3D 对象
 * 3. QSphereMesh / QCuboidMesh / QPlaneMesh 基础几何体
 * 4. QCamera 相机配置 + QOrbitCameraController 轨道控制
 * 5. QDirectionalLight 方向光 + QPhongMaterial Phong 材质
 *
 * 核心要点：
 * - Entity 是容器，Component 是功能，addComponent 挂载
 * - 可见物体需要 Mesh + Material + Transform 三个组件
 * - 轨道控制器让鼠标可以旋转/缩放/平移视角
 * - Phong 材质通过 ambient/diffuse/specular 模拟光照效果
 */

#include <QColor>
#include <QVector3D>

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>

#include <Qt3DExtras/Qt3DWindow>

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
                                const QColor &color);

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
                              const QColor &color);

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
                               const QColor &color);

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
    float intensity);

// ========================================
// 辅助函数：添加旋转动画
// ========================================

/// @brief 给实体的 Transform 组件添加 Y 轴旋转动画
/// @param entity 目标实体（必须已挂载 QTransform）
/// @param duration_ms 旋转一圈的毫秒数
void addRotationAnimation(Qt3DCore::QEntity *entity, int duration_ms);

// ========================================
// 场景构建：组装所有 3D 对象
// ========================================

/// @brief 构建 3D 场景（相机、光源、几何体）
/// @param view Qt3DWindow 窗口实例
void setupScene(Qt3DExtras::Qt3DWindow *view);
