/// @file    state_machine_demo.h
/// @brief   演示 QStateMachine 驱动动画：多状态切换、QParallelAnimationGroup
///          并行组合、QPropertyAnimation 属性动画（geometry/pos/windowOpacity）。
///
/// 对应教程：进阶层 03-QtWidgets/09-动画进阶。

#pragma once

#include <QWidget>

class QLabel;
class QPushButton;

/// 状态机动画演示控件。
///
/// 展示四个核心知识点：
/// - QStateMachine + QState 建模多状态界面（Normal / Focused / Animated / Reset）
/// - QPropertyAnimation 驱动 geometry、pos、windowOpacity 属性
/// - QParallelAnimationGroup 同时运行多个动画
/// - addTransition 信号驱动状态切换，addAnimation 关联过渡动画
class StateMachineDemo : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，创建界面、构建状态机和动画。
    /// @param[in] parent 父控件指针。
    explicit StateMachineDemo(QWidget* parent = nullptr);

private:
    /// @brief 创建界面控件（目标方块、操作按钮、状态标签）。
    void setupUI();

    /// @brief 构建 QStateMachine 和所有状态/过渡/动画。
    void buildStateMachine();

private:
    QLabel* m_target;              // 被动画驱动的目标方块
    QPushButton* m_focusBtn;      // 触发"聚焦"状态
    QPushButton* m_animateBtn;    // 触发"动画"状态
    QPushButton* m_resetBtn;      // 触发"重置"状态
    QLabel* m_statusLabel;        // 显示当前状态名称
};
