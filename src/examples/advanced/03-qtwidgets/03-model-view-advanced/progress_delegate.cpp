/// @file    progress_delegate.cpp
/// @brief   ProgressDelegate 类实现——在进度列中绘制 QStyleOptionProgressBar。
///
/// 对应教程：进阶层 03-QtWidgets/03-Model/View 进阶。

#include "progress_delegate.h"

#include <QApplication>
#include <QModelIndex>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionProgressBar>
#include <QStyleOptionViewItem>

// ─────────────────────────────────────────────────────────────────────────────
// 常量
// ─────────────────────────────────────────────────────────────────────────────

namespace
{
    constexpr int kProgressColumn = 2;       // 进度列索引（第 3 列）
    constexpr int kBarPadding = 4;           // 进度条与单元格边界的内边距
}

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

ProgressDelegate::ProgressDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

// ─────────────────────────────────────────────────────────────────────────────
// paint 重写——在进度列中绘制进度条
// ─────────────────────────────────────────────────────────────────────────────

void ProgressDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                             const QModelIndex& index) const
{
    // 非进度列回退到默认绘制
    if (index.column() != kProgressColumn) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    painter->save();

    // 必须先用 drawControl(CE_ItemViewItem) 画背景
    // 否则选中行的蓝色高亮不会出现
    auto* style = option.widget ? option.widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &option, painter, option.widget);

    // 从 model 获取进度值（使用 EditRole 获取原始 int 值）
    const int progress = index.data(Qt::EditRole).toInt();

    // 构建 QStyleOptionProgressBar 并让 style 绘制原生进度条
    QStyleOptionProgressBar bar;
    bar.rect = option.rect.adjusted(kBarPadding, kBarPadding, -kBarPadding, -kBarPadding);
    bar.minimum = 0;
    bar.maximum = 100;
    bar.progress = progress;
    bar.text = QStringLiteral("%1%").arg(progress);
    bar.textVisible = true;
    bar.state = option.state;

    style->drawControl(QStyle::CE_ProgressBar, &bar, painter, option.widget);

    painter->restore();
}
