/// @file    custom_handle.h
/// @brief   Custom QSplitterHandle with painted grip dots.
///
/// Overrides paintEvent to draw a visually distinct handle with
/// colored grip dots, making the splitter handle easier to see
/// and drag.
///
/// 对应教程：进阶层 03-QtWidgets/42-QSplitter 自定义拖动手柄外观。

#pragma once

#include <QColor>
#include <QSplitterHandle>

/// @brief A splitter handle that paints custom grip dots instead of
///        the default platform appearance.
///
/// The handle draws a subtle background and a column/row of evenly
/// spaced circular grip dots in a configurable color.
class CustomHandle : public QSplitterHandle
{
    Q_OBJECT

public:
    /// @brief Constructs a custom splitter handle.
    /// @param[in] orientation Horizontal or vertical splitter orientation.
    /// @param[in] parent      The parent QSplitter that owns this handle.
    explicit CustomHandle(Qt::Orientation orientation, QSplitter* parent);

    /// @brief Sets the color used for the grip dots.
    /// @param[in] color The dot fill color.
    void setDotColor(const QColor& color);

    /// @brief Returns the current grip dot color.
    /// @return The dot color.
    QColor dotColor() const;

protected:
    /// @brief Paints the custom handle with grip dots.
    /// @param[in] event The paint event (unused beyond triggering repaint).
    /// @note The handle background is drawn slightly different from the default
    ///       to give visual feedback that this is a draggable area.
    void paintEvent(QPaintEvent* event) override;

private:
    QColor m_dotColor{QColor("#607D8B")};  ///< Color of the grip dots
};
