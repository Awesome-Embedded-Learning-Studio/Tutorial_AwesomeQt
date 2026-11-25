---
title: "4.2 UDP 进阶：多播、大数据分片重组"
description: "入门篇我们把 QUdpSocket 的基本收发流程跑通了——绑定端口、send/receive datagram、广播。老实说，会这些就能写个局域网聊天室或者设备发现服务了。但 UDP 的进阶用法远不止于此。"
---

# 现代Qt开发教程（进阶篇）4.2——UDP 进阶：多播、大数据分片重组

## 1. 前言 / UDP 的进阶战场

入门篇我们把 QUdpSocket 的基本收发流程跑通了——绑定端口、send/receive datagram、广播。老实说，会这些就能写个局域网聊天室或者设备发现服务了。但 UDP 的进阶用法远不止于此。

首先是多播（Multicast）。广播是向整个子网喊话，所有设备都会收到——不管它想不想听。多播则是向一个「频道」喊话，只有订阅了这个频道的设备才会收到。这在流媒体分发、集群节点通信、物联网设备组网中是比广播更优雅也更高效的方案。操作系统和路由器会配合做多播组的成员管理，数据包只在需要的网络分支上转发，不会像广播那样把整条网路塞满。

然后是大数据的分片重组。UDP 单个数据报的理论最大尺寸受 IP 协议限制（65535 字节），但实际上大多数操作系统会进一步限制——Linux 默认的 socket 缓冲区通常只接受 2KB 左右的 UDP 数据报，超过就会被截断或丢弃。如果你想用 UDP 传一个 100KB 的文件块，就必须自己在应用层做分片、编号、重组。这还不算完——UDP 本身不保证到达顺序，也不保证不丢包，所以你的重组逻辑还得处理乱序到达和分片丢失的情况。

这篇我们就一起来把多播组管理、大数据分片重组协议、以及一个简单的 ARQ（自动重传请求）机制搞定。

## 2. 环境说明

本文档基于 Qt 6.2+ 编写，使用 C++17 标准和 CMake 3.26+ 构建系统。默认你已经掌握入门篇中 QUdpSocket 的基本用法，包括 `bind()`、`writeDatagram()`、`readDatagram()`、`readyRead` 信号。本篇依赖 Qt6::Network 模块。多播功能需要在支持 IGMP 的网络环境中测试，单机开发可以用 127.0.0.1 的多播地址回环测试。

## 3. 核心概念讲解

### 3.1 多播组——从「群发广播」到「频道订阅」

多播使用 D 类 IP 地址（224.0.0.0 ~ 239.255.255.255）作为组地址。其中 224.0.0.0 ~ 224.0.0.255 是保留的本地网络控制地址，224.0.1.0 ~ 238.255.255.255 是全球可路由的多播地址，239.0.0.0 ~ 239.255.255.255 是本地管理范围的多播地址（类似局域网保留地址）。开发测试中我们通常使用 239.x.x.x 范围。

在 Qt 中加入多播组非常简单——先 bind 一个端口，然后调用 `joinMulticastGroup()` 指定组地址。操作系统底层会发送 IGMP 加入报文，路由器收到后开始向你的网络分支转发该组的数据。

```cpp
QUdpSocket *socket = new QUdpSocket(this);

// 绑定端口，必须用 ShareAddress 模式允许多个 socket 监听同一端口
if (!socket->bind(QHostAddress::AnyIPv4, 42424,
                  QAbstractSocket::ShareAddress)) {
    qDebug() << "Bind failed:" << socket->errorString();
    return;
}

// 加入多播组
QHostAddress groupAddress("239.255.43.21");
if (!socket->joinMulticastGroup(groupAddress)) {
    qDebug() << "Failed to join multicast group:" << socket->errorString();
    return;
}

connect(socket, &QUdpSocket::readyRead, [=]() {
    while (socket->hasPendingDatagrams()) {
        QByteArray buffer;
        buffer.resize(socket->pendingDatagramSize());
        socket->readDatagram(buffer.data(), buffer.size());
        qDebug() << "Multicast received:" << buffer;
    }
});
```

