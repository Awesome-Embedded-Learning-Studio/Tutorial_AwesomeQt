/// @file    tri_state_button.h
/// @brief   三态按钮控件声明——自定义 QAbstractButton 子类，演示 tri-state 状态机与手动绘制。
///
/// 对应教程：进阶层 03-QtWidgets/12-QAbstractButton 基类进阶。

#pragma once

#include <QAbstractButton>

/// 三态按钮，继承 QAbstractButton 并手动管理 Unchecked / PartiallyChecked / Checked 循环。
///
/// 核心知识点：
/// - 自定义 QAbstractButton 子类，重写 paintEvent 绘制三种视觉状态
/// - 重写 nextCheckState() 控制三态循环路径
/// - 重写 hitButton() 限定圆形点击区域
/// - setCheckable(true) + 手动 checkState 管理
class TriStateButton : public QAbstractButton
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化三态按钮。
    /// @param[in] text   按钮显示文本。
    /// @param[in] parent 父控件指针。
    explicit TriStateButton(const QString& text, QWidget* parent = nullptr);

    /// @brief 获取当前三态选中状态。
    /// @return 当前 Qt::CheckState 值。
    Qt::CheckState checkState() const;

    /// @brief 程序化设置三态选中状态，不触发用户点击信号。
    /// @param[in] state 目标状态。
    void setCheckState(Qt::CheckState state);

Q_SIGNALS:
    /// @brief 三态变化信号，比 toggled(bool) 更精确。
    /// @param state 新的 Qt::CheckState 枚举值。
    void stateChanged(int state);

protected:
    /// @brief 重写绘制——根据当前状态画不同颜色的圆形按钮。
    /// @param[in] event 绘制事件（未使用）。
    void paintEvent(QPaintEvent* event) override;

    /// @brief 重写下一状态逻辑——三态循环 Unchecked -> PartiallyChecked -> Checked。
    void nextCheckState() override;

    /// @brief 重写点击区域判定——只在圆形区域内响应鼠标。
    /// @param[in] pos 鼠标位置。
    /// @return 是否在圆形区域内。
    bool hitButton(const QPoint& pos) const override;

private:
    Qt::CheckState m_checkState;  // 三态选中状态
};
