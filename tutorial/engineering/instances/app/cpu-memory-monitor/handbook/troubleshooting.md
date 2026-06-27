---
title: "卡住怎么办"
description: "按症状查：CPU% 恒定不变、Windows 路径未验、内存占用率虚高、采样失败 CPU% 飙升、出现负占用/>100%、采样间隔太短狂抖、平台不支持崩溃、曲线无效点画到 0、曲线窗口未满挤左——给方向指向教程章，不直接给答案。"
---

# 卡住怎么办

← [手册首页](./index.md)

按症状查。每条给方向，不给整段答案——成品 repo 在 `app/04-system-tools/cpu-memory-monitor/`，对照着看。

::: warning ⚠️ Windows 路径未验
本仓库 offscreen CI 在 Linux/WSL 跑，**只验证了 Linux `/proc` 路径**。Windows 分支（`GlobalMemoryStatusEx` / `GetSystemTimes`）代码已写、`#ifdef Q_OS_WIN` 编译隔离，**但从未编译过、未实跑**。若你在 Windows 实机遇到问题，多半是这些路径的逻辑未经验证——见下方「Windows 路径行为未知」。
:::

## CPU% 恒定不变 / 显示成开机累计值

- `utilization()` 是不是没用差值、直接返回了 `/proc/stat` 的累计计数？那是开机以来的累计，不是瞬时占用。
- 要 `sample()` 存基线，`utilization()` 用 `Δbusy/Δtotal` 算。→ `demo/system_stats.cpp:100-113`、`115-139`
- 窗口构造时有没有先 `cpu_sampler_.sample()` 拿基线？没基线第一拍算不出。→ `demo/system_monitor_window.cpp:37`
- 进阶排查：[QTimer 定时器](../../../../../beginner/01-qtbase/11-timer-beginner.md)

## CPU% 长期偏低甚至恒 0，且无任何报错（字段畸形被静默吞掉）

- `readCounters` 是不是用**单个共享 `bool ok`** 串接所有 `toLongLong`、最后不再校验？中间任一字段（user/nice/system/idle）畸形会被后续字段把 `ok` 覆盖成 true，畸形必备字段静默变 0 却判采样成功（review Qt6 实测复现）。
- 必备字段（user/nice/system/idle）要**逐个独立 `ok`**、任一失败即 `return false`；可选字段（iowait 起向后）越界→0 即可。→ `demo/system_stats.cpp:161-179`

## readCounters 拿到的是单核（cpu0）而非全核合计（aggregate）

- 首行判定是不是用了 `startsWith("cpu")`？它连单核行 `cpu0`/`cpu1` 也匹配（前缀都是 `cpu`）。
- 要收紧成 `startsWith("cpu ")`——aggregate 行 `cpu` 后跟空格，单核行 `cpu0` 紧跟数字不匹配。→ `demo/system_stats.cpp:151-153`

## Windows 路径行为未知 / 实机崩溃或返回错值（⚠️ 未验）

