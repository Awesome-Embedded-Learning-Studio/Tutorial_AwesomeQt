/// @file    modal_strategy_demo.h
/// @brief   演示 Qt::WindowModal 与 Qt::ApplicationModal 模态范围差异。
///
/// 对应教程：进阶层 03-QtWidgets/06-对话框进阶。

#pragma once

#include <QWidget>

class QLabel;
class QPushButton;

/// 模态策略演示主窗口。
///
/// 提供两个按钮分别弹出 WindowModal 和 ApplicationModal 对话框，
/// 同时主窗口旁边有一个独立的工具窗口（无父子关系）。
/// WindowModal 只阻塞父窗口，工具窗口仍可操作；
/// ApplicationModal 则阻塞所有窗口。
class ModalStrategyDemo : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，创建主界面和独立工具窗口。
    /// @param[in] parent 父控件指针。
    explicit ModalStrategyDemo(QWidget* parent = nullptr);

private:
    /// @brief 弹出 WindowModal 对话框——只阻塞父窗口。
    void showWindowModal();

    /// @brief 弹出 ApplicationModal 对话框——阻塞所有窗口。
    void showApplicationModal();

    QWidget* m_toolWindow;  // 独立工具窗口（与主窗口无父子关系）
    QLabel* m_statusLabel;  // 状态提示标签
};
