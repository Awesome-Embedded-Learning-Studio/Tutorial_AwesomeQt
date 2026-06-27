---
title: "Step 1：跨平台系统读取 + CPU 两次采样差值"
description: "内存跨平台读取（Linux /proc/meminfo 的 MemAvailable、Windows GlobalMemoryStatusEx）+ CPU 占用率采样器状态机（两次采样差值 Δbusy/Δtotal、滚动基线、计数钳制防御、两种失败语义）。"
---

# Step 1：跨平台系统读取 + CPU 两次采样差值

← [手册首页](./index.md) · 下一步 [Step 2 自绘历史曲线](./02-history-chart-paint.md) →

## Step 1：内存跨平台读取 + CPU 采样器状态机

### 目标

写一个 `system_stats.h/.cpp`，提供两个能力：① `readMemoryStats()` 自由函数——传入无参数，返回一个 `MemoryStats` 结构体（总量/可用/占用%/是否有效）；② `CpuSampler` 类——构造后 `sample()` 取基线，隔一会儿再 `utilization()` 得到 0..100 的占用率。这一步**只做数据层（纯系统读取 + 算法），不碰 UI**——先在 main 里用 `qDebug` 打印验证内存和 CPU% 数值合理。

### 提示

- **`MemoryStats` 结构体怎么定？** `qint64 total_bytes / available_bytes`（字节）、`int used_percent`（-1 表无效）、`bool valid`。Linux 读到的单位是 KiB，记得 `× 1024` 转字节。
- **内存为什么用 `MemAvailable` 而不是 `MemFree`？** `MemFree` 是完全空闲的内存，Linux 把空闲内存拿去做 buffer/cache 后 `MemFree` 会很小，用它算占用会把缓存当占用、**高估占用率**。`MemAvailable`（3.3+ 内核）含可回收缓存，才是真能用的内存。
- **`/proc/meminfo` 怎么逐行扫？** `QFile` + `QTextStream`，`readLine()` 循环；行格式 `"MemTotal:   16384000 kB"`。写个 `parseKib` helper：`indexOf(':')` 取冒号位置、`mid(colon+1).simplified()` 去空白、`indexOf(' ')` 切出数字、`toLongLong × 1024`。两项都拿到就 `break`，不必读完整个文件。
- **CPU 为什么不能单次直读？** `/proc/stat` 第一行是**开机以来累计** jiffies 计数，单次读到的是「从开机到现在 CPU 累计忙了多少」，不是「现在多忙」。必须 `sample()` 存基线（`prev_busy_`/`prev_total_`），隔几百毫秒后 `utilization()` 再读一次，用**差值** `Δbusy/Δtotal` 算这段时间窗的占用。
- **`/proc/stat` 哪几项算 busy？** aggregate（全核合计）行 `cpu  user nice system idle iowait irq softirq steal ...`——busy = user + nice + system + irq + softirq + steal + **iowait**（iowait 算 busy 因核被占着等 IO）；total = busy + idle。
- **怎么锁定 aggregate 行、别误匹配单核行？** `/proc/stat` 紧跟 aggregate 行后面还有 `cpu0`/`cpu1`/... 每核一行，**前缀都是 `cpu`**。首行判定用 `startsWith("cpu")` 会连单核行 `cpu0` 一起匹配。要收紧成 `startsWith("cpu ")`——aggregate 行 `cpu` 后**跟空格**，单核行 `cpu0` 紧跟数字、不匹配。读第一行 `readLine()` 后立刻校验前缀。
- **数字字段怎么校验、别静默变 0？** aggregate 行 split 出的字符串逐项 `toLongLong(&ok)`——**必备字段（user/nice/system/idle）必须各自带独立 `ok`、逐个校验**，任一失败即 `return false`。坑：若用**单个共享 `ok`** 串接 8 个 `toLongLong`、最后不再校验，中间任一字段畸形会被后续字段把 `ok` 覆盖成 true，畸形必备字段静默变 0 却判成功（review Qt6 实测复现，CPU% 长期算错且无报错）。可选字段（iowait 起向后）用 `parts.value(n)` 越界返回空串、`toLongLong` 失败→0 即可，不必强校验。
- **⚠️ Windows 路径怎么写？** 内存走 `GlobalMemoryStatusEx`（`MEMORYSTATUSEX` 结构，**调用前必须 `dwLength = sizeof(MEMORYSTATUSEX)`**，否则函数失败；`dwMemoryLoad` 直接给 0..100 占用比）；CPU 走 `GetSystemTimes`（返回 idle/kernel/user 三个 `FILETIME`，**注意 kernel 含 idle**，故 `busy = kernel - idle + user`）。整段用 `#ifdef Q_OS_WIN` 包住，`#include <windows.h>`。**这条路径未实跑，需 Windows 实机复验。**
- **`utilization()` 要不要滚动基线？** 要——算完差值后立刻把本次读数覆盖到 `prev_`（滑动窗口），下一拍算「上一拍到这一拍」，CPU% 跟得上当前负载（否则永远和开机基线比、越来越钝）。
- **两种失败语义怎么区分？** `sample()` 失败 → `has_prev_=false` **丢弃旧基线**（否则下次新读数减陈旧基线得出荒谬值）；`utilization()` 本次 `readCounters` 失败 → **保留旧基线**只返回 -1（下次成功还能用旧基线）。
- **计数回绕要不要防？** 要——理论上单调增，但虚拟化/容器见过跳变。钳制：`d_total <= 0`→-1、`d_busy < 0`→0、`d_busy > d_total`→total。

