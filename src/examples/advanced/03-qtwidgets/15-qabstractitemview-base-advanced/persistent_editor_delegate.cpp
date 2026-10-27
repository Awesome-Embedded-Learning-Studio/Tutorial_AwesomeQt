/// @file    persistent_editor_delegate.cpp
/// @brief   PersistentEditorDelegate 类实现——持久进度条编辑器委托。
///
/// 对应教程：进阶层 03-QtWidgets/15-QAbstractItemView 基类进阶。

#include "persistent_editor_delegate.h"

#include <QAbstractItemModel>
#include <QProgressBar>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

PersistentEditorDelegate::PersistentEditorDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

// ─────────────────────────────────────────────────────────────────────────────
// createEditor：创建进度条作为持久编辑器
// ─────────────────────────────────────────────────────────────────────────────

QWidget* PersistentEditorDelegate::createEditor(QWidget* parent,
                                                const QStyleOptionViewItem& option,
                                                const QModelIndex& index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    // 创建进度条并设置范围，文本显示百分比
    auto* bar = new QProgressBar(parent);
    bar->setRange(0, 100);
    bar->setTextVisible(true);
    bar->setFormat(QStringLiteral("%p%"));

    return bar;
}

// ─────────────────────────────────────────────────────────────────────────────
// setEditorData：模型 → 编辑器
// ─────────────────────────────────────────────────────────────────────────────

void PersistentEditorDelegate::setEditorData(QWidget* editor,
                                             const QModelIndex& index) const
{
    auto* bar = qobject_cast<QProgressBar*>(editor);
    if (!bar) {
        return;
    }

    // 从模型的 DisplayRole 读取进度值
    bool ok = false;
    const int value = index.data(Qt::DisplayRole).toInt(&ok);
    if (ok) {
        bar->setValue(qBound(0, value, 100));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// setModelData：编辑器 → 模型
// ─────────────────────────────────────────────────────────────────────────────

void PersistentEditorDelegate::setModelData(QWidget* editor,
                                            QAbstractItemModel* model,
                                            const QModelIndex& index) const
{
    auto* bar = qobject_cast<QProgressBar*>(editor);
    if (!bar) {
        return;
    }

    // 将进度条当前值写回模型
    model->setData(index, bar->value(), Qt::EditRole);
}

// ─────────────────────────────────────────────────────────────────────────────
// updateEditorGeometry：让编辑器填满单元格
// ─────────────────────────────────────────────────────────────────────────────

void PersistentEditorDelegate::updateEditorGeometry(QWidget* editor,
                                                     const QStyleOptionViewItem& option,
                                                     const QModelIndex& index) const
{
    Q_UNUSED(index)

    // 使用 option.rect 让进度条精确覆盖单元格区域
    editor->setGeometry(option.rect);
}
