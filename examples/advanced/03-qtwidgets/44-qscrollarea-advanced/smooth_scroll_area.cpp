/// @file    smooth_scroll_area.cpp
/// @brief   Implementation of SmoothScrollArea animated scrolling.
///
/// Animates the vertical scrollbar value using QPropertyAnimation
/// with an OutQuad easing curve for a natural deceleration feel.
///
/// 对应教程：进阶层 03-QtWidgets/44-QScrollArea 平滑滚动动画。

#include "smooth_scroll_area.h"

#include <QPropertyAnimation>
#include <QScrollBar>

SmoothScrollArea::SmoothScrollArea(QWidget* parent)
    : QScrollArea(parent)
{
    setWidgetResizable(true);
    configureScrollbar();
}

void SmoothScrollArea::smoothScrollTo(int targetY)
{
    QScrollBar* vBar = verticalScrollBar();
    if (!vBar) {
        return;
    }

    // Clamp target to valid scrollbar range
    targetY = std::max(vBar->minimum(), std::min(targetY, vBar->maximum()));

    const int currentValue = vBar->value();
    const int distance = std::abs(targetY - currentValue);

    // Skip animation if already at the target
    if (distance == 0) {
        return;
    }

    // Stop any running animation before starting a new one
    if (m_scrollAnimation && m_scrollAnimation->state() == QAbstractAnimation::Running) {
        m_scrollAnimation->stop();
    }

    // Duration scales with distance, clamped between 200ms and 800ms
    const int duration = std::max(200, std::min(800, distance * 2));

    // Use the existing animation object if possible to avoid repeated allocation
    if (!m_scrollAnimation) {
        m_scrollAnimation = new QPropertyAnimation(vBar, "value", this);
    } else {
        // Re-target to the current scrollbar in case it was replaced
        m_scrollAnimation->setTargetObject(vBar);
        m_scrollAnimation->setPropertyName("value");
    }

    m_scrollAnimation->setDuration(duration);
    m_scrollAnimation->setStartValue(currentValue);
    m_scrollAnimation->setEndValue(targetY);
    // OutQuad provides a natural deceleration feel
    m_scrollAnimation->setEasingCurve(QEasingCurve::OutQuad);
    m_scrollAnimation->start(QAbstractAnimation::DeleteWhenStopped);

    // After DeleteWhenStopped fires, the pointer becomes dangling
    // Reset to nullptr when the animation is destroyed
    connect(m_scrollAnimation, &QObject::destroyed, this,
            [this]() { m_scrollAnimation = nullptr; });
}

void SmoothScrollArea::scrollToTop()
{
    smoothScrollTo(0);
}

void SmoothScrollArea::scrollToBottom()
{
    QScrollBar* vBar = verticalScrollBar();
    if (vBar) {
        smoothScrollTo(vBar->maximum());
    }
}

void SmoothScrollArea::configureScrollbar()
{
    QScrollBar* vBar = verticalScrollBar();
    if (!vBar) {
        return;
    }

    // singleStep: increment when clicking scroll arrows or pressing arrow keys
    vBar->setSingleStep(30);
    // pageStep: increment when clicking the scrollbar track or pressing Page Up/Down
    vBar->setPageStep(300);
}
