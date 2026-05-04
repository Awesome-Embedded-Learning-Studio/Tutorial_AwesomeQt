# 现代Qt开发教程（新手篇）5.19--QtWebSockets 服务端实战

## 1. 前言：为什么要在 Qt 里直接起一个 WebSocket 服务

上一篇文章我们用 QtHttpServer 搭了一个 REST API，那套东西处理请求-响应模型绰绰有余。但如果你要做一个实时双向通信的场景——比如聊天室、实时数据推送、多人协作白板、设备状态监控面板——HTTP 的请求-响应模式就显得笨拙了。你得用轮询（polling）或者长轮询（long polling），前者浪费带宽，后者阻塞连接，两种方案都不优雅。WebSocket 协议就是为这种全双工通信场景设计的：客户端和服务端建立一次 TCP 连接后，双方随时可以主动发消息，直到连接关闭。

Qt 的 WebSockets 模块提供了完整的 WebSocket 协议实现——QWebSocketServer 在服务端监听端口、接受连接，QWebSocket 在客户端或服务端代表一个单独的 WebSocket 连接。这套 API 的设计风格和 QTcpServer / QTcpSocket 非常类似，如果你熟悉 Qt 的网络编程，上手基本没有门槛。

这篇我们要做的是用 QWebSocketServer 搭建一个 WebSocket 服务端，管理多个客户端连接，实现广播消息，并配置 SSL 加密的 WSS 服务。

## 2. 环境说明

本篇基于 Qt 6.9.1，需要引入 WebSockets 模块。CMake 配置如下：

```cmake
find_package(Qt6 REQUIRED COMPONENTS WebSockets)
```

QtWebSockets 从 Qt 5.3 开始提供，在 Qt 6 中属于附加模块（Qt Add-On）。它在协议层面完整实现了 RFC 6455（WebSocket 协议）和 RFC 7692（permessage-deflate 压缩扩展），支持文本帧和二进制帧，支持 ping/pong 心跳检测。底层传输层可以选择普通的 ws://（明文 TCP）或加密的 wss://（TLS over TCP）。

工具链方面没有特殊要求：MSVC 2019+、GCC 11+、Clang 14+，C++17 标准，CMake 3.26+。纯 CPU 运算，不依赖 GPU 或其他硬件。唯一需要注意的是 WSS 模式需要 SSL 证书文件，开发阶段可以用自签名证书，生产环境需要正规 CA 签发的证书。

## 3. 核心概念讲解

### 3.1 QWebSocketServer 监听端口与接受连接

QWebSocketServer 的用法和 QTcpServer 几乎一模一样：构造时指定服务器名称和加密模式，调用 listen() 绑定地址和端口，然后通过 newConnection 信号接收新连接。每来一个客户端连接，QWebSocketServer 会自动完成 WebSocket 握手协议（包括 Sec-WebSocket-Key / Sec-WebSocket-Accept 的计算），握手成功后发出 newConnection 信号，你通过 nextPendingConnection() 拿到一个已经就绪的 QWebSocket 指针。

```cpp
#include <QWebSocketServer>
#include <QWebSocket>

// 构造 WebSocket 服务器
// QWebSocketServer::NonSecureMode 表示不使用 SSL
QWebSocketServer server(
    QStringLiteral("MyWsServer"), QWebSocketServer::NonSecureMode);

// 监听所有网卡的 12345 端口
if (!server.listen(QHostAddress::Any, 12345)) {
    qCritical() << "监听失败:" << server.errorString();
    return -1;
}

// 有新连接时触发
QObject::connect(&server, &QWebSocketServer::newConnection, [&]() {
    QWebSocket* client = server.nextPendingConnection();
    qDebug() << "新客户端连接:" << client->peerAddress().toString()
             << "port:" << client->peerPort();

    // 处理这个客户端的消息和断开事件
    // ...
});
```

这里有几个细节值得展开说一下。listen() 的第一个参数是 QHostAddress::Any，表示监听所有网络接口——如果你只想本机访问，可以用 QHostAddress::LocalHost。第二个参数是端口号，选一个 1024 以上的非保留端口就行，记得确认端口没被其他程序占用。server 的构造函数第二个参数 QWebSocketServer::NonSecureMode 表示使用明文的 ws:// 协议，后面讲 WSS 的时候会切换到 SecureMode。

### 3.2 管理多个客户端连接

一个 WebSocket 服务端通常要同时管理多个客户端连接。你需要用一个容器把所有活跃的 QWebSocket 指针保存起来，在连接建立时加入容器，在连接断开时从容器中移除。QWebSocket 在断开连接时会发出 disconnected 信号，你需要连接这个信号做清理工作。另一个很关键的信号是 errorOccurred，客户端异常断开（网络中断、超时等）时会触发——这时候 disconnected 信号也会随后触发，但 errorOccurred 能让你区分正常关闭和异常断开。

