// QtWidgets 入门示例 08: 图形视图框架基础
// GraphicsView 实现

#include "graphicsview.h"

#include <QWheelEvent>

GraphicsView::GraphicsView(QGraphicsScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent)
{
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::RubberBandDrag);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setMinimumSize(600, 400);
}

void GraphicsView::wheelEvent(QWheelEvent *event)
{
    double factor = 1.15;
    if (event->angleDelta().y() > 0) {
        scale(factor, factor);     // 放大
    } else {
        scale(1.0 / factor, 1.0 / factor);  // 缩小
    }
}
