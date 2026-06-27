---
title: "Step 1：TCP Server 起监听 + 多客户端"
description: "QTcpServer::listen 起监听（端口 0 系统分配、serverPort 取真实端口）、newConnection 接客户端入 QList、readyRead while bytesAvailable 累积收、teardown 先 disconnect 信号再清理防悬空。"
---

# Step 1：TCP Server 起监听 + 多客户端

← [手册首页](./index.md) · 下一步 [Step 2 TCP Client 异步连接](./02-tcp-client-async-connect.md) →

## Step 1：TCP Server 起监听 + 接多客户端 + 循环收数据

### 目标

选 `TCP Server`、填本机端口、点 `Start`，窗口进入监听态，状态栏显示真实监听端口和客户端数。用任意 TCP 客户端（`nc`、另一个网络助手）连进来，接收区打印 `Client connected: <IP>:<port>`；客户端发数据，接收区带时间戳打印 `← 数据`；在发送框输入文字点 Send，**所有已连客户端都收到**。点 `Stop` 干净停掉，控制台无崩溃、无悬空指针。

### 提示

- **端口校验**：填空或非法（非 0..65535）弹 warning 拒绝。**端口 0 是合法**——让系统分配空闲端口。
- listen 成功后**别假设**用户填的端口就是监听端口：用 `tcp_server_->serverPort()` 取真实端口反馈（填 0 时尤其关键，否则状态栏显示 0 误导）。
- listen 失败（端口占用/权限不够）要读 `tcp_server_->errorString()` 弹窗，并 `delete` + 置空回滚，别留个没监听上的空壳。
- 接客户端用 `QTcpServer::newConnection` 信号，里面 `while hasPendingConnections()` 循环 `nextPendingConnection()`——**别只接一次**，高并发会一次积压多个 pending。
- 每个客户端 `QTcpSocket` 接**自己的** `readyRead`/`disconnected`。收数据 slot 里用 `qobject_cast<QTcpSocket*>(sender())` 区分是哪个客户端。
- **收数据循环**：`readyRead` 只保证「至少一字节可读」，大消息会分片到——`while (socket->bytesAvailable() > 0)` 循环 `readAll` 累积，别只读一次。
- 多客户端用 `QList<QTcpSocket*>` 管理。客户端断开：`disconnected` 信号 → 从列表移除 + `deleteLater`。**接进来的 socket 要 `setParent(this)`**——`nextPendingConnection()` 默认无父，退出时没有事件循环跑 `deleteLater` 会泄漏，挂到窗口对象树上让析构兜底。
- Stop（`teardownAll`）顺序很关键：**`QTcpServer::close()` 不会主动断已接的客户端**，要显式遍历 `tcp_clients_` 逐个 `disconnectFromHost` + `deleteLater`；且清理前先 `disconnect(disconnected signal)`，避免断开信号回调到正在删的对象。

### 检查点

起服务 → 客户端连进来出现 `Client connected` → 客户端发数据接收区出现 `← xxx` → 发送框输入点 Send，客户端收到 → 客户端断开出现 `Client disconnected` → 点 `Stop` 干净停。**反复 Start/Stop、多客户端并发连，控制台无崩溃无 warning** = 资源生命周期管对了。

> QTcpServer 不熟？先读 [TCP Socket 编程](../../../../../beginner/04-qtnetwork/01-tcp-socket-beginner.md)。
> readyRead / bytesAvailable 异步机制看 [TCP Socket 编程（进阶）](../../../../../advanced/04-qtnetwork/01-tcp-advanced.md)。

### 对照答案

- Start 时 TCP Server 分支装配（端口校验 + listen + 失败回滚）：`demo/network_tool_window.cpp:201-228`
- 真实端口反馈（serverPort 不假设）：`demo/network_tool_window.cpp:227`
- 接客户端（hasPendingConnections 循环 + setParent 挂父 + 入列 + 接各自信号）：`demo/network_tool_window.cpp:313-325`
- 收数据（sender 区分 + while bytesAvailable 累积）：`demo/network_tool_window.cpp:333-348`
- 客户端断开移除 + deleteLater：`demo/network_tool_window.cpp:358-373`
- teardownAll（server close → 客户端先 disconnect 信号再 disconnectFromHost → deleteLater）：`demo/network_tool_window.cpp:283-295`

---

下一步：换协议——TCP Client 异步连远端，靠 `connected`/`disconnected` 信号驱动状态、未连拒绝发送——[Step 2 TCP Client 异步连接](./02-tcp-client-async-connect.md)。
