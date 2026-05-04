// QtWidgets 入门示例 47: QListView Model 驱动列表视图
// 演示：与 QStringListModel 配合
//       setViewMode 列表/图标模式
//       setSpacing / setGridSize 图标布局
//       自定义 ItemDelegate 改变显示样式

#ifndef COLORITEMDELEGATE_H
#define COLORITEMDELEGATE_H

#include <QStyledItemDelegate>

// ============================================================================
// ColorItemDelegate: 自定义 delegate，在条目上绘制颜色色块
// ============================================================================
class ColorItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

private:
    /// @brief 列表模式绘制：文本 + 右侧小色块
    void paintListMode(QPainter *painter,
                       const QStyleOptionViewItem &option,
                       const QString &colorName,
                       const QColor &color) const;

    /// @brief 图标模式绘制：中心大色圆 + 下方文字
    void paintIconMode(QPainter *painter,
                       const QStyleOptionViewItem &option,
                       const QString &colorName,
                       const QColor &color) const;
};

#endif // COLORITEMDELEGATE_H
