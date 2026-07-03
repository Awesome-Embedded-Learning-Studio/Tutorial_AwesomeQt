---
title: "Network Tool 手搓手册"
description: "从空 QMainWindow 一行行搓出 TCP/UDP 网络调试助手：QTcpServer 起监听接多客户端、QTcpSocket 异步连接、QUdpSocket bind 收发数据报、Start/Stop 单按钮两态、teardown 顺序清理。"
---

# Network Tool 手搓手册

> **source**：成品答案在 `app/02-network-tools/network-tool/`（做完对照）· **related**：app 栏网络工具类整机成品

::: tip 这是「手搓手册」
不是参考手册（查完走），是 workbook（跟着搓）。每个 step 给**目标 → 提示 → 检查点**，成品 repo 当答案钥匙——卡住了去对照，别整段复制。
:::

## 0. 你将学到

搓完这个网络调试助手，你会打通这几样 Qt 能力（每样后面都有教程深挖，这里先用起来）：

- **QTcpServer**：`listen` 起监听、`newConnection` 信号接客户端、`nextPendingConnection` 取 socket、`serverPort` 取真实端口
- **QTcpSocket**：`connectToHost` 异步连接、`connected`/`disconnected`/`readyRead`/`errorOccurred` 信号驱动、`bytesAvailable` + `readAll` 累积收
- **QUdpSocket**：`bind` 收数据报、`hasPendingDatagrams` + `receiveDatagram` 循环消费、`writeDatagram` 发、`QNetworkDatagram` 取来源
- **多客户端管理**：`QList<QTcpSocket*>` + `sender()` 区分来源、断开 `deleteLater` 防悬空指针
- **单按钮两态**：Start/Stop 同一按钮，状态以「资源是否存在」为准而非按钮文案
- **资源释放顺序**：先 disconnect 信号再 disconnectFromHost、`QTcpServer::close` 不主动断客户端需显式清理
- **QMainWindow 整机装配**：配置面板（QFormLayout）+ 收发区 + 状态栏 + QComboBox 驱动协议显隐

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

> 注意：工程要链 `Qt6::Network`（`CMakeLists.txt` 里 `find_package(Qt6 ... COMPONENTS Network)` + `target_link_libraries(... Qt6::Network)`），不链的话 `#include <QTcpServer>` 都找不到。

## 2. 任务清单

分 3 步（按协议递进），每步：**目标 → 提示 → 检查点**。卡住翻 [卡住怎么办](./troubleshooting.md)。

| Step | 目标 | 进 |
|---|---|---|
| 1 | TCP Server：listen 起监听 + newConnection 接多客户端 + readyRead 循环收 + teardown 清理 | [01](./01-tcp-server-listen-and-clients.md) |
| 2 | TCP Client：connectToHost 异步 + connected/disconnected 状态驱动 + 未连拒绝发送 | [02](./02-tcp-client-async-connect.md) |
| 3 | UDP：bind 收数据报（hasPendingDatagrams + receiveDatagram，不是 readAll）+ writeDatagram 留空回环 | [03](./03-udp-datagrams.md) |

成品对照：`app/02-network-tools/network-tool/`（按 [成品导览](../) 的「怎么读」顺序对照）。

> 三步共用同一套 UI（配置面板 + 收发区 + 状态栏）和 `onStartStopToggled`/`onSend` 的协议分流。Step 1 先把骨架和 TCP Server 打通，Step 2/3 在同一套骨架里加另两种协议分支——三协议**互斥**，同一时刻只活一套资源。

## 3. 进阶挑战（可选）

搓完基础版想再深一层：

- **十六进制收发**：现在是 UTF-8 文本显示，调试二进制协议要 hex。提示：`payload.toHex(' ')` 显示、发送时 `QByteArray::fromHex`。
- **多客户端单发**：现在 Server 模式广播所有客户端。提示：维护当前选中客户端（右键列表选），`write` 只发给它。
- **断线重连**：TCP Client 断开后自动重连。提示：`disconnected` 信号里起 `QTimer::singleShot` 重试 `connectToHost`，设最大重试次数。
- **流量统计曲线**：状态栏现在是累计数字。提示：把 RX/TX 按 1 秒采样进 `QChart`，画实时折线。
- **跨线程收发**：现在全在 UI 线程，高吞吐会卡。提示：把 socket 移到 `QThread`，用信号槽跨线程传 `QByteArray`（注意 `deleteLater` 与线程退出顺序）。
- **下一站**：app 栏的 serial-tool / image-viewer——换皮复用 QMainWindow 整机骨架，但引入 QSerialPort / 自定义绘制。
