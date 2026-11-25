---
title: "Step 1：配置面板 + 枚举端口 + open/close"
description: "装配配置面板（QFormLayout + 6 个 QComboBox）、QSerialPortInfo::availablePorts 枚举端口（空列表容忍）、QSerialPort 复用实例 open/close、开/关两态控件互斥。"
---

# Step 1：配置面板 + 枚举端口 + open/close

← [手册首页](./index.md) · 下一步 [Step 2 接收 + Hex/ASCII](./02-receive-and-hex-ascii.md) →

## Step 1：配置面板装配 + 枚举端口 + 复用实例 open/close

### 目标

左边一个配置面板（端口 / 波特率 / 数据位 / 停止位 / 校验 / 流控 6 个 combo）+ Open/Refresh 两个按钮；点 Refresh 能重新枚举端口（无硬件时列表为空也不崩）；点 Open 把选中的参数写进 `QSerialPort` 并尝试 open，状态栏显示 Open + 端口名；**再点一次（此时文案变 Close）关掉**，配置控件解锁。

### 提示

- **`port_` 只 new 一次、整生命周期复用**——构造函数里 `port_ = new QSerialPort(this)`，开/关只是 `open`/`close`，不要每次开关都 new/delete。信号槽（readyRead / errorOccurred）在 `wireSerial` 里接驳一次即可。
- 端口列表用 `QSerialPortInfo::availablePorts()` 枚举，每个 `info.portName()` 加进 combo——**无硬件环境返回空列表是正常的**，combo 容忍空，别假设一定有项。
- 填 combo 时 `blockSignals(true)` 防 `currentIndexChanged` 误触发；波特率 combo `setEditable(true)` 让用户能输入自定义值（如嵌入式 250000）。
- **`applyConfigToPort` 只在 open 之前调一次**：把 6 个 combo 的当前选中项映射进 `port_`——baud 用 `currentText().toUInt(&ok)` 解析任意合法值；data_bits/stop_bits/parity/flow_control **一律用 `currentIndex`**（不依赖文案）`switch` 映射到 `QSerialPort::Data5..8` / `OneStop/OneAndHalfStop/TwoStop` / `NoParity/EvenParity/OddParity` / `NoFlowControl/HardwareControl` 枚举——combo `{"8","7","6","5"}` → index 0..3，避免 combo 改可编辑/翻译文案后 `currentText().toInt()` 错映射。
- open 失败要读 `port_->errorString()` 反馈给用户（QMessageBox），别静默。
- **同一按钮两态切换**：`onOpenCloseToggled` 先判 `port_->isOpen()`——开着就走 close 分支、关着就走 open 分支；`refreshOpenControls(bool)` 统一改按钮文案（Open↔Close）+ 锁/解全部配置 combo（开着时锁）。

### 检查点

无硬件也能跑：程序起来 → 端口 combo 为空（或列出的可用端口）→ 点 Open 提示「No port selected. Refresh the port list first.」（如果空）→ 状态栏初始显示 Closed。接上硬件后：Refresh 出现端口名 → 选好参数 → Open → 状态栏 `Port: <name> · Open · RX 0 bytes · TX 0 bytes` → 配置 combo 全变灰 → 点 Close → combo 解锁、状态栏回 Closed。

> QSerialPort / QSerialPortInfo 不熟？先读 [Qt 串口通信](../../../../../beginner/04-qtnetwork/06-serial-port-beginner.md)。
> 信号槽（lambda 写法）不熟看 [Signal / Slot](../../../../../beginner/01-qtbase/02-signal-slot-beginner.md)。

### 对照答案

- `port_` 构造一次复用：`demo/serial_tool_window.cpp:34`
- 配置面板装配（6 combo + QFormLayout + Open/Refresh 按钮）：`demo/serial_tool_window.cpp:53-103`
- availablePorts 枚举 + blockSignals：`demo/serial_tool_window.cpp:183-191`
- onOpenCloseToggled 两态切换（open 失败读 errorString）：`demo/serial_tool_window.cpp:202-239`
- refreshOpenControls 开/关两态锁控件：`demo/serial_tool_window.cpp:241-254`
- applyConfigToPort（combo → QSerialPort 枚举映射，data/stop/parity/flow 一律 currentIndex）：`demo/serial_tool_window.cpp:259-315`
- CMake find SerialPort + 链 Qt6::SerialPort：`demo/CMakeLists.txt:3`, `:11-16`

---

下一步：接上 `readyRead` 把收到的字节流式累积进接收区，再做 Hex/ASCII 切换——[Step 2 接收 + Hex/ASCII](./02-receive-and-hex-ascii.md)。
