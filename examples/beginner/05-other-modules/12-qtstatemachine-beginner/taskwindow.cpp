#include "taskwindow.h"

#include <QApplication>
#include <QFinalState>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QSignalTransition>
#include <QState>
#include <QStateMachine>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

// ========================================
// 状态机驱动的任务管理器窗口
// ========================================

TaskWindow::TaskWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Qt StateMachine 任务管理器");
    resize(500, 200);

    auto *layout = new QVBoxLayout(this);

    // 状态标签
    status_label_ = new QLabel("空闲", this);
    QFont statusFont;
    statusFont.setPointSize(12);
    statusFont.setBold(true);
    status_label_->setFont(statusFont);
    status_label_->setAlignment(Qt::AlignCenter);
    layout->addWidget(status_label_);

    // 进度条
    progress_bar_ = new QProgressBar(this);
    progress_bar_->setRange(0, 100);
    progress_bar_->setValue(0);
    layout->addWidget(progress_bar_);

    // 按钮区域
    auto *btnLayout = new QHBoxLayout();

    start_btn_ = new QPushButton("开始任务", this);
    pause_btn_ = new QPushButton("暂停", this);
    resume_btn_ = new QPushButton("恢复", this);
    stop_btn_ = new QPushButton("停止", this);

    btnLayout->addWidget(start_btn_);
    btnLayout->addWidget(pause_btn_);
    btnLayout->addWidget(resume_btn_);
    btnLayout->addWidget(stop_btn_);
    layout->addLayout(btnLayout);

    // 初始化按钮状态
    pause_btn_->setEnabled(false);
    resume_btn_->setEnabled(false);
    stop_btn_->setEnabled(false);

    // 构建状态机
    setupStateMachine();
}

