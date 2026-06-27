---
title: "卡住怎么办"
description: "按症状查：端口填 0 状态栏显示 0、TCP 收只到半截、UDP 收粘包、未连点 Send 丢数据、客户端断开崩溃、Server close 没断客户端、漏接多客户端、运行中改配置无效、编译报 Network 找不到、listen/bind 静默失败、UDP 目标控件被隐藏只发 loopback、Server 客户端 socket 退出泄漏、广播 TX 字节只计一份、Client 断开后 Send 还亮、清自身 socket 没屏蔽 disconnected——给方向指向教程章，不直接给答案。"
---

# 卡住怎么办

← [手册首页](./index.md)

按症状查。每条给方向，不给整段答案——成品 repo 在 `app/02-network-tools/network-tool/`，对照着看。

## 端口填 0 起服务，状态栏/日志也显示 0

- 端口 0 让系统分配空闲端口，**实际监听端口**要用 `serverPort()`（TCP）/ `localPort()`（UDP）取回，不是回显用户填的值。
- 反馈真实端口：TCP Server `demo/network_tool_window.cpp:227`；UDP `demo/network_tool_window.cpp:269`。
- 进阶排查：[TCP Socket 编程](../../../../../beginner/04-qtnetwork/01-tcp-socket-beginner.md)

## TCP 收数据只显示前半截，后半截丢/串行

- `readyRead` 只保证「至少一字节可读」，大消息可能分多次信号到达。
- 有没有 `while (socket->bytesAvailable() > 0)` 循环 `readAll` 累积？只读一次就丢后半截。→ `demo/network_tool_window.cpp:340-347`
- 进阶排查：[TCP Socket 编程（进阶）](../../../../../advanced/04-qtnetwork/01-tcp-advanced.md)

## UDP 收数据粘包 / 边界错乱 / 收不到

- 是不是用了 TCP 的 `readAll` 收 UDP？UDP 是离散数据报，`readAll` 会粘包/丢边界。
- UDP 收必须 `while hasPendingDatagrams()` + `receiveDatagram()` 逐个消费。→ `demo/network_tool_window.cpp:380-389`
- 进阶排查：[UDP Socket 编程（进阶）](../../../../../advanced/04-qtnetwork/02-udp-advanced.md)

## TCP Client 还没 connected 就点 Send，数据丢了/报错

- `connectToHost` 异步，立即返回时连接还没建；此时 `write` 被丢弃。
- 发送前有没有用 `tcp_client_->state() == QAbstractSocket::ConnectedState` 前置校验？以 `connected` 信号态为准。→ `demo/network_tool_window.cpp:433`
- 进阶排查：[TCP Socket 编程（进阶）](../../../../../advanced/04-qtnetwork/01-tcp-advanced.md)

## 客户端断开后程序崩 / 二次断开回调已删对象

- `disconnected` 信号回调里访问了已 `deleteLater` 的 socket；或 `teardownAll` 时 socket 还会发 disconnected 串到正在清理的对象。
- `teardownAll` 清理客户端前有没有先 `disconnect(disconnected signal)`？→ `demo/network_tool_window.cpp:290`（自身 `tcp_client_` 分支也补了对称的 disconnect：`297`）
- Server 客户端断开用 lambda 捕获 socket 指针区分谁断的，断开里 `removeOne` + `deleteLater`。→ `demo/network_tool_window.cpp:324-325` / `369-370`
- 进阶排查：[TCP Socket 编程（进阶）](../../../../../advanced/04-qtnetwork/01-tcp-advanced.md)

## TCP Server 停服务时客户端没断，泄漏

- `QTcpServer::close()` 只停监听，**不会主动断开已接的客户端 socket**。
- `teardownAll` 里 server close 之后有没有**显式遍历** `tcp_clients_` 逐个 `disconnectFromHost` + `deleteLater`？→ `demo/network_tool_window.cpp:290-294`

## 同一客户端连进来只接一条，丢一条

- 高并发可能一次积压多个 pending connection，只 `nextPendingConnection()` 一次接不到全部。
- 有没有 `while hasPendingConnections()` 循环接完所有 pending？→ `demo/network_tool_window.cpp:315`

## 运行中改端口/IP，状态/行为没变化

