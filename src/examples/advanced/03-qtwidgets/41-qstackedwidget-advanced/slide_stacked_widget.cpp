/// @file    slide_stacked_widget.cpp
/// @brief   Implementation of SlideStackedWidget animated page transitions.
///
/// Uses QPropertyAnimation on the "pos" property of stacked child widgets
/// to create horizontal slide-in/slide-out effects.
///
/// 对应教程：进阶层 03-QtWidgets/41-QStackedWidget 滑动切换动画实现。

#include "slide_stacked_widget.h"

#include <QParallelAnimationGroup>
#include <QWidget>

SlideStackedWidget::SlideStackedWidget(QWidget* parent)
    : QStackedWidget(parent)
{
}

void SlideStackedWidget::setCurrentIndex(int index)
{
    // Guard: ignore invalid index or request to stay on the same page
    if (index < 0 || index >= count() || index == currentIndex()) {
        return;
    }

    // Do not start a new animation if one is already in progress
    if (m_animating) {
        return;
    }

    slideToPage(currentIndex(), index);
}

void SlideStackedWidget::setAnimationDuration(int durationMs)
{
    m_animationDuration = durationMs;
}

int SlideStackedWidget::animationDuration() const
{
    return m_animationDuration;
}

void SlideStackedWidget::slideToPage(int fromIndex, int toIndex)
{
    QWidget* oldWidget = widget(fromIndex);
    QWidget* newWidget = widget(toIndex);

    if (!oldWidget || !newWidget) {
        return;
    }

    m_animating = true;

    const int width = this->width();
    const bool forward = isForward(fromIndex, toIndex);

    // Position the new widget off-screen on the appropriate side
    // For forward navigation: new page starts to the right
    // For backward navigation: new page starts to the left
    const int offsetX = forward ? width : -width;
    newWidget->move(offsetX, 0);
    newWidget->show();
    newWidget->raise();

    // Old widget slides out in the opposite direction
    const QPoint oldEndPos = forward ? QPoint(-width, 0) : QPoint(width, 0);

    // Use a parallel group so both animations run simultaneously
    auto* group = new QParallelAnimationGroup(this);

    // Animate old widget sliding out
    auto* oldAnim = new QPropertyAnimation(oldWidget, "pos", this);
    oldAnim->setDuration(m_animationDuration);
    oldAnim->setStartValue(oldWidget->pos());
    oldAnim->setEndValue(oldEndPos);
    oldAnim->setEasingCurve(QEasingCurve::InOutQuad);
    group->addAnimation(oldAnim);

    // Animate new widget sliding in from the side
    auto* newAnim = new QPropertyAnimation(newWidget, "pos", this);
    newAnim->setDuration(m_animationDuration);
    newAnim->setStartValue(QPoint(offsetX, 0));
    newAnim->setEndValue(QPoint(0, 0));
    newAnim->setEasingCurve(QEasingCurve::InOutQuad);
    group->addAnimation(newAnim);

    // When animation finishes, update the stacked widget's logical index
    connect(group, &QParallelAnimationGroup::finished, this,
            [this, toIndex, group]() {
                // QStackedWidget::setCurrentIndex (non-virtual call via scope resolution)
                // to update internal state without re-triggering our override
                QStackedWidget::setCurrentIndex(toIndex);
                m_animating = false;
                group->deleteLater();
            });

    group->start(QAbstractAnimation::DeleteWhenStopped);
}

bool SlideStackedWidget::isForward(int fromIndex, int toIndex) const
{
    return toIndex > fromIndex;
}
