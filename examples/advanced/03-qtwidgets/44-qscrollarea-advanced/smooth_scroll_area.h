/// @file    smooth_scroll_area.h
/// @brief   QScrollArea subclass with animated smooth scrolling.
///
/// Adds smoothScrollTo() that animates the vertical scrollbar from its
/// current position to a target value using QPropertyAnimation, providing
/// a fluid scrolling experience instead of abrupt jumps.
///
/// 对应教程：进阶层 03-QtWidgets/44-QScrollArea 平滑滚动动画。

#pragma once

#include <QScrollArea>

class QPropertyAnimation;

/// @brief A QScrollArea that supports smooth animated scrolling via
///        QPropertyAnimation on the vertical scrollbar value.
///
/// Call smoothScrollTo() with a target Y position to trigger a fluid
/// animated scroll. The animation duration scales with the scroll distance.
class SmoothScrollArea : public QScrollArea
{
    Q_OBJECT

public:
    /// @brief Constructs the smooth scroll area.
    /// @param[in] parent Parent widget for Qt ownership.
    explicit SmoothScrollArea(QWidget* parent = nullptr);

    /// @brief Smoothly scrolls to the given vertical position.
    /// @param[in] targetY Target vertical scroll position in pixels.
    /// @note Uses QPropertyAnimation on the scrollbar's "value" property.
    ///       Duration is proportional to distance, clamped between 200-800 ms.
    ///       If an animation is already running, it is stopped and restarted.
    void smoothScrollTo(int targetY);

    /// @brief Smoothly scrolls to the top of the content.
    void scrollToTop();

    /// @brief Smoothly scrolls to the bottom of the content.
    void scrollToBottom();

private:
    /// @brief Configures scrollbar step sizes for better UX.
    /// @note singleStep controls arrow-key / scroll-button increments;
    ///       pageStep controls the page-up/down / track-click increment.
    void configureScrollbar();

    QPropertyAnimation* m_scrollAnimation{nullptr};  ///< Current scroll animation (owned by this)
};
