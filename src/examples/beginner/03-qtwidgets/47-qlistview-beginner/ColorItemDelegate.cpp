// QtWidgets 入门示例 47: QListView Model 驱动列表视图
// 演示：与 QStringListModel 配合
//       setViewMode 列表/图标模式
//       setSpacing / setGridSize 图标布局
//       自定义 ItemDelegate 改变显示样式

#include "ColorItemDelegate.h"

#include <QApplication>
#include <QColor>
#include <QPainter>
#include <QStyleOptionViewItem>

// ============================================================================
// ColorItemDelegate: 自定义 delegate，在条目上绘制颜色色块
// ============================================================================
void ColorItemDelegate::paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    // 先绘制默认的选中背景和焦点框
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, QColor("#E3F2FD"));
    }

    QString colorName = index.data(Qt::DisplayRole).toString();
    QColor color(colorName);

    // 检查 QListView 当前是否处于图标模式
    // 通过 iconSize 来判断：图标模式用大图标
    bool isIconMode = (option.decorationSize.width() > 40);

    if (isIconMode) {
        paintIconMode(painter, option, colorName, color);
    } else {
        paintListMode(painter, option, colorName, color);
    }
}

QSize ColorItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    QSize base = QStyledItemDelegate::sizeHint(option, index);
    // 列表模式额外加高一点，图标模式由 gridSize 控制
    bool isIconMode = (option.decorationSize.width() > 40);
    if (!isIconMode) {
        return QSize(base.width(), base.height() + 4);
    }
    return base;
}

void ColorItemDelegate::paintListMode(QPainter *painter,
                                      const QStyleOptionViewItem &option,
                                      const QString &colorName,
                                      const QColor &color) const
{
    painter->save();

    // 绘制颜色名称文本
    QRect textRect = option.rect.adjusted(8, 0, -30, 0);
    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft,
                      colorName);

    // 在右侧画一个圆角色块
    if (color.isValid()) {
        int tagSize = 14;
        int margin = 8;
        QRect tagRect(
            option.rect.right() - tagSize - margin,
            option.rect.top()
                + (option.rect.height() - tagSize) / 2,
            tagSize, tagSize);

        painter->setRenderHint(QPainter::Antialiasing);
        painter->setBrush(color);
        painter->setPen(QColor(0, 0, 0, 40));
        painter->drawRoundedRect(tagRect, 3, 3);
    }

    painter->restore();
}

void ColorItemDelegate::paintIconMode(QPainter *painter,
                                      const QStyleOptionViewItem &option,
                                      const QString &colorName,
                                      const QColor &color) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    int circleSize = qMin(option.rect.width(),
                          option.rect.height()) / 2 - 8;
    circleSize = qMax(circleSize, 16);

    // 画颜色圆
    if (color.isValid()) {
        QRect circleRect(
            option.rect.center().x() - circleSize,
            option.rect.top() + 10,
            circleSize * 2,
            circleSize * 2);

        // 画阴影
        painter->setBrush(QColor(0, 0, 0, 30));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(circleRect.adjusted(
            2, 2, 2, 2));

        // 画颜色圆
        painter->setBrush(color);
        painter->setPen(QColor(255, 255, 255, 80));
        painter->drawEllipse(circleRect);
    }

    // 画文字
    QRect textRect(
        option.rect.left(),
        option.rect.top() + circleSize * 2 + 16,
        option.rect.width(),
        20);
    QFont font = painter->font();
    font.setPointSize(9);
    painter->setFont(font);
    painter->setPen(QColor("#333"));
    painter->drawText(textRect,
                      Qt::AlignTop | Qt::AlignHCenter,
                      colorName);

    painter->restore();
}
