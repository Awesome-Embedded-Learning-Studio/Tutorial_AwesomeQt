/// @file    custom_splitter.h
/// @brief   QSplitter subclass that creates CustomHandle instances.
///
/// Overrides createHandle() to return CustomHandle objects, enabling
/// fully custom splitter handle appearance while retaining all standard
/// QSplitter behavior.
///
/// 对应教程：进阶层 03-QtWidgets/42-QSplitter 自定义拖动手柄外观。

#pragma once

#include "custom_handle.h"

#include <QSplitter>

/// @brief A QSplitter that produces CustomHandle instances between widgets.
///
/// By overriding createHandle(), every handle inserted between child
/// widgets is a CustomHandle with painted grip dots instead of the
/// default platform handle.
class CustomSplitter : public QSplitter
{
    Q_OBJECT

public:
    /// @brief Constructs the custom splitter.
    /// @param[in] orientation Initial layout direction (horizontal/vertical).
    /// @param[in] parent      Parent widget for Qt ownership.
    explicit CustomSplitter(Qt::Orientation orientation = Qt::Horizontal,
                            QWidget* parent = nullptr);

protected:
    /// @brief Factory method called by QSplitter to create handle widgets.
    /// @return A new CustomHandle owned by this splitter.
    /// @note QSplitter calls this internally each time it needs a handle
    ///       between two child widgets. Overriding is the official way to
    ///       customize handle appearance.
    QSplitterHandle* createHandle() override;
};
