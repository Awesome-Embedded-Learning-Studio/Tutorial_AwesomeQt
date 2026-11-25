---
title: "Step 3：UDP 数据报收发"
description: "QUdpSocket::bind 收数据报、hasPendingDatagrams + receiveDatagram 循环消费（不是 readAll）、QNetworkDatagram 取来源、writeDatagram 要明确目标留空回环自测。"
---

# Step 3：UDP 数据报收发

← [Step 2 TCP Client](./02-tcp-client-async-connect.md) · [手册首页](./index.md) · [卡住怎么办](./troubleshooting.md) →

## Step 3：UDP bind 收数据报 + writeDatagram 发

### 目标

协议切到 `UDP`，配置面板同时显示「本机端口」和「目标 IP/端口」（UDP 既要 bind 一个本机端口收、又要 writeDatagram 一个目标发）。填本机端口点 `Start`，`QUdpSocket::bind` 起收，状态栏显示 `Bound` 和真实端口；别人往这个端口发数据报，接收区带时间戳和 `[from IP:port]` 打印 `← 数据`；发送框输入点 Send，按目标 IP/端口 `writeDatagram` 发出。**目标留空**时按回环自测（发给自己 bind 的端口），便于一台机器自检。点 `Stop` 干净停。

### 提示

- **UDP 收绝不能用 `readAll`**——UDP 是离散数据报，`readAll` 会粘包/丢边界。必须 `while hasPendingDatagrams()` + `receiveDatagram()` 逐个消费。
- 一次 `readyRead` 可能含**多个数据报**——`while udp_socket_->hasPendingDatagrams()` 循环到没有 pending，别只收一个。
- `receiveDatagram()` 返回 `QNetworkDatagram`，先 `isValid()` 判有效性再读 `data()`；来源用 `senderAddress()`/`senderPort()` 标注。
- `bind(QHostAddress::Any, port)`：端口 0 同样让系统分配，真实端口用 `udp_socket_->localPort()` 取（别假设）。
- bind 失败（端口占用）读 `errorString()` 弹窗 + `delete` + 置空回滚。
- **发送要明确目标**（UDP 无连接）：从配置区读 target IP/port。UDP 模式同时显示本机端口 + 目标 IP/端口（`need_target` 对 UDP 也置 `true`）——UDP 既要 bind 一个本机端口收、又要 writeDatagram 一个目标发，两头都要。**目标留空**时 onSend 容错：回环自测（`QHostAddress::LocalHost` + `localPort()`），便于一台机器自检。
- `writeDatagram(payload, target, port)` 返回写入字节数，<0 算失败（错误由 `errorOccurred` 上报）。
- Stop：`udp_socket_->close()` + `deleteLater()` + 置空。

### 检查点

切 UDP → 配置面板显示本机端口 → bind 成功状态栏 `Bound port xxxx` → 远端往这端口发数据报，接收区出现 `← 数据   [from IP:port]` → 发送框留空目标点 Send，自己收到自己发的（回环）→ 填目标 IP/端口点 Send，远端收到 → 反复收发，数据报**不粘不乱** = UDP 边界管对了。

> QUdpSocket 不熟？先读 [UDP Socket 编程](../../../../../beginner/04-qtnetwork/02-udp-socket-beginner.md)。
> 数据报边界 / QNetworkDatagram 看 [UDP Socket 编程（进阶）](../../../../../advanced/04-qtnetwork/02-udp-advanced.md)。

### 对照答案

- Start 时 UDP 分支（bind + 接 readyRead/errorOccurred + 失败回滚）：`demo/network_tool_window.cpp:249-270`
- 真实端口反馈（localPort 不假设）：`demo/network_tool_window.cpp:269`
- 收数据报（hasPendingDatagrams + receiveDatagram 循环 + isValid + 来源标注）：`demo/network_tool_window.cpp:378-391`
- 发送 UDP 分支（目标留空回环、否则 writeDatagram）：`demo/network_tool_window.cpp:441-464`
- teardown UDP（close + deleteLater）：`demo/network_tool_window.cpp:302-306`

---

三种协议都搓通了。回 [手册首页](./index.md) 看进阶挑战，或回 [成品导览](../) 对照整份 code 怎么织成一台机器。