### 检查点

在 main 里写个测试循环：`readMemoryStats()` 打印 `used_percent`（应是你当前内存占用、30-80% 合理）；构造 `CpuSampler`，`sample()` 后 `QThread::sleep(1)`，再 `utilization()` 打印（应是 0..100 合理值）。**内存数值不离谱、CPU% 随负载变化** = 数据层通了。

> QFile/QTextStream 不熟？先读 [文件与 IO](../../../../../beginner/01-qtbase/08-file-io-beginner.md)。
> QObject 状态机不熟（`CpuSampler` 继承 QObject 存基线）？先读 [QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)。
> 进阶想深挖 QtGlobal 平台宏分发？看 [QtGlobal 平台宏](https://doc.qt.io/qt-6/qtglobal.html)。

### 对照答案

- `MemoryStats` 结构体（字段 + valid 语义）：`demo/system_stats.h:22-32`
- `CpuSampler` 接口（sample/utilization/readCounters + 基线成员）：`demo/system_stats.h:38-60`
- `readMemoryStats` Linux 分支（逐行扫 + MemAvailable + 四舍五入）：`demo/system_stats.cpp:29-62`
- `readMemoryStats` Windows 分支（⚠️ 未验）：`demo/system_stats.cpp:63-75`
- `readMemoryStats` fallback（未覆盖平台）：`demo/system_stats.cpp:76-79`
- `parseKib` helper（冒号→数字→KiB 转字节）：`demo/system_stats.cpp:83-95`
- `sample` 取基线（失败丢旧基线）：`demo/system_stats.cpp:100-113`
- `utilization` 差值计算（滚动基线 + 钳制防御）：`demo/system_stats.cpp:115-139`
- `readCounters` Linux 分支（`/proc/stat` aggregate 解析 + `startsWith("cpu ")` 收紧 + 必备字段逐个校验 + busy/total 拼装）：`demo/system_stats.cpp:142-183`
- `readCounters` Windows 分支（⚠️ 未验，`busy = kernel - idle + user`）：`demo/system_stats.cpp:184-207`
- `readCounters` fallback：`demo/system_stats.cpp:208-212`
- 平台 `#ifdef` 分发（include 隔离）：`demo/system_stats.cpp:13-19`

---

下一步：自绘 CPU 历史曲线（滚动窗口 + QPainter 折线 + 面积填充）——[Step 2 自绘历史曲线](./02-history-chart-paint.md)。
