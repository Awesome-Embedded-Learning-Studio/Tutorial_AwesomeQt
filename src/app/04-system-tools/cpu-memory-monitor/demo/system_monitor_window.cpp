/**
 * @file system_monitor_window.cpp
 * @brief SystemMonitorWindow 实现——装配进度条/曲线 + QTimer 1s 采样刷新
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "system_monitor_window.h"

#include "cpu_history_view.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QStatusBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

namespace {
/// @brief 字节 → 易读 GB（2 位小数）。
QString formatGb(qint64 bytes) {
    constexpr double k = 1024.0 * 1024.0 * 1024.0;
    return QString::number(static_cast<double>(bytes) / k, 'f', 2) + " GB";
}
} // namespace

SystemMonitorWindow::SystemMonitorWindow(QWidget* parent)
    : QMainWindow(parent), cpu_sampler_(this) {
    setWindowTitle("CPU / Memory Monitor");
    resize(420, 360);

    setupCentral();
    setupStatusBar();

    // CPU% 要差值：构造时先拿一次基线，第一个 onTick 才有有效占用率。
    cpu_sampler_.sample();

    timer_ = new QTimer(this);
    timer_->setInterval(1000); // 1s 采样——间隔太短 CPU% 抖动大
    // 不开 PreciseTimer：系统读取（/proc、Win32）本身有耗时，精确定时器无意义还更耗能。
    connect(timer_, &QTimer::timeout, this, &SystemMonitorWindow::onTick);
    timer_->start();
    // 立即先刷一次内存（CPU% 第一拍没基线差，等下个 tick）
    onTick();
}

void SystemMonitorWindow::setupCentral() {
    auto* central = new QWidget(this);
    auto* root = new QVBoxLayout(central);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(12);

    // ---- 内存分组 ----
    auto* mem_group = new QGroupBox("Memory", central);
    auto* mem_vbox = new QVBoxLayout(mem_group);
    mem_bar_ = new QProgressBar(mem_group);
    mem_bar_->setRange(0, 100);
    mem_bar_->setTextVisible(false); // 数值另用 label 显示（已用/总量更直观）
    mem_label_ = new QLabel("--", mem_group);
    auto* mem_inner = new QHBoxLayout;
    mem_inner->addWidget(mem_bar_, 1);
    mem_inner->addWidget(mem_label_);
    mem_vbox->addLayout(mem_inner);
    root->addWidget(mem_group);

    // ---- CPU 分组 ----
    auto* cpu_group = new QGroupBox("CPU", central);
    auto* cpu_vbox = new QVBoxLayout(cpu_group);
    cpu_bar_ = new QProgressBar(cpu_group);
    cpu_bar_->setRange(0, 100);
    cpu_bar_->setTextVisible(false);
    cpu_label_ = new QLabel("--", cpu_group);
    auto* cpu_inner = new QHBoxLayout;
    cpu_inner->addWidget(cpu_bar_, 1);
    cpu_inner->addWidget(cpu_label_);
    cpu_vbox->addLayout(cpu_inner);

    cpu_history_ = new CpuHistoryView(60, cpu_group);
    cpu_vbox->addWidget(cpu_history_);
    root->addWidget(cpu_group);

    root->addStretch(1);
    setCentralWidget(central);
}

void SystemMonitorWindow::setupStatusBar() {
    status_label_ = new QLabel("Sampling every 1 s");
    statusBar()->addWidget(status_label_);
}

void SystemMonitorWindow::onTick() {
    const MemoryStats mem = readMemoryStats();
    refreshMemory(mem);

    // CPU%：utilization() 内部已滚动基线（本次读数→下次的 prev_），故每拍只调它一次。
    // 返回 -1 的两种情况——构造时未 sample 过基线 / 本次 readCounters 失败：
    // 补一次 sample() 重建基线，保证下一拍能算出差值（utilization 提前返回时不会动 prev_）。
    const int cpu_pct = cpu_sampler_.utilization();
    if (cpu_pct < 0) {
        cpu_sampler_.sample();
    }
    refreshCpu(cpu_pct);
}

void SystemMonitorWindow::refreshMemory(const MemoryStats& s) {
    if (!s.valid) {
        mem_bar_->setValue(0);
        mem_label_->setText("N/A");
        return;
    }
    mem_bar_->setValue(s.used_percent);
    const qint64 used = s.total_bytes - s.available_bytes;
    mem_label_->setText(QString("%1%   (%2 / %3)")
                            .arg(s.used_percent)
                            .arg(formatGb(used))
                            .arg(formatGb(s.total_bytes)));
}

void SystemMonitorWindow::refreshCpu(int percent) {
    if (percent < 0) {
        cpu_bar_->setValue(0);
        cpu_label_->setText("N/A");
        cpu_history_->push(-1); // 无效点：曲线留缺口
        return;
    }
    cpu_bar_->setValue(percent);
    cpu_label_->setText(QString("%1%").arg(percent));
    cpu_history_->push(percent);
}
