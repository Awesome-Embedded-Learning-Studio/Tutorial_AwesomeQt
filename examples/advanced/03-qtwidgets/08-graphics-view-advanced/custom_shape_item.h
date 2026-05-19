/// @file    custom_shape_item.h
/// @brief   自定义 QGraphicsItem 子类——重写 shape() 实现精确碰撞检测，
///          支持圆形、矩形和自定义多边形三种图元类型。
///
/// 对应教程：进阶层 03-QtWidgets/08-图形视图进阶。

#pragma once

#include <QGraphicsItem>

/// 自定义图形图元，演示 boundingRect / paint / shape / type 四个关键虚函数。
///
/// 三种图元形状：
/// - Circle：圆形，shape() 返回椭圆路径，碰撞检测精确到弧线
/// - Rectangle：矩形，shape() 返回矩形路径
/// - Polygon：自定义多边形，shape() 返回多边形路径
///
/// 碰撞时自动切换为红色边框，离开碰撞时恢复原始颜色。
class CustomShapeItem : public QGraphicsItem
{
public:
    /// 图元形状枚举
    enum class ShapeKind { kCircle, kRectangle, kPolygon };

    /// @brief 构造函数。
    /// @param[in] kind 图元形状类型。
    /// @param[in] size 图元基准尺寸。
    /// @param[in] parent 父图元指针。
    explicit CustomShapeItem(ShapeKind kind, qreal size,
                             QGraphicsItem* parent = nullptr);

    /// @brief 返回图元类型 ID，用于运行时类型识别。
    /// @note 在子类中定义 enum { Type = UserType + 1 }，支持 qgraphicsitem_cast。
    int type() const override;

    /// @brief 返回包围矩形，必须覆盖 paint() 所有绘制区域。
    /// @note 包含 pen 半宽 + 2px 余量，防止旋转或缩放时裁剪。
    QRectF boundingRect() const override;

    /// @brief 绘制图元内容。
    /// @param[in] painter 已配置好坐标系的画笔。
    /// @param[in] option 风格选项（提供状态信息如选中状态）。
    /// @param[in] widget 所属视图控件（可选）。
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget) override;

    /// @brief 返回精确轮廓路径，用于碰撞检测和鼠标命中判断。
    /// @note 默认实现返回 boundingRect 的矩形路径，重写后支持非矩形精确碰撞。
    QPainterPath shape() const override;

    /// @brief 更新碰撞视觉状态。
    /// @param[in] colliding 是否处于碰撞中。
    void setColliding(bool colliding);

private:
    /// @brief 根据形状类型构建对应的 QPainterPath。
    /// @return 精确轮廓路径。
    QPainterPath buildShapePath() const;

    ShapeKind m_kind;             // 图元形状类型
    qreal m_size;                 // 基准尺寸（半径或边长）
    bool m_colliding;             // 当前是否处于碰撞状态
};
