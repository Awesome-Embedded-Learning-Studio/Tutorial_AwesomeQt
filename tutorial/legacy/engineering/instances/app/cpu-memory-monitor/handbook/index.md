---
title: "CPU / Memory Monitor 手搓手册"
description: "从读 /proc/meminfo 的命令行小程序一行行搓出系统监控窗口：跨平台系统读取（Linux /proc、Windows Win32 API）+ CPU 两次采样差值算瞬时占用 + QProgressBar 进度条 + 自绘 CPU 历史曲线（QPainter 折线+面积填充+滚动窗口）+ QTimer 1s 节拍刷新。"
---

# CPU / Memory Monitor 手搓手册

> **source**：成品答案在 `app/04-system-tools/cpu-memory-monitor/`（做完对照）· **related**：app 栏系统工具类整机成品

::: warning ⚠️ Windows 路径尚未验证
本手册所有 Windows 路径（`GlobalMemoryStatusEx` / `GetSystemTimes`）的描述均基于代码逻辑推断，**未实跑验证**——offscreen 环境跑不了 Windows。如果你在 Linux/WSL 上跟着搓，验证的是 `/proc` 路径；Windows 路径代码照写、编译用 `#ifdef Q_OS_WIN` 隔离，但**需要 Windows 实机复验**。详见 [troubleshooting](./troubleshooting.md)。
:::

::: tip 这是「手搓手册」
不是参考手册（查完走），是 workbook（跟着搓）。每个 step 给**目标 → 提示 → 检查点**，成品 repo 当答案钥匙——卡住了去对照，别整段复制。
:::

## 0. 你将学到

搓完这个系统监控，你会打通这几样 Qt 能力（每样后面都有教程深挖，这里先用起来）：

- **跨平台系统读取**：`#ifdef Q_OS_LINUX / Q_OS_WIN` 分发，Linux 读 `/proc` 文本、Windows 调 Win32 API，未覆盖平台 fallback
- **QFile 文本读取**：`QFile` + `QTextStream` 逐行扫 `/proc/meminfo`、`split` 解析 `/proc/stat` 数字
- **两次采样差值**：理解为什么 CPU 占用率不能单次直读（开机累计值 vs 瞬时占用），用基线 + 差值算
- **QPainter 自绘曲线**：`QPainterPath` 画折线 + 半透明面积填充 + 参考网格 + 滚动窗口
- **QTimer 节拍刷新**：`timeout` 信号驱动周期采样，采样间隔与精度的权衡
- **QProgressBar 进度条**：`setRange(0,100)` + 数值另用 label 显示（已用/总量更直观）
- **无效态降级**：平台不支持或读取失败时显示 N/A、曲线留缺口，不崩不留哑数据
- **QMainWindow 整机装配**：QVBoxLayout + QGroupBox 分组（Memory/CPU）+ 状态栏

## 1. 起点

先有个能读 `/proc/meminfo` 并打印到终端的小程序。最小 Qt 工程（Core 即可，先不画 UI）：

```cpp
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>
int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    QFile f("/proc/meminfo");
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&f);
        qDebug().noquote() << in.readLine();  // 第一行 "MemTotal:   ..."
    }
    return 0;
}
```

能打印出 `MemTotal:   16384000 kB` = `/proc` 读取通了。QFile 不熟先看 [文件与 IO](../../../../../beginner/01-qtbase/08-file-io-beginner.md)，QObject/状态机不熟看 [QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)。

> 注意：工程链 `Qt6::Widgets`（后面要画 UI）；Windows 分支系统调用只在 `Q_OS_WIN` 时才 `#include <windows.h>`，Linux 编译时整段被 `#ifdef` 剔掉。`CMakeLists.txt` 的 `target_link_libraries` 加 `Qt6::Core Qt6::Gui Qt6::Widgets`，Windows 额外加 `if(WIN32) target_link_libraries(... kernel32) endif()`。

## 2. 任务清单

分 3 步（一文件/一阶段一步），每步：**目标 → 提示 → 检查点**。卡住翻 [卡住怎么办](./troubleshooting.md)。

| Step | 目标 | 进 |
|---|---|---|
| 1 | 跨平台系统读取（内存 `/proc/meminfo` + Windows `GlobalMemoryStatusEx`）+ CPU 两次采样差值（`CpuSampler` 状态机） | [01](./01-system-read-and-sampling.md) |
| 2 | 自绘 CPU 历史曲线（滚动窗口 + QPainter 折线 + 面积填充 + 参考网格 + 无效点断开留缺口） | [02](./02-history-chart-paint.md) |
| 3 | 主窗口装配（QProgressBar + QGroupBox 分组）+ QTimer 1s 节拍刷新 + 无效态降级 + Windows kernel32 链接 | [03](./03-window-and-timer.md) |

成品对照：`app/04-system-tools/cpu-memory-monitor/`（按 [成品导览](../) 的「怎么读」顺序对照）。

## 3. 进阶挑战（可选）

搓完基础版想再深一层：

- **每核 CPU**：现在读 `/proc/stat` aggregate（所有核合计）。提示：往下读 `cpu0`/`cpu1`/... 每行，每个核一个 `CpuSampler`，曲线画成多层堆叠。
- **磁盘/网络 IO**：提示：Linux 读 `/proc/diskstats`（磁盘）、`/proc/net/dev`（网络），同样两次采样差值算速率，曲线复用 `CpuHistoryView` 换皮。
- **进程列表**：提示：Linux 遍历 `/proc/[pid]/` 读 `stat`/`status`/`cmdline`，QTableWidget 展示 top-N 进程。
- **macOS 支持**：现在 macOS 走 fallback 显示 N/A。提示：加 `#elif Q_OS_DARWIN`，用 `host_statistics64`（`mach/mach_host.h`）读内存、`host_processor_info` 读 CPU。
- **Windows 实机复验**：⚠️ 现在最大的技术债——Windows 路径代码写好未跑。提示：在 Windows 实机编译运行，验证 `GlobalMemoryStatusEx`（`dwLength` 必须先设）和 `GetSystemTimes`（`busy = kernel - idle + user`），把实测结果回填文档。
- **采样精度自适应**：现在固定 1s。提示：UI 提供档位（500ms / 1s / 2s），动态 `setInterval`，看不同档位下曲线平滑度差异。
- **数据持久化**：提示：把历史采样存 CSV/SQLite，下次启动可回放历史曲线（参考 sqlite-browser 的数据建模）。
- **下一站**：app 栏的 serial-tool / network-tool——换皮复用「周期采样 + 自绘曲线 + QTimer 节拍」骨架，但数据源换成 QSerialPort / QTcpSocket。
