# 现代Qt开发教程（新手篇）4.2——UDP 通信基础

## 1. 前言：为什么还需要 UDP

上一篇我们讲了 TCP，它可靠、有序、自带重传，听上去很完美。但如果你做过实时音视频、在线游戏或者局域网设备发现这类场景，你一定知道 TCP 在这些场合反而成了累赘——它的三次握手、拥塞控制、重传机制会引入不可接受的延迟。

UDP 就不一样了。它没有连接建立的过程，没有重传机制，不管你数据到没到它只管发。这听起来像是偷懒，但在很多场景下这恰恰是你需要的：与其等一个迟到的数据包重传到达（那时候已经没用了），不如直接丢掉它用最新的数据。实时性要求高、能容忍少量丢包的场景，UDP 才是正解。

Qt 给我们提供了 QUdpSocket 这个类来处理 UDP 通信。和 QTcpSocket 不同，QUdpSocket 不需要建立连接——你只需要绑定一个端口就能收数据，或者直接指定目标地址和端口就能发数据。它也支持广播和组播，这在局域网内做设备发现、服务发现的时候特别好用。

这一篇我们把 QUdpSocket 的收发流程、广播机制以及 UDP 的不可靠性带来的上层设计考量全部过一遍。

## 2. 环境说明

本教程基于 Qt 6.9.1，适用于 Windows 10/11（MSVC 2019+ 或 MinGW 11.2+）、Linux（GCC 11+）、WSL2 + WSLg（GUI 支持）。所有示例代码均使用 C++17 标准和 CMake 3.26+ 构建系统。本篇同样需要链接 Qt6::Network 模块。

## 3. 核心概念讲解

### 3.1 UDP 和 TCP 的核心区别

在动手写代码之前，我们先把 UDP 和 TCP 的本质区别搞清楚，不然后面遇到的各种行为你会觉得莫名其妙。

TCP 是面向连接的字节流协议。连接建立后，数据像水流一样连续传输，接收端看到的是一条无界的字节流，没有「消息」的概念。TCP 保证数据有序到达、不丢失、不重复，代价是延迟可能较高。

UDP 是无连接的数据报协议。每次发送都是一个独立的数据报（datagram），每个数据报都有明确的边界——你发一个 100 字节的数据报，接收端要么收到完整的 100 字节，要么什么都收不到。UDP 不保证数据到达、不保证顺序、不保证不重复，但它的延迟极低，开销极小。

这个区别直接决定了我们写代码的方式。TCP 用 `write()` 和 `readAll()` 处理字节流，需要自己做消息分帧；UDP 用 `writeDatagram()` 和 `readDatagram()` 处理独立的数据报，每次收发都是完整的消息，天然就有边界。

另外，UDP 有一个 TCP 没有的能力：广播。TCP 只能一对一通信，而 UDP 可以向整个子网发送广播数据报，局域网内所有监听该端口的程序都能收到。这在设备发现、局域网聊天等场景下非常方便。

### 3.2 bind 绑定端口接收数据

要接收 UDP 数据报，你必须先绑定一个端口。绑定操作告诉操作系统：「这个端口上的数据给我，别人别抢」。这和 TCP 服务端的 `listen()` 有点像，但更轻量——不需要监听连接，只需要声明端口归属。

```cpp
QUdpSocket *socket = new QUdpSocket(this);

// 绑定到 12345 端口，允许地址重用
if (!socket->bind(QHostAddress::Any, 12345,
                  QAbstractSocket::ShareAddress)) {
    qDebug() << "Bind failed:" << socket->errorString();
    return;
}

qDebug() << "UDP socket bound to port 12345";
```

`bind()` 的第一个参数是监听地址，`QHostAddress::Any` 表示监听所有网卡接口。第二个参数是端口号。第三个参数是绑定模式，`QAbstractSocket::ShareAddress` 允许多个 Socket 绑定同一个端口——这在广播场景下很常见，因为你可能希望同一台机器上的多个程序都能收到广播数据。

绑定成功后，当有数据报到达这个端口时，Socket 会发出 `readyRead` 信号。和 TCP 一样，我们通过连接这个信号来异步读取数据。

