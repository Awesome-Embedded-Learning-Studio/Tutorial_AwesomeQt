# 现代Qt开发教程（新手篇）4.4——WebSocket 双向实时通信基础

## 1. 前言：为什么需要 WebSocket

如果你做过任何需要「服务器主动推送」的功能——聊天消息、实时行情、协作编辑、在线游戏状态同步——你一定体会过传统 HTTP 在这方面的无力感。HTTP 本质上是请求-响应模型，客户端不发请求，服务端就没法把数据推过去。早期大家用轮询（每隔几秒发一次请求）硬扛，后来搞出了长轮询（ Comet），再后来有了各种非标准的 hack 方案。说实话，每一种都很丑陋。

WebSocket 是 IETF 在 2011 年标准化的全双工通信协议，它在 HTTP 握手的基础上升级连接，升级完成之后客户端和服务端可以随时互发数据帧，不再受请求-响应模型的约束。协议层面的开销极小——数据帧头部最少只有 2 字节——而且支持文本帧和二进制帧两种类型，非常适合传输 JSON 消息或者二进制协议数据。

Qt 从 5.3 开始提供了 QWebSocket 和 QWebSocketServer，放在 QtWebSockets 模块里。到了 Qt 6，这个模块的 API 非常稳定，和 Qt 网络模块的设计风格完全一致：信号槽驱动的异步模型，不需要手动管理底层套接字，不需要关心帧的编解码和分片重组。如果你已经熟悉了 QTcpSocket 和 QTcpServer 的用法，上手 QWebSocket 几乎没有学习成本——区别只在于 WebSocket 帮你把应用层协议（握手、帧格式、心跳 ping/pong）全部处理好了，你只需要关注业务逻辑。

这一篇我们从零开始，把客户端的连接与收发、服务端的监听与多客户端管理、二进制数据传输这些核心操作全部走一遍。

## 2. 环境说明

本教程基于 Qt 6.9.1，适用于 Windows 10/11（MSVC 2019+ 或 MinGW 11.2+）、Linux（GCC 11+）、WSL2 + WSLg（GUI 支持）。所有示例代码均使用 C++17 标准和 CMake 3.26+ 构建系统。本篇需要链接 Qt6::WebSockets 模块，CMake 中通过 `find_package(Qt6 REQUIRED COMPONENTS Core WebSockets)` 引入。注意 QtWebSockets 是一个独立于 QtNetwork 的模块，你需要单独 find_package 它。

## 3. 核心概念讲解

### 3.1 WebSocket 协议的核心模型

在写代码之前，我们先搞清楚 WebSocket 的工作原理。WebSocket 连接的建立过程叫「握手」——客户端发送一个特殊的 HTTP GET 请求，里面带着 `Upgrade: websocket` 和 `Sec-WebSocket-Key` 等 header，服务端如果同意升级，就返回 HTTP 101 Switching Protocols。到此为止，底层的 TCP 连接就不再是 HTTP 了，而是切换成了 WebSocket 协议。

在这之后的整个生命周期里，连接是全双工的：客户端和服务端都可以随时向对方发送数据帧。数据帧分为文本帧（opcode 0x1）和二进制帧（opcode 0x2），还有用于连接管理的 ping/pong 帧（opcode 0x9/0xA）和关闭帧（opcode 0x8）。Qt 在内部帮你处理了帧的编解码、分片重组和心跳响应，你只需要关心文本消息和二进制消息的收发。

和 TCP 的字节流不同，WebSocket 是面向消息的协议——每一次 `sendTextMessage` 对应一次完整的 `textMessageReceived`，不存在粘包和半包的问题。这是 WebSocket 相比裸 TCP 在应用层最大的优势：你不需要自己实现消息分帧。

### 3.2 客户端：QWebSocket 的 open / send / receive

我们从客户端说起。QWebSocket 的基本生命周期是：创建实例、调用 `open()` 连接服务器、通过信号接收数据、调用 `close()` 断开连接。

