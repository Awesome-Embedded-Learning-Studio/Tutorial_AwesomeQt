/// @file    progress_dialog_demo.h
/// @brief   演示 QProgressDialog 与异步任务的取消与进度精确同步。
///
/// 对应教程：进阶层 03-QtWidgets/67-QProgressDialog 进阶。
/// 核心知识点：setMinimumDuration 延迟显示、canceled() 信号中断任务、
///             QTimer 模拟分步异步任务。

#pragma once

#include <QWidget>

class QLabel;
class QPushButton;
class QProgressDialog;
class QTimer;

/// @brief 主窗口，演示 QProgressDialog 与异步任务的取消协作。
class ProgressDialogDemo : public QWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父控件指针，Qt 对象树自动管理生命周期。
    explicit ProgressDialogDemo(QWidget* parent = nullptr);

private slots:
    /// @brief 启动模拟异步任务，弹出 QProgressDialog。
    /// @note 使用 setMinimumDuration(1000) 让对话框仅在任务持续超过 1 秒时才显示，
    ///       避免瞬态任务闪烁。
    void startTask();

    /// @brief 定时器回调，推进任务进度。
    /// @note 每次递增 m_currentProgress 并更新对话框值；达到 100 时自动结束。
    void advanceProgress();

    /// @brief 用户点击取消后标记任务中断。
    /// @note 仅设置标志位，实际清理在 advanceProgress 中完成，保证线程安全。
    void onCanceled();

private:
    /// @brief 初始化界面布局与信号槽连接。
    void setupUI();

    /// @brief 任务完成或取消后的收尾工作。
    /// @param[in] completed true 表示正常完成，false 表示被取消。
    void finishTask(bool completed);

    QPushButton* m_startButton;      ///< "启动任务" 按钮
    QLabel* m_statusLabel;           ///< 显示任务结果的状态标签
    QProgressDialog* m_progressDlg;  ///< 进度对话框（由本对象持有）
    QTimer* m_taskTimer;             ///< 驱动分步任务的定时器
    int m_currentProgress;           ///< 当前进度值 (0-100)
};
