# 现代Qt开发教程（新手篇）4.1——TCP 通信基础

## 1. 前言：为什么需要 TCP

说实话，如果你写过任何跟网络有关的程序——哪怕只是大学课设里那种简陋的聊天室——你大概率已经和 TCP 打过交道了。TCP 是整个互联网的脊梁，HTTP 建在它上面，数据库连接建在它上面，你手机里几乎每一个 App 的后台通信都建在它上面。所以学会用 Qt 做 TCP 通信，基本上等于拿到了网络编程的入场券。

但如果你跟我一样，当初学网络编程的时候用的是原生 Socket API，那你一定记得那段痛苦的经历：`socket()`、`bind()`、`listen()`、`accept()`，一套组合拳打下来，光是处理地址结构体和字节序就能把人折腾得半死。更要命的是，原生 API 是阻塞的，要想不卡界面还得自己搞多线程或者 select/epoll，代码量直接爆炸。

Qt 给我们提供了 QTcpSocket 和 QTcpServer 两个类，把上面那一坨东西全部封装成了信号槽驱动的异步模型。你不需要手动管理文件描述符，不需要关心 `EAGAIN` 和 `EINTR`，不需要自己写事件循环——Qt 的事件循环帮你搞定了这一切。你只需要连几个信号、写几个槽函数，就能实现一个完整的 TCP 客户端和服务端。

这一篇我们从零开始，把客户端的连接/发送/接收、服务端的监听/接受连接、断线检测与重连策略全部走一遍。不玩虚的，看完就能用。

## 2. 环境说明

本教程基于 Qt 6.9.1，适用于 Windows 10/11（MSVC 2019+ 或 MinGW 11.2+）、Linux（GCC 11+）、WSL2 + WSLg（GUI 支持）。所有示例代码均使用 C++17 标准和 CMake 3.26+ 构建系统。本篇需要链接 Qt6::Network 模块，CMake 中通过 `find_package(Qt6 REQUIRED COMPONENTS Network)` 引入。

## 3. 核心概念讲解

### 3.1 TCP 通信的基本模型

TCP 通信的核心模型其实就是一个「打电话」的过程：服务端先开机等待（listen），客户端拨号过去（connect），双方建立连接后可以双向通话（read/write），聊完了挂电话（close）。这个比喻虽然老套，但它确实精确地描述了 TCP 三次握手和四次挥手的本质。

在 Qt 中，这个模型被映射到了两个类上面：QTcpServer 负责监听和接受连接，QTcpSocket 负责实际的数据传输。服务端每接受一个新连接，就会产生一个新的 QTcpSocket 对象来和客户端一对一通信。这意味着一个服务端可以同时和多个客户端通信，每个客户端都有自己独立的 Socket。

这里有一个很关键的设计决策：QTcpSocket 是异步的。所有的网络操作（连接、读取、写入）都不会阻塞调用线程，而是通过信号通知你结果。这正是 Qt 网络编程的核心优势——你不需要在主线程里 `recv()` 卡半天等数据，也不需要为每个连接开一个线程。事件循环帮你管理一切。

### 3.2 客户端：connectToHost / write / readAll

先从客户端说起。一个 TCP 客户端的生命周期非常简单：创建 Socket，连接服务器，发送数据，接收数据，断开连接。

```cpp
QTcpSocket *socket = new QTcpSocket(this);

// 连接服务器
socket->connectToHost(QHostAddress("127.0.0.1"), 12345);

// 等待连接建立（异步方式推荐用信号，这里先用同步做演示）
if (socket->waitForConnected(3000)) {
    qDebug() << "Connected to server!";

    // 发送数据
    socket->write("Hello, TCP Server!");
    socket->flush();
} else {
    qDebug() << "Connection failed:" << socket->errorString();
}
```

上面这段代码用了 `waitForConnected()` 来同步等待连接，在实际项目中我们更推荐用信号槽的方式，不过这里先用它把基本流程跑通。

`connectToHost()` 的两个参数分别是目标地址和端口号。QHostAddress 可以接受 IP 地址字符串，也可以用 `QHostAddress::LocalHost` 表示本机。端口号是一个 16 位无符号整数，范围是 0-65535，其中 0-1023 是系统保留的知名端口（比如 80 给 HTTP、443 给 HTTPS），我们的示例程序应该使用 1024 以上的端口。