发送多播数据报和发送普通 UDP 数据报完全一样——只是目标地址填多播组地址。所有加入了这个组的成员都会收到。

```cpp
// 向多播组发送数据
QByteArray message = "Hello, multicast group!";
socket->writeDatagram(message, groupAddress, 42424);
```

离开多播组用 `leaveMulticastGroup()`。这会触发操作系统发送 IGMP 离开报文，路由器在确认该网段没有其他成员后停止转发该组的数据。如果你忘了调用 leave 而是直接析构 socket，操作系统会自动处理——但显式离开是好习惯，特别是在资源敏感的嵌入式环境中。

这里有几个细节值得说一下。`bind()` 时必须使用 `QHostAddress::AnyIPv4` 而不是 `QHostAddress::Any`——在双栈系统上 `Any` 可能绑定到 IPv6，而多播组的 IPv4 地址无法在 IPv6 socket 上使用。如果你确实需要 IPv6 多播，组地址要换成 IPv6 的 `ff0x::` 前缀格式，并使用 `joinMulticastGroup` 的 IPv6 重载。

另一个细节是多播回环（Multicast Loopback）。默认情况下，同一台机器上发送的多播数据报会被自己收到。这在单机调试时很方便，但在生产环境中可能导致应用程序收到自己发出的消息。可以通过 `setSocketOption(QAbstractSocket::MulticastLoopbackOption, 0)` 关闭回环。

`QNetworkDatagram` 类是 Qt 5.8 引入的、比 `readDatagram()` 更高级的数据报封装。它不仅包含数据本身，还携带了发送方地址、目标地址、跳数限制（hop limit / TTL）等元信息。在多播场景下，你需要知道每条数据报来自哪个成员，`QNetworkDatagram` 就特别有用了。

```cpp
connect(socket, &QUdpSocket::readyRead, [=]() {
    while (socket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = socket->receiveDatagram();
        qDebug() << "From:" << datagram.senderAddress()
                 << datagram.senderPort()
                 << "Data:" << datagram.data();
    }
});
```

`receiveDatagram()` 返回一个 `QNetworkDatagram` 对象，你可以直接访问 `senderAddress()` 和 `senderPort()` 来确定消息来源。这比旧式 `readDatagram(data, size, &addr, &port)` 的四个参数更清晰，也避免了忘记传地址参数导致信息丢失的问题。

### 3.2 大数据分片——当单个数据报装不下

UDP 数据报的大小有一个硬性上限：IP 协议的最大包长 65535 字节，减去 IP 头 20 字节和 UDP 头 8 字节，理论上一个 UDP 数据报最多携带 65507 字节的数据。但这只是理论值。实际上，IP 层在传输超过 MTU（通常 1500 字节）的数据报时需要分片，而任何一个 IP 分片丢失都会导致整个数据报被丢弃。更大的问题是，很多操作系统的 socket 接收缓冲区会截断超大的 UDP 数据报——Linux 上默认的 `SO_RCVBUF` 对应的 UDP 接收上限只有 2KB 左右。

所以如果你的应用需要传输大于 1KB 的数据块（比如一个小图片、一段 JSON 配置、一个传感器数据批次），就必须在应用层做分片。

我们来设计一个简单的分片协议。假设每个数据块最大 100KB，我们把它切分成不超过 1400 字节的片段（留 100 字节给 IP/UDP 头和我们的协议头，避免 IP 层分片）。协议头包含：数据块 ID（4 字节，区分不同的数据块）、总片数（2 字节）、当前片序号（2 字节）、有效载荷。接收端按数据块 ID 分组，收齐所有片后按序号拼接。