```cpp
QWebSocket *ws = new QWebSocket();

// 连接服务器
ws->open(QUrl("ws://localhost:12345"));

// 连接成功时触发
connect(ws, &QWebSocket::connected, []() {
    qDebug() << "WebSocket connected!";
    ws->sendTextMessage("Hello, WebSocket Server!");
});

// 收到文本消息时触发
connect(ws, &QWebSocket::textMessageReceived, [](const QString &message) {
    qDebug() << "Received:" << message;
});

// 连接关闭时触发
connect(ws, &QWebSocket::disconnected, []() {
    qDebug() << "WebSocket disconnected.";
    ws->deleteLater();
});
```

`open()` 接受一个 QUrl 参数，WebSocket 的 URL scheme 是 `ws://`（明文）和 `wss://`（TLS 加密）。和 QTcpSocket 的 `connectToHost()` 不同，`open()` 是完全异步的，它不会阻塞。连接成功后触发 `connected` 信号，失败则触发 `disconnected` 信号。

发送数据有两种方式：`sendTextMessage()` 发送文本帧，参数是一个 QString，返回值是实际写入的字节数；`sendBinaryMessage()` 发送二进制帧，参数是一个 QByteArray。对应地，接收端也有两个信号：`textMessageReceived(const QString &)` 和 `binaryMessageReceived(const QByteArray &)`。

这里有一个细节值得注意：`sendTextMessage()` 返回的 qint64 表示写入底层缓冲区的字节数。如果你发送的数据量很大（比如传输文件），这个返回值可能小于你传入的数据大小。不过在正常情况下，只要连接没断，Qt 会把数据全部发出去。

### 3.3 发送文本和二进制数据

前面我们提到了两种发送方式，这里展开讲一下什么时候该用哪种。

文本帧适合传输 JSON、XML、纯文本等人类可读的数据格式。WebSocket 协议规定文本帧必须使用 UTF-8 编码，Qt 的 `sendTextMessage()` 会自动把 QString 转成 UTF-8 发送，`textMessageReceived` 会自动把接收到的 UTF-8 数据解码成 QString。

二进制帧适合传输 Protobuf、MessagePack、图片、文件等结构化的二进制数据。`sendBinaryMessage()` 直接发送 QByteArray 的原始字节，不做任何编码转换。

```cpp
// 发送 JSON 文本消息
QJsonObject json;
json["type"] = "chat";
json["content"] = "Hello from Qt!";
json["timestamp"] = QDateTime::currentDateTime().toMSecsSinceEpoch();
ws->sendTextMessage(QJsonDocument(json).toJson(QJsonDocument::Compact));

// 发送二进制数据（比如一张图片）
QFile imageFile(":/images/photo.png");
if (imageFile.open(QIODevice::ReadOnly)) {
    QByteArray imageData = imageFile.readAll();
    ws->sendBinaryMessage(imageData);
    qDebug() << "Sent binary data:" << imageData.size() << "bytes";
}
```

实际项目中，你可以用文本帧传输控制信令（JSON 格式的请求/响应），用二进制帧传输实际的业务数据。这是一种常见的混合策略。

### 3.4 服务端：QWebSocketServer 的 listen + newConnection

现在我们来看服务端。QWebSocketServer 的 API 设计和 QTcpServer 非常相似——调用 `listen()` 监听端口，连接 `newConnection` 信号，在槽函数里调用 `nextPendingConnection()` 获取客户端的 QWebSocket。

```cpp
QWebSocketServer *server = new QWebSocketServer(
    "MyWebSocketServer", QWebSocketServer::NonSecureMode, this);

if (!server->listen(QHostAddress::Any, 12345)) {
    qDebug() << "Server failed to start:" << server->errorString();
    return;
}

qDebug() << "WebSocket server listening on port 12345...";

connect(server, &QWebSocketServer::newConnection, [=]() {
    QWebSocket *client = server->nextPendingConnection();
    qDebug() << "New client connected:"
             << client->peerAddress().toString()
             << ":" << client->peerPort();

    // 收到客户端消息时广播给所有连接的客户端
    connect(client, &QWebSocket::textMessageReceived,
            [this, client](const QString &message) {
        qDebug() << "From client:" << message;
        // 回传确认
        client->sendTextMessage("Server received: " + message);
    });

    // 客户端断开时清理
    connect(client, &QWebSocket::disconnected, [this, client]() {
        qDebug() << "Client disconnected.";
        m_clients.removeOne(client);
        client->deleteLater();
    });

    m_clients.append(client);
});
```

