/// @file    combo_item_delegate.cpp
/// @brief   ComboItemDelegate 类实现——弹窗行中绘制名称、编码与地区三列。
///
/// 对应教程：进阶层 03-QtWidgets/27-QComboBox 进阶。

#include "combo_item_delegate.h"

#include <QPainter>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

ComboItemDelegate::ComboItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

// ─────────────────────────────────────────────────────────────────────────────
// paint——自定义弹窗行绘制
// ─────────────────────────────────────────────────────────────────────────────

void ComboItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                              const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    // 先让基类绘制背景和选中高亮
    QStyledItemDelegate::paint(painter, opt, index);

    painter->save();

    // 从 Model 的三列中读取数据
    // 本 delegate 安装在列 0 的 QComboBox 上，通过 sibling() 访问同行的其他列
    QString name = index.data(Qt::DisplayRole).toString();
    QString code = index.sibling(index.row(), 1).data(Qt::DisplayRole).toString();
    QString region = index.sibling(index.row(), 2).data(Qt::DisplayRole).toString();

    // 将行区域水平三等分
    int third = opt.rect.width() / 3;
    QRect nameRect(opt.rect.left(), opt.rect.top(), third, opt.rect.height());
    QRect codeRect(opt.rect.left() + third, opt.rect.top(), third, opt.rect.height());
    QRect regionRect(opt.rect.left() + 2 * third, opt.rect.top(), third, opt.rect.height());

    // 名称列：粗体左对齐
    QFont boldFont = opt.font;
    boldFont.setBold(true);
    painter->setFont(boldFont);
    painter->setPen(opt.palette.text().color());
    painter->drawText(nameRect, Qt::AlignVCenter | Qt::AlignLeft, name);

    // 编码列：普通字体居中
    painter->setFont(opt.font);
    painter->drawText(codeRect, Qt::AlignVCenter | Qt::AlignCenter, code);

    // 地区列：普通字体右对齐
    painter->drawText(regionRect, Qt::AlignVCenter | Qt::AlignRight, region);

    painter->restore();
}

// ─────────────────────────────────────────────────────────────────────────────
// sizeHint——行高加大以容纳多列内容
// ─────────────────────────────────────────────────────────────────────────────

QSize ComboItemDelegate::sizeHint(const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const
{
    // 在默认 sizeHint 基础上增加高度，给三列布局留出垂直空间
    QSize hint = QStyledItemDelegate::sizeHint(option, index);
    hint.setHeight(qMax(hint.height(), 32));
    return hint;
}
