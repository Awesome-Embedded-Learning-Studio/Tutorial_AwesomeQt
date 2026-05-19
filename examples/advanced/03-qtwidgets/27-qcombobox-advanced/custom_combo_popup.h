/// @file    custom_combo_popup.h
/// @brief   覆写 showPopup 的自定义 QComboBox，防止弹窗被屏幕边缘截断。
///
/// 对应教程：进阶层 03-QtWidgets/27-QComboBox 进阶。

#pragma once

#include <QComboBox>

/// 自定义 QComboBox，覆写 showPopup 实现弹窗屏幕边缘防截断定位。
///
/// 默认 QComboBox::showPopup 只做垂直方向的简单边界检测，
/// 水平方向的处理非常有限。本类在父类实现之后手动检查弹窗
/// geometry 是否超出屏幕可用区域，超出则调整位置。
class CustomComboPopup : public QComboBox
{
    Q_OBJECT

public:
    /// @brief 构造函数，设置 SizeAdjustPolicy 为 AdjustToContents。
    /// @param[in] parent 父控件指针。
    explicit CustomComboPopup(QWidget* parent = nullptr);

protected:
    /// @brief 覆写 showPopup，在父类弹出后修正弹窗位置防止截断。
    /// @note 先调用 QComboBox::showPopup 让默认实现创建弹窗，
    ///       再通过 findChild 找到弹窗 QFrame 并调整 geometry。
    void showPopup() override;

private:
    /// @brief 将弹窗约束在当前屏幕可用区域内。
    /// @param[in,out] popup 弹窗容器的原始 geometry，会被修正。
    void clampToScreen(QRect& popup) const;
};
