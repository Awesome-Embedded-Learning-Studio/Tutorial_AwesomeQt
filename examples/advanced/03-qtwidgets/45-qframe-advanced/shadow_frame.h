/// @file    shadow_frame.h
/// @brief   QFrame subclass with configurable rounded corners and drop shadow.
///
/// Overrides paintEvent to draw a rounded rectangle with a soft drop shadow
/// using QPainterPath, allowing full control over border radius, shadow
/// offset, and shadow blur radius.
///
/// 对应教程：进阶层 03-QtWidgets/45-QFrame 自定义圆角阴影边框绘制。

#pragma once

#include <QColor>
#include <QFrame>

/// @brief A QFrame that paints a rounded-rectangle background with a
///        soft drop shadow effect.
///
/// All visual parameters (border radius, shadow color, shadow offset,
/// shadow blur) are configurable via setter methods. The frame automatically
/// reserves padding for the shadow so it is not clipped.
class ShadowFrame : public QFrame
{
    Q_OBJECT

    /// @brief Property for border radius (used by QSS if desired).
    Q_PROPERTY(int borderRadius READ borderRadius WRITE setBorderRadius)
    /// @brief Property for shadow blur radius.
    Q_PROPERTY(int shadowBlur READ shadowBlur WRITE setShadowBlur)
    /// @brief Property for shadow X offset.
    Q_PROPERTY(int shadowOffsetX READ shadowOffsetX WRITE setShadowOffsetX)
    /// @brief Property for shadow Y offset.
    Q_PROPERTY(int shadowOffsetY READ shadowOffsetY WRITE setShadowOffsetY)

public:
    /// @brief Constructs the shadow frame.
    /// @param[in] parent Parent widget for Qt ownership.
    explicit ShadowFrame(QWidget* parent = nullptr);

    // --- Configuration accessors ---

    /// @brief Returns the current border corner radius.
    /// @return Border radius in pixels.
    int borderRadius() const;

    /// @brief Sets the border corner radius.
    /// @param[in] radius Corner radius in pixels.
    void setBorderRadius(int radius);

    /// @brief Returns the shadow blur radius.
    /// @return Blur radius in pixels.
    int shadowBlur() const;

    /// @brief Sets the shadow blur radius (controls softness).
    /// @param[in] blur Blur radius in pixels.
    void setShadowBlur(int blur);

    /// @brief Returns the shadow horizontal offset.
    /// @return X offset in pixels.
    int shadowOffsetX() const;

    /// @brief Sets the shadow horizontal offset.
    /// @param[in] offsetX X offset in pixels.
    void setShadowOffsetX(int offsetX);

    /// @brief Returns the shadow vertical offset.
    /// @return Y offset in pixels.
    int shadowOffsetY() const;

    /// @brief Sets the shadow vertical offset.
    /// @param[in] offsetY Y offset in pixels.
    void setShadowOffsetY(int offsetY);

    /// @brief Returns the shadow color.
    /// @return Shadow color.
    QColor shadowColor() const;

    /// @brief Sets the shadow color.
    /// @param[in] color Shadow color (use alpha for transparency).
    void setShadowColor(const QColor& color);

    /// @brief Returns the background color.
    /// @return Background fill color.
    QColor backgroundColor() const;

    /// @brief Sets the background fill color.
    /// @param[in] color Background color.
    void setBackgroundColor(const QColor& color);

    /// @brief Returns the border color.
    /// @return Border stroke color.
    QColor borderColor() const;

    /// @brief Sets the border stroke color.
    /// @param[in] color Border color.
    void setBorderColor(const QColor& color);

protected:
    /// @brief Paints the rounded rectangle with drop shadow.
    /// @param[in] event The paint event.
    /// @note The shadow is drawn first as a filled rounded rect at the offset
    ///       position, then the main background rect is drawn on top. This avoids
    ///       needing QGraphicsDropShadowEffect, which can cause performance issues.
    void paintEvent(QPaintEvent* event) override;

private:
    /// @brief Returns the content rectangle, inset to leave room for the shadow.
    /// @return The rectangle available for the main frame content.
    /// @note The inset is calculated from the shadow blur, offset, and a small
    ///       margin so the shadow is never clipped at the widget edges.
    QRect contentRect() const;

    /// @brief Updates the layout content margins to match the shadow padding.
    /// @note Without this, child widgets overlap the shadow area and get clipped.
    void updateContentMargins();

    int m_borderRadius{12};          ///< Rounded corner radius in pixels
    int m_shadowBlur{15};            ///< Shadow blur/softness radius
    int m_shadowOffsetX{4};          ///< Horizontal shadow offset
    int m_shadowOffsetY{4};          ///< Vertical shadow offset
    QColor m_shadowColor{0, 0, 0, 60};   ///< Shadow color with alpha
    QColor m_backgroundColor{255, 255, 255, 255};  ///< Frame background color
    QColor m_borderColor{200, 200, 200, 255};       ///< Border stroke color
};