- socket 已建好，改配置输入框不会重建 socket——让用户产生「改了没反应」的错觉。
- 运行中有没有 `setReadOnly`/`setEnabled(false)` 锁住协议切换和配置输入？改要走 Stop→重新 Start。→ `demo/network_tool_window.cpp:176-179`

## 编译报 `QHostAddress` / `QTcpServer` / `QUdpSocket` 找不到

- 工程链了 `Qt6::Network` 吗？不链整个编译过不了。
- `CMakeLists.txt` 里 `find_package(Qt6 ... COMPONENTS Network)` + `target_link_libraries(... Qt6::Network)`。→ `demo/CMakeLists.txt:3,15`

## listen / bind 失败静默，用户不知道端口被占

- 端口占用/权限不够时 `listen`/`bind` 返回 false，没读 `errorString` 用户就以为服务正常。
- 失败时有没有 `QMessageBox::warning` 弹 `errorString()` 并 `delete` + 置空回滚？→ TCP `demo/network_tool_window.cpp:217-223`；UDP `262-267`

## UDP 发送提示「No connected client」或不知发给谁

- UDP 无连接，`writeDatagram` 必须明确目标。目标框在 UDP 模式默认隐藏，发不出去。
- 有没有在 onSend 里给 UDP 容错：目标留空时回环自测（`LocalHost` + `localPort()`），填了就按填的发？→ `demo/network_tool_window.cpp:441-464`

## UDP 模式填了目标 IP/端口，发送还是只发给自己（loopback）

- UDP 无连接，`writeDatagram` 要明确目标；但 `refreshConfigVisibility` 若把 UDP 的 `need_target` 置 `false`，目标 IP/端口控件会被隐藏——onSend 取不到用户填的目标，只能退回 loopback 自测。
- UDP 是「本机端口 + 目标 IP/端口」两头都要：`need_target` 对 UDP 也置 `true`，让 UDP 模式同时显示本机端口和目标框。→ `demo/network_tool_window.cpp:161`
- 进阶排查：[UDP Socket 编程（进阶）](../../../../../advanced/04-qtnetwork/02-udp-advanced.md)

## TCP Server 客户端 socket 退出时没回收（泄漏）

- `nextPendingConnection()` 返回的 socket 默认**无父对象**；靠 `disconnected → deleteLater` 回收，但程序退出走对象树析构、此时没有事件循环跑 `deleteLater`，于是泄漏。
- 接客户端后有没有 `socket->setParent(this)` 挂到窗口对象树上，让析构兜底？→ `demo/network_tool_window.cpp:317`
- 进阶排查：[QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)

## TCP Server 广播，TX 字节只 +N×1（或 +1），不是 +N×payload；个别失败 break 掉整轮

- 累加逻辑只加一份 payload（漏算 N 倍）；或循环里 `write < 0` 就 `break`，抹掉前面已成功写入的字节。
- 循环内有没有累加**每个客户端 `write` 的实发返回值**，且部分失败不 break？→ `demo/network_tool_window.cpp:415-429`
- 进阶排查：[TCP Socket 编程（进阶）](../../../../../advanced/04-qtnetwork/01-tcp-advanced.md)

## TCP Client 连上后远端断开，Send 按钮还亮着

- 自身 `disconnected` 没禁用 `send_button_`——UI 与连接态脱节，点了才报 Not connected。
- `onTcpDisconnected(socket=nullptr)`（自身断开）有没有 `send_button_->setEnabled(false)`？`onTcpConnected` 重连恢复。→ 禁用 `demo/network_tool_window.cpp:361`；恢复 `352`

## 清理自身 tcp_client_ 时偶尔崩，清 Server 客户端没事

- `teardownAll` 清 `tcp_clients_` 有 `disconnect(disconnected)` 屏蔽信号，清 `tcp_client_` 没补——不对称，`deleteLater` 前发的 `disconnected` 串进来触发回调。
- 清 `tcp_client_` 有没有补上对称的 `disconnect(disconnected signal)`？→ `demo/network_tool_window.cpp:297`

## moc 报错 / Q_OBJECT 相关

- `NetworkToolWindow` 头里有 `Q_OBJECT`（用了信号槽必须有），CMake **开了 AUTOMOC 吗**？`qt_add_executable` 默认开。
- 进阶排查：[QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)
