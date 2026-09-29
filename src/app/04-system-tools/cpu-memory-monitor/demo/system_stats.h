/**
 * @file system_stats.h
 * @brief 跨平台 CPU/内存读取封装——纯系统 API，不依赖 Qt 网络组件
 * @copyright Copyright (c) 2026 AwesomeQt
 *
 * 设计要点：
 * - 内存：Linux 读 /proc/meminfo（MemTotal / MemAvailable）；Windows GlobalMemoryStatusEx。
 * - CPU：必须两次采样算差值（Δbusy/Δtotal），单次读的是开机累计值非瞬时占用。
 * - 用 #ifdef Q_OS_LINUX / Q_OS_WIN 分发；未覆盖平台（mac 等）fallback 返回无效值（valid=false）。
 *
 * 注意：Windows 分支（GlobalMemoryStatusEx / GetSystemTimes）在本仓库的 Linux offscreen
 * CI 里不参与编译（整段 #ifdef 掉），代码写好但**尚未验证**——需 Windows 实机复验。
 */
#pragma once

#include <QObject>
#include <QSize>

// Q_OS_* 宏在 QtGlobal，已被 QObject 间接拉入；保险起见显式 include 一次
#include <QtGlobal>

/// @brief 系统内存统计（单次快照）。
struct MemoryStats {
    /// @brief 总物理内存（字节）；读不到时为 0。
    qint64 total_bytes = 0;
    /// @brief 可用内存（字节）——Linux 取 MemAvailable（含可回收缓存），非 MemFree。
    qint64 available_bytes = 0;
    /// @brief 占用百分比 [0,100]；valid=false 时为 -1。
    int used_percent = -1;
    /// @brief 读取是否成功（平台支持 + 文件/系统调用没失败）。
    bool valid = false;
};

/// @brief CPU 占用率采样器（两次采样算差值）。
///
/// 用法：构造后调用 sample() 取第一次基线，隔 ≥ 几百毫秒再调用 utilization()
/// 得到这段时间窗的占用率。单次 sample() 拿到的是累计计数，必须两次相减才是瞬时占用。
class CpuSampler : public QObject {
    Q_OBJECT
  public:
    explicit CpuSampler(QObject* parent = nullptr);

    /// @brief 取一次基线采样。失败返回 false（此后 utilization() 也会返回 -1）。
    bool sample();

    /// @brief 用本次采样与上次 sample() 的差值算占用率。
    /// @return [0,100]，需先 sample() 两次且 total 差值非 0；否则 -1。
    int utilization();

  private:
    /// @brief 平台相关：取一次 CPU 累计计数（Linux=proc/stat，Windows=GetSystemTimes）。
    /// @param[out] busy 忙时计数（user+nice+system+irq+softirq+steal+iowait 等非 idle 项）。
    /// @param[out] total 全部计数（含 idle）。
    /// @return 读取成功。
    bool readCounters(qint64& busy, qint64& total);

    qint64 prev_busy_ = 0;  ///< 上次采样的 busy 累计计数
    qint64 prev_total_ = 0; ///< 上次采样的 total 累计计数
    bool has_prev_ = false; ///< 是否已有基线（sample() 至少成功过一次）
};

/// @brief 一次性读取系统内存统计（跨平台）。
MemoryStats readMemoryStats();
