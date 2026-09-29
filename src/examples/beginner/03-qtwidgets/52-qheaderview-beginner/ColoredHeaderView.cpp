#include "ColoredHeaderView.h"

#include <QColor>
#include <QFont>
#include <QLinearGradient>
#include <QPainter>
#include <QPen>
#include <QPolygon>

ColoredHeaderView::ColoredHeaderView(Qt::Orientation orientation,
                                     QWidget *parent)
    : QHeaderView(orientation, parent)
{
    setAttribute(Qt::WA_Hover);
    setSortIndicatorShown(true);

    // 调色板：每列一个颜色
    kColors = {
        QColor("#4A90D9"),  // 蓝色
        QColor("#5CB85C"),  // 绿色
        QColor("#F0AD4E"),  // 橙色
        QColor("#D9534F"),  // 红色
        QColor("#9B59B6"),  // 紫色
        QColor("#1ABC9C"),  // 青色
    };
}

void ColoredHeaderView::paintSection(QPainter *painter, const QRect &rect,
                                     int logicalIndex) const
{
    if (!rect.isValid()) return;

    painter->save();

    // 渐变背景
    QLinearGradient gradient(rect.topLeft(), rect.bottomLeft());
    gradient.setColorAt(0.0, QColor("#F8F9FA"));
    gradient.setColorAt(1.0, QColor("#E9ECEF"));
    painter->fillRect(rect, gradient);

    // 底部分隔线
    painter->setPen(QPen(QColor("#DEE2E6"), 1));
    painter->drawLine(rect.bottomLeft(), rect.bottomRight());

    // 右侧分隔线
    painter->drawLine(rect.topRight(), rect.bottomRight());

    // 左侧彩色方块标记
    int tagSize = 10;
    int tagMargin = 8;
    QRect tagRect(rect.left() + tagMargin,
                  rect.top() + (rect.height() - tagSize) / 2,
                  tagSize, tagSize);

    QColor tagColor = (logicalIndex < kColors.size())
                          ? kColors[logicalIndex]
                          : QColor("#999999");
    painter->setPen(Qt::NoPen);
    painter->setBrush(tagColor);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawRoundedRect(tagRect, 2, 2);

    // 获取表头文本
    QString text = model()->headerData(
        logicalIndex, orientation(), Qt::DisplayRole).toString();

    // 绘制文本（左侧预留方块的空间）
    painter->setPen(QColor("#212529"));
    painter->setFont(QFont("sans-serif", 10, QFont::Bold));
    painter->setRenderHint(QPainter::Antialiasing, false);
    int textLeft = tagRect.right() + 6;
    QRect textRect(textLeft, rect.top(),
                   rect.right() - textLeft - 8, rect.height());
    painter->drawText(textRect,
                      Qt::AlignVCenter | Qt::AlignLeft, text);

    // 排序指示箭头
    if (isSortIndicatorShown()
        && sortIndicatorSection() == logicalIndex) {
        drawSortArrow(painter, rect);
    }

    painter->restore();
}

/// @brief 绘制排序方向箭头
void ColoredHeaderView::drawSortArrow(QPainter *painter, const QRect &rect) const
{
    int arrowSize = 5;
    int x = rect.right() - 16;
    int y = rect.center().y();

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor("#495057"));
    painter->setRenderHint(QPainter::Antialiasing);

    QPolygon triangle;
    if (sortIndicatorOrder() == Qt::AscendingOrder) {
        triangle << QPoint(x, y + arrowSize)
                 << QPoint(x - arrowSize, y - arrowSize / 2)
                 << QPoint(x + arrowSize, y - arrowSize / 2);
    } else {
        triangle << QPoint(x, y - arrowSize)
                 << QPoint(x - arrowSize, y + arrowSize / 2)
                 << QPoint(x + arrowSize, y + arrowSize / 2);
    }
    painter->drawPolygon(triangle);
}