这里有一个比较常见的坑：如果你绑定的时候不指定 `QAbstractSocket::ShareAddress`（或者 `QAbstractSocket::ReuseAddressHint`），在同一台机器上运行两个实例绑定同一个端口会失败。这在调试的时候经常遇到——你启动了程序 A 绑了 12345 端口，然后启动程序 B 也想绑 12345，结果程序 B 的 bind 直接报错。解决办法就是加上 `ShareAddress` 或者给不同的实例分配不同的端口。

### 3.3 writeDatagram / readDatagram 数据报收发

UDP 的发送和接收分别通过 `writeDatagram()` 和 `readDatagram()` 完成。这两个方法是 UDP 和 TCP 在 API 层面最大的区别。

发送数据报只需要指定目标地址和端口，不需要提前建立连接：

```cpp
QUdpSocket socket;

// 向目标主机发送数据报
QByteArray data = "Hello, UDP!";
qint64 bytesWritten = socket.writeDatagram(
    data, QHostAddress("127.0.0.1"), 12345);

if (bytesWritten == -1) {
    qDebug() << "Send failed:" << socket.errorString();
} else {
    qDebug() << "Sent" << bytesWritten << "bytes";
}
```

`writeDatagram()` 的三个参数分别是：要发送的数据（QByteArray）、目标地址（QHostAddress）、目标端口号。返回值是实际写入的字节数，-1 表示发送失败。

接收数据报则通过 `readyRead` 信号驱动：

```cpp
connect(socket, &QUdpSocket::readyRead, [=]() {
    while (socket->hasPendingDatagrams()) {
        // 获取数据报大小，用于预分配缓冲区
        qint64 size = socket->pendingDatagramSize();
        QByteArray buffer;
        buffer.resize(size);

        QHostAddress senderAddr;
        quint16 senderPort;

        // 读取数据报，同时获取发送方的地址和端口
        qint64 bytesRead = socket->readDatagram(
            buffer.data(), buffer.size(), &senderAddr, &senderPort);

        if (bytesRead == -1) {
            qDebug() << "Read failed:" << socket->errorString();
            continue;
        }

        qDebug() << "Received" << bytesRead << "bytes from"
                 << senderAddr.toString() << ":" << senderPort
                 << "->" << buffer;
    }
});
```

这里有几个细节值得注意。`hasPendingDatagrams()` 检查缓冲区中是否还有未读的数据报，`pendingDatagramSize()` 返回下一个数据报的字节数。因为每个数据报的大小可能不同，所以最好先查询大小再分配缓冲区。

`readDatagram()` 的最后两个参数是可选的输出参数，用来获取发送方的地址和端口。这在很多场景下都很有用——比如你想给发送方回复消息，就需要知道它的地址和端口。

另外，`readDatagram()` 是一次只读一个数据报。如果缓冲区里有多个数据报，需要用 while 循环把它们全部读完。这和 TCP 的 `readAll()` 不同——TCP 读完缓冲区里所有的字节流，UDP 读完一个完整的数据报就结束。

### 3.4 QHostAddress::Broadcast 局域网广播

UDP 的广播功能是它和 TCP 相比最独特的优势之一。通过向广播地址发送数据报，你可以让局域网内所有监听对应端口的程序都收到消息。这在设备发现、局域网聊天、服务注册等场景下非常有用。

广播地址有两种：定向广播地址（子网的最后一个地址，比如 192.168.1.255）和全局广播地址（255.255.255.255）。Qt 提供了 `QHostAddress::Broadcast` 来表示全局广播地址：

```cpp
QUdpSocket socket;
QByteArray data = "Discovery: Who is there?";

// 向整个局域网广播
socket.writeDatagram(data, QHostAddress::Broadcast, 12345);
qDebug() << "Broadcast sent on port 12345";
```

这段代码会把消息发到局域网内所有设备上。任何绑定了 12345 端口的程序都会收到这条消息。

但是，广播也有它的限制。首先，广播只限于同一个子网——路由器不会转发广播包。如果你想跨子网发送，需要用组播（Multicast）而不是广播。其次，广播是一种「扰民」行为——它会占用网络带宽，甚至可能导致广播风暴。所以广播消息的频率和数据量要控制好，别动不动就发个几 MB 的广播包。

