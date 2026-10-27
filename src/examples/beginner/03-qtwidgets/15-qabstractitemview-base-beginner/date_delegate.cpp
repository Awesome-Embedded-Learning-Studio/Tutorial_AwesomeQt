// QtWidgets 入门示例 15: QAbstractItemView 视图基类
// DateDelegate: 日期列的自定义委托，双击弹出日历选择器

#include "date_delegate.h"

#include <QDateEdit>

QWidget *DateDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    auto *editor = new QDateEdit(parent);
    editor->setCalendarPopup(true);
    editor->setDisplayFormat("yyyy-MM-dd");
    return editor;
}

void DateDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QDate date = index.data(Qt::EditRole).toDate();
    auto *dateEdit = qobject_cast<QDateEdit *>(editor);
    if (dateEdit) {
        dateEdit->setDate(date.isValid() ? date : QDate::currentDate());
    }
}

void DateDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                 const QModelIndex &index) const
{
    auto *dateEdit = qobject_cast<QDateEdit *>(editor);
    if (dateEdit) {
        model->setData(index, dateEdit->date(), Qt::EditRole);
    }
}

void DateDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}