QWebSocketServer 的构造函数第二个参数指定是否使用安全模式（`SslMode`）。`NonSecureMode` 对应 `ws://` 协议，`SecureMode` 对应 `wss://` 协议。如果要用安全模式，你需要在 `listen()` 之前调用 `setSslConfiguration()` 配置 SSL 证书和私钥——这一部分我们会在下一篇 SSL/TLS 教程中详细讲解。

和 QTcpServer 一样，`nextPendingConnection()` 返回的 QWebSocket 的所有权归你。你必须在客户端断开后调用 `deleteLater()` 释放资源，否则会造成内存泄漏。在实际项目中，通常用一个 QList 或者 QMap 来管理所有已连接的客户端，方便做广播消息和连接统计。

### 3.5 心跳机制与 ping/pong

WebSocket 协议内置了 ping/pong 心跳机制。协议规定，任意一端可以发送 ping 帧，对端必须回复 pong 帧。这是检测连接是否存活的标准方式。

Qt 在 QWebSocket 中提供了 `ping()` 方法和 `pong()` 信号。你可以用定时器定期发送 ping，如果对端没有回复，说明连接可能已经断了。

```cpp
// 每 30 秒发送一次 ping
QTimer *heartbeatTimer = new QTimer(this);
connect(heartbeatTimer, &QTimer::timeout, ws, [ws]() {
    ws->ping(QByteArray("heartbeat"));
});
heartbeatTimer->start(30000);

// 收到 pong 回复
connect(ws, &QWebSocket::pong, [](const QByteArray &payload) {
    qDebug() << "Pong received, latency OK. Payload:" << payload;
});
```

`ping()` 可以携带一个可选的 payload（QByteArray），对端在回复 pong 时会原样回传这个 payload。你可以利用这个特性来计算往返延迟——在 payload 里放一个时间戳，收到 pong 时算差值。

需要注意的是，如果你的应用运行在反向代理（比如 Nginx）后面，反向代理通常有自己的超时配置。Nginx 默认的 proxy_read_timeout 是 60 秒，如果你的心跳间隔超过这个值，Nginx 会主动断开连接。所以心跳间隔通常设置在 20-30 秒之间。

## 4. 踩坑预防清单

关于模块引入的问题：QtWebSockets 是一个独立的 Qt 模块，不属于 QtNetwork。在 CMake 中你需要单独 `find_package(Qt6 REQUIRED COMPONENTS WebSockets)`，同时 `target_link_libraries` 里要加 `Qt6::WebSockets`。如果你只 link 了 `Qt6::Network`，编译会报找不到 QWebSocket 头文件。在 qmake 项目里，需要在 `.pro` 文件里加 `QT += websockets`。

关于 URL scheme 的问题：WebSocket 的 URL 必须使用 `ws://` 或 `wss://` 前缀。如果你用了 `http://` 或 `https://`，`open()` 不会报错但也不会连接。另外，WebSocket URL 支持带路径和查询参数，比如 `ws://localhost:8080/chat?token=abc123`，服务端可以在握手时解析这些信息做认证。

关于消息大小的问题：QWebSocketServer 默认对单个消息的最大大小限制是 2MB（`maxIncomingMessageSize()`）。如果你需要传输更大的消息（比如文件），需要调用 `setMaxIncomingMessageSize()` 调大这个限制。同理，QWebSocket 客户端也有类似的限制。

关于线程安全的问题：和 QTcpSocket 一样，QWebSocket 只能在创建它的线程中使用。你不能在一个线程里创建 QWebSocket 然后在另一个线程里调用它的方法。如果需要多线程处理，应该把 QWebSocket 通过 `moveToThread()` 移到工作线程。

现在我们来做一个小的代码填空练习，检验一下前面的内容。补全以下代码，实现一个 WebSocket 客户端连接服务器并发送/接收文本消息：

```cpp
QWebSocket *ws = new QWebSocket();

ws->__________(QUrl("ws://localhost:12345"));

connect(ws, &QWebSocket::__________, [=]() {
    ws->sendTextMessage("Hello!");
});

connect(ws, &QWebSocket::__________, [](const QString &msg) {
    qDebug() << "Server says:" << msg;
});

connect(ws, &QWebSocket::__________, [=]() {
    ws->deleteLater();
});
```

