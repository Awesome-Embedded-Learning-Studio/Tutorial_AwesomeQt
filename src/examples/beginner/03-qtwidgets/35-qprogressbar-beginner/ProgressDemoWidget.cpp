#include "ProgressDemoWidget.h"

// ============================================================================
// ProgressDemoWidget: QProgressBar 综合演示窗口
// ============================================================================
ProgressDemoWidget::ProgressDemoWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("QProgressBar 综合演示 — 跨线程进度更新");
    resize(520, 480);
    initUi();
}

/// @brief 初始化界面
void ProgressDemoWidget::initUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // ================================================================
    // 区域 1: 基本进度条 + 自定义格式
    // ================================================================
    auto *basicGroup = new QGroupBox("基本进度条 (setRange + setValue)");
    auto *basicLayout = new QVBoxLayout(basicGroup);

    // 进度条：百分比格式
    m_percentBar = new QProgressBar;
    m_percentBar->setRange(0, 100);
    m_percentBar->setValue(0);
    m_percentBar->setFormat("处理进度: %p%");
    m_percentBar->setMinimumHeight(28);
    basicLayout->addWidget(m_percentBar);

    // 进度条：详细值格式
    m_detailBar = new QProgressBar;
    m_detailBar->setRange(0, 100);
    m_detailBar->setValue(0);
    m_detailBar->setFormat("已处理 %v 个，共 %m 个");
    m_detailBar->setMinimumHeight(28);
    basicLayout->addWidget(m_detailBar);

    // 详细信息标签
    m_detailLabel = new QLabel("当前值: 0 / 100");
    m_detailLabel->setAlignment(Qt::AlignCenter);
    m_detailLabel->setStyleSheet("color: #666; font-size: 12px;");
    basicLayout->addWidget(m_detailLabel);

    mainLayout->addWidget(basicGroup);

    // ================================================================
    // 区域 2: 无限进度条
    // ================================================================
    auto *busyGroup = new QGroupBox("无限进度条 (setRange(0, 0))");
    auto *busyLayout = new QVBoxLayout(busyGroup);

    m_busyBar = new QProgressBar;
    m_busyBar->setRange(0, 0);
    m_busyBar->setMinimumHeight(24);
    m_busyBar->setTextVisible(false);
    m_busyBar->hide();  // 初始隐藏
    busyLayout->addWidget(m_busyBar);

    m_busyLabel = new QLabel("空闲");
    m_busyLabel->setAlignment(Qt::AlignCenter);
    m_busyLabel->setStyleSheet("color: #888; font-size: 12px;");
    busyLayout->addWidget(m_busyLabel);

    mainLayout->addWidget(busyGroup);

    // ================================================================
    // 控制按钮
    // ================================================================
    auto *btnLayout = new QHBoxLayout;

    m_startBtn = new QPushButton("开始处理");
    m_startBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #1976D2; color: white;"
        "  border: none; border-radius: 6px;"
        "  padding: 8px 24px; font-size: 13px; font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #1565C0; }"
        "QPushButton:pressed { background-color: #0D47A1; }"
        "QPushButton:disabled { background-color: #BDBDBD; }");
    connect(m_startBtn, &QPushButton::clicked,
            this, &ProgressDemoWidget::startWork);

    m_cancelBtn = new QPushButton("取消");
    m_cancelBtn->setEnabled(false);
    m_cancelBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #D32F2F; color: white;"
        "  border: none; border-radius: 6px;"
        "  padding: 8px 24px; font-size: 13px; font-weight: bold;"
        ""
        "}"
        "QPushButton:hover { background-color: #C62828; }"
        "QPushButton:pressed { background-color: #B71C1C; }"
        "QPushButton:disabled { background-color: #BDBDBD; }");
    connect(m_cancelBtn, &QPushButton::clicked,
            this, &ProgressDemoWidget::cancelWork);

    btnLayout->addStretch();
    btnLayout->addWidget(m_startBtn);
    btnLayout->addWidget(m_cancelBtn);
    btnLayout->addStretch();

    mainLayout->addLayout(btnLayout);

    // ================================================================
    // 状态标签
    // ================================================================
    m_statusLabel = new QLabel("点击「开始处理」启动跨线程任务");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet(
        "color: #888; font-size: 12px;"
        "padding: 8px;"
        "background-color: #FAFAFA;"
        "border: 1px solid #E0E0E0;"
        "border-radius: 4px;");
    mainLayout->addWidget(m_statusLabel);
}

