/// @file    slide_stacked_widget.h
/// @brief   QStackedWidget subclass with animated slide transitions.
///
/// Demonstrates overriding setCurrentIndex to trigger QPropertyAnimation
/// on child widget geometry, producing a horizontal sliding effect when
/// switching pages.
///
/// 对应教程：进阶层 03-QtWidgets/41-QStackedWidget 滑动切换动画实现。

#pragma once

#include <QPropertyAnimation>
#include <QStackedWidget>

/// @brief A QStackedWidget that animates page transitions with a slide effect.
///
/// When setCurrentIndex() or setCurrentWidget() is called, the old page slides
/// out and the new page slides in using QPropertyAnimation on the widgets'
/// geometry property. The animation direction reverses automatically when
/// navigating backward.
class SlideStackedWidget : public QStackedWidget
{
    Q_OBJECT

public:
    /// @brief Constructs the slide-stacked widget.
    /// @param[in] parent Parent widget for Qt object-tree ownership.
    explicit SlideStackedWidget(QWidget* parent = nullptr);

    /// @brief Sets the current page index with a slide animation.
    /// @param[in] index Zero-based page index to switch to.
    /// @note QStackedWidget::setCurrentIndex is not virtual, so this method
    ///       shadows the base class version. Callers must use a SlideStackedWidget
    ///       pointer or reference (not a QStackedWidget pointer) to get animation.
    ///       If an animation is already running, it is stopped before starting a new one.
    void setCurrentIndex(int index);

    /// @brief Sets the animation duration in milliseconds.
    /// @param[in] durationMs Duration in milliseconds (default 300).
    void setAnimationDuration(int durationMs);

    /// @brief Returns the current animation duration in milliseconds.
    /// @return Duration in milliseconds.
    int animationDuration() const;

private:
    /// @brief Performs the actual slide animation between two widget indices.
    /// @param[in] fromIndex The page index currently visible.
    /// @param[in] toIndex   The page index to animate into view.
    /// @note Uses two parallel QPropertyAnimation instances: one to slide the
    ///       old widget out and another to slide the new widget in. Both operate
    ///       on the widget's "pos" property so only the position changes, not size.
    void slideToPage(int fromIndex, int toIndex);

    /// @brief Determines whether the navigation is forward (left-to-right).
    /// @param[in] fromIndex Current page index.
    /// @param[in] toIndex   Target page index.
    /// @return True if target index is greater than current (forward direction).
    bool isForward(int fromIndex, int toIndex) const;

    int m_animationDuration{300};  ///< Animation duration in milliseconds
    bool m_animating{false};       ///< True while a slide animation is active
};
