---
title: "Step 3：主窗口装配 + QTimer 1s 节拍刷新"
description: "SystemMonitorWindow 整机装配：QVBoxLayout + QGroupBox 分组（Memory/CPU）+ QProgressBar 进度条（数值另用 label 显示已用/总量 GB）+ QTimer 1s 节拍（timeout→onTick 读内存+CPU%、-1 补 sample 重建基线）+ 无效态降级（N/A + 曲线缺口）+ Windows kernel32 链接（if(WIN32) 守门）。"
---

# Step 3：主窗口装配 + QTimer 1s 节拍刷新

← 上一步 [Step 2 自绘曲线](./02-history-chart-paint.md) · [手册首页](./index.md) →

## Step 3：SystemMonitorWindow 整机装配 + 节拍刷新

### 目标

把 Step 1 的数据层 + Step 2 的曲线控件，用 `SystemMonitorWindow : public QMainWindow` 装成整机：中央区两个 `QGroupBox`（Memory 行：进度条 + label；CPU 行：进度条 + label + 历史曲线），`QTimer` 每 1s `timeout` → `onTick()` 读内存刷进度条、读 CPU% 刷进度条和曲线，无效态显示 N/A + 曲线留缺口。构造时先 `cpu_sampler_.sample()` 拿基线，第一个 tick 才有有效 CPU%。跑起来是一个能持续刷新的监控窗口。

### 提示

- **布局怎么搭？** `QMainWindow` 中央放一个 `QWidget`，根 `QVBoxLayout`（`contentsMargins(12,12,12,12)` + `spacing(12)`）。Memory 分组：`QGroupBox("Memory")` 内 `QVBoxLayout`，一行 `QHBoxLayout`（进度条 stretch=1 + label）。CPU 分组同理，但 VBox 里多加一行 `CpuHistoryView`。底部 `addStretch(1)` 顶上去。
- **进度条数值显示在哪？** `QProgressBar::setTextVisible(false)`——别用进度条自带的百分比文字，另开 `QLabel` 显示「75% (12.34 / 16.00 GB)」更直观（内存要显示已用/总量，进度条文字塞不下）。`setRange(0,100)` + `setValue(used_percent)`。
- **字节怎么转 GB？** 写个 `formatGb` helper（匿名 namespace 里）：`QString::number(bytes / (1024.0*1024.0*1024.0), 'f', 2) + " GB"`，2 位小数。
- **QTimer 怎么装配？** 构造里 `timer_ = new QTimer(this)`、`setInterval(1000)`（**1s，别太短**，100ms 会让 CPU% 狂抖不可读）、`connect(timeout, onTick)`、`start()`。**不开 `PreciseTimer`**——系统读取（`/proc`/Win32）本身有毫秒级耗时，精确定时器无意义还耗能，默认 `CoarseTimer` 足够。
- **CPU 基线什么时候拿？** 构造里、Timer `start()` 之前调一次 `cpu_sampler_.sample()`——CPU% 要差值，没基线第一拍算不出。`start()` 后**立即调一次 `onTick()`** 先刷内存（CPU% 第一拍差值还没意义，等下个 tick）。
- **`onTick` 一拍做什么？** ① `readMemoryStats()` → `refreshMemory`（无效态显示 N/A）② `cpu_sampler_.utilization()` 拿 CPU%——**返回 -1 时补一次 `sample()` 重建基线**（保证下拍能算；utilization 提前返回时不会动 prev_）③ `refreshCpu`（无效态进度条归零 + label N/A + 曲线 push(-1)）。
- **无效态降级怎么做？** `refreshMemory`：`!s.valid` 时进度条 `setValue(0)` + label `"N/A"` + return；`refreshCpu`：`percent < 0` 时进度条归零 + label N/A + `cpu_history_->push(-1)`（曲线留缺口）+ return。
- **⚠️ CMake 怎么配 Windows 链接？** `qt_add_executable` + 链 `Qt6::Core/Gui/Widgets`；**`if(WIN32) target_link_libraries(... PRIVATE kernel32) endif()`**——只在 Windows 编译时才链 kernel32（Linux 编译时 Windows 代码被 `#ifdef` 剔掉，此链接不触发，不会因找不到 kernel32 报错）。加 `WIN32_EXECUTABLE ON` 不弹控制台。

### 检查点

跑起来（Linux/WSL）：窗口标题 `CPU / Memory Monitor`，Memory 分组进度条随占用变化、label 显示「xx% (xx.xx / xx.xx GB)」，CPU 分组进度条每秒刷新、曲线从左向右铺开滚动，状态栏 `Sampling every 1 s`。**狂跑别的程序看 CPU% 和曲线上升** = 整机通了。**断网/改坏路径测试 N/A 降级**。⚠️ **Windows 路径需实机复验**。

> QTimer 不熟？先读 [QTimer 定时器](../../../../../beginner/01-qtbase/11-timer-beginner.md)（进阶看 [QTimer 进阶](../../../../../advanced/01-qtbase/11-qtimer-advanced.md)）。
> QProgressBar 不熟？先读 [QProgressBar](../../../../../beginner/03-qtwidgets/35-qprogressbar-beginner.md)。
> QMainWindow 装配不熟？先读 [QMainWindow 主窗口](../../../../../beginner/03-qtwidgets/55-qmainwindow-beginner.md)。
> QGroupBox 不熟？先读 [QGroupBox](../../../../../beginner/03-qtwidgets/38-qgroupbox-beginner.md)。

### 对照答案

- `SystemMonitorWindow` 接口（6 控件指针 + cpu_sampler_ + timer_ + onTick slot）：`demo/system_monitor_window.h:24-48`
- `formatGb` helper（字节→GB 2 位小数）：`demo/system_monitor_window.cpp:22-25`
- 构造（装配 + sample 基线 + QTimer 1s + 立即 onTick）：`demo/system_monitor_window.cpp:28-46`
- `setupCentral`（QVBoxLayout + 两 QGroupBox + 进度条 setTextVisible(false) + 曲线）：`demo/system_monitor_window.cpp:48-85`
- `setupStatusBar`：`demo/system_monitor_window.cpp:87-90`
- `onTick`（读内存 + CPU% -1 补 sample + 刷新）：`demo/system_monitor_window.cpp:92-104`
- `refreshMemory`（无效态降级 + formatGb label）：`demo/system_monitor_window.cpp:106-118`
- `refreshCpu`（无效态降级 + 曲线 push）：`demo/system_monitor_window.cpp:120-130`
- `main`（applicationName + show）：`demo/main.cpp:11-17`
- CMake（qt_add_executable + if(WIN32) 链 kernel32 + WIN32_EXECUTABLE）：`demo/CMakeLists.txt:3-29`

---

搓完了。回 [手册首页](./index.md) 看进阶挑战（每核 CPU / 磁盘网络 IO / macOS 支持 / **Windows 实机复验**），或回 [成品导览](../) 对照「怎么读」复盘架构。⚠️ 别忘了 Windows 路径（`GlobalMemoryStatusEx` / `GetSystemTimes`）是本成品最大的技术债——代码写好未跑，需 Windows 实机复验后回填文档。
