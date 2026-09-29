/// @file    line_number_sidebar.cpp
/// @brief   LineNumberSidebar 类实现——行号侧边栏绘制。
///
/// 对应教程：进阶层 03-QtWidgets/24-QPlainTextEdit 进阶。

#include "line_number_editor.h"
#include "line_number_sidebar.h"

#include <QFontMetrics>
#include <QPaintEvent>
#include <QPainter>
#include <QTextBlock>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

LineNumberSidebar::LineNumberSidebar(LineNumberEditor* editor)
    : QWidget(editor)
    , m_editor(editor)
{
    // 行号区域不接收鼠标事件，让事件穿透到编辑器
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

// ─────────────────────────────────────────────────────────────────────────────
// 公有方法
// ─────────────────────────────────────────────────────────────────────────────

int LineNumberSidebar::calculateWidth() const
{
    // 根据 blockCount 的位数计算宽度，至少显示一位数
    const int digits = qMax(1, QString::number(m_editor->blockCount()).size());

    // 字体度量 + 左右各 4 像素边距
    const int charWidth = fontMetrics().horizontalAdvance(QLatin1Char('9'));
    return charWidth * digits + 8;
}

LineNumberEditor* LineNumberSidebar::editor() const
{
    return m_editor;
}

// ─────────────────────────────────────────────────────────────────────────────
// paintEvent
// ─────────────────────────────────────────────────────────────────────────────

void LineNumberSidebar::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);

    // 行号区域背景色
    painter.fillRect(event->rect(), QColor(240, 240, 240));

    // 从编辑器获取第一个可见 block 及其几何信息
    QTextBlock block = m_editor->firstVisibleBlock();
    int blockNumber = block.blockNumber();

    // blockBoundingRect 返回文档坐标系，需要减去 contentOffset 转为视口坐标
    qreal blockTop = m_editor->blockBoundingRect(block).top()
                     - m_editor->contentOffset().y();
    qreal blockHeight = m_editor->blockBoundingRect(block).height();

    painter.setPen(QColor(120, 120, 120));

    // 遍历所有可见 block，绘制行号
    while (block.isValid()) {
        // 超出重绘区域底部则停止
        if (blockTop > event->rect().bottom()) {
            break;
        }

        // 只绘制在重绘区域内的行号
        if (block.isVisible() && (blockTop + blockHeight) >= event->rect().top()) {
            const int top = static_cast<int>(blockTop);
            const int height = static_cast<int>(blockHeight);

            // 行号从 1 开始（blockNumber 从 0 开始）
            const QString number = QString::number(blockNumber + 1);

            // 右对齐、垂直居中绘制行号
            painter.drawText(0, top, width() - 4, height,
                             Qt::AlignRight | Qt::AlignVCenter, number);
        }

        // 移动到下一个 block
        block = block.next();
        blockTop += blockHeight;
        blockHeight = m_editor->blockBoundingRect(block).height();
        ++blockNumber;
    }
}
