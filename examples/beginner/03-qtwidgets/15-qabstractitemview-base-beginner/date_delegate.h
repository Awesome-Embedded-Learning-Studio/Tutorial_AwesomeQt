// QtWidgets 入门示例 15: QAbstractItemView 视图基类
// DateDelegate: 日期列的自定义委托，双击弹出日历选择器

#ifndef DATE_DELEGATE_H
#define DATE_DELEGATE_H

#include <QStyledItemDelegate>

class DateDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    using QStyledItemDelegate::QStyledItemDelegate;

    /// @brief 创建 QDateEdit 编辑器
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                           const QModelIndex &index) const override;

    /// @brief 把模型数据设到编辑器上
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    /// @brief 把编辑器的值写回模型
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                       const QModelIndex &index) const override;

    /// @brief 调整编辑器位置和大小
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                               const QModelIndex &index) const override;
};

#endif // DATE_DELEGATE_H
