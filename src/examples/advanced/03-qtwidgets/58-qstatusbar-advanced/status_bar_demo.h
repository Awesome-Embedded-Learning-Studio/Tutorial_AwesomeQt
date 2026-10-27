/// @file    status_bar_demo.h
/// @brief   演示 QStatusBar 多区域复杂状态显示。
///
/// 对应教程：进阶层 03-QtWidgets/58-QStatusBar 进阶。
/// 展示永久控件（坐标、进度条、模式指示器）与临时消息的协调。

#pragma once

#include <QMainWindow>

class QLabel;
class QProgressBar;
class QCheckBox;

/// @brief 主窗口，演示 QStatusBar 多区域状态显示。
class StatusBarDemo : public QMainWindow
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父控件指针，Qt 对象树自动管理生命周期。
    explicit StatusBarDemo(QWidget* parent = nullptr);

protected:
    /// @brief 鼠标移动事件，更新状态栏中的坐标显示。
    /// @param[in] event 鼠标事件对象。
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    /// @brief 构建界面与状态栏。
    void setupUI();

    /// @brief 模拟一个异步任务，更新进度条。
    void startSimulatedTask();

    QLabel* m_posLabel;             ///< 鼠标坐标标签（永久控件）
    QProgressBar* m_progressBar;    ///< 任务进度条（永久控件）
    QLabel* m_modeLabel;            ///< 模式指示器标签（永久控件）
    QCheckBox* m_editModeCheck;     ///< 编辑模式复选框（永久控件）
    int m_taskProgress;             ///< 当前模拟任务进度值
};