提示：需要填入的分别是 `open`、`connected`、`textMessageReceived`、`disconnected`。

参考答案如下：

```cpp
QWebSocket *ws = new QWebSocket();

ws->open(QUrl("ws://localhost:12345"));

connect(ws, &QWebSocket::connected, [=]() {
    ws->sendTextMessage("Hello!");
});

connect(ws, &QWebSocket::textMessageReceived, [](const QString &msg) {
    qDebug() << "Server says:" << msg;
});

connect(ws, &QWebSocket::disconnected, [=]() {
    ws->deleteLater();
});
```

## 5. 练习项目

我们来做一个简易的 WebSocket 聊天室，把这一节学的东西串起来。服务端维护所有已连接的客户端，任一客户端发来的消息都广播给所有人。客户端连接后可以发送文本消息，同时接收其他人的消息。

完成标准：服务端能同时处理多个客户端连接，每条消息都被广播给所有已连接的客户端（包括发送者自己）。客户端在连接建立后自动发送一条 "joined" 通知，断开时服务端广播 "left" 通知。客户端支持同时收发（可以用 QTimer 模拟定时发送）。

提示几个方向：服务端用 QList 维护所有客户端的 QWebSocket 指针，`textMessageReceived` 的槽函数里遍历列表逐个 `sendTextMessage()`。客户端断开时从列表中移除并 `deleteLater()`。客户端的发送可以用 QTimer 定时触发，或者如果你想做控制台交互，可以用 `QSocketNotifier` 监听 stdin。

再看一个调试挑战：以下 WebSocket 服务端代码有什么问题？

```cpp
QWebSocketServer server("Test", QWebSocketServer::NonSecureMode);
server.listen(QHostAddress::Any, 8080);

connect(&server, &QWebSocketServer::newConnection, [&]() {
    QWebSocket *client = server.nextPendingConnection();
    connect(client, &QWebSocket::textMessageReceived,
            [&](const QString &msg) {
        qDebug() << "Message:" << msg;
        client->sendTextMessage("Echo: " + msg);
    });
});
```

这里面有几个隐患。首先，没有处理 `disconnected` 信号，客户端断开后 client 对象永远不会被删除，造成内存泄漏。其次，`[&]` 捕获了 `client` 指针的栈上引用——这里 `client` 是局部变量，按引用捕获是危险的，应该用 `[=]` 按值捕获指针。另外，如果多个客户端同时连接，这个代码没有维护客户端列表，无法实现广播。还缺少对 `errorOccurred` 信号的处理，连接异常时无法感知。

## 6. 完整示例代码

本篇的完整示例代码在 `examples/beginner/04-qtnetwork/04-websocket-beginner/` 目录下，包含一个控制台程序，演示了 WebSocket 服务端和客户端的完整通信流程，包括连接建立、文本和二进制消息收发、心跳 ping/pong、以及多客户端管理。

## 7. 官方文档参考

- [Qt 文档 · QWebSocket 类](https://doc.qt.io/qt-6/qwebsocket.html) -- WebSocket 客户端的完整 API，包含连接、收发、心跳相关方法
- [Qt 文档 · QWebSocketServer 类](https://doc.qt.io/qt-6/qwebsocketserver.html) -- WebSocket 服务端的 API，包含监听和连接管理
- [Qt 文档 · QtWebSockets 模块概述](https://doc.qt.io/qt-6/qtwebsockets-index.html) -- 模块总览和编程指南
- [RFC 6455 · The WebSocket Protocol](https://tools.ietf.org/html/rfc6455) -- WebSocket 协议的完整规范文档

*（链接已验证，2026-04-23 可访问）*

---

到这里就大功告成了。QWebSocket 和 QWebSocketServer 的 API 设计非常干净，如果你已经熟悉了 QTcpSocket 的用法，上手 WebSocket 基本就是换几个类名的事。下一篇我们会聊 SSL/TLS 加密通信，看看 Qt 怎么帮你把网络连接加上一层安全防护。