/// 构建状态机
void TaskWindow::setupStateMachine()
{
    machine_ = new QStateMachine(this);

    // ========================================
    // 定义四个顶层状态
    // ========================================

    auto *idle = new QState();       // 空闲
    auto *running = new QState();    // 运行中（包含子状态）
    auto *paused = new QState();     // 暂停
    auto *done = new QState();       // 完成（用普通状态代替 QFinalState 以支持 assignProperty/addTransition）

    // ========================================
    // 层次状态机："运行中"包含子状态
    // ========================================

    auto *init_phase = new QState(running);      // 初始化阶段
    auto *process_phase = new QState(running);    // 处理阶段
    running->setInitialState(init_phase);

    // 子状态之间的转换
    init_phase->addTransition(
        this, &TaskWindow::initFinished, process_phase);

    // ========================================
    // assignProperty：每个状态控制 UI 属性
    // ========================================

    // 空闲状态
    idle->assignProperty(start_btn_, "enabled", true);
    idle->assignProperty(pause_btn_, "enabled", false);
    idle->assignProperty(resume_btn_, "enabled", false);
    idle->assignProperty(stop_btn_, "enabled", false);
    idle->assignProperty(status_label_, "text",
                         "空闲 - 点击\"开始任务\"");
    idle->assignProperty(progress_bar_, "value", 0);

    // 初始化阶段
    init_phase->assignProperty(start_btn_, "enabled", false);
    init_phase->assignProperty(pause_btn_, "enabled", false);
    init_phase->assignProperty(resume_btn_, "enabled", false);
    init_phase->assignProperty(stop_btn_, "enabled", true);
    init_phase->assignProperty(status_label_, "text",
                               "运行中 - 初始化...");

    // 处理阶段
    process_phase->assignProperty(start_btn_, "enabled", false);
    process_phase->assignProperty(pause_btn_, "enabled", true);
    process_phase->assignProperty(resume_btn_, "enabled", false);
    process_phase->assignProperty(stop_btn_, "enabled", true);
    process_phase->assignProperty(status_label_, "text",
                                  "运行中 - 处理中...");

    // 暂停状态
    paused->assignProperty(start_btn_, "enabled", false);
    paused->assignProperty(pause_btn_, "enabled", false);
    paused->assignProperty(resume_btn_, "enabled", true);
    paused->assignProperty(stop_btn_, "enabled", true);
    paused->assignProperty(status_label_, "text",
                           "已暂停 - 点击\"恢复\"继续");

    // 完成状态
    done->assignProperty(start_btn_, "enabled", true);
    done->assignProperty(pause_btn_, "enabled", false);
    done->assignProperty(resume_btn_, "enabled", false);
    done->assignProperty(stop_btn_, "enabled", false);
    done->assignProperty(status_label_, "text",
                         "完成 - 任务执行成功");
    done->assignProperty(progress_bar_, "value", 100);

    // ========================================
    // 状态转换条件
    // ========================================

    // 空闲 -> 运行中
    idle->addTransition(
        start_btn_, &QPushButton::clicked, running);

    // 运行中 -> 暂停（父状态上的转换对所有子状态生效）
    running->addTransition(
        pause_btn_, &QPushButton::clicked, paused);

    // 暂停 -> 运行中（恢复到之前离开的子状态）
    paused->addTransition(
        resume_btn_, &QPushButton::clicked, running);

    // 运行中 -> 完成（任务完成信号）
    running->addTransition(
        this, &TaskWindow::taskFinished, done);

    // 运行中 -> 空闲（停止按钮）
    running->addTransition(
        this, &TaskWindow::taskStopped, idle);

    // 暂停 -> 空闲（停止按钮）
    paused->addTransition(
        stop_btn_, &QPushButton::clicked, idle);

    // 完成后重新启动
    done->addTransition(
        start_btn_, &QPushButton::clicked, idle);

    // ========================================
    // 注册状态并设置初始状态
    // ========================================

    machine_->addState(idle);
    machine_->addState(running);
    machine_->addState(paused);
    machine_->addState(done);
    machine_->setInitialState(idle);

    // ========================================
    // 状态进入时的日志输出（调试用）
    // ========================================

    connect(idle, &QState::entered, this, []() {
        qDebug() << "[状态] -> 空闲";
    });
    connect(init_phase, &QState::entered, this, [this]() {
        qDebug() << "[状态] -> 初始化阶段";
        simulateInit();
    });
    connect(process_phase, &QState::entered, this, [this]() {
        qDebug() << "[状态] -> 处理阶段";
        simulateProcessing();
    });
    connect(paused, &QState::entered, this, []() {
        qDebug() << "[状态] -> 暂停";
    });
    connect(done, &QState::entered, this, []() {
        qDebug() << "[状态] -> 完成";
    });

    // ========================================
    // 按钮点击：发射自定义信号触发状态转换
    // ========================================

    // 停止按钮需要在 running 子状态下才能停止
    connect(stop_btn_, &QPushButton::clicked, this, [this]() {
        // 停止进度定时器
        if (progress_timer_) {
            progress_timer_->stop();
        }
        emit taskStopped();
    });

    // 启动状态机
    machine_->start();
    qDebug() << "状态机已启动";
}

/// 模拟初始化阶段
void TaskWindow::simulateInit()
{
    // 2 秒后完成初始化
    QTimer::singleShot(2000, this, [this]() {
        // 检查当前是否还在运行状态
        // （可能在定时器触发前已经停止或暂停）
        if (machine_->configuration()
                .contains(machine_->findChild<QState*>())) {
            // 安全检查：只在实际处于运行状态时才发射信号
        }
        emit initFinished();
    });
}

/// 模拟处理阶段（更新进度条）
void TaskWindow::simulateProcessing()
{
    if (progress_timer_) {
        progress_timer_->stop();
        progress_timer_->deleteLater();
    }

    progress_timer_ = new QTimer(this);
    int progress = progress_bar_->value();

    connect(progress_timer_, &QTimer::timeout, this, [this, progress]() {
        int current = progress_bar_->value();
        if (current >= 100) {
            progress_timer_->stop();
            emit taskFinished();
            return;
        }
        progress_bar_->setValue(current + 2);
    });

    progress_timer_->start(100);  // 每 100ms 增加 2%
}
