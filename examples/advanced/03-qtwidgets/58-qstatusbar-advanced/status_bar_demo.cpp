/// @file    status_bar_demo.cpp
/// @brief   StatusBarDemo 实现——多区域 QStatusBar 状态显示。
///
/// 对应教程：进阶层 03-QtWidgets/58-QStatusBar 进阶。

#include "status_bar_demo.h"

#include <QCheckBox>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QStatusBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QMouseEvent>

StatusBarDemo::StatusBarDemo(QWidget* parent)
    : QMainWindow(parent)
    , m_posLabel(nullptr)
    , m_progressBar(nullptr)
    , m_modeLabel(nullptr)
    , m_editModeCheck(nullptr)
    , m_taskProgress(0)
{
    setupUI();

    setWindowTitle(tr("QStatusBar Advanced Demo"));
    resize(600, 400);
}

void StatusBarDemo::setupUI()
{
    // 中央控件——需要启用鼠标追踪才能实时获取 mouseMoveEvent
    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);

    auto* infoLabel = new QLabel(
        tr("Move the mouse here to see coordinates in the status bar.\n"
           "Click 'Start Task' to see a timed progress bar."), this);
    infoLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(infoLabel);

    // @note QStatusBar 本身有 showMessage 方法，但永久控件不会被打断
    setCentralWidget(central);
    central->setMouseTracking(true);

    // --- 状态栏永久控件 ---

    // 区域 1: 鼠标坐标
    m_posLabel = new QLabel(tr("Pos: (0, 0)"), this);
    m_posLabel->setMinimumWidth(120);
    // @note addPermanentWidget 将控件放在状态栏右侧，不会被 showMessage 覆盖
    statusBar()->addPermanentWidget(m_posLabel);

    // 区域 2: 进度条（初始隐藏）
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setFixedWidth(150);
    m_progressBar->hide();
    statusBar()->addPermanentWidget(m_progressBar);

    // 区域 3: 编辑模式复选框
    m_editModeCheck = new QCheckBox(tr("Edit Mode"), this);
    statusBar()->addPermanentWidget(m_editModeCheck);

    // 区域 4: 模式指示器
    m_modeLabel = new QLabel(tr("Mode: View"), this);
    m_modeLabel->setMinimumWidth(100);
    statusBar()->addPermanentWidget(m_modeLabel);

    connect(m_editModeCheck, &QCheckBox::toggled, this, [this](bool checked) {
        m_modeLabel->setText(checked ? tr("Mode: Edit") : tr("Mode: View"));
    });

    // 添加一个 "Start Task" 按钮到中央区域
    auto* taskButton = new QPushButton(tr("Start Task (3s)"), central);
    connect(taskButton, &QPushButton::clicked, this, &StatusBarDemo::startSimulatedTask);
    layout->addWidget(taskButton);

    // 临时消息示例
    statusBar()->showMessage(tr("Ready — move mouse or start a task"), 3000);
}

void StatusBarDemo::mouseMoveEvent(QMouseEvent* event)
{
    const QPoint pos = event->position().toPoint();
    m_posLabel->setText(tr("Pos: (%1, %2)").arg(pos.x()).arg(pos.y()));
}

void StatusBarDemo::startSimulatedTask()
{
    m_taskProgress = 0;
    m_progressBar->setValue(0);
    m_progressBar->show();

    // @note 使用 QTimer 模拟异步任务的进度更新
    auto* timer = new QTimer(this);
    timer->setInterval(30);    // ~33 次/秒

    connect(timer, &QTimer::timeout, this, [this, timer]() {
        m_taskProgress += 1;
        m_progressBar->setValue(m_taskProgress);

        if (m_taskProgress >= 100) {
            timer->stop();
            timer->deleteLater();
            // @note 任务完成后延迟隐藏进度条
            QTimer::singleShot(500, this, [this]() {
                m_progressBar->hide();
                statusBar()->showMessage(tr("Task completed!"), 3000);
            });
        }
    });

    timer->start();
    statusBar()->showMessage(tr("Task running..."));
}