发送数据用 `write()` 方法，它接受一个 QByteArray 或者 `const char*`，返回实际写入的字节数。需要注意，`write()` 并不保证数据立刻被发送到网络上——它只是把数据放到了 Qt 的发送缓冲区里。如果你希望数据尽快发出去，可以调用 `flush()`。

接下来是读取数据。在异步模型下，我们通过 `readyRead` 信号来获知有新数据到达：

```cpp
connect(socket, &QTcpSocket::readyRead, [=]() {
    QByteArray data = socket->readAll();
    qDebug() << "Received:" << data;
});
```

`readAll()` 会一次性读取缓冲区中所有可用的数据并返回一个 QByteArray。你可能需要注意一个细节：TCP 是字节流协议，不保证消息边界。也就是说，发送端调用一次 `write("Hello")` 再调用一次 `write("World")`，接收端可能只收到一次 `readyRead` 信号，内容是 `"HelloWorld"`。这就意味着如果你需要按消息来解析数据，必须在应用层自己实现消息分帧——比如用固定长度的消息头标记消息体长度，或者用特定的分隔符。

### 3.3 服务端：QTcpServer::listen + newConnection 信号

现在我们来看服务端。QTcpServer 的职责是监听某个端口，等待客户端连上来。每当有新客户端连接时，它会发出 `newConnection` 信号，你在槽函数里调用 `nextPendingConnection()` 拿到和这个客户端通信的 QTcpSocket。

```cpp
QTcpServer *server = new QTcpServer(this);

// 监听本机的 12345 端口
if (!server->listen(QHostAddress::Any, 12345)) {
    qDebug() << "Server could not start:" << server->errorString();
    return;
}

qDebug() << "Server listening on port 12345...";

// 有新连接时触发
connect(server, &QTcpServer::newConnection, [=]() {
    QTcpSocket *client = server->nextPendingConnection();
    qDebug() << "New client connected:"
             << client->peerAddress().toString()
             << ":" << client->peerPort();

    // 读取客户端发来的数据
    connect(client, &QTcpSocket::readyRead, [=]() {
        QByteArray data = client->readAll();
        qDebug() << "From client:" << data;

        // 回一条消息
        client->write("Message received.");
        client->flush();
    });

    // 客户端断开时清理
    connect(client, &QTcpSocket::disconnected, [=]() {
        qDebug() << "Client disconnected.";
        client->deleteLater();
    });
});
```

`listen()` 的第一个参数是监听地址，`QHostAddress::Any` 表示监听所有网卡接口（包括 localhost 和局域网 IP）。如果你只想本机访问，可以用 `QHostAddress::LocalHost`。第二个参数是端口号。

这里有一个特别容易踩的坑：`nextPendingConnection()` 返回的 QTcpSocket 的所有权归你。如果你不接管它（不设 parent、不调用 deleteLater），它就会一直存在造成内存泄漏。所以务必在 `disconnected` 信号的处理函数里调用 `deleteLater()`。

另外，你可能会问，如果同时有大量客户端连接怎么办？QTcpServer 内部会维护一个待处理连接的队列，`newConnection` 信号会在队列中有新连接时触发。如果你处理得不够快，队列满了之后新的连接可能会被系统拒绝。默认队列大小取决于操作系统的 `SOMAXCONN` 参数，Linux 下通常是 128。如果需要更大的并发量，可以在 listen 之后通过操作系统的 API 调整。

### 3.4 readyRead 异步读取的细节

`readyRead` 信号是 Qt 网络编程中你打交道最多的信号之一。它在新数据可读时被触发，但它的触发行为有一些微妙之处值得深入了解。

首先，`readyRead` 不一定每次只对应一条「消息」。前面说过 TCP 是字节流，所以 `readyRead` 触发时缓冲区里可能有一整条消息，也可能有半条消息，还可能有多条消息拼接在一起。你的读取逻辑必须能处理这些情况。

其次，`readyRead` 不会在缓冲区有数据时连续触发。它是边缘触发（edge-triggered）的——当有新数据到达时触发一次，之后如果你不把数据读完，它不会再触发，直到又有新数据进来。这听起来有点反直觉，但实际上 Qt 在实现上做了一些处理：在事件循环中，只要缓冲区里还有未读数据且没有被 read 操作中断过，Qt 会持续发出 `readyRead`。不过最安全的做法还是在槽函数里用循环把缓冲区读干净：