```cpp
QList<QWebSocket*> clients;

QObject::connect(&server, &QWebSocketServer::newConnection, [&]() {
    QWebSocket* client = server.nextPendingConnection();

    // 将新客户端加入列表
    clients.append(client);
    qDebug() << "客户端已连接，当前连接数:" << clients.size();

    // 接收消息
    QObject::connect(client, &QWebSocket::textMessageReceived,
        [client](const QString &message) {
            qDebug() << "收到消息:" << message
                     << "来自:" << client->peerAddress().toString();
        });

    // 客户端断开连接时清理
    QObject::connect(client, &QWebSocket::disconnected,
        [client, &clients]() {
            clients.removeOne(client);
            client->deleteLater();
            qDebug() << "客户端已断开，剩余连接数:" << clients.size();
        });
});
```

这里面最容易踩的坑是内存管理。nextPendingConnection() 返回的 QWebSocket 指针所有权转移给了调用者——也就是说你必须负责在适当的时候删除它。最稳妥的做法是在 disconnected 信号的槽函数里调用 deleteLater()，让 Qt 事件循环在安全时机销毁对象。千万不要在 disconnected 槽里直接 delete，因为此时可能还有待处理的事件在队列中，直接删除会导致未定义行为。

另一个需要注意的点是 clients 容器的生命周期。这里用了一个局部变量 clients，server 对象和 clients 必须在同一个作用域内存活——如果你的 server 是堆分配的或者 clients 是成员变量，就不存在这个问题。在示例代码中我们把它放在 main() 函数里，server 和 clients 都在栈上，app.exec() 返回之前它们一直有效。

### 3.3 广播消息给所有客户端

广播是 WebSocket 服务端最核心的能力之一。实现方式很简单：遍历客户端列表，对每个 QWebSocket 调用 sendTextMessage() 或 sendBinaryMessage()。sendTextMessage() 接受一个 QString 参数，用于发送 UTF-8 文本帧；sendBinaryMessage() 接受一个 QByteArray 参数，用于发送二进制帧。两个方法都返回 qint64 表示实际发送的字节数。

```cpp
/// @brief 向所有已连接的客户端广播文本消息
void broadcast_message(const QList<QWebSocket*> &clients,
                       const QString &message)
{
    for (QWebSocket* client : clients) {
        if (client->state() == QAbstractSocket::ConnectedState) {
            client->sendTextMessage(message);
        }
    }
    qDebug() << "广播消息给" << clients.size() << "个客户端:"
             << message.left(50);
}
```

广播之前检查 client->state() 是一个好习惯。虽然 disconnected 信号理论上会在客户端断开后触发清理，但信号传递有延迟——在清理完成之前的极短窗口内，列表中可能还存在已经断开但尚未 deleteLater 的连接。检查 state 可以避免向已断开的 socket 写数据（这会产生警告但不至于崩溃）。

如果你需要一个聊天室的效果——某个客户端发消息，服务端收到后广播给所有其他客户端——只需要在 textMessageReceived 的槽函数里把收到的消息转发出去：

```cpp
QObject::connect(client, &QWebSocket::textMessageReceived,
    [client, &clients](const QString &message) {
        QString formatted = QStringLiteral("[%1:%2] %3")
            .arg(client->peerAddress().toString())
            .arg(client->peerPort())
            .arg(message);

        // 广播给所有客户端（包括发送者）
        for (QWebSocket* c : clients) {
            if (c->state() == QAbstractSocket::ConnectedState) {
                c->sendTextMessage(formatted);
            }
        }
    });
```

如果不想把消息回发给发送者，加一个 `c != client` 的判断就行。这个设计选择取决于你的业务需求——聊天室通常会回发给所有人（包括发送者，作为消息确认），而推送通知场景可能只需要发给其他人。

### 3.4 SSL WebSocket（WSS）服务端配置

如果你的 WebSocket 服务需要安全传输——比如客户端是浏览器页面，而页面是 HTTPS 的，浏览器会拒绝 ws:// 的混合内容，强制要求 wss://——那么就需要配置 SSL。QWebSocketServer 的 WSS 配置和 QSslSocket 的配置方式一致：构造时传入 QWebSocketServer::SecureMode，然后设置 QSslConfiguration。

