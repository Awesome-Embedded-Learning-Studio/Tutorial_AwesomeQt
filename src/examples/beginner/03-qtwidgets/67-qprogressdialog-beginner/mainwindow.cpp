#include "mainwindow.h"

#include <QProgressDialog>
#include <QVBoxLayout>

// ============================================================================
// MainWindow: 启动后台任务并显示进度对话框
// ============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(
        "QProgressDialog 进度对话框演示");
    resize(400, 200);

    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);

    m_startButton = new QPushButton("开始处理");
    m_startButton->setMinimumHeight(60);
    m_statusLabel = new QLabel(
        "点击按钮开始模拟长任务");
    m_statusLabel->setAlignment(Qt::AlignCenter);

    layout->addStretch();
    layout->addWidget(m_startButton);
    layout->addWidget(m_statusLabel);
    layout->addStretch();

    setCentralWidget(central);

    connect(m_startButton, &QPushButton::clicked,
            this, &MainWindow::onStartTask);
}

MainWindow::~MainWindow()
{
    // 确保后台线程在窗口关闭时安全退出
    if (m_thread && m_thread->isRunning()) {
        if (m_worker) {
            m_worker->cancel();
        }
        m_thread->quit();
        m_thread->wait(3000);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_thread && m_thread->isRunning()) {
        if (m_worker) {
            m_worker->cancel();
        }
        m_thread->quit();
        m_thread->wait(3000);
    }
    event->accept();
}

void MainWindow::onStartTask()
{
    const int totalSteps = 200;

    m_startButton->setEnabled(false);
    m_statusLabel->setText("任务执行中...");

    // ---- 创建 Worker 并移入线程 ----
    m_worker = new Worker(totalSteps);
    m_thread = new QThread;
    m_worker->moveToThread(m_thread);

    // ---- 创建进度对话框 ----
    auto *progress = new QProgressDialog(
        "正在读取数据...", "取消",
        0, totalSteps, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setWindowTitle("处理进度");
    progress->setMinimumDuration(0);
    progress->setAutoClose(false);
    progress->setAutoReset(false);

    // ---- 连接信号 ----

    // 线程启动后开始工作
    connect(m_thread, &QThread::started,
            m_worker, &Worker::doWork);

    // 进度更新
    connect(m_worker, &Worker::progressChanged,
            progress, &QProgressDialog::setValue);
    connect(m_worker, &Worker::labelChanged,
            progress, &QProgressDialog::setLabelText);

    // 用户取消
    connect(progress, &QProgressDialog::canceled,
            m_worker, &Worker::cancel);

    // 任务完成
    connect(m_worker, &Worker::finished,
            this,
            [this, progress](
                bool completed, qint64 elapsedMs) {
                const double secs =
                    elapsedMs / 1000.0;

                if (completed) {
                    progress->setLabelText(
                        QString("处理完成! "
                                "共耗时 %1 秒")
                            .arg(secs, 0, 'f', 1));
                    progress->setValue(
                        progress->maximum());
                    m_statusLabel->setText(
                        QString("上次任务完成，"
                                "耗时 %1 秒")
                            .arg(secs, 0, 'f', 1));
                } else {
                    progress->setLabelText("已取消");
                    progress->setValue(
                        progress->value());
                    m_statusLabel->setText(
                        "上次任务被用户取消");
                }
                m_startButton->setEnabled(true);
            });

    // 清理资源
    connect(m_worker, &Worker::finished,
            m_thread, &QThread::quit);
    connect(m_worker, &Worker::finished,
            m_worker, &Worker::deleteLater);
    connect(m_thread, &QThread::finished,
            m_thread, &QThread::deleteLater);
    connect(m_thread, &QThread::finished,
            this, [this]() {
        m_thread = nullptr;
        m_worker = nullptr;
    });

    // 启动
    m_thread->start();
}