```cpp
// 分片发送端
struct FragmentHeader
{
    quint32 blockId;
    quint16 totalFragments;
    quint16 fragmentIndex;
};

QByteArray serializeHeader(const FragmentHeader &header)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << header.blockId << header.totalFragments << header.fragmentIndex;
    return data;
}

void sendFragmentedData(QUdpSocket *socket, const QByteArray &payload,
                        const QHostAddress &target, quint16 port)
{
    static constexpr int kMaxPayload = 1400;
    static constexpr int kHeaderSize = 8;  // 4 + 2 + 2
    static quint32 globalBlockId = 0;

    quint32 blockId = globalBlockId++;
    int fragmentCount = (payload.size() + kMaxPayload - 1) / kMaxPayload;

    for (int i = 0; i < fragmentCount; ++i) {
        int offset = i * kMaxPayload;
        int chunkSize = qMin(kMaxPayload, payload.size() - offset);

        FragmentHeader header;
        header.blockId = blockId;
        header.totalFragments = static_cast<quint16>(fragmentCount);
        header.fragmentIndex = static_cast<quint16>(i);

        QByteArray packet = serializeHeader(header);
        packet.append(payload.mid(offset, chunkSize));

        socket->writeDatagram(packet, target, port);
    }
}
```

接收端的重组逻辑需要一个状态机。对每个正在接收的数据块，我们需要追踪：已经收到了哪些片、还缺哪些片、超时时间。用一个 `QMap<quint32, ReassemblyState>` 来管理所有正在重组的数据块。

```cpp
struct ReassemblyState
{
    quint16 totalFragments = 0;
    QSet<quint16> receivedIndices;
    QByteArray assembledData;
    QElapsedTimer lastActivity;
};

class FragmentReassembler : public QObject
{
    Q_OBJECT
public:
    explicit FragmentReassembler(QUdpSocket *socket, QObject *parent = nullptr)
        : QObject(parent), socket_(socket)
    {
        connect(socket_, &QUdpSocket::readyRead,
                this, &FragmentReassembler::onReadyRead);

        // 定期清理超时的数据块
        cleanupTimer_.setInterval(5000);
        connect(&cleanupTimer_, &QTimer::timeout,
                this, &FragmentReassembler::cleanupStale);
        cleanupTimer_.start();
    }

signals:
    void dataAssembled(QByteArray data);

private:
    void onReadyRead()
    {
        while (socket_->hasPendingDatagrams()) {
            QByteArray packet;
            packet.resize(socket_->pendingDatagramSize());
            socket_->readDatagram(packet.data(), packet.size());

            if (packet.size() < kHeaderSize) continue;

            // 解析头部
            QDataStream stream(packet.left(kHeaderSize));
            stream.setByteOrder(QDataStream::BigEndian);
            FragmentHeader header;
            stream >> header.blockId >> header.totalFragments
                   >> header.fragmentIndex;

            QByteArray fragment = packet.mid(kHeaderSize);

            auto &state = reassemblyMap_[header.blockId];
            state.totalFragments = header.totalFragments;
            state.lastActivity.start();

            // 重组：按序号放到正确位置
            int offset = header.fragmentIndex * kMaxPayload;
            if (state.assembledData.size() < offset + fragment.size()) {
                state.assembledData.resize(offset + fragment.size());
            }
            std::memcpy(state.assembledData.data() + offset,
                        fragment.constData(), fragment.size());
            state.receivedIndices.insert(header.fragmentIndex);

            // 检查是否收齐
            if (state.receivedIndices.size() == state.totalFragments) {
                emit dataAssembled(state.assembledData);
                reassemblyMap_.remove(header.blockId);
            }
        }
    }

    void cleanupStale()
    {
        const qint64 kTimeoutMs = 10000;  // 10 秒超时
        QList<quint32> stale;
        for (auto it = reassemblyMap_.constBegin();
             it != reassemblyMap_.constEnd(); ++it) {
            if (it->lastActivity.elapsed() > kTimeoutMs) {
                stale.append(it.key());
            }
        }
        for (quint32 id : stale) {
            qDebug() << "Dropping stale block:" << id;
            reassemblyMap_.remove(id);
        }
    }

    QUdpSocket *socket_;
    QTimer cleanupTimer_;
    QMap<quint32, ReassemblyState> reassemblyMap_;

    static constexpr int kHeaderSize = 8;
    static constexpr int kMaxPayload = 1400;
};
```

