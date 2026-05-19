/// @file    sync_text_area.cpp
/// @brief   SyncTextArea 类实现——自定义滚动文本区域与同步信号。
///
/// 对应教程：进阶层 03-QtWidgets/14-QAbstractScrollArea 基类进阶。

#include "sync_text_area.h"

#include <QApplication>
#include <QFontMetrics>
#include <QPaintEvent>
#include <QPainter>
#include <QScrollBar>
#include <QWheelEvent>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

SyncTextArea::SyncTextArea(QWidget* parent)
    : QAbstractScrollArea(parent)
    , m_lineHeight(0)
    , m_totalHeight(0)
{
    // 根据当前字体计算行高
    QFontMetrics fm(font());
    m_lineHeight = fm.height() + 4;  // 4 像素行间距

    // 隐藏水平滚动条——本示例只关注垂直滚动同步
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 加载示例文本
    setTextLines(generateSampleText());
}

// ─────────────────────────────────────────────────────────────────────────────
// 公有方法
// ─────────────────────────────────────────────────────────────────────────────

void SyncTextArea::setTextLines(const QStringList& lines)
{
    m_lines = lines;
    updateScrollBarRange();
    viewport()->update();
}

int SyncTextArea::scrollValue() const
{
    return verticalScrollBar()->value();
}

void SyncTextArea::setScrollValueSilently(int value)
{
    // blockSignals 防止 setValue 触发 valueChanged 导致循环同步
    verticalScrollBar()->blockSignals(true);
    verticalScrollBar()->setValue(value);
    verticalScrollBar()->blockSignals(false);
    viewport()->update();
}

// ─────────────────────────────────────────────────────────────────────────────
// 保护方法
// ─────────────────────────────────────────────────────────────────────────────

void SyncTextArea::scrollContentsBy(int dx, int dy)
{
    Q_UNUSED(dx)
    Q_UNUSED(dy)

    // 滚动时通知外部当前滚动位置，用于双区域同步
    emit scrollChanged(verticalScrollBar()->value());
}

void SyncTextArea::paintEvent(QPaintEvent* event)
{
    QPainter painter(viewport());

    // 内容坐标系：scrollY 表示内容被向上滚动的像素数
    const int scrollY = verticalScrollBar()->value();
    const int viewportWidth = viewport()->width();

    // 计算第一个可见行：scrollY / m_lineHeight 得到被完全滚出视口顶部的行数
    const int firstLine = scrollY / m_lineHeight;
    // 第一行在视口中的 Y 偏移：取模得到行内偏移
    const int yOffset = firstLine * m_lineHeight - scrollY;

    // 只绘制落在 event->rect() 范围内的行，避免不必要的绘制
    const int lastLine = qMin(firstLine + viewport()->height() / m_lineHeight + 2,
                              m_lines.size());

    painter.setPen(Qt::black);

    for (int i = firstLine; i < lastLine; ++i) {
        const int y = yOffset + (i - firstLine) * m_lineHeight;
        // 视口坐标系：y 是相对于视口左上角的位置
        painter.drawText(4, y + m_lineHeight - 5, m_lines.at(i));
    }

    // 绘制分隔线，增强视觉效果
    painter.setPen(QColor(220, 220, 220));
    for (int i = firstLine; i < lastLine; ++i) {
        const int y = yOffset + (i - firstLine) * m_lineHeight;
        painter.drawLine(0, y + m_lineHeight - 1, viewportWidth, y + m_lineHeight - 1);
    }
}

void SyncTextArea::wheelEvent(QWheelEvent* event)
{
    // 将滚轮事件转发给垂直滚动条
    QApplication::sendEvent(verticalScrollBar(), event);
}

// ─────────────────────────────────────────────────────────────────────────────
// 私有方法
// ─────────────────────────────────────────────────────────────────────────────

void SyncTextArea::updateScrollBarRange()
{
    m_totalHeight = m_lines.size() * m_lineHeight;

    // singleStep = 一行的距离，pageStep = 视口高度
    verticalScrollBar()->setSingleStep(m_lineHeight);
    verticalScrollBar()->setPageStep(viewport()->height());
    // maximum = 总高度 - 视口高度，确保滚动到底时最后一行可见
    verticalScrollBar()->setRange(0, qMax(0, m_totalHeight - viewport()->height()));
}

QStringList SyncTextArea::generateSampleText()
{
    QStringList lines;
    for (int i = 1; i <= 100; ++i) {
        lines << QStringLiteral("Line %1: QAbstractScrollArea provides a scrolling area with "
                                "on-demand scroll bars.")
                     .arg(i, 3, 10, QLatin1Char('0'));
    }
    return lines;
}