另外，在某些操作系统上（特别是 Windows），发送广播可能需要特殊权限或者 socket 选项。Qt 在内部已经处理了大部分平台差异，但如果你的广播发不出去，可以检查一下防火墙设置和 Socket 的 `SocketOption::Broadcast` 选项是否被正确设置。

### 3.5 UDP 的不可靠性与上层确认机制

这是很多初学者容易忽略的一点：UDP 不保证数据送达。你调用 `writeDatagram()` 成功了，只是说明数据被放到了内核的发送缓冲区，不意味着对方收到了。

在实际项目中，如果你用 UDP 来传输关键数据（比如配置信息、控制指令），就必须在应用层实现确认机制。一个简单的方案是「请求-应答」模式：发送方发出数据后启动一个定时器，如果在超时时间内没收到对方的确认回复，就重发。重发若干次后仍然失败，则认为对方不可达。

```cpp
// 简化的确认重发逻辑
void sendWithAck(QUdpSocket *socket,
                 const QByteArray &data,
                 const QHostAddress &addr,
                 quint16 port,
                 int maxRetries = 3)
{
    int retries = 0;
    QTimer *timer = new QTimer;
    timer->setSingleShot(true);

    // 收到 ACK 后停止重发
    QObject::connect(socket, &QUdpSocket::readyRead, [=]() {
        while (socket->hasPendingDatagrams()) {
            QByteArray reply;
            reply.resize(socket->pendingDatagramSize());
            socket->readDatagram(reply.data(), reply.size());
            if (reply == "ACK") {
                timer->stop();
                timer->deleteLater();
                qDebug() << "ACK received, stop retrying.";
            }
        }
    });

    // 超时重发
    QObject::connect(timer, &QTimer::timeout, [=]() {
        if (retries < maxRetries) {
            ++retries;
            qDebug() << "Timeout, retrying..." << retries;
            socket->writeDatagram(data, addr, port);
            timer->start(1000);
        } else {
            qDebug() << "Max retries reached, giving up.";
            timer->deleteLater();
        }
    });

    // 首次发送
    socket->writeDatagram(data, addr, port);
    timer->start(1000);
}
```

这段代码展示了一个最基本的确认重发逻辑。当然，在生产环境中你还需要考虑更多细节：给每个消息加序号来区分不同的消息、处理乱序到达的 ACK、设置合理的超时时间、实现指数退避等。说白了，你在 UDP 上要做的事情就是把 TCP 的可靠性机制根据你的实际需求重新实现一遍——只不过你可以选择性地实现，不必像 TCP 那样做全套。

这也引出了一个重要的设计原则：如果你的应用场景对可靠性要求很高，而且不需要极致的低延迟，那直接用 TCP 就好了，别在 UDP 上重复造轮子。UDP 的优势在于它的灵活性和低延迟，适合那些「丢了就丢了，无所谓」或者「我自己来实现可靠性」的场景。

## 4. 踩坑预防清单

关于数据报大小的问题：UDP 数据报的最大安全大小是 512 字节。虽然理论上 IPv4 的 UDP 数据报最大可以是 65507 字节，但超过 MTU（通常是 1500 字节）的数据报会被 IP 层分片，而任何一个分片丢了整个数据报就废了。所以如果你的数据可能超过 MTU，要么控制在安全范围内，要么自己做拆包和重组。

关于 `writeDatagram()` 的返回值：它返回的是写入的字节数，不是对方收到的字节数。即使返回值等于你发送的数据长度，也不代表数据送达了。UDP 没有确认机制，发送成功只意味着数据从用户空间拷贝到了内核缓冲区。

关于端口绑定冲突：在同一台机器上调试 UDP 程序时，经常遇到端口被占用的问题。可以用 `netstat -an | grep 12345` 或者 `ss -an | grep 12345` 来检查端口占用情况。如果绑定时加上 `QAbstractSocket::ShareAddress` 标志，同一端口可以被多个 Socket 共享绑定。

关于跨平台的行为差异：不同操作系统对 UDP 广播的处理方式不完全一样。Linux 默认允许广播，Windows 可能需要显式开启 `SO_BROADCAST` 选项。Qt 的 QUdpSocket 在底层做了适配，但如果你发现广播发不出去，还是检查一下系统防火墙和网络配置。

现在我们来做一个小的代码填空练习，检验一下前面的内容。补全以下代码，实现 UDP 接收端绑定端口并打印收到的数据报：

