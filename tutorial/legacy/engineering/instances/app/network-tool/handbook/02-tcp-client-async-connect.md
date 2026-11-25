---
title: "Step 2：TCP Client 异步连接"
description: "QTcpSocket::connectToHost 异步连接、connected/disconnected/errorOccurred 信号驱动状态、未 ConnectedState 前置拒绝发送、断开以信号态为准。"
---

# Step 2：TCP Client 异步连接

← [Step 1 TCP Server](./01-tcp-server-listen-and-clients.md) · [手册首页](./index.md) · 下一步 [Step 3 UDP 数据报](./03-udp-datagrams.md) →

## Step 2：TCP Client 异步连接 + connected 状态驱动

### 目标

协议切到 `TCP Client`，配置面板**自动**变成「目标 IP + 目标端口」（本机端口框隐藏）。填好远端 IP/端口点 `Start`，开始异步连接，状态栏显示 `Connecting`；连上后变 `Connected` 并解锁发送；远端断开/出错，状态栏如实反映、接收区打印断开信息。连接没建立时点 Send，**前置拒绝**并提示，不让 `write` 被静默丢弃。点 `Stop` 干净断开。

### 提示

- **协议显隐**：`onProtocolChanged` → `refreshConfigVisibility`。TCP Client 时 `local_port_label_/edit_` 隐藏、`target_label_/ip_edit_` 和 `target_port_` 容器显出。**target_port 在单独行**（parentWidget 连同显隐），别和 target_ip 强行对齐一行。
- `connectToHost` 是**异步**的——立即返回时连接还没建，要靠信号驱动后续态：接 `connected`/`disconnected`/`readyRead`/`errorOccurred` 四个信号。
- 自身断开用 `disconnected`，但 Server 客户端断开也走同一个 slot——用 `socket=nullptr` 表示 Client 自身断开、非空表示 Server 某客户端断开（Step 1 已用这个约定）。**自身断开要禁用 `send_button_`**——让 UI 与连接态对齐，避免按钮还亮着、点了才报 Not connected；`onTcpConnected` 重连再恢复。
- **发送前置校验**：`connectToHost` 返回后 socket 还没 connected，此时 `write` 会被 Qt 丢弃/报错。用 `tcp_client_->state() == QAbstractSocket::ConnectedState` 判断——以**信号态为准**，别用按钮文案。
- 收数据复用 Step 1 的 `onTcpReadyRead`（`sender()` 区分来源），TCP Client 的 socket `readyRead` 也接这个 slot。
- 状态栏的 `Connecting` vs `Connected`：`ConnectedState` 才算连上，否则 `Connecting`（或更靠后的 `HostLookupState`/`ConnectingState`）。
- Stop 时 `disconnectFromHost` + `deleteLater` + 置空。

### 检查点

切 TCP Client → 配置面板自动变成目标 IP+端口 → 填错 IP 连不上，状态栏卡在 `Connecting`、接收区刷 socket error → 填对远端，状态栏 `Connected` → 发送框输入点 Send，远端收到、接收区 `→ xxx` → 断开远端，接收区打印 `disconnected` → 没连上就点 Send 被拒绝。**整个连接/断开过程状态栏实时准确** = 异步信号态管对了。

> TCP Socket 异步连接不熟？先读 [TCP Socket 编程](../../../../../beginner/04-qtnetwork/01-tcp-socket-beginner.md)。
> connectToHost 异步 + 状态机看 [TCP Socket 编程（进阶）](../../../../../advanced/04-qtnetwork/01-tcp-advanced.md)。

### 对照答案

- 协议显隐（Client 显示目标、隐藏本机端口）：`demo/network_tool_window.cpp:155-171`
- Start 时 TCP Client 分支（connectToHost + 接四信号）：`demo/network_tool_window.cpp:230-247`
- onTcpConnected（连上 → 状态栏 + 恢复发送按钮）：`demo/network_tool_window.cpp:351-355`
- onTcpDisconnected（socket=nullptr 表示自身断开：禁用发送按钮；非空表示 Server 某客户端断开）：`demo/network_tool_window.cpp:358-373`
- 发送前置 ConnectedState 校验：`demo/network_tool_window.cpp:432-440`
- 状态栏 Connecting vs Connected 判定：`demo/network_tool_window.cpp:526-535`

---

下一步：换无连接协议——UDP 用 `bind` 收、`hasPendingDatagrams` + `receiveDatagram` 循环消费（**不是 readAll**）、`writeDatagram` 要明确目标——[Step 3 UDP 数据报](./03-udp-datagrams.md)。
