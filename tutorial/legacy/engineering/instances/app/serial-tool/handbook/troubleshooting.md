---
title: "卡住怎么办"
description: "按症状查：QSerialPort 编译/链接不过、readyRead 收到数据缺一截、切 Hex/ASCII 历史段残留、ASCII 模式丢字节（Latin-1 保底非严格忠实）、close 后状态栏刷错误噪声、开着端口改 combo 没生效、Hex 混入非法字符静默截断、open 失败无提示、无硬件端口列表为空、自定义波特率没法输入、dataBits 文案错映射、拔线后半开态 Send 静默失败、TX 入队虚高 close 丢字节、errorString stale 文案对不上——给方向指向教程章，不直接给答案。"
---

# 卡住怎么办

← [手册首页](./index.md)

按症状查。每条给方向，不给整段答案——成品 repo 在 `app/02-network-tools/serial-tool/`，对照着看。

## `QSerialPort: No such file` / 链接 `undefined reference to QSerialPort`

- 顶层 app/CMakeLists 只 `find_package` 了 Core/Gui/Widgets——**SerialPort 是额外组件**，必须单独 find + 链。→ `demo/CMakeLists.txt:3`（find）+ `:11-16`（link Qt6::SerialPort）。
- 进阶排查：[Qt Serial Port 模块](https://doc.qt.io/qt-6/qtserialport-index.html)

## readyRead 收到数据「缺一截」/ 按定长切包粘包截断

- 是不是把每次 `readAll` 的结果当「一条完整消息」了？串口是字节流，OS 一批数据可能分多个 readyRead 到。
- 必须 `append` 进 `receive_buffer_` 累积，只渲染**本次新增**片段。→ `demo/serial_tool_window.cpp:320-336`
- 进阶排查：[Qt 串口通信](../../../../../beginner/04-qtnetwork/06-serial-port-beginner.md)

## 切 Hex/ASCII 后，历史段还留在旧模式（接收区半 Hex 半 ASCII）

- 切模式只渲染了「之后」的新数据，没重渲已收的历史段。
- `refreshReceiveDisplay` 要用**整段** `receive_buffer_` 按新模式 `setPlainText` 重渲。→ `demo/serial_tool_window.cpp:338-343`
- `receive_buffer_` 存的是不是**原始字节**？别把渲染后的字符串存进 buffer，否则切模式没法重渲。

## ASCII 模式收到 `0x00`/`0xFF` 丢字符 / 乱码 / 长度对不上

- 是不是直接 `QString(bytes)` 走 UTF-8 解码了？非法字节序列会被替换或截断，丢字节。
- ASCII 模式用 `QString::fromLatin1(bytes)` 逐字节保底映射，不丢字节不抛异常。**但 Latin-1 是保底非严格忠实**：CR(`0x0D`) 会回车覆盖、NUL/控制字符在 QPlainTextEdit 显示失真——要逐字节精确看请切 Hex。→ `demo/serial_tool_window.cpp:345-355`
- 进阶排查：[字符串与编码](../../../../../beginner/01-qtbase/03-string-encoding-beginner.md)

## 正常 close 端口后状态栏被刷成「Port error [...]」

- 部分 close 路径会顺带触发 `errorOccurred`（如 ResourceError），是关闭副产物不是真错误。
- close 前后有没有置 `suppress_error_ = true/false`？`onPortError` 检测到 flag 直接 return。→ `demo/serial_tool_window.cpp:203-211`, `:408-414`

## 开着端口时改了 combo，参数没生效 / 改端口名崩了

- `applyConfigToPort` 只在 open 前调一次；运行中 combo 还能编辑就会让用户误以为改了。
- `refreshOpenControls` 开端口时要**锁全部配置 combo + 刷新按钮**，关掉才解锁。→ `demo/serial_tool_window.cpp:241-254`

## Hex 发送框输入 `"48 65 6C 6C 6F"` 混入非法字符（如 `4G`）后，发出去的是被静默截断的字节流而非报错

- `QByteArray::fromHex` 对非 hex 字符是**跳过（continue）**而非报错（实证 `qbytearray.cpp:4641`），混入非法字符会静默丢字节；奇数长度还会吞掉最高位半字节——肉眼对不上、调试被误导。
- **发送前正向校验**：去空白后必须全是 `[0-9a-fA-F]`（`std::all_of` + `is_hex_digit`）且长度偶数，任一不满足直接报「invalid hex input」不发。→ `demo/serial_tool_window.cpp:371-389`
- 进阶排查：[字符串与编码](../../../../../beginner/01-qtbase/03-string-encoding-beginner.md)

## 打开失败没提示 / 用户不知道为啥开不了

- `port_->open(ReadWrite)` 返回 false 后有没有读 `port_->errorString()` 反馈？
- open 失败用 QMessageBox 带「端口名 + errorString」。→ `demo/serial_tool_window.cpp:228-234`

## 无硬件环境下端口列表为空，程序卡住 / 用户以为坏了

- `QSerialPortInfo::availablePorts` 在没接串口硬件时返回**空列表**——这是正常现象不是 bug。
- combo 容忍空列表，配「Refresh ports」按钮；无端口时打开要提示「No port selected. Refresh the port list first.」。→ `demo/serial_tool_window.cpp:183-191`, `:213-217`

## 波特率只能选预设档位，自定义值（如 250000）没法用

- 波特率 combo 是不是 `setEditable(false)` 写死了档位？
- combo `setEditable(true)`，`applyConfigToPort` 用 `currentText().toUInt(&ok)` 解析任意合法值。→ `demo/serial_tool_window.cpp:59-62`, `:261-264`

## moc 报错 / QSerialPort 信号槽接不上

- `SerialToolWindow` 头里**有没有 Q_OBJECT**？（本例用了自定义槽，必须有）
- CMake **开了 AUTOMOC 吗**？（`qt_add_executable` 默认开，但确认下）
- 信号槽写法是不是用了**新式 `connect`**（函数指针式）？`connect(port_, &QSerialPort::readyRead, this, &SerialToolWindow::onReadyRead)`。→ `demo/serial_tool_window.cpp:173-178`
- 进阶排查：[Signal / Slot](../../../../../beginner/01-qtbase/02-signal-slot-beginner.md)

## 数据位选「7」实际开成了「8」/ 翻译或文案改动后参数错映射

- dataBits 是不是用 `currentText().toInt()` 按文案匹配的？combo 改可编辑、翻译文案变了、或文案带空格时 `toInt` 会解析偏离。
- 统一用 `currentIndex` 映射：combo `{"8","7","6","5"}` → index 0..3，与 stop/parity/flow 同口径，不依赖文案。→ `demo/serial_tool_window.cpp:268-281`

## 拔线 / 被占用 / 权限错误后端口卡在「Open」、Send 静默失败、配置控件锁死无法自救

- 拔线/被占用/权限错误后 `port_->isOpen()` 可能**仍 true**——`errorOccurred` 触发了但没收端口，留在半开态。
- 对 `ResourceError`/`OpenError`/`PermissionError`/`DeviceNotFoundError` 这类不可恢复错误，`onPortError` 要**主动 close 收敛到关闭态**（顺带置 `suppress_error_` 防自身 close 噪声），刷新控件到关闭态。→ `demo/serial_tool_window.cpp:419-430`

## Send 后 TX 计数加了、但 close 端口时字节没真正发出 / 真实物理发出量不明

- `QSerialPort::write` 返回的是**入队字节**（Qt 内部缓冲），不是物理发出字节；不 flush 直接 close 可能丢 pending 字节。
- write 后调 `flush()` 把字节推到驱动层；TX 计数注明是入队字节，严格物理发出量需用 `bytesWritten` 信号。→ `demo/serial_tool_window.cpp:394-401`

## 状态栏错误文案与实际错误对不上（显示的是上一次的 errorString）

- `errorOccurred` 是**排队派发**到槽的，等槽跑时 `errorString()` 可能已被后续操作覆盖（stale）。
- 用 error 码自带映射 `errorToString(error)`（不依赖二次查询的 errorString），状态栏带错误码 `[N]` 方便对照枚举。→ `demo/serial_tool_window.cpp:415-417`, `:433-446`

## 真实收发怎么验证（无硬件）

- 串口收发依赖真实硬件——本仓库无法离线验证收发链路。
- 无硬件时至少 offscreen 验：①`refreshPorts`（空列表不崩）、②`applyConfigToPort`（combo 选中项 → QSerialPort 枚举映射正确）、③Hex/ASCII 编解码（构造假字节喂 `renderChunk` / `fromHex` 对答案）。
- 接硬件实测：开端口 → 对端回环自收自发 → 拔线看 errorOccurred 落状态栏。