```cpp
connect(socket, &QTcpSocket::readyRead, [=]() {
    while (socket->bytesAvailable() > 0) {
        QByteArray data = socket->read(1024);  // 每次最多读 1024 字节
        // 处理数据...
    }
});
```

这种写法比 `readAll()` 更可控，特别是当数据量很大的时候，可以分块处理，避免一次性分配一大块内存。

### 3.5 disconnected 信号与重连策略

网络连接断开是正常现象——可能是用户主动断开，可能是网络波动，也可能是服务端崩了。QTcpSocket 提供了 `disconnected` 信号来通知你连接已断开，同时还有 `errorOccurred` 信号来通知你发生了错误。

```cpp
connect(socket, &QTcpSocket::disconnected, [=]() {
    qDebug() << "Connection closed.";
});

connect(socket, &QAbstractSocket::errorOccurred, [=](QAbstractSocket::SocketError error) {
    qDebug() << "Socket error:" << error << socket->errorString();
});
```

在实际项目中，客户端通常需要实现自动重连机制。一个简单的做法是在检测到断开后，启动一个定时器延迟几秒后尝试重连。之所以要延迟而不是立刻重连，是因为如果服务端确实挂了，立刻重连只会白白消耗资源。

```cpp
void reconnect(QTcpSocket *socket) {
    qDebug() << "Attempting to reconnect in 3 seconds...";
    QTimer::singleShot(3000, [=]() {
        socket->connectToHost(QHostAddress("127.0.0.1"), 12345);
        if (!socket->waitForConnected(5000)) {
            qDebug() << "Reconnect failed, retrying...";
            reconnect(socket);  // 递归重试
        } else {
            qDebug() << "Reconnected successfully!";
        }
    });
}
```

这里用 `QTimer::singleShot` 做延迟重连，比 `QThread::sleep()` 好得多——它不会阻塞事件循环。当然，在生产环境中你可能还需要加上最大重试次数限制、指数退避策略（每次重连间隔加倍）等，避免无限重连把系统搞崩。

另一个需要注意的点是：当 `disconnected` 信号触发时，Socket 的状态变成了 `UnconnectedState`，但 Socket 对象本身还没被销毁。如果你想复用这个 Socket 进行重连，直接调用 `connectToHost()` 就行，不需要重新 new 一个。但如果你要彻底放弃这个连接，记得调用 `deleteLater()` 清理资源。

## 4. 踩坑预防清单

关于 Socket 状态的问题：QTcpSocket 有一个 `state()` 方法返回当前状态，在调试的时候特别有用。状态值包括 `UnconnectedState`、`HostLookupState`、`ConnectingState`、`ConnectedState`、`ClosingState` 等。如果你发现某个操作失败了，先检查当前状态——比如在 `UnconnectedState` 下调用 `write()` 是不会成功的。

关于缓冲区的问题：发送端连续调用多次 `write()`，接收端不一定能通过 `readyRead` 的次数来对应。TCP 是流式的，它不维护消息边界。如果你需要按消息来处理，必须在应用层实现分帧，比如在每条消息前面加一个固定长度的 header 标记消息体长度，或者用换行符作为消息分隔。

关于 QTcpServer 的线程模型：默认情况下，QTcpServer 在主线程监听和发出 `newConnection` 信号，通过 `nextPendingConnection()` 拿到的 QTcpSocket 也在主线程。这意味着所有的 I/O 回调（`readyRead`、`disconnected` 等）都在主线程执行。对于大多数应用来说这没问题，因为 Qt 的异步 I/O 本身不会阻塞主线程。但如果你的单客户端数据处理逻辑特别重（比如大数据量的解析和计算），可能会影响界面响应。这时候可以考虑把 QTcpSocket 通过 `moveToThread()` 移到工作线程，或者用 `QTcpServer::incomingConnection()` 的重写来自己管理线程分配。

还有一个很多人忽略的事情是处理 `connectToHost()` 失败的情况。如果目标主机不可达或者端口没开放，`connectToHost()` 不会立刻失败——它会进入 `ConnectingState`，然后在超时或收到拒绝后触发 `errorOccurred` 信号。所以你必须监听这个信号来判断连接是否成功，不能只看 `waitForConnected()` 的返回值就完事了。

