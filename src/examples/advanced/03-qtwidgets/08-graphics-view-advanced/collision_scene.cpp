/// @file    collision_scene.cpp
/// @brief   CollisionScene 实现——碰撞检测与多形状图元管理。
///
/// 对应教程：进阶层 03-QtWidgets/08-图形视图进阶。

#include "collision_scene.h"
#include "custom_shape_item.h"

#include <QGraphicsScene>

// ─── 常量 ───────────────────────────────────────────────────────────────────────

/// 场景宽度。
static constexpr qreal kSceneWidth = 800.0;

/// 场景高度。
static constexpr qreal kSceneHeight = 600.0;

/// 图元基准尺寸。
static constexpr qreal kItemSize = 60.0;

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

CollisionScene::CollisionScene(QWidget* parent)
    : QGraphicsView(parent)
{
    auto* scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, kSceneWidth, kSceneHeight);
    setScene(scene);

    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::RubberBandDrag);
    setWindowTitle(QStringLiteral("Collision Detection Demo"));
    resize(static_cast<int>(kSceneWidth), static_cast<int>(kSceneHeight));

    populateScene();

    // 场景变化后（拖拽、添加/移除图元）重新检测碰撞
    connect(scene, &QGraphicsScene::changed, this, &CollisionScene::detectCollisions);

    // 初始检测一次
    detectCollisions();
}

// ─────────────────────────────────────────────────────────────────────────────
// 创建预设图元
// ─────────────────────────────────────────────────────────────────────────────

void CollisionScene::populateScene()
{
    auto* s = scene();

    // 两个圆形——初始位置有一定重叠，便于观察碰撞效果
    auto* circle1 = new CustomShapeItem(CustomShapeItem::ShapeKind::kCircle, kItemSize);
    circle1->setPos(100, 100);
    s->addItem(circle1);

    auto* circle2 = new CustomShapeItem(CustomShapeItem::ShapeKind::kCircle, kItemSize);
    circle2->setPos(140, 120);
    s->addItem(circle2);

    // 两个矩形
    auto* rect1 = new CustomShapeItem(CustomShapeItem::ShapeKind::kRectangle, kItemSize);
    rect1->setPos(350, 80);
    s->addItem(rect1);

    auto* rect2 = new CustomShapeItem(CustomShapeItem::ShapeKind::kRectangle, kItemSize);
    rect2->setPos(380, 100);
    s->addItem(rect2);

    // 两个多边形（五角星）
    auto* poly1 = new CustomShapeItem(CustomShapeItem::ShapeKind::kPolygon, kItemSize);
    poly1->setPos(550, 150);
    s->addItem(poly1);

    auto* poly2 = new CustomShapeItem(CustomShapeItem::ShapeKind::kPolygon, kItemSize);
    poly2->setPos(580, 170);
    s->addItem(poly2);

    // 跨类型碰撞组——圆形和多边形相邻
    auto* circle3 = new CustomShapeItem(CustomShapeItem::ShapeKind::kCircle, kItemSize);
    circle3->setPos(200, 350);
    s->addItem(circle3);

    auto* rect3 = new CustomShapeItem(CustomShapeItem::ShapeKind::kRectangle, kItemSize);
    rect3->setPos(230, 370);
    s->addItem(rect3);

    // 提示文字
    auto* hint = s->addText(QStringLiteral(
        "拖拽图元观察碰撞检测——碰撞时变为红色，"
        "shape() 提供精确轮廓而非 boundingRect 矩形近似"));
    hint->setPos(10, kSceneHeight - 60);
}

// ─────────────────────────────────────────────────────────────────────────────
// 碰撞检测
// ─────────────────────────────────────────────────────────────────────────────

void CollisionScene::detectCollisions()
{
    const auto items = scene()->items();

    // 先重置所有图元为非碰撞状态
    for (auto* item : items) {
        auto* custom = dynamic_cast<CustomShapeItem*>(item);
        if (custom) {
            custom->setColliding(false);
        }
    }

    // 用 collidingItems(Qt::IntersectsItemShape) 做精确碰撞判断
    for (auto* item : items) {
        auto* custom = dynamic_cast<CustomShapeItem*>(item);
        if (!custom) {
            continue;
        }

        // IntersectsItemShape：shape() 有交集即算碰撞
        const auto colliders = custom->collidingItems(Qt::IntersectsItemShape);
        for (auto* collider : colliders) {
            auto* other = dynamic_cast<CustomShapeItem*>(collider);
            if (other) {
                // 双方都标记为碰撞状态
                custom->setColliding(true);
                other->setColliding(true);
            }
        }
    }
}