重组逻辑中有两个关键设计。第一个是超时清理：如果一个数据块在 10 秒内没有收齐所有分片，直接丢弃并清理内存。网络中总有丢包，死等一个永远凑不齐的数据块只会让 `reassemblyMap_` 越来越大。第二个是按偏移量直接写入 `assembledData`——因为我们用的固定片大小，所以第 N 片的数据在最终缓冲区中的偏移量就是 `N * kMaxPayload`，直接 memcpy 过去即可。最后一片可能不满，但 `resize` 已经把缓冲区扩到了正确的大小。

### 3.3 简单 ARQ——让 UDP 稍微可靠一点

分片重组解决了大数据的传输问题，但没有解决丢包问题。如果丢了一个分片，整个数据块就废了。在某些场景下（比如固件升级包传输、配置文件同步），我们需要 UDP 有一定程度的可靠性，但又不想承受 TCP 的连接开销和延迟。

最简单的可靠传输方案是停等式 ARQ（Stop-and-Wait ARQ）：发送一个分片，等接收端回复 ACK，收到 ACK 后再发下一个。超时未收到 ACK 就重发。这个方案吞吐量很低（同一时间只有一片在飞），但实现极简，适合低带宽、低丢包率的场景。

```cpp
// 简化的停等 ARQ 发送端
class StopAndWaitSender : public QObject
{
    Q_OBJECT
public:
    StopAndWaitSender(QUdpSocket *socket, QObject *parent = nullptr)
        : QObject(parent), socket_(socket)
    {
        connect(socket_, &QUdpSocket::readyRead,
                this, &StopAndWaitSender::onAckReceived);
        connect(&retransmitTimer_, &QTimer::timeout,
                this, &StopAndWaitSender::onRetransmit);
        retransmitTimer_.setSingleShot(true);
    }

    void sendReliable(const QByteArray &data,
                      const QHostAddress &target, quint16 port)
    {
        target_ = target;
        port_ = port;
        currentPayload_ = data;
        sendAttempt_ = 0;
        sendPacket();
    }

private:
    void sendPacket()
    {
        QByteArray packet;
        // 标记为数据包（0x01）
        packet.append(static_cast<char>(0x01));
        packet.append(currentPayload_);
        socket_->writeDatagram(packet, target_, port_);
        sendAttempt_++;

        if (sendAttempt_ >= kMaxRetries) {
            qDebug() << "Max retries reached, giving up.";
            retransmitTimer_.stop();
            return;
        }
        retransmitTimer_.start(kRetransmitMs);
    }

    void onAckReceived()
    {
        while (socket_->hasPendingDatagrams()) {
            QByteArray packet;
            packet.resize(socket_->pendingDatagramSize());
            socket_->readDatagram(packet.data(), packet.size());

            if (packet.size() == 1 && packet[0] == 0x02) {
                // ACK 包
                retransmitTimer_.stop();
                emit sendComplete();
            }
        }
    }

    void onRetransmit()
    {
        qDebug() << "Timeout, retransmitting (attempt" << sendAttempt_ << ")";
        sendPacket();
    }

signals:
    void sendComplete();

    QUdpSocket *socket_;
    QHostAddress target_;
    quint16 port_ = 0;
    QByteArray currentPayload_;
    QTimer retransmitTimer_;
    int sendAttempt_ = 0;

    static constexpr int kRetransmitMs = 500;
    static constexpr int kMaxRetries = 5;
};
```

接收端收到数据包后回复一个 ACK 包（`0x02`），发送端收到 ACK 后停止重传定时器。超时 500ms 未收到 ACK 就重发，最多重试 5 次。

说实话，这个方案非常简陋。它每次只传一片数据，如果数据量大吞吐量很惨。但它确实解决了「UDP 丢包导致数据丢失」的核心问题，而且代码量极少。如果你需要更高的吞吐量，可以升级为滑动窗口协议（Go-Back-N 或 Selective Repeat），同时允许多个分片在飞，通过序号追踪每个分片的确认状态。那属于传输层协议设计的范畴了，本文的篇幅就不展开了。

现在有一道思考题。假设你的多播应用在局域网测试时一切正常，但部署到跨网段的生产环境后，某些节点收不到多播数据。可能的原因是什么？

