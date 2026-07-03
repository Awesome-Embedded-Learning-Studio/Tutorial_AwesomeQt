/**
 * @file system_stats.cpp
 * @brief 跨平台 CPU/内存读取实现——/proc 与 Win32 API 分发
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "system_stats.h"

#include <QFile>
#include <QTextStream>
#include <QtCore/qnumeric.h>

#if defined(Q_OS_LINUX)
// Linux：读 /proc 文本，无需链接额外库
#elif defined(Q_OS_WIN)
// 尚未验证：offscreen 在 Linux 跑，此 Windows 路径未实跑，需 Windows 实机复验。
// Windows：GlobalMemoryStatusEx（内存）+ GetSystemTimes（CPU），需 windows.h（kernel32）。
#    include <windows.h>
#endif

// 静态 helper（声明在文件内，避免污染头文件）：解析 "MemTotal:   16384000 kB" 取数值
static qint64 parseKib(const QString& line);

// ============================================================================
// 内存
// ============================================================================
MemoryStats readMemoryStats() {
    MemoryStats s;
#if defined(Q_OS_LINUX)
    // /proc/meminfo 行格式 "Key:   value kB"，单位 KiB（×1024 → 字节）。
    // 关键用 MemAvailable（3.3+ 内核）：含可回收的 buffer/cache，比 MemFree 更贴合
    // 「真正能用的内存」，用 MemFree 会高估占用（把缓存当成占用）。
    QFile f("/proc/meminfo");
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return s; // valid 保持 false
    }
    qint64 mem_total = 0;
    qint64 mem_avail = 0;
    bool got_total = false;
    bool got_avail = false;
    QTextStream in(&f);
    for (QString line = in.readLine(); !line.isNull(); line = in.readLine()) {
        if (line.startsWith("MemTotal:")) {
            mem_total = parseKib(line);
            got_total = true;
        } else if (line.startsWith("MemAvailable:")) {
            mem_avail = parseKib(line);
            got_avail = true;
        }
        if (got_total && got_avail) {
            break; // 两项都拿到即可，不必读完整个文件
        }
    }
    if (!got_total || mem_total <= 0 || mem_avail < 0 || mem_avail > mem_total) {
        return s; // 数据异常，不算成功
    }
    s.total_bytes = mem_total;
    s.available_bytes = mem_avail;
    const qint64 used = mem_total - mem_avail;
    s.used_percent = static_cast<int>((used * 100 + mem_total / 2) / mem_total); // 四舍五入
    s.valid = true;
    return s;
#elif defined(Q_OS_WIN)
    // 尚未验证：offscreen 在 Linux 跑，此 Windows 路径未实跑，需 Windows 实机复验。
    // MEMORYSTATUSEX 调用前必须设 dwLength = sizeof(MEMORYSTATUSEX)，否则函数失败返回 0。
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(MEMORYSTATUSEX);
    if (!GlobalMemoryStatusEx(&status)) {
        return s; // 调用失败，valid 保持 false
    }
    s.total_bytes = static_cast<qint64>(status.ullTotalPhys);
    s.available_bytes = static_cast<qint64>(status.ullAvailPhys);
    s.used_percent = static_cast<int>(status.dwMemoryLoad); // Windows 直接给 0..100 占用比
    s.valid = (s.total_bytes > 0);
    return s;
#else
    // 未覆盖平台（macOS / BSD 等）：fallback 返回无效值，不崩、不留哑数据。
    return s;
#endif
}

// 静态 helper（实现）：解析 "MemTotal:   16384000 kB" 取数值（×1024 → 字节）
static qint64 parseKib(const QString& line) {
    // 取冒号后的数字部分，单位是 KiB
    const int colon = line.indexOf(':');
    if (colon < 0) {
        return 0;
    }
    const QString tail = line.mid(colon + 1).simplified();
    const int space = tail.indexOf(' ');
    const QString num_str = (space < 0) ? tail : tail.left(space);
    bool ok = false;
    const qint64 kib = num_str.toLongLong(&ok);
    return ok ? kib * 1024 : 0;
}

// ============================================================================
// CPU 采样器
// ============================================================================
CpuSampler::CpuSampler(QObject* parent) : QObject(parent) {}

bool CpuSampler::sample() {
    qint64 busy = 0;
    qint64 total = 0;
    if (!readCounters(busy, total)) {
        has_prev_ = false; // 读取失败，丢弃旧基线，避免下次拿新基线与旧值乱减
        return false;
    }
    prev_busy_ = busy;
    prev_total_ = total;
    has_prev_ = true;
    return true;
}

int CpuSampler::utilization() {
    if (!has_prev_) {
        return -1; // 还没基线
    }
    qint64 busy = 0;
    qint64 total = 0;
    if (!readCounters(busy, total)) {
        return -1; // 本次读取失败，保留旧基线不污染
    }
    const qint64 d_total = total - prev_total_;
    qint64 d_busy = busy - prev_busy_;
    // 滚动基线：下次 utilization() 以本次为新基准（滑动窗口，CPU% 跟得上当前负载）
    prev_busy_ = busy;
    prev_total_ = total;
    if (d_total <= 0) {
        return -1; // 计数回绕或两次采样间系统未累积，没法算
    }
    if (d_busy < 0) {
        d_busy = 0; // 防御：理论上单调增，回绕时钳到 0
    }
    if (d_busy > d_total) {
        d_busy = d_total; // 防御：busy 不应超过 total
    }
    return static_cast<int>((d_busy * 100 + d_total / 2) / d_total); // 四舍五入
}

bool CpuSampler::readCounters(qint64& busy, qint64& total) {
#if defined(Q_OS_LINUX)
    // /proc/stat 第一行是 aggregate（所有核合计）：cpu user nice system idle iowait irq softirq
    // steal guest guest_nice。busy = 除 idle 外各项（iowait 算 busy 因核被占着等 IO）。
    QFile f("/proc/stat");
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream in(&f);
    const QString first = in.readLine();
    if (!first.startsWith("cpu ")) { // aggregate 行 "cpu " 后跟空格；单核行 "cpu0" 不匹配
        return false;
    }
    const QStringList parts = first.split(' ', Qt::SkipEmptyParts);
    if (parts.size() < 5) {
        return false; // 至少 cpu + user+nice+system+idle
    }
    // 必备字段（user/nice/system/idle）逐个独立校验，任一解析失败即判整段失败——
    // 否则单共享 ok 会被后续字段覆盖，畸形字段静默变 0 却判成功（review 实测复现）。
    // 索引：1=user 2=nice 3=system 4=idle 5=iowait 6=irq 7=softirq 8=steal(可选)
    bool ok = false;
    const qint64 user = parts.value(1).toLongLong(&ok);
    if (!ok)
        return false;
    const qint64 nice = parts.value(2).toLongLong(&ok);
    if (!ok)
        return false;
    const qint64 system = parts.value(3).toLongLong(&ok);
    if (!ok)
        return false;
    const qint64 idle = parts.value(4).toLongLong(&ok);
    if (!ok)
        return false;
    // 可选字段（iowait 起向后）可能缺省：parts.value 越界返回空串→toLongLong 失败→0，符合预期
    bool ok_opt = false;
    const qint64 iowait = parts.value(5).toLongLong(&ok_opt);
    const qint64 irq = parts.value(6).toLongLong(&ok_opt);
    const qint64 softirq = parts.value(7).toLongLong(&ok_opt);
    const qint64 steal = parts.value(8).toLongLong(&ok_opt);

    busy = user + nice + system + irq + softirq + steal + iowait;
    total = busy + idle;
    return true;
#elif defined(Q_OS_WIN)
    // 尚未验证：offscreen 在 Linux 跑，此 Windows 路径未实跑，需 Windows 实机复验。
    // GetSystemTimes：与 Linux 同理需两次采样（idle/total 差值）。
    FILETIME idle_time;
    FILETIME kernel_time;
    FILETIME user_time;
    if (!GetSystemTimes(&idle_time, &kernel_time, &user_time)) {
        return false;
    }
    // FILETIME 是 100ns 计数，转 qint64（ULARGE_INTEGER 绕开结构体对齐坑）
    const auto to_int64 = [](const FILETIME& ft) {
        ULARGE_INTEGER ul;
        ul.LowPart = ft.dwLowDateTime;
        ul.HighPart = ft.dwHighDateTime;
        return static_cast<qint64>(ul.QuadPart);
    };
    const qint64 idle = to_int64(idle_time);
    const qint64 kernel = to_int64(kernel_time);
    const qint64 user = to_int64(user_time);
    // 注意：kernel_time 含 idle 时间（Win32 文档明示），故 busy = kernel - idle + user。
    const qint64 busy_kernel = kernel - idle;
    busy = busy_kernel + user;
    total = busy + idle;
    return true;
#else
    Q_UNUSED(busy);
    Q_UNUSED(total);
    return false; // 未覆盖平台：返回失败，UI 显示无效态
#endif
}
