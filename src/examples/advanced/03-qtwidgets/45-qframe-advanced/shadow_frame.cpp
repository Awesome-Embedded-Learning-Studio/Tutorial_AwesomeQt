/// @file    shadow_frame.cpp
/// @brief   Implementation of ShadowFrame custom painting.
///
/// Draws a rounded rectangle with a manually painted drop shadow using
/// QPainterPath, giving full control over shadow appearance without
/// QGraphicsDropShadowEffect.
///
/// 对应教程：进阶层 03-QtWidgets/45-QFrame 自定义圆角阴影边框绘制。

#include "shadow_frame.h"

#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>

#include <cmath>

ShadowFrame::ShadowFrame(QWidget* parent)
    : QFrame(parent)
{
    // QFrame must have no default frame shape/style; we paint everything ourselves
    setFrameShape(QFrame::NoFrame);
    setAttribute(Qt::WA_StyledBackground, false);
    updateContentMargins();
}

int ShadowFrame::borderRadius() const
{
    return m_borderRadius;
}

void ShadowFrame::setBorderRadius(int radius)
{
    m_borderRadius = radius;
    update();
}

int ShadowFrame::shadowBlur() const
{
    return m_shadowBlur;
}

void ShadowFrame::setShadowBlur(int blur)
{
    m_shadowBlur = blur;
    update();
}

int ShadowFrame::shadowOffsetX() const
{
    return m_shadowOffsetX;
}

void ShadowFrame::setShadowOffsetX(int offsetX)
{
    m_shadowOffsetX = offsetX;
    update();
}

int ShadowFrame::shadowOffsetY() const
{
    return m_shadowOffsetY;
}

void ShadowFrame::setShadowOffsetY(int offsetY)
{
    m_shadowOffsetY = offsetY;
    update();
}

QColor ShadowFrame::shadowColor() const
{
    return m_shadowColor;
}

void ShadowFrame::setShadowColor(const QColor& color)
{
    m_shadowColor = color;
    update();
}

QColor ShadowFrame::backgroundColor() const
{
    return m_backgroundColor;
}

void ShadowFrame::setBackgroundColor(const QColor& color)
{
    m_backgroundColor = color;
    update();
}

QColor ShadowFrame::borderColor() const
{
    return m_borderColor;
}

void ShadowFrame::setBorderColor(const QColor& color)
{
    m_borderColor = color;
    update();
}

QRect ShadowFrame::contentRect() const
{
    // Reserve space for the shadow on all sides so it is not clipped.
    // The margin accounts for both the blur spread and the offset direction.
    const int margin = m_shadowBlur + std::abs(m_shadowOffsetX)
                       + std::abs(m_shadowOffsetY) + 2;
    return rect().adjusted(margin, margin, -margin, -margin);
}

void ShadowFrame::updateContentMargins()
{
    // @note 子控件布局必须与阴影预留边距对齐，否则文字会画进阴影区域
    const int margin = m_shadowBlur + std::abs(m_shadowOffsetX)
                       + std::abs(m_shadowOffsetY) + 2;
    setContentsMargins(margin, margin, margin, margin);
}

void ShadowFrame::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QRect content = contentRect();
    if (content.width() <= 0 || content.height() <= 0) {
        return;
    }

    // --- Draw the drop shadow ---
    // Multiple passes with increasing alpha create a soft shadow effect
    // that approximates a gaussian blur without requiring a blur effect.
    const int passes = 4;
    for (int i = passes; i >= 1; --i) {
        // Each pass expands the shadow rect and reduces alpha
        const int expand = static_cast<int>(
            m_shadowBlur * (static_cast<double>(i) / passes));
        const int alphaStep = m_shadowColor.alpha() / passes;

        QColor passColor = m_shadowColor;
        passColor.setAlpha(alphaStep);

        QRectF shadowRect = QRectF(content)
            .translated(m_shadowOffsetX, m_shadowOffsetY)
            .adjusted(-expand, -expand, expand, expand);

        QPainterPath shadowPath;
        shadowPath.addRoundedRect(shadowRect, m_borderRadius + expand,
                                  m_borderRadius + expand);

        painter.fillPath(shadowPath, passColor);
    }

    // --- Draw the main background with rounded corners ---
    QPainterPath contentPath;
    contentPath.addRoundedRect(content, m_borderRadius, m_borderRadius);
    painter.fillPath(contentPath, m_backgroundColor);

    // --- Draw the border stroke ---
    painter.setPen(QPen(m_borderColor, 1));
    painter.drawPath(contentPath);
}