/// @brief 启动工作线程
void ProgressDemoWidget::startWork()
{
    m_startBtn->setEnabled(false);
    m_cancelBtn->setEnabled(true);

    // 重置进度条
    m_percentBar->setValue(0);
    m_detailBar->setValue(0);
    m_detailLabel->setText("当前值: 0 / 100");
    m_statusLabel->setText("处理中... (工作线程运行中)");
    m_statusLabel->setStyleSheet(
        "color: #1976D2; font-size: 12px;"
        "padding: 8px;"
        "background-color: #E3F2FD;"
        "border: 1px solid #90CAF9;"
        "border-radius: 4px;");

    // 显示无限进度条
    m_busyBar->show();
    m_busyLabel->setText("后台处理中...");
    m_busyLabel->setStyleSheet("color: #F57C00; font-size: 12px;");

    // 创建 Worker 和 Thread
    const int totalCount = 100;
    m_worker = new Worker(totalCount);
    m_thread = new QThread;

    m_worker->moveToThread(m_thread);

    // 连接信号槽
    connect(m_thread, &QThread::started,
            m_worker, &Worker::doWork);

    // 进度更新（跨线程信号槽，默认 Qt::QueuedConnection）
    connect(m_worker, &Worker::progressChanged,
            m_percentBar, &QProgressBar::setValue);
    connect(m_worker, &Worker::progressChanged,
            m_detailBar, &QProgressBar::setValue);

    connect(m_worker, &Worker::detailChanged, this,
            [this](int current, int total) {
        m_detailLabel->setText(
            QString("当前值: %1 / %2").arg(current).arg(total));
    });

    // 完成处理
    connect(m_worker, &Worker::finished, this,
            [this]() {
        cleanupThread();
        m_statusLabel->setText("处理完成！");
        m_statusLabel->setStyleSheet(
            "color: #388E3C; font-size: 12px;"
            "padding: 8px;"
            "background-color: #E8F5E9;"
            "border: 1px solid #A5D6A7;"
            "border-radius: 4px;");
        m_busyLabel->setText("已完成");
        m_busyLabel->setStyleSheet(
            "color: #388E3C; font-size: 12px;");
    });

    // 取消处理
    connect(m_worker, &Worker::canceled, this,
            [this](int atPos) {
        cleanupThread();
        m_statusLabel->setText(
            QString("已取消（停在第 %1 步）").arg(atPos));
        m_statusLabel->setStyleSheet(
            "color: #F57C00; font-size: 12px;"
            "padding: 8px;"
            "background-color: #FFF3E0;"
            "border: 1px solid #FFCC80;"
            "border-radius: 4px;");
        m_busyLabel->setText("已取消");
        m_busyLabel->setStyleSheet(
            "color: #F57C00; font-size: 12px;");
    });

    // 线程清理
    connect(m_worker, &Worker::finished,
            m_worker, &Worker::deleteLater);
    connect(m_worker, &Worker::canceled,
            m_worker, &Worker::deleteLater);
    connect(m_thread, &QThread::finished,
            m_thread, &QThread::deleteLater);

    // 启动线程
    m_thread->start();
}

/// @brief 取消工作线程
void ProgressDemoWidget::cancelWork()
{
    if (m_worker) {
        m_worker->requestCancel();
    }
    m_cancelBtn->setEnabled(false);
    m_statusLabel->setText("正在取消...");
}

/// @brief 清理线程后的 UI 恢复
void ProgressDemoWidget::cleanupThread()
{
    m_startBtn->setEnabled(true);
    m_cancelBtn->setEnabled(false);

    // 隐藏无限进度条（延迟一下让用户看到完成状态）
    QTimer::singleShot(1500, this, [this]() {
        m_busyBar->hide();
    });
}