```cpp
#include <QSslCertificate>
#include <QSslConfiguration>
#include <QSslKey>

// 构造 SSL 模式的 WebSocket 服务器
QWebSocketServer secureServer(
    QStringLiteral("MySecureWsServer"), QWebSocketServer::SecureMode);

// 加载 SSL 证书和私钥
QFile certFile(QStringLiteral("/path/to/server.crt"));
QFile keyFile(QStringLiteral("/path/to/server.key"));

if (!certFile.open(QIODevice::ReadOnly)
    || !keyFile.open(QIODevice::ReadOnly)) {
    qCritical() << "无法打开证书或密钥文件";
    return -1;
}

QSslCertificate certificate(&certFile, QSsl::Pem);
QSslKey sslKey(&keyFile, QSsl::Rsa, QSsl::Pem);

// 配置 SSL
QSslConfiguration sslConfig = secureServer.sslConfiguration();
sslConfig.setLocalCertificate(certificate);
sslConfig.setPrivateKey(sslKey);
secureServer.setSslConfiguration(sslConfig);

// 监听端口
if (!secureServer.listen(QHostAddress::Any, 8443)) {
    qCritical() << "WSS 监听失败:" << secureServer.errorString();
    return -1;
}

qDebug() << "WSS 服务已启动，监听端口 8443";
```

这里有个重要的事情要说清楚：开发阶段你可以用 OpenSSL 生成自签名证书来测试 WSS，命令是 `openssl req -x509 -newkey rsa:2048 -keyout server.key -out server.crt -days 365 -nodes`。但自签名证书在浏览器中会报安全警告，生产环境必须用 Let's Encrypt 或者商业 CA 签发的证书。另外 QSslKey 的第二个参数要和你证书的密钥类型匹配——RSA 证书用 QSsl::Rsa，EC 证书用 QSsl::Ec。

还有一点，SSL 配置完成后，QWebSocketServer 会在每个新连接的 TLS 握手阶段自动使用你设置的证书和密钥。如果握手失败（客户端不信任你的证书、协议版本不匹配等），会发出 sslErrors 信号，你可以连接这个信号来记录错误信息。

## 4. 综合示例：WebSocket 聊天室服务端

把前面的内容整合起来，我们写一个基于 QWebSocketServer 的聊天室服务端。它监听 12345 端口（明文 ws://），管理所有客户端连接，实现消息广播，并在控制台输出连接日志和消息日志。这是一个纯控制台程序（QCoreApplication），不需要 GUI。

CMake 配置：

```cmake
find_package(Qt6 REQUIRED COMPONENTS WebSockets)

qt_add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::WebSockets)
```

main.cpp 的完整代码：

```cpp
#include <QCoreApplication>
#include <QDebug>
#include <QHostAddress>
#include <QList>
#include <QWebSocket>
#include <QWebSocketServer>

/// @brief 向所有已连接的客户端广播文本消息
static void broadcast_message(const QList<QWebSocket*> &clients,
                              const QString &message)
{
    for (QWebSocket* client : clients) {
        if (client->state() == QAbstractSocket::ConnectedState) {
            client->sendTextMessage(message);
        }
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "QtWebSockets 聊天室服务端示例";
    qDebug() << "本示例演示 QWebSocketServer + 多客户端管理 + 广播消息";

    QWebSocketServer server(
        QStringLiteral("ChatServer"), QWebSocketServer::NonSecureMode);

    if (!server.listen(QHostAddress::Any, 12345)) {
        qCritical() << "监听失败:" << server.errorString();
        return -1;
    }

    QList<QWebSocket*> clients;

    // 新客户端连接
    QObject::connect(&server, &QWebSocketServer::newConnection, [&]() {
        QWebSocket* client = server.nextPendingConnection();
        clients.append(client);

        QString addr = client->peerAddress().toString();
        quint16 port = client->peerPort();
        qDebug() << "客户端连接:" << addr << "port:" << port
                 << "当前在线:" << clients.size();

        // 广播加入通知
        broadcast_message(clients,
            QStringLiteral("[系统] 新用户加入 (%1:%2)，当前在线: %3 人")
                .arg(addr)
                .arg(port)
                .arg(clients.size()));

        // 接收消息并广播
        QObject::connect(client, &QWebSocket::textMessageReceived,
            [client, &clients](const QString &message) {
                QString addr = client->peerAddress().toString();
                quint16 port = client->peerPort();

                QString formatted = QStringLiteral("[%1:%2] %3")
                                        .arg(addr)
                                        .arg(port)
                                        .arg(message);

                qDebug() << "消息:" << formatted.left(80);
                broadcast_message(clients, formatted);
            });

        // 客户端断开连接
        QObject::connect(client, &QWebSocket::disconnected,
            [client, &clients]() {
                QString addr = client->peerAddress().toString();
                quint16 port = client->peerPort();

                clients.removeOne(client);
                client->deleteLater();

                qDebug() << "客户端断开:" << addr << "port:" << port
                         << "剩余在线:" << clients.size();

                // 广播离开通知
                broadcast_message(clients,
                    QStringLiteral("[系统] 用户离开 (%1:%2)，当前在线: %3 人")
                        .arg(addr)
                        .arg(port)
                        .arg(clients.size()));
            });
    });

    qDebug() << "WebSocket 聊天室服务已启动";
    qDebug() << "监听地址: ws://0.0.0.0:12345";
    qDebug() << "";
    qDebug() << "测试方法:";
    qDebug() << "  使用浏览器 WebSocket 客户端或其他 ws 工具连接";
    qDebug() << "  JavaScript 示例:";
    qDebug() << "    const ws = new WebSocket('ws://localhost:12345');";
    qDebug() << "    ws.onmessage = (e) => console.log(e.data);";
    qDebug() << "    ws.send('Hello from browser!');";

    return app.exec();
}
```

