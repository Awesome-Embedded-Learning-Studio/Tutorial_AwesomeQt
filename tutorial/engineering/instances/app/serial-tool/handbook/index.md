---
title: "Serial Tool 手搓手册"
description: "从空 QMainWindow 一行行搓出串口调试助手：QSerialPortInfo 枚举端口 + 配置面板、QSerialPort 打开/关闭（复用实例）、readyRead 累积接收、Hex↔ASCII 双向编解码、errorOccurred 屏蔽自身 close 噪声。"
---

# Serial Tool 手搓手册

> **source**：成品答案在 `app/02-network-tools/serial-tool/`（做完对照）· **related**：app 栏网络工具类整机成品

::: tip 这是「手搓手册」
不是参考手册（查完走），是 workbook（跟着搓）。每个 step 给**目标 → 提示 → 检查点**，成品 repo 当答案钥匙——卡住了去对照，别整段复制。
:::

::: warning 真实收发需硬件
串口收发要真实硬件（USB 转串口 / 对端设备 / 虚拟串口对）。本手册的 UI / 配置 / 编解码逻辑可离线搓出来并 offscreen 验；但「打开→收→发」的端到端要接硬件实测。无硬件时，至少把 `refreshPorts`（空列表容忍）和 `applyConfigToPort`（combo→枚举映射）跑通。
:::

## 0. 你将学到

搓完这个串口助手，你会打通这几样 Qt 能力（每样后面都有教程深挖，这里先用起来）：

- **QSerialPort 配置与生命周期**：`setPortName`/`setBaudRate`/`setDataBits`/`setStopBits`/`setParity`/`setFlowControl`，`open(ReadWrite)`/`close`，**实例复用不反复 new/delete**
- **QSerialPortInfo 枚举端口**：`availablePorts()` 实时列系统串口，处理无硬件时的空列表
- **readyRead 异步流式接收**：一次数据可能分多个信号到，`readAll` 累积到 buffer、不假设一次到齐
- **Hex ↔ ASCII 编解码**：接收 `QByteArray::toHex(' ')` 渲染 / `QString::fromLatin1` 保底（**Latin-1 是保底非严格忠实**，CR/控制字符显示失真要切 Hex）；发送 `QByteArray::fromHex` **发送前正向校验防静默截断** / `toUtf8`
- **errorOccurred 屏蔽自身噪声**：主动 close 会触发 errorOccurred，用 flag 屏蔽
- **QMainWindow 整机装配**：QHBoxLayout 分区 / QFormLayout 配置表单 / QRadioButton 切换 / QStatusBar 计数
- **配置控件开/关两态互斥**：开着锁配置、关掉解锁，防运行中改参数

## 1. 起点

先有个能跑的空主窗口。最小 Qt Widgets 工程，main 里弹个 QMainWindow：

```cpp
#include <QApplication>
#include <QMainWindow>
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QMainWindow w;
    w.resize(820, 560);
    w.show();
    return app.exec();
}
```

弹出空白主窗口 = 环境通了。QMainWindow 不熟先看 [QMainWindow 主窗口](../../../../../beginner/03-qtwidgets/55-qmainwindow-beginner.md)。

> 注意：本目录 `CMakeLists.txt` 要单独 `find_package(Qt6 REQUIRED COMPONENTS SerialPort)` 并链 `Qt6::SerialPort`——顶层 app/CMakeLists 只提供 Core/Gui/Widgets，SerialPort 是额外组件，不 find 不链的话 `#include <QSerialPort>` 都编译不过。

## 2. 任务清单

分 3 步（一文件一步），每步：**目标 → 提示 → 检查点**。卡住翻 [卡住怎么办](./troubleshooting.md)。

| Step | 目标 | 进 |
|---|---|---|
| 1 | 配置面板装配 + QSerialPortInfo 枚举端口 + 复用实例 open/close | [01](./01-config-and-open-port.md) |
| 2 | readyRead 流式累积接收 + Hex/ASCII 切换重渲 | [02](./02-receive-and-hex-ascii.md) |
| 3 | 发送（Hex 发送前正向校验防 fromHex 静默截断 / ASCII）+ errorOccurred 屏蔽自身 close 噪声 + 不可恢复错误收敛 | [03](./03-send-and-error-handling.md) |

成品对照：`app/02-network-tools/serial-tool/`（按 [成品导览](../) 的「怎么读」顺序对照）。

## 3. 进阶挑战（可选）

搓完基础版想再深一层：

- **定时发送**：现在只能手点 Send。提示：加个 `QTimer`（间隔可配），tick 时调 `onSend`；状态栏加 Start/Stop。
- **多串口同时打开**：现在只一个 `port_`。提示：把配置 + port 封成 `SerialSession`，主窗口持 `QList<SerialSession*>`，标签页管理。
- **协议帧解析**：现在只显示裸字节流。提示：在 `appendReceive` 里按定长头/尾切帧，命中完整帧才高亮一行。
- **收发日志落盘**：把收发数据带时间戳写文件。提示：`QFile` + `QTextStream`，按 `QDateTime::currentDateTime` 打戳。
- **数据曲线**：把收到的数值画成实时曲线。提示：QtCharts 的 `QLineSeries` + `QChartView`，readyRead 里 push 新点。
- **下一站**：app 栏的 sqlite-browser / image-viewer——换皮复用 QMainWindow 整机骨架，但引入 QtSql / 自定义绘制。
