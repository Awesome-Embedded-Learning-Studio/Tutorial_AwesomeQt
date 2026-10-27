/// @file    color_dialog_demo.h
/// @brief   演示 QColorDialog 集成到自定义颜色选择器面板。
///
/// 对应教程：进阶层 03-QtWidgets/64-QColorDialog 进阶。
/// 展示预定义色板 + QColorDialog 自定义颜色、hex/RGB 显示、alpha 支持。

#pragma once

#include <QWidget>

class QLabel;
class QPushButton;

/// @brief 自定义颜色选择器面板，集成 QColorDialog。
class ColorDialogDemo : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父控件指针，Qt 对象树自动管理生命周期。
    explicit ColorDialogDemo(QWidget* parent = nullptr);

private slots:
    /// @brief 打开 QColorDialog 选择颜色（带 alpha 通道）。
    void pickColor();

    /// @brief 从预定义色板中选择颜色。
    void selectPresetColor();

private:
    /// @brief 构建界面。
    void setupUI();

    /// @brief 更新颜色预览和信息显示。
    /// @param[in] color 新选中的颜色。
    void updateColorDisplay(const QColor& color);

    QLabel* m_previewLabel;         ///< 颜色预览色块
    QLabel* m_infoLabel;            ///< 颜色 hex/RGB 信息
    QColor m_currentColor;          ///< 当前选中的颜色
};
