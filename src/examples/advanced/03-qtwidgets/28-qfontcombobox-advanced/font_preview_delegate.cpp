/// @file    font_preview_delegate.cpp
/// @brief   FontPreviewDelegate 类实现——下拉列表中用对应字体渲染预览文本。
///
/// 对应教程：进阶层 03-QtWidgets/28-QFontComboBox 进阶。

#include "font_preview_delegate.h"

#include <QPainter>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

FontPreviewDelegate::FontPreviewDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

// ─────────────────────────────────────────────────────────────────────────────
// paint——左侧字体名称 + 右侧预览文本
// ─────────────────────────────────────────────────────────────────────────────

void FontPreviewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    // 先让基类绘制背景和选中高亮
    QStyledItemDelegate::paint(painter, opt, index);

    painter->save();

    // 获取字体家族名称
    QString family = index.data(Qt::DisplayRole).toString();

    // 构造该字体家族的 QFont 用于预览渲染
    QFont previewFont(family);

    // --- 左侧：用该字体以粗体渲染字体名称 ---
    QFont nameFont = previewFont;
    nameFont.setBold(true);
    nameFont.setPointSize(opt.font.pointSize());

    QRect nameRect = opt.rect.adjusted(4, 0, -opt.rect.width() / 2, 0);
    painter->setFont(nameFont);
    painter->setPen(opt.palette.text().color());
    painter->drawText(nameRect, Qt::AlignVCenter | Qt::AlignLeft, family);

    // --- 右侧：用该字体渲染预览示例文本 ---
    QFont sampleFont = previewFont;
    sampleFont.setPointSize(opt.font.pointSize());

    QRect previewRect = opt.rect.adjusted(opt.rect.width() / 2, 0, -4, 0);
    painter->setFont(sampleFont);
    // 使用较浅的颜色区分预览文本和名称文本
    painter->setPen(opt.palette.color(QPalette::PlaceholderText));
    painter->drawText(previewRect, Qt::AlignVCenter | Qt::AlignRight,
                      QLatin1String(kPreviewText));

    painter->restore();
}

// ─────────────────────────────────────────────────────────────────────────────
// sizeHint——行高适当增加
// ─────────────────────────────────────────────────────────────────────────────

QSize FontPreviewDelegate::sizeHint(const QStyleOptionViewItem& option,
                                    const QModelIndex& index) const
{
    QSize hint = QStyledItemDelegate::sizeHint(option, index);
    hint.setHeight(qMax(hint.height(), 30));
    return hint;
}
