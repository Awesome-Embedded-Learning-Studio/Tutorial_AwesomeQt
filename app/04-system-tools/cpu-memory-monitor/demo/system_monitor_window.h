/**
 * @file system_monitor_window.h
 * @brief 系统监控主窗口——内存/CPU 进度条 + CPU 历史曲线 + QTimer 1s 采样
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include <QMainWindow>

#include "system_stats.h"

class CpuHistoryView;
class QLabel;
class QProgressBar;
class QTimer;

/// @brief 系统监控主窗口（app 栏整机范式）。
///
/// QMainWindow 中央区：内存行（进度条 + 已用/总量 GB）、CPU 行（进度条 + 占用%）、
/// CPU 历史曲线。QTimer 每 1s 采样一次刷新 UI。
///
/// CPU% 需两次采样差值，所以构造时先 sample() 一次拿基线，第一个 tick 才出有效值；
/// 采样间隔太短 CPU% 抖动大，1s 合理，且 QTimer 不开 PreciseTimer（系统读取本身有耗时）。
class SystemMonitorWindow : public QMainWindow {
    Q_OBJECT
  public:
    explicit SystemMonitorWindow(QWidget* parent = nullptr);

  private slots:
    void onTick();

  private:
    void setupCentral();
    void setupStatusBar();
    void refreshMemory(const MemoryStats& s);
    void refreshCpu(int percent);

    QProgressBar* mem_bar_{nullptr};
    QLabel* mem_label_{nullptr};
    QProgressBar* cpu_bar_{nullptr};
    QLabel* cpu_label_{nullptr};
    CpuHistoryView* cpu_history_{nullptr};

    QLabel* status_label_{nullptr};

    QTimer* timer_{nullptr};
    CpuSampler cpu_sampler_; // CPU 占用率采样器（状态：上次基线计数）
};
