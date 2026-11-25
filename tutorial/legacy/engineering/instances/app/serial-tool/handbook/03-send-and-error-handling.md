---
title: "Step 3：发送 + errorOccurred 屏蔽"
description: "发送区装配（QLineEdit + Send，回车也触发）、Hex 发送前正向校验防 fromHex 静默截断（去空白后全 hex 且长度偶数）/ ASCII toUtf8、write 返回值判负 + flush、errorOccurred 接状态栏（errorToString 自带映射避免 errorString stale）+ suppress_error_ 屏蔽主动 close 的噪声 + 不可恢复错误主动 close 收敛。"
---

# Step 3：发送 + errorOccurred 屏蔽

← [Step 2 接收 + Hex/ASCII](./02-receive-and-hex-ascii.md) · [手册首页](./index.md)

## Step 3：发送 + errorOccurred 屏蔽

### 目标

右下发送区（QLineEdit + Send，Send as 有 Hex/ASCII 切换）：点 Send 或回车把文本按当前模式编码后 `write` 出去，TX 计数累加（入队字节）；Hex 输入非法（如 `4G`）报错不静默（**发送前正向校验防 fromHex 静默截断**）；`errorOccurred`（拔线/占用/权限）落状态栏带错误码；**主动 Close 端口时不报噪声**；不可恢复错误主动 close 收敛到关闭态。

### 提示

- 发送按钮 + `QLineEdit::returnPressed` 都接 `onSend`——回车也能发。
- **Hex 模式必须发送前正向校验**：用户输入 `"48 65 6C 6C 6F"`，目标解成字节。但 `QByteArray::fromHex` 对非 hex 字符是**跳过（continue）而非报错**（实证 `qbytearray.cpp:4641`）——混入非法字符会静默截断字节流，奇数长度还会吞掉最高位半字节。所以**先校验再 fromHex**：把 `text` 去掉空白得到 `compact`，必须全是 `[0-9a-fA-F]`（`std::all_of` + `is_hex_digit`）且长度偶数——任一不满足直接报「invalid hex input」不发。空输入（`text.isEmpty()`）直接 return。
- **ASCII 模式**：`text.toUtf8()`。
- `port_->write(payload)` 返回的是**入队字节**（Qt 内部缓冲，非物理发出），**返回负值表示失败**——读 `port_->errorString()` 报状态栏。write 后调 `port_->flush()` 把字节推到驱动层，否则 close 时 pending 字节可能丢。`tx_bytes_ += written` 累加（注明是入队字节，严格物理发出量需 `bytesWritten` 信号）。
- 端口没开就发：状态栏提示「Cannot send: port is closed」，return。
- **errorOccurred 屏蔽 + 不可恢复错误收敛**：维护 `bool suppress_error_`。正常 close 一个 `QSerialPort`，部分平台会顺带发一个 errorOccurred（如 ResourceError）——这是关闭副产物不是真错误，置 `suppress_error_` 屏蔽。`onPortError` 里 `error == NoError` 直接 return，`suppress_error_` 为真也 return，否则才报错。**关键**：对 `ResourceError`/`OpenError`/`PermissionError`/`DeviceNotFoundError` 这类不可恢复错误（拔线/被占用/权限/找不到），`isOpen()` 可能仍 true——必须主动 `close` 收敛到关闭态，否则卡在半开态，后续 Send 静默失败、配置控件锁死无法自救。
- 错误文案用 error 码自带映射 `errorToString(error)`，**不依赖 `errorString()`**——`errorOccurred` 是排队派发到槽的，等槽跑时 `errorString()` 可能已被后续操作覆盖（stale）。错误码用 `static_cast<int>(error)` 显示，方便对照 `QSerialPort::SerialPortError` 枚举值排查。

### 检查点

无硬件可 offscreen 验编解码：构造一段 hex 字符串喂给解码逻辑，确认 `48 65 6C 6C 6F` 解出 `Hello`、`4G` 报 invalid hex、空输入静默。接硬件后：ASCII 模式发 `Hello` 对端收到 `Hello`、Hex 模式发 `48 65 6C 6C 6F` 对端收到同样 5 字节、状态栏 TX 累加 5；**正常 Close 端口状态栏不刷错误噪声**；拔线后状态栏出现 `Port error [<code>]: ...`。

> QSerialPort::write / errorOccurred 不熟？看 [Qt 串口通信](../../../../../beginner/04-qtnetwork/06-serial-port-beginner.md)。
> QByteArray::fromHex 对非 hex 字符是「跳过」而非报错（会静默截断），看官方 [QByteArray 文档](https://doc.qt.io/qt-6/qbytearray.html#fromHex)。

### 对照答案

- 发送区装配（QLineEdit + Send + Hex/ASCII radio + returnPressed 接驳）：`demo/serial_tool_window.cpp:123-145`, `:159-162`
- onSend（Hex 发送前正向校验防 fromHex 静默截断 + ASCII toUtf8 + write 判负 + flush）：`demo/serial_tool_window.cpp:360-403`
- onPortError（NoError / suppress_error_ 屏蔽 + errorToString 自带映射 + 不可恢复错误主动 close 收敛）：`demo/serial_tool_window.cpp:408-431`
- close 分支置 suppress_error_：`demo/serial_tool_window.cpp:203-211`, `:220-224`
- updateStatus（状态栏 Open/Closed + RX/TX 计数）：`demo/serial_tool_window.cpp:451-458`

---

搓完了。回 [手册首页](./index.md) 看进阶挑战（定时发送 / 多串口 / 协议帧解析 / 收发日志落盘 / 数据曲线），或对照 [成品导览](../) 复盘整体架构。
