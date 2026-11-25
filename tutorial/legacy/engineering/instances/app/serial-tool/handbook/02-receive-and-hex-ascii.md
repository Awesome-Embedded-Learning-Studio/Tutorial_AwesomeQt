---
title: "Step 2：readyRead 累积接收 + Hex/ASCII 切换"
description: "readyRead 流式累积接收（一次数据可能分多片到，readAll 累积到 buffer 再渲染新增）、Hex/ASCII 纯显示层切换（整段 buffer 重渲）、appendPlainText vs insertPlainText 选型。"
---

# Step 2：readyRead 累积接收 + Hex/ASCII 切换

← [Step 1 配置 + open/close](./01-config-and-open-port.md) · 下一步 [Step 3 发送 + 错误屏蔽](./03-send-and-error-handling.md) →

## Step 2：readyRead 流式累积接收 + Hex/ASCII 切换

### 目标

右上只读接收区（QPlainTextEdit）能流式显示收到的字节；Hex/ASCII 两个 QRadioButton 切换显示模式——**切换时已收的整段数据按新模式重渲**，不留旧模式残段；状态栏 RX 计数累加。

### 提示

- **一次数据可能分多个 readyRead 到**——串口是字节流不是消息，OS 把到达字节切成不定大小的片递给 Qt，每片一个 readyRead。`onReadyRead` 里 `readAll()` 只拿到**本次片段**，绝不能假设「一个 readyRead = 一条完整消息」。
- 必须累积：维护一个 `QByteArray receive_buffer_`，每次 `readAll` 的片段 `append` 进去；**渲染时只渲染本次新增片段**追加进接收区。
- `rx_bytes_ += chunk.size()` 累计字节计数。
- 流式追加用 `moveCursor(QTextCursor::End)` + `insertPlainText(rendered)`——**别用 `appendPlainText`**，它会在每次追加前强制插一个换行，把连续字节流切断。
- **Hex/ASCII 是纯显示层**：`receive_buffer_` 永远存原始字节。`renderChunk(bytes, hexMode)`：Hex 走 `bytes.toHex(' ')`（每字节两 hex + 空格分隔）、ASCII 走 `QString::fromLatin1(bytes)`（Latin-1 保底，`0x00`/`0xFF` 不丢字节也不抛异常——**但非严格字节忠实**：CR(`0x0D`) 在 QPlainTextEdit 会回车覆盖、NUL/控制字符显示失真，要逐字节精确看请切 Hex）。
- 切 Hex/ASCII 时 `refreshReceiveDisplay` 用**整段** `receive_buffer_` 按新模式 `setPlainText` 重渲——别只渲新数据，否则历史段留在旧模式。
- 接收区 `setReadOnly(true)`、`setPlaceholderText` 占位提示。

### 检查点

无硬件时可 offscreen 验：构造时塞一段假字节进 `receive_buffer_` 调 `refreshReceiveDisplay`，看接收区按 ASCII 显示 → 切 Hex 看变 `48 65 6C 6C 6F` → 切回 ASCII 看整段重渲无残留。接硬件后：对端发 `Hello`，ASCII 模式收到 `Hello`、Hex 模式收到 `48 65 6C 6C 6F`、状态栏 RX 累加 5。**反复切 Hex/ASCII，整段始终一致无残段** = buffer 累积 + 整段重渲做对了。

> QByteArray::toHex / fromLatin1 不熟？看 [字符串与编码](../../../../../beginner/01-qtbase/03-string-encoding-beginner.md)。
> QPlainTextEdit 流式追加（moveCursor + insertPlainText）看 [QPlainTextEdit](../../../../../beginner/03-qtwidgets/24-qplaintextedit-beginner.md)。

### 对照答案

- readyRead → readAll → appendReceive：`demo/serial_tool_window.cpp:320-323`
- appendReceive 累积 + 渲染新增：`demo/serial_tool_window.cpp:325-336`
- renderChunk（Hex toHex(' ') / ASCII fromLatin1 保底非严格忠实）：`demo/serial_tool_window.cpp:345-355`
- refreshReceiveDisplay 切模式整段重渲：`demo/serial_tool_window.cpp:338-343`
- 接收区装配（只读 + placeholder + Hex/ASCII radio）：`demo/serial_tool_window.cpp:104-121`

---

下一步：发送区（Hex 发送前正向校验防 fromHex 静默截断 / ASCII），再把 errorOccurred 接上并屏蔽自家 close 的噪声——[Step 3 发送 + 错误屏蔽](./03-send-and-error-handling.md)。