运行程序后，服务端在 12345 端口等待连接。打开浏览器控制台，输入上面的 JavaScript 代码连接服务端。多开几个浏览器标签页同时连接，就可以模拟多人聊天——一个标签页发消息，所有标签页都会收到广播。服务端控制台会实时打印连接、断开和消息转发的日志。

几个实现细节值得一提。broadcast_message 函数在每次调用时遍历整个 clients 列表，对于少量客户端（几十个）完全没有性能问题。如果客户端数量上千，可以考虑用 QWebSocket 的 bytesToWrite() 做背压控制——当某个客户端的发送缓冲区积压太多未发送数据时暂时跳过它，避免内存持续增长。另外，示例中的 clients 列表在 disconnected 槽函数中通过 lambda 捕获引用来访问，这要求 clients 对象的生命周期覆盖所有客户端的连接周期——在本例中 server 和 clients 都在 main 的栈上，app.exec() 返回之前一直有效，所以没问题。

## 5. 练习项目

练习项目：带用户名和私聊功能的聊天室。

在基础聊天室的基础上增加用户身份和私聊能力：客户端连接后第一条消息作为用户名注册（格式约定为 "REGISTER:username"），服务端维护一个用户名到 QWebSocket 的映射；广播消息时显示用户名而不是 IP:端口；客户端发送 "WHISPER:username:message" 格式的消息时，服务端只把消息转发给指定用户名的客户端，不广播；服务端定期（每 30 秒）向所有客户端发送 ping 帧，如果某个客户端 10 秒内没有回复 pong 则判定断开并清理。

完成标准是这样的：三个客户端分别注册为 Alice、Bob、Charlie，Alice 发消息 "Hi all"，Bob 和 Charlie 都能收到 "[Alice] Hi all"；Alice 发送 "WHISPER:Bob:Secret"，只有 Bob 收到 "[Alice -> 你] Secret"，Charlie 收不到；强制关闭 Bob 的连接后，服务端自动检测到并广播 "[系统] Bob 已离开"。

几个实现提示：QWebSocket 有 ping() 方法发送 ping 帧，对应的 pongReceived 信号在收到 pong 回复时触发。用户名注册需要处理重名的情况——可以拒绝重名注册并要求客户端重新选择，或者在用户名后追加序号自动去重。私聊时如果目标用户不存在，需要给发送者返回一个错误提示消息。

## 6. 官方文档参考

[Qt 文档 · QWebSocketServer](https://doc.qt.io/qt-6/qwebsocketserver.html) -- WebSocket 服务端类，监听端口并接受客户端连接

[Qt 文档 · QWebSocket](https://doc.qt.io/qt-6/qwebsocket.html) -- WebSocket 连接类，发送和接收消息

[Qt 文档 · QtWebSockets 模块](https://doc.qt.io/qt-6/qtwebsockets-index.html) -- WebSockets 模块总览

[Qt 文档 · QSslConfiguration](https://doc.qt.io/qt-6/qsslconfiguration.html) -- SSL 配置类，用于 WSS 加密连接

---

到这里就大功告成了。QWebSocketServer 把 WebSocket 协议的服务端能力直接带进了 Qt 应用——listen() 监听端口、nextPendingConnection() 接受连接、sendTextMessage() 广播消息，加上 QSslConfiguration 配置 WSS 加密，整个双向实时通信的链路用纯 Qt C++ 就能搞定。对于聊天室、实时数据推送、设备状态监控这些场景，QtWebSockets 比你自己去解析 TCP 流或者引入第三方库简洁得多。