答案是多播路由。多播数据包的转发依赖网络中的路由器支持 IGMP 协议和 PIM（Protocol Independent Multicast）路由协议。如果两个网段之间的路由器没有配置多播路由，多播数据包就无法跨越网段。解决方案有两个：在路由器上启用多播路由（需要网络管理员配合），或者退回到应用层转发——选一个节点做中继，把多播数据封装成单播转发到其他网段。

## 4. 踩坑预防

第一个坑是 `bind()` 时地址族不匹配导致 `joinMulticastGroup()` 失败。如果你用 `QHostAddress::Any` 绑定，在双栈系统上可能绑定到 IPv6，然后尝试加入 IPv4 多播组（239.x.x.x）就会失败，错误信息通常是 "The address is not in the appropriate format"。解决方案是明确使用 `QHostAddress::AnyIPv4`。如果你确实需要同时支持 IPv4 和 IPv6 多播，必须创建两个独立的 socket，分别绑定到 `AnyIPv4` 和 `AnyIPv6`，使用各自对应的组地址。

第二个坑是 UDP 数据报被截断。当发送的数据报超过操作系统的接收缓冲区大小时，不同系统行为不一致：Linux 会截断数据报（你只收到前面一部分），macOS 可能丢弃整个数据报。更阴险的是，`pendingDatagramSize()` 可能返回完整大小，但实际读到的数据被截断了——你以为读完了，其实丢了一截。解决方案是控制单个数据报大小不超过 1400 字节（以太网 MTU 1500 减去 IP/UDP 头），大数据走分片协议。

第三个坑是多播组的「幽灵成员」。进程崩溃退出时没有调用 `leaveMulticastGroup()`，操作系统会通过 IGMP 离开报文自动处理。但如果网络中断（网线拔了、WiFi 断了），路由器不知道成员已离开，还会继续往这个网段转发多播数据，白白占用带宽。IGMP 有成员查询机制（路由器定期问「还有人在吗？」），但间隔较长（默认 125 秒）。如果你的场景对带宽敏感，可以考虑在应用层做周期性的成员心跳——定期广播一个「我还活着」的消息，长时间没心跳的成员视为离线。

## 5. 练习项目

练习项目：多播文件传输工具。我们要实现一个基于多播的文件分发工具，一台发送端把文件切成分片通过多播组发送，多台接收端同时接收并重组。

具体要求：发送端把文件切成 1400 字节的分片，使用本文的分片协议格式（block ID + 总片数 + 片序号）。接收端使用 FragmentReassembler 重组文件。添加简单的停等式 ARQ——接收端每收齐一个数据块回复 ACK，发送端超时重发。完成标准：发送端能把一个 500KB 的文件完整地传输给 3 个接收端、丢包模拟下接收端能通过重传获取缺失分片、接收端正确重组文件并校验完整性。

提示几个关键点：在分片协议头里额外加一个 4 字节的 CRC32 校验和字段，接收端重组后先校验再写入文件。ACK 可以用数据块 ID 来标识，不需要逐片确认。丢包模拟可以用一个概率过滤器——发送端以 10% 的概率随机丢弃分片。

## 6. 官方文档参考链接

[Qt 文档 · QUdpSocket](https://doc.qt.io/qt-6/qudpsocket.html) -- UDP socket 完整 API，包含多播组管理和数据报收发

[Qt 文档 · QNetworkDatagram](https://doc.qt.io/qt-6/qnetworkdatagram.html) -- UDP 数据报封装类，包含发送方/目标地址和跳数限制等元信息

[Qt 文档 · QAbstractSocket](https://doc.qt.io/qt-6/qabstractsocket.html) -- Socket 基类，定义了 MulticastLoopbackOption 等多播相关 socket 选项

*（链接已验证，2026-06-11 可访问）*

---

到这里就大功告成了。多播、分片重组、简单 ARQ——这三个工具组合起来，你就能用 UDP 做很多 TCP 做不好或者做起来太重的事情。下一篇我们来看 HTTP 的进阶用法，包括请求队列、拦截器和 Cookie 管理。