```cpp
QUdpSocket *socket = new QUdpSocket(this);
socket->__________(QHostAddress::Any, 12345);

connect(socket, &QUdpSocket::readyRead, [=]() {
    while (socket->__________()) {
        QByteArray buffer;
        buffer.resize(socket->___________________());
        QHostAddress addr;
        quint16 port;
        socket->__________(buffer.data(), buffer.size(), &addr, &port);
        qDebug() << "From" << addr << ":" << port << "->" << buffer;
    }
});
```

提示：需要填入的方法分别是 `bind`、`hasPendingDatagrams`、`pendingDatagramSize`、`readDatagram`。

参考答案如下：

```cpp
QUdpSocket *socket = new QUdpSocket(this);
socket->bind(QHostAddress::Any, 12345);

connect(socket, &QUdpSocket::readyRead, [=]() {
    while (socket->hasPendingDatagrams()) {
        QByteArray buffer;
        buffer.resize(socket->pendingDatagramSize());
        QHostAddress addr;
        quint16 port;
        socket->readDatagram(buffer.data(), buffer.size(), &addr, &port);
        qDebug() << "From" << addr << ":" << port << "->" << buffer;
    }
});
```

## 5. 练习项目

我们来做一个局域网设备发现工具，把这一节学的东西串起来。一台机器作为发现者向局域网广播 "DISCOVER" 消息，其他机器收到后回复自己的主机名和 IP 地址。

完成标准：发现者每 5 秒广播一次 "DISCOVER" 消息到 12345 端口，响应者在收到 "DISCOVER" 后将自己的主机名和 IP 通过 UDP 单播回复给发现者。发现者维护一个在线设备列表，3 次广播周期内没有回复的设备视为下线并移除。

提示几个方向：发现者用 `QHostAddress::Broadcast` 广播，响应者绑定 12345 端口监听。回复时用 `readDatagram` 获取到的发送方地址和端口进行单播。设备列表可以用 `QMap<QHostAddress, QDateTime>` 记录每个设备的最后活跃时间，每次广播后检查超时并清理过期条目。

再看一个调试挑战：以下 UDP 接收代码有什么问题？

```cpp
QUdpSocket socket;
socket.bind(QHostAddress::Any, 12345);

connect(&socket, &QUdpSocket::readyRead, [&]() {
    QByteArray data = socket.readAll();
    qDebug() << "Received:" << data;
});
```

这里面有几个问题。首先，`readAll()` 是 QIODevice 的通用方法，在 QUdpSocket 上它会把所有待处理的数据报拼接成一个 QByteArray，丢失了数据报的边界信息。UDP 应该用 `readDatagram()` 逐个读取。其次，没有用 `hasPendingDatagrams()` 循环读取所有待处理的数据报，`readyRead` 只触发一次但缓冲区可能有多个数据报。另外，没有获取发送方的地址和端口，如果需要回复就做不到了。还有 `readAll()` 在 UDP 上可能把多个数据报混在一起，导致数据解析错误。

## 6. 完整示例代码

本篇的完整示例代码在 `examples/beginner/04-qtnetwork/02-udp-socket-beginner/` 目录下，包含一个控制台程序，演示了 UDP 单播、广播以及数据报收发的完整流程。

## 7. 官方文档参考

- [Qt 文档 · QUdpSocket 类](https://doc.qt.io/qt-6/qudpsocket.html) -- UDP Socket 的完整 API 参考，包含广播和组播的说明
- [Qt 文档 · QHostAddress 类](https://doc.qt.io/qt-6/qhostaddress.html) -- IP 地址封装类，包含 Broadcast 等特殊地址常量
- [Qt 文档 · QNetworkDatagram 类](https://doc.qt.io/qt-6/qnetworkdatagram.html) -- Qt 5.8+ 提供的数据报封装类，包含更丰富的元信息

*（链接已验证，2026-04-23 可访问）*

---

到这里就大功告成了。UDP 比 TCP 简单很多，但它的不可靠性意味着你需要在应用层做更多的工作。选 TCP 还是选 UDP，永远取决于你的应用场景。下一篇我们会讲 QNetworkAccessManager，看看 Qt 怎么帮你优雅地处理 HTTP 请求。
