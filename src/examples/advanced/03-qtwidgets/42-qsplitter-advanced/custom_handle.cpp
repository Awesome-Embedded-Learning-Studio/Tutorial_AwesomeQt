/// @file    custom_handle.cpp
/// @brief   Implementation of CustomHandle paint logic.
///
/// Draws a row or column of evenly spaced circular dots on the handle
/// surface, providing a visual cue that the handle is draggable.
///
/// 对应教程：进阶层 03-QtWidgets/42-QSplitter 自定义拖动手柄外观。

#include "custom_handle.h"

#include <QPainter>
#include <QPaintEvent>

CustomHandle::CustomHandle(Qt::Orientation orientation, QSplitter* parent)
    : QSplitterHandle(orientation, parent)
{
}

void CustomHandle::setDotColor(const QColor& color)
{
    m_dotColor = color;
    update();
}

QColor CustomHandle::dotColor() const
{
    return m_dotColor;
}

void CustomHandle::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QRect rect = this->rect();

    // Fill the handle background with a subtle color
    painter.fillRect(rect, QColor("#ECEFF1"));

    // Draw a thin border line for visual definition
    painter.setPen(QPen(QColor("#B0BEC5"), 1));
    painter.drawRect(rect.adjusted(0, 0, -1, -1));

    // Draw grip dots centered in the handle
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_dotColor);

    const int dotRadius = 3;
    const int dotSpacing = 8;
    const int dotCount = 4;

    if (orientation() == Qt::Horizontal) {
        // Handle is a narrow vertical strip; dots arranged vertically
        const int centerX = rect.width() / 2;
        const int totalHeight = (dotCount - 1) * dotSpacing;
        const int startY = (rect.height() - totalHeight) / 2;

        for (int i = 0; i < dotCount; ++i) {
            int y = startY + i * dotSpacing;
            painter.drawEllipse(QPoint(centerX, y), dotRadius, dotRadius);
        }
    } else {
        // Handle is a narrow horizontal strip; dots arranged horizontally
        const int centerY = rect.height() / 2;
        const int totalWidth = (dotCount - 1) * dotSpacing;
        const int startX = (rect.width() - totalWidth) / 2;

        for (int i = 0; i < dotCount; ++i) {
            int x = startX + i * dotSpacing;
            painter.drawEllipse(QPoint(x, centerY), dotRadius, dotRadius);
        }
    }
}