- Windows 分支整段被 `#ifdef Q_OS_WIN` 隔离，Linux CI 既不编译也不运行，**代码从未编译过、未实跑**。
- 内存读取失败？`MEMORYSTATUSEX` 调用前**必须 `dwLength = sizeof(MEMORYSTATUSEX)`**，不设函数失败返回 0。→ `demo/system_stats.cpp:66-68`
- CPU 算错？`GetSystemTimes` 的 `kernel_time` **含 idle**，故 `busy = kernel - idle + user`，不能直接 `busy = kernel + user`（会双计 idle）。→ `demo/system_stats.cpp:203-206`
- Link 报错找不到 `GlobalMemoryStatusEx`/`GetSystemTimes`？CMake 要 `if(WIN32) target_link_libraries(... PRIVATE kernel32) endif()`。→ `demo/CMakeLists.txt:21-23`
- **这些都需要你在 Windows 实机复验后回填**。参考 [GlobalMemoryStatusEx](https://learn.microsoft.com/windows/win32/api/sysinfoapi/nf-sysinfoapi-globalmemorystatusex)、[GetSystemTimes](https://learn.microsoft.com/windows/win32/api/processthreadsapi/nf-processthreadsapi-getsystemtimes)。

## 内存占用率高得离谱（90%+ 常态）

- 用的是 `MemFree` 吗？`MemFree` 把 buffer/cache 当成占用，**高估占用率**。
- 要用 `MemAvailable`（3.3+ 内核，含可回收缓存）。→ `demo/system_stats.cpp:46-49`
- `MemAvailable` 只在 3.3+ 内核有，极旧内核缺失时退而求其次用 `MemFree`（但要知道不准）。

## 采样失败后下一拍 CPU% 突然飙到 100% / 负数

- `sample()` 失败有没有丢旧基线？没丢的话，下次 `utilization()` 拿全新读数减陈旧基线，算出荒谬值。
- `sample()` 失败要 `has_prev_ = false` 丢旧基线；`utilization()` 本次失败要**保留旧基线**只返回 -1。→ `demo/system_stats.cpp:105-107`、`121-123`
- `onTick` 见 `utilization()` 返回 -1 要补一次 `sample()` 重建基线。→ `demo/system_monitor_window.cpp:99-103`

## 多核 / 容器 / 虚拟机下出现负占用或 >100%

- CPU 计数理论上单调增，但虚拟化/容器曾见计数跳变/回绕，差值越界。
- 要钳制：`d_total <= 0`→-1、`d_busy < 0`→0、`d_busy > d_total`→total。→ `demo/system_stats.cpp:129-137`

## 采样间隔太短，CPU% 狂抖不可读

- 间隔是不是设成了 100ms？两次采样 total 差值太小，一点调度抖动放大成几十百分点跳变。
- 取 1s 间隔。→ `demo/system_monitor_window.cpp:40`
- 别开 `PreciseTimer`——系统读取本身有耗时，精确定时器无意义还耗能。→ `demo/system_monitor_window.cpp:41`
- 进阶排查：[QTimer 进阶](../../../../../advanced/01-qtbase/11-qtimer-advanced.md)

## 平台不支持（macOS）/ 读取失败时窗口崩溃

- 没做无效态降级？直接用未初始化值或裸指针会崩。
- 未覆盖平台要 fallback `valid=false` / 返回 -1；UI 层见无效态显示 N/A。→ `demo/system_stats.cpp:76-79`、`208-212`、`demo/system_monitor_window.cpp:107-110`、`121-125`
- 要支持 macOS，加 `#elif Q_OS_DARWIN`（`host_statistics64` / `host_processor_info`）。

## CPU 曲线无效点画到 0，看着像 CPU 突然空载

- `paintEvent` 对 -1 点没断笔，直接 `lineTo` 到 y=0 了？
- 无效点要跳过、不参与折线；按「连续有效段」分组，每段单独画折线，缺口处折线自然断开留缺口。→ `demo/cpu_history_view.cpp:60-83`

## CPU 曲线缺口下方多出一片半透明填充，折线说没数据、填充却涂了色

- 面积填充是不是把**整条路径**闭合到底边？那样缺口下方照样涂色，折线与填充语义矛盾。
- 要按「连续有效段」分组，**每段单独**复制折线、下到段首闭合画填充，缺口下不画填充；折线与填充同段同断。→ `demo/cpu_history_view.cpp:84-89`

## CPU 曲线窗口未满时挤在左边 / 单点不画

- x 坐标是不是按 `capacity_` 等分？样本数少于 capacity 就会挤左。
- x 要按**已有样本数**等分铺满宽度（`dx = w / (size-1)`）。→ `demo/cpu_history_view.cpp:56-58`
- 折线至少要两个点，某段有效点 < 2 跳过该段（单点不画是合理的）。→ `demo/cpu_history_view.cpp:75-77`

## 进度条数值和曲线对不上 / label 显示乱码

- `refreshMemory`/`refreshCpu` 有没有按 `valid`/`percent >= 0` 分支处理？无效态没降级会显示乱。
- 内存 label 用 `QString("%1% (%2 / %3)").arg(used_percent).arg(formatGb(used)).arg(formatGb(total))`。→ `demo/system_monitor_window.cpp:106-118`
- `formatGb` 用 `'f', 2` 两位小数，别用默认科学计数。→ `demo/system_monitor_window.cpp:22-25`

## 进程列表（进阶挑战）读不到 `/proc/[pid]`

- 现在只读 `/proc/stat`（aggregate），不读进程。进阶挑战里若遍历 `/proc/[pid]/` 读 `stat`/`cmdline`，注意权限（部分系统进程 `stat` 读不到，要跳过）。
- 这是本成品未实现的扩展，参考 [proc 文档](https://man7.org/linux/man-pages/man5/proc.5.html)。

## moc 报错 / Q_OBJECT 相关

- `CpuSampler`/`CpuHistoryView`/`SystemMonitorWindow` 头里**有没有 Q_OBJECT**？`CpuSampler` 继承 QObject、后两者有信号槽相关，必须有。→ `demo/system_stats.h:39`、`cpu_history_view.h:16`、`system_monitor_window.h:25`
- CMake **开了 AUTOMOC 吗**？`qt_add_executable` 默认开，但手写 `add_executable` 要 `set(CMAKE_AUTOMOC ON)`。
- 进阶排查：[QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)