现在我们来做一个小的代码填空练习，检验一下前面的内容。补全以下代码，实现一个 TCP 客户端连接到服务器并发送一条消息：

```cpp
QTcpSocket *socket = new QTcpSocket(this);
socket->__________(QHostAddress("127.0.0.1"), 12345);

connect(socket, &QTcpSocket::readyRead, [=]() {
    QByteArray data = socket->__________();
    qDebug() << "Server response:" << data;
});

connect(socket, &QTcpSocket::__________, [=]() {
    qDebug() << "Connection lost.";
    socket->deleteLater();
});
```

提示：需要填入的方法分别是 `connectToHost`、`readAll`、`disconnected`。

参考答案如下：

```cpp
QTcpSocket *socket = new QTcpSocket(this);
socket->connectToHost(QHostAddress("127.0.0.1"), 12345);

connect(socket, &QTcpSocket::readyRead, [=]() {
    QByteArray data = socket->readAll();
    qDebug() << "Server response:" << data;
});

connect(socket, &QTcpSocket::disconnected, [=]() {
    qDebug() << "Connection lost.";
    socket->deleteLater();
});
```

## 5. 练习项目

我们来做一个简易回声服务器（Echo Server），把这一节学的东西串起来。服务端监听端口，客户端连上去发什么，服务端就原样发回来。

完成标准：服务端能同时处理多个客户端连接，每个客户端发送的消息都被原样回传。客户端发送 "quit" 后服务端主动断开该连接。断线的客户端被正确清理，不会内存泄漏。

提示几个方向：服务端在 `newConnection` 的槽函数里为每个客户端设置 `readyRead` 和 `disconnected` 的信号连接，`readyRead` 里读取数据并原样 `write` 回去，`disconnected` 里调用 `deleteLater()`。客户端可以做一个简单的命令行交互循环，用 `QSocketNotifier` 或者 `QCoreApplication::processEvents()` 来同时处理标准输入和 Socket 事件。

再看一个调试挑战：以下服务端代码有什么问题？

```cpp
QTcpServer server;
server.listen(QHostAddress::Any, 12345);

connect(&server, &QTcpServer::newConnection, [&]() {
    QTcpSocket *client = server.nextPendingConnection();
    connect(client, &QTcpSocket::readyRead, [=]() {
        QByteArray data = client->readAll();
        qDebug() << "Received:" << data;
    });
});
```

这里面有几个隐患。首先，没有处理 `disconnected` 信号，客户端断开后 client 对象永远不会被删除，造成内存泄漏。其次，如果同一个客户端多次触发 `readyRead`，读取的数据可能会粘在一起或者不完整，因为 TCP 不保证消息边界。另外，没有对 `listen()` 的返回值做检查，如果端口被占用程序会静默失败。还缺少对 `errorOccurred` 信号的处理，网络异常时无法感知。

## 6. 完整示例代码

本篇的完整示例代码在 `examples/beginner/04-qtnetwork/01-tcp-socket-beginner/` 目录下，包含一个控制台程序，演示了 TCP 服务端和客户端的基本通信流程，包括连接建立、数据收发和断线检测。

## 7. 官方文档参考

- [Qt 文档 · QTcpSocket 类](https://doc.qt.io/qt-6/qtcpsocket.html) -- TCP 客户端的完整 API 参考，包含所有信号和状态说明
- [Qt 文档 · QTcpServer 类](https://doc.qt.io/qt-6/qtcpserver.html) -- TCP 服务端的 API 参考，包含监听和连接管理
- [Qt 文档 · QAbstractSocket 类](https://doc.qt.io/qt-6/qabstractsocket.html) -- QTcpSocket 的基类，定义了 Socket 状态和错误枚举

*（链接已验证，2026-04-23 可访问）*

---

到这里就大功告成了。掌握了 QTcpSocket 和 QTcpServer，你就有了在 Qt 里做任何 TCP 网络通信的基础。不管是自定义协议的服务端、还是接第三方 TCP 服务，这套模型都是通用的。下一节我们会讲 UDP 通信，看看无连接的数据报传输又是怎么一回事。
