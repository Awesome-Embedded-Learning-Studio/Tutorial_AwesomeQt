/// @file    custom_shape_item.cpp
/// @brief   CustomShapeItem 实现——三种图元形状的精确碰撞检测与视觉反馈。
///
/// 对应教程：进阶层 03-QtWidgets/08-图形视图进阶。

#include "custom_shape_item.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

// ─── 常量 ───────────────────────────────────────────────────────────────────────

/// 画笔宽度，boundingRect 需要向外扩展 pen 半宽。
static constexpr qreal kPenWidth = 2.0;

/// 选中/碰撞状态额外余量（虚线框、抗锯齿）。
static constexpr qreal kPadding = 3.0;

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

CustomShapeItem::CustomShapeItem(ShapeKind kind, qreal size, QGraphicsItem* parent)
    : QGraphicsItem(parent)
    , m_kind(kind)
    , m_size(size)
    , m_colliding(false)
{
    // 启用鼠标悬停检测，用于高亮反馈
    setAcceptHoverEvents(true);
    // 允许选中
    setFlag(ItemIsSelectable);
    // 允许拖拽
    setFlag(ItemIsMovable);
    // 启用位置变化通知，itemChange 才会被调用
    setFlag(ItemSendsGeometryChanges);
}

// ─────────────────────────────────────────────────────────────────────────────
// type()——运行时类型识别
// ─────────────────────────────────────────────────────────────────────────────

int CustomShapeItem::type() const
{
    // 自定义类型 ID，必须大于 UserType
    return UserType + 1;
}

// ─────────────────────────────────────────────────────────────────────────────
// boundingRect——包围矩形
// ─────────────────────────────────────────────────────────────────────────────

QRectF CustomShapeItem::boundingRect() const
{
    // 向外扩展 pen 半宽 + 额外余量，防止旋转或选中虚线框被裁剪
    const qreal pad = kPenWidth / 2.0 + kPadding;

    switch (m_kind) {
    case ShapeKind::kCircle: {
        // 圆心在 (m_size, m_size)，半径为 m_size
        return QRectF(-pad, -pad, m_size * 2 + pad * 2, m_size * 2 + pad * 2);
    }
    case ShapeKind::kRectangle: {
        // 矩形左上角在 (0, 0)，宽高为 m_size x m_size
        return QRectF(-pad, -pad, m_size + pad * 2, m_size + pad * 2);
    }
    case ShapeKind::kPolygon: {
        // 多边形的包围矩形覆盖所有顶点
        return QRectF(-pad, -pad, m_size + pad * 2, m_size + pad * 2);
    }
    }
    return QRectF();
}

// ─────────────────────────────────────────────────────────────────────────────
// paint——绘制
// ─────────────────────────────────────────────────────────────────────────────

void CustomShapeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                            QWidget* /*widget*/)
{
    // 去掉默认选中虚线框，我们用自己的视觉反馈
    QStyleOptionGraphicsItem opt(*option);
    opt.state &= ~QStyle::State_Selected;

    // 碰撞时红色边框，正常时深灰色边框
    const QColor borderColor = m_colliding ? QColor(220, 50, 50) : QColor(60, 60, 60);
    // 碰撞时浅红填充，正常时浅蓝填充
    const QColor fillColor = m_colliding ? QColor(255, 200, 200, 180)
                                         : QColor(180, 210, 240, 180);

    painter->setPen(QPen(borderColor, kPenWidth));
    painter->setBrush(QBrush(fillColor));
    painter->setRenderHint(QPainter::Antialiasing);

    switch (m_kind) {
    case ShapeKind::kCircle: {
        // 圆心在 (m_size, m_size)，半径为 m_size
        painter->drawEllipse(0, 0, m_size * 2, m_size * 2);
        break;
    }
    case ShapeKind::kRectangle: {
        painter->drawRect(0, 0, static_cast<int>(m_size), static_cast<int>(m_size));
        break;
    }
    case ShapeKind::kPolygon: {
        // 五角星形状
        QPolygonF star;
        const qreal cx = m_size / 2.0;
        const qreal cy = m_size / 2.0;
        const qreal outerR = m_size / 2.0;
        const qreal innerR = outerR * 0.4;
        for (int i = 0; i < 5; ++i) {
            // 外顶点
            const qreal outerAngle = (i * 72.0 - 90.0) * M_PI / 180.0;
            star << QPointF(cx + outerR * std::cos(outerAngle),
                            cy + outerR * std::sin(outerAngle));
            // 内顶点
            const qreal innerAngle = ((i * 72.0 + 36.0) - 90.0) * M_PI / 180.0;
            star << QPointF(cx + innerR * std::cos(innerAngle),
                            cy + innerR * std::sin(innerAngle));
        }
        painter->drawPolygon(star);
        break;
    }
    }

    // 绘制形状标签
    painter->setPen(Qt::black);
    QFont font = painter->font();
    font.setPointSize(8);
    painter->setFont(font);

    QString label;
    switch (m_kind) {
    case ShapeKind::kCircle:    label = QStringLiteral("Circle"); break;
    case ShapeKind::kRectangle: label = QStringLiteral("Rect"); break;
    case ShapeKind::kPolygon:   label = QStringLiteral("Star"); break;
    }

    // 标签居中
    const QRectF textRect = boundingRect().adjusted(kPadding, kPadding, -kPadding, -kPadding);
    painter->drawText(textRect, Qt::AlignCenter, label);
}

// ─────────────────────────────────────────────────────────────────────────────
// shape()——精确碰撞轮廓
// ─────────────────────────────────────────────────────────────────────────────

QPainterPath CustomShapeItem::shape() const
{
    return buildShapePath();
}

// ─────────────────────────────────────────────────────────────────────────────
// setColliding——碰撞状态更新
// ─────────────────────────────────────────────────────────────────────────────

void CustomShapeItem::setColliding(bool colliding)
{
    if (m_colliding != colliding) {
        m_colliding = colliding;
        // 触发重绘以更新视觉反馈
        update();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// buildShapePath——构建精确轮廓路径
// ─────────────────────────────────────────────────────────────────────────────

QPainterPath CustomShapeItem::buildShapePath() const
{
    QPainterPath path;

    switch (m_kind) {
    case ShapeKind::kCircle: {
        // 圆形使用 addEllipse，碰撞检测精确到弧线而非矩形
        path.addEllipse(0, 0, m_size * 2, m_size * 2);
        break;
    }
    case ShapeKind::kRectangle: {
        // 矩形使用 addRect
        path.addRect(0, 0, m_size, m_size);
        break;
    }
    case ShapeKind::kPolygon: {
        // 五角星——和 paint() 中保持一致
        QPolygonF star;
        const qreal cx = m_size / 2.0;
        const qreal cy = m_size / 2.0;
        const qreal outerR = m_size / 2.0;
        const qreal innerR = outerR * 0.4;
        for (int i = 0; i < 5; ++i) {
            const qreal outerAngle = (i * 72.0 - 90.0) * M_PI / 180.0;
            star << QPointF(cx + outerR * std::cos(outerAngle),
                            cy + outerR * std::sin(outerAngle));
            const qreal innerAngle = ((i * 72.0 + 36.0) - 90.0) * M_PI / 180.0;
            star << QPointF(cx + innerR * std::cos(innerAngle),
                            cy + innerR * std::sin(innerAngle));
        }
        path.addPolygon(star);
        path.closeSubpath();
        break;
    }
    }

    return path;
}
