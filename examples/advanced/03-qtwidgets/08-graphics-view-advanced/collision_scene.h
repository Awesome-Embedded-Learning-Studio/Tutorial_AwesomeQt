/// @file    collision_scene.h
/// @brief   碰撞检测场景——管理 CustomShapeItem 集合，实时检测并反馈碰撞状态。
///
/// 对应教程：进阶层 03-QtWidgets/08-图形视图进阶。

#pragma once

#include <QGraphicsView>

/// 碰撞检测演示视图。
///
/// 管理场景中多种类型的 CustomShapeItem（圆形、矩形、多边形），
/// 在每次图元位置变化后调用 collidingItems() 检测碰撞，
/// 并通过 setColliding() 更新视觉反馈（红色边框高亮）。
class CollisionScene : public QGraphicsView
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化场景并创建预设图元。
    /// @param[in] parent 父控件指针。
    explicit CollisionScene(QWidget* parent = nullptr);

private:
    /// @brief 在场景中创建预设的多种图元。
    void populateScene();

    /// @brief 遍历所有图元，检测碰撞并更新视觉状态。
    /// @note 使用 collidingItems(Qt::IntersectsItemShape) 做精确碰撞判断。
    void detectCollisions();
};
