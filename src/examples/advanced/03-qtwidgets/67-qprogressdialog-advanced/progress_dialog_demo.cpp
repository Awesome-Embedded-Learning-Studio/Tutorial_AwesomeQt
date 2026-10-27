/// @file    progress_dialog_demo.cpp
/// @brief   ProgressDialogDemo 类的实现。
///
/// 对应教程：进阶层 03-QtWidgets/67-QProgressDialog 进阶。

#include "progress_dialog_demo.h"

#include <QLabel>
#include <QProgressDialog>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

ProgressDialogDemo::ProgressDialogDemo(QWidget* parent)
    : QWidget(parent)
    , m_startButton(nullptr)
    , m_statusLabel(nullptr)
    , m_progressDlg(nullptr)
    , m_taskTimer(nullptr)
    , m_currentProgress(0)
{
    setupUI();
}

void ProgressDialogDemo::setupUI()
{
    auto* layout = new QVBoxLayout(this);

    m_statusLabel = new QLabel(tr("Ready. Click the button to start a task."), this);
    m_startButton = new QPushButton(tr("Start Task"), this);

    layout->addWidget(m_statusLabel);
    layout->addWidget(m_startButton);
    layout->addStretch();

    // 使用 Qt 父对象机制创建定时器，随窗口销毁自动释放
    m_taskTimer = new QTimer(this);
    m_taskTimer->setInterval(50); // 每 50ms 推进一步，总共约 5 秒

    connect(m_startButton, &QPushButton::clicked,
            this, &ProgressDialogDemo::startTask);
    connect(m_taskTimer, &QTimer::timeout,
            this, &ProgressDialogDemo::advanceProgress);
}

void ProgressDialogDemo::startTask()
{
    m_currentProgress = 0;
    m_startButton->setEnabled(false);
    m_statusLabel->setText(tr("Task running..."));

    // @note QProgressDialog 的父对象设为 this，确保对话框生命周期由主窗口管理。
    // 每次启动新任务都重新创建，避免残留上一次的状态。
    if (m_progressDlg) {
        m_progressDlg->close();
        delete m_progressDlg;
    }
    m_progressDlg = new QProgressDialog(tr("Processing data..."), tr("Cancel"),
                                        0, 100, this);
    m_progressDlg->setWindowTitle(tr("Async Task"));
    m_progressDlg->setWindowModality(Qt::WindowModal);
    // @note setMinimumDuration(1000)：进度对话框在任务运行超过 1 秒后才显示，
    // 如果任务在 1 秒内完成，用户不会看到闪烁的对话框。
    m_progressDlg->setMinimumDuration(1000);
    m_progressDlg->setAutoClose(false);
    m_progressDlg->setAutoReset(false);
    m_progressDlg->setValue(0);

    // @note 必须在 canceled 信号中停止定时器，否则对话框关闭后定时器仍会触发。
    connect(m_progressDlg, &QProgressDialog::canceled,
            this, &ProgressDialogDemo::onCanceled);

    m_taskTimer->start();
}

void ProgressDialogDemo::advanceProgress()
{
    if (!m_progressDlg) {
        return;
    }

    m_currentProgress += 1;
    m_progressDlg->setValue(m_currentProgress);

    if (m_currentProgress >= 100) {
        m_taskTimer->stop();
        finishTask(true);
    }
}

void ProgressDialogDemo::onCanceled()
{
    m_taskTimer->stop();
    finishTask(false);
}

void ProgressDialogDemo::finishTask(bool completed)
{
    if (completed) {
        m_statusLabel->setText(tr("Task completed successfully."));
    } else {
        m_statusLabel->setText(
            tr("Task cancelled at %1%.").arg(m_currentProgress));
    }

    // @note 重置进度对话框状态并清理，防止下次启动时残留旧值。
    if (m_progressDlg) {
        m_progressDlg->reset();
        delete m_progressDlg;
        m_progressDlg = nullptr;
    }

    m_startButton->setEnabled(true);
}
