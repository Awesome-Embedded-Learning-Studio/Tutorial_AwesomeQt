// QtWidgets 入门示例 08: 图形视图框架基础
// InteractiveScene 实现

#include "interactivescene.h"

#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QRandomGenerator>
#include <QDebug>

InteractiveScene::InteractiveScene(QObject *parent)
    : QGraphicsScene(parent)
{
    setSceneRect(0, 0, 800, 600);

    // 预设一些图元
    setupPresetItems();
}

void InteractiveScene::addRandomRect()
{
    int w = 80 + QRandomGenerator::global()->bounded(120);
    int h = 60 + QRandomGenerator::global()->bounded(80);
    qreal x = QRandomGenerator::global()->bounded(
        static_cast<int>(sceneRect().width()) - w);
    qreal y = QRandomGenerator::global()->bounded(
        static_cast<int>(sceneRect().height()) - h);

    QColor color = randomColor();

    auto *rect = addRect(x, y, w, h,
        QPen(color.darker(120), 2), QBrush(color));
    rect->setFlags(QGraphicsItem::ItemIsMovable |
                   QGraphicsItem::ItemIsSelectable |
                   QGraphicsItem::ItemSendsGeometryChanges);
    rect->setAcceptHoverEvents(true);
}

void InteractiveScene::addRandomEllipse()
{
    int w = 60 + QRandomGenerator::global()->bounded(100);
    int h = 60 + QRandomGenerator::global()->bounded(100);
    qreal x = QRandomGenerator::global()->bounded(
        static_cast<int>(sceneRect().width()) - w);
    qreal y = QRandomGenerator::global()->bounded(
        static_cast<int>(sceneRect().height()) - h);

    QColor color = randomColor();

    auto *ellipse = addEllipse(x, y, w, h,
        QPen(color.darker(120), 2), QBrush(color));
    ellipse->setFlags(QGraphicsItem::ItemIsMovable |
                      QGraphicsItem::ItemIsSelectable |
                      QGraphicsItem::ItemSendsGeometryChanges);
    ellipse->setAcceptHoverEvents(true);
}

void InteractiveScene::deleteSelectedItems()
{
    auto selected = selectedItems();
    for (auto *item : selected) {
        removeItem(item);
        delete item;
    }
}

void InteractiveScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF scenePos = event->scenePos();

    // 查找点击位置的图元，打印坐标转换信息
    QGraphicsItem *item = itemAt(scenePos, views().isEmpty()
        ? QTransform() : views().first()->transform());

    if (item) {
        QPointF itemPos = item->mapFromScene(scenePos);
        qDebug() << "点击图元:"
                 << "场景坐标:" << scenePos
                 << "图元坐标:" << itemPos
                 << "图元场景位置:" << item->pos();
    } else {
        qDebug() << "点击空白区域，场景坐标:" << scenePos;
    }

    // 必须调用基类实现，否则图元不会收到事件
    QGraphicsScene::mousePressEvent(event);
}

void InteractiveScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // 实时报告鼠标位置
    emit mouseScenePosChanged(event->scenePos());
    QGraphicsScene::mouseMoveEvent(event);
}

void InteractiveScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    // 释放时更新选中数量
    emit selectionCountChanged(selectedItems().size());
    QGraphicsScene::mouseReleaseEvent(event);
}

void InteractiveScene::setupPresetItems()
{
    // 几个预设的矩形
    auto *r1 = addRect(50, 50, 180, 100,
        QPen(QColor("#2980B9"), 2), QBrush(QColor("#D6EAF8")));
    r1->setFlags(QGraphicsItem::ItemIsMovable |
                 QGraphicsItem::ItemIsSelectable |
                 QGraphicsItem::ItemSendsGeometryChanges);
    r1->setAcceptHoverEvents(true);

    auto *r2 = addRect(300, 80, 160, 120,
        QPen(QColor("#27AE60"), 2), QBrush(QColor("#D5F5E3")));
    r2->setFlags(QGraphicsItem::ItemIsMovable |
                 QGraphicsItem::ItemIsSelectable |
                 QGraphicsItem::ItemSendsGeometryChanges);
    r2->setAcceptHoverEvents(true);

    // 几个预设的椭圆
    auto *e1 = addEllipse(100, 220, 140, 140,
        QPen(QColor("#E74C3C"), 2), QBrush(QColor("#FADBD8")));
    e1->setFlags(QGraphicsItem::ItemIsMovable |
                 QGraphicsItem::ItemIsSelectable |
                 QGraphicsItem::ItemSendsGeometryChanges);
    e1->setAcceptHoverEvents(true);

    auto *e2 = addEllipse(350, 250, 120, 100,
        QPen(QColor("#F39C12"), 2), QBrush(QColor("#FEF9E7")));
    e2->setFlags(QGraphicsItem::ItemIsMovable |
                 QGraphicsItem::ItemIsSelectable |
                 QGraphicsItem::ItemSendsGeometryChanges);
    e2->setAcceptHoverEvents(true);

    // 标题文字图元
    auto *title = addText("Graphics View 图形画板",
        QFont("Arial", 18, QFont::Bold));
    title->setDefaultTextColor(QColor("#2C3E50"));
    title->setPos(500, 30);
    title->setFlags(QGraphicsItem::ItemIsMovable |
                    QGraphicsItem::ItemIsSelectable);

    // 说明文字
    auto *desc = addText(
        "拖拽移动图元 | 橡皮带选择多个 | "
        "滚轮缩放 | 工具栏添加/删除图元",
        QFont("Arial", 11));
    desc->setDefaultTextColor(QColor("#7F8C8D"));
    desc->setPos(500, 70);
    desc->setFlags(QGraphicsItem::ItemIsMovable |
                   QGraphicsItem::ItemIsSelectable);
}

QColor InteractiveScene::randomColor() const
{
    int hue = QRandomGenerator::global()->bounded(360);
    return QColor::fromHsv(hue, 120, 220);
}
