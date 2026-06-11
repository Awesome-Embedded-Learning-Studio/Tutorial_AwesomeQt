---
title: "5.19 WebSocket 服务端进阶：房间广播、消息队列"
description: "入门篇我们写了 WebSocket 的基础通信——客户端连上来、发消息、收消息。进阶篇要把 WebSocket 用到真正的实时通信场景中：多客户端房间管理、发布/订阅广播、消息队列缓冲。这是即时通讯、实时协作、多人游戏服务端的底层模式。"
---

# 现代Qt开发教程（进阶篇）5.19——WebSocket 服务端进阶：房间广播、消息队列

## 1. 前言

入门篇我们用 `QWebSocketServer` 搭了个基本的 WebSocket 服务端——客户端连上来可以收发消息。但那种一对一的简单通信在真实场景中基本不够用。想想常见的实时应用：聊天室里消息要广播给所有在线成员；协作白板里一个人的操作要同步给其他所有人；多人游戏中玩家的移动要广播给同一房间的其他玩家。

这些场景的核心模式是一样的：多个客户端按「房间」分组，消息在房间内广播。客户端可能临时断线，断线期间的消息需要缓存，重连后补发。这就是这篇要讲的内容——房间管理、发布/订阅广播、消息队列缓冲。

Qt 的 `QWebSocketServer` 只提供了底层的 WebSocket 连接管理，不提供任何上层的「房间」或「频道」概念。我们需要自己在这之上构建房间系统。

## 2. 环境说明

本文档基于 Qt 6.2+ 编写，需要 Qt6::WebSockets 模块。CMake 中通过 `find_package(Qt6 REQUIRED COMPONENTS WebSockets)` 引入。本篇全部使用 C++ API，不涉及 QML。`QWebSocketServer` 底层基于 `QTcpServer`，网络 IO 是异步非阻塞的。

## 3. 核心概念讲解

### 3.1 多客户端房间管理

房间的概念很简单：一组客户端的集合。客户端加入房间、离开房间、在房间内广播消息。我们用一个 `Room` 类来封装这个逻辑。

```cpp
/// @brief 聊天室——管理一组 WebSocket 客户端连接。
class Room
{
public:
    /// @brief 将客户端加入房间。
    /// @param[in] client WebSocket 连接。
    /// @param[in] nickname 客户端昵称。
    void join(QWebSocket* client, const QString& nickname)
    {
        m_members[client] = nickname;
        // 通知房间内所有人有新成员加入
        broadcast_system_message(nickname + " 加入了房间");
    }

    /// @brief 将客户端移出房间。
    /// @param[in] client WebSocket 连接。
    void leave(QWebSocket* client)
    {
        QString kNickname = m_members.value(client, "未知用户");
        m_members.remove(client);
        broadcast_system_message(kNickname + " 离开了房间");
    }

    /// @brief 向房间内所有成员广播消息（排除发送者）。
    /// @param[in] message 消息内容。
    /// @param[in] sender 发送者（不接收自己的消息），传 nullptr 则所有人都能收到。
    void broadcast(const QString& message, QWebSocket* sender = nullptr)
    {
        for (auto it = m_members.cbegin(); it != m_members.cend(); ++it) {
            if (it.key() != sender) {
                it.key()->sendTextMessage(message);
            }
        }
    }

    /// @brief 获取房间成员数量。
    int member_count() const { return m_members.size(); }

private:
    /// @brief 广播系统消息（所有人都能收到）。
    void broadcast_system_message(const QString& text)
    {
        QJsonObject kMsg;
        kMsg["type"] = "system";
        kMsg["content"] = text;
        QJsonDocument kDoc(kMsg);
        broadcast(QString::fromUtf8(kDoc.toJson(QJsonDocument::Compact)));
    }

    QMap<QWebSocket*, QString> m_members;  // 连接 -> 昵称映射
};
```

这个 `Room` 类管理一个 `QMap<QWebSocket*, QString>`，键是连接，值是昵称。`broadcast` 方法遍历所有成员发送消息，可选排除发送者。`join` 和 `leave` 方法负责成员管理和系统消息广播。

接下来是服务端的主类，它管理多个房间，处理新连接和消息分发：

```cpp
/// @brief WebSocket 聊天服务端，管理多个聊天室。
class ChatServer : public QObject
{
    Q_OBJECT

public:
    explicit ChatServer(quint16 port, QObject* parent = nullptr)
        : QObject(parent)
        , m_server("ChatServer", QWebSocketServer::NonSecureMode)
    {
        connect(&m_server, &QWebSocketServer::newConnection,
                this, &ChatServer::on_new_connection);
        m_server.listen(QHostAddress::Any, port);
    }

private slots:
    /// @brief 处理新的 WebSocket 连接。
    void on_new_connection()
    {
        QWebSocket* kClient = m_server.nextPendingConnection();
        connect(kClient, &QWebSocket::textMessageReceived,
                this, &ChatServer::on_text_message);
        connect(kClient, &QWebSocket::disconnected,
                this, &ChatServer::on_disconnected);

        // 新连接暂时不加入任何房间，等待客户端发送加入房间的消息
        m_pending_clients.insert(kClient);
    }

    /// @brief 处理客户端发来的文本消息。
    void on_text_message(const QString& message)
    {
        auto* kClient = qobject_cast<QWebSocket*>(sender());
        QJsonObject kMsg = QJsonDocument::fromJson(message.toUtf8()).object();
        QString kType = kMsg["type"].toString();

        if (kType == "join") {
            QString kRoomId = kMsg["room"].toString();
            QString kNickname = kMsg["nickname"].toString();

            // 创建房间（如果不存在）
            if (!m_rooms.contains(kRoomId)) {
                m_rooms[kRoomId] = std::make_unique<Room>();
            }

            m_rooms[kRoomId]->join(kClient, kNickname);
            m_client_to_room[kClient] = kRoomId;
            m_pending_clients.remove(kClient);
        }
        else if (kType == "message") {
            QString kRoomId = m_client_to_room.value(kClient);
            if (m_rooms.contains(kRoomId)) {
                m_rooms[kRoomId]->broadcast(message, kClient);
            }
        }
    }

    /// @brief 处理客户端断开连接。
    void on_disconnected()
    {
        auto* kClient = qobject_cast<QWebSocket*>(sender());
        QString kRoomId = m_client_to_room.value(kClient);

        if (m_rooms.contains(kRoomId)) {
            m_rooms[kRoomId]->leave(kClient);
        }

        m_client_to_room.remove(kClient);
        m_pending_clients.remove(kClient);
        kClient->deleteLater();
    }

private:
    QWebSocketServer m_server;
    QMap<QString, std::unique_ptr<Room>> m_rooms;   // 房间 ID -> 房间对象
    QMap<QWebSocket*, QString> m_client_to_room;     // 连接 -> 所在房间 ID
    QSet<QWebSocket*> m_pending_clients;              // 尚未加入任何房间的连接
};
```

这个服务端维护三个数据结构：`m_rooms` 是房间 ID 到房间对象的映射，`m_client_to_room` 是连接到所在房间 ID 的映射（用于消息路由），`m_pending_clients` 存储已连接但还没加入房间的客户端。客户端连接后先发一条 `{"type":"join", "room":"xxx", "nickname":"yyy"}` 加入房间，之后的消息会被路由到对应房间进行广播。

### 3.2 发布/订阅消息广播模式

上面的房间广播其实就是一种发布/订阅模式——房间是「主题」，加入房间就是「订阅」，离开房间就是「取消订阅」，广播就是「发布」。我们可以把这个模式抽象出来，支持更灵活的订阅关系。

```cpp
/// @brief 简单的发布/订阅消息总线。
class MessageBus : public QObject
{
    Q_OBJECT

public:
    /// @brief 订阅指定主题。
    /// @param[in] topic 主题名称。
    /// @param[in] subscriber 订阅者 WebSocket 连接。
    void subscribe(const QString& topic, QWebSocket* subscriber)
    {
        m_subscribers[topic].insert(subscriber);
        m_subscriber_topics[subscriber].insert(topic);
    }

    /// @brief 取消订阅指定主题。
    /// @param[in] topic 主题名称。
    /// @param[in] subscriber 订阅者 WebSocket 连接。
    void unsubscribe(const QString& topic, QWebSocket* subscriber)
    {
        m_subscribers[topic].remove(subscriber);
        m_subscriber_topics[subscriber].remove(topic);
    }

    /// @brief 发布消息到指定主题。
    /// @param[in] topic 主题名称。
    /// @param[in] message 消息内容。
    /// @param[in] exclude 排除的订阅者（可选）。
    void publish(const QString& topic, const QString& message,
                 QWebSocket* exclude = nullptr)
    {
        const QSet<QWebSocket*>& kSubs = m_subscribers[topic];
        for (QWebSocket* sub : kSubs) {
            if (sub != exclude && sub->isValid()) {
                sub->sendTextMessage(message);
            }
        }
    }

    /// @brief 移除订阅者的所有订阅（断开连接时调用）。
    /// @param[in] subscriber 订阅者 WebSocket 连接。
    void remove_all_subscriptions(QWebSocket* subscriber)
    {
        const QSet<QString>& kTopics = m_subscriber_topics[subscriber];
        for (const QString& topic : kTopics) {
            m_subscribers[topic].remove(subscriber);
        }
        m_subscriber_topics.remove(subscriber);
    }

private:
    QMap<QString, QSet<QWebSocket*>> m_subscribers;       // 主题 -> 订阅者集合
    QMap<QWebSocket*, QSet<QString>> m_subscriber_topics; // 订阅者 -> 主题集合
};
```

这个 `MessageBus` 支持多主题订阅——一个客户端可以同时订阅多个主题（比如同时订阅多个聊天频道、同时监听系统通知和私聊）。`publish` 方法把消息发给指定主题的所有订阅者，可选排除发送者。`remove_all_subscriptions` 在客户端断开时清理所有订阅关系。

现在有个思考题：如果客户端 A 同时订阅了主题 X 和主题 Y，有人往 X 和 Y 各发了一条相同的消息，客户端 A 会收到两次。怎么去重？

最简单的方法是在消息中加一个唯一 ID（比如 UUID），客户端收到消息后检查 ID 是否已处理过。服务端也可以在每个主题的发布中记录已发送的消息 ID，但这会增加复杂度。实际应用中，如果不同主题的消息内容确实不同（通常是这样），重复就不是问题。只有在「通配符订阅」这种场景下才需要去重。

### 3.3 消息队列缓冲——离线消息缓存

WebSocket 是实时协议——消息发送时客户端必须在线才能收到。如果客户端临时断线（比如网络波动），发送期间的消息就丢了。对于聊天应用来说这是不可接受的。

消息队列缓冲的思路是：当客户端断线时，发往该客户端的消息先缓存在队列中，等客户端重连后补发。

```cpp
/// @brief 带离线缓存的消息队列。
class BufferedMessageQueue
{
public:
    /// @brief 缓存一条消息给指定客户端。
    /// @param[in] client_id 客户端标识（比如用户 ID，不是 WebSocket 指针）。
    /// @param[in] message 消息内容。
    void enqueue(const QString& client_id, const QString& message)
    {
        m_queues[client_id].enqueue(message);

        // 防止队列无限增长——断线太久的客户端缓存太多消息没意义
        if (m_queues[client_id].size() > kMaxQueueSize) {
            m_queues[client_id].dequeue();  // 丢弃最旧的消息
        }
    }

    /// @brief 客户端重连后，发送所有缓存消息。
    /// @param[in] client_id 客户端标识。
    /// @param[in] socket 重连后的 WebSocket 连接。
    void flush(const QString& client_id, QWebSocket* socket)
    {
        if (!m_queues.contains(client_id)) {
            return;
        }

        QQueue<QString>& kQueue = m_queues[client_id];
        while (!kQueue.isEmpty()) {
            socket->sendTextMessage(kQueue.dequeue());
        }
        m_queues.remove(client_id);
    }

    /// @brief 检查指定客户端是否有缓存消息。
    bool has_pending(const QString& client_id) const
    {
        return m_queues.contains(client_id) && !m_queues[client_id].isEmpty();
    }

private:
    static constexpr int kMaxQueueSize = 100;  // 每个客户端最多缓存 100 条
    QMap<QString, QQueue<QString>> m_queues;    // 客户端 ID -> 消息队列
};
```

使用消息队列时需要注意的几个问题。首先是客户端标识——不能用 `QWebSocket*` 指针作为标识，因为断线重连后是新的 `QWebSocket` 对象。应该用用户 ID 或 session ID 之类的持久标识。其次是队列大小限制——如果一个客户端断线太久，缓存的消息可能堆积到几百上千条，重连后一次性发送会冲击网络和服务端性能。设置一个合理的上限（比如 100 条），超出的丢弃最旧的消息。最后，如果需要持久化（服务端重启后恢复离线消息），需要把消息写入数据库或文件，内存队列重启后会丢失。

把这个队列集成到房间广播中：当房间广播消息时，如果目标客户端在线就直接发送，如果离线就调用 `enqueue` 缓存。客户端重连并重新加入房间后，调用 `flush` 补发缓存消息。

## 4. 踩坑预防

第一个坑是 `QWebSocket` 对象在 `disconnected` 信号处理完之后必须调用 `deleteLater()`，但不能在遍历 `m_members` 的时候删除它。如果你在 `Room::leave()` 中直接 `delete` 了 `QWebSocket`，而外层代码还在遍历成员列表，程序会 crash（访问已释放的内存）。解决方案是用 `deleteLater()` 延迟删除，确保当前事件循环迭代完成后再释放对象。同时在 `broadcast` 中检查 `socket->isValid()` 避免向已断开的连接发送。

第二个坑是消息广播的顺序一致性问题。假设房间里有 5 个成员，你遍历发送消息。如果第 3 个成员的 `sendTextMessage` 触发了它所在的另一个事件循环嵌套（比如通过信号槽间接调用了 `processEvents`），可能导致消息发送顺序混乱。这不是常见的坑，但在复杂的事件驱动架构中可能出现。解决方案是避免在消息处理过程中调用 `processEvents`。

第三个坑是用 `QWebSocket*` 指针做 Map 的 key。断线重连后客户端会有新的 `QWebSocket` 对象，旧的 key 就失效了。如果你需要追踪客户端身份（比如支持离线消息缓存），必须用用户 ID 或 session token 做标识，而不是用裸指针。

## 5. 练习项目

练习项目是一个多人聊天室服务端。我们要构建一个支持多房间的 WebSocket 聊天服务。

服务端监听指定端口，客户端连接后通过 JSON 消息 `{"type":"join", "room":"general", "nickname":"Alice"}` 加入指定房间。客户端发送 `{"type":"message", "content":"Hello!"}` 时，消息广播给同房间所有其他成员。客户端断开时自动离开房间并通知其他成员。服务端还需要支持离线消息缓存——客户端断线期间的房间消息被缓存，重连后自动补发（用昵称作为客户端标识）。额外要求是服务端提供一个管理接口：通过特殊的 JSON 命令可以列出所有房间及其成员数量、强制踢出指定用户。

完成标准是多个客户端能正确加入同一房间并互相收发消息、断线重连后能收到离线期间的消息、系统消息（加入/离开通知）正确广播、管理命令正常工作。测试可以用多个 `QWebSocket` 客户端实例或浏览器 WebSocket 控制台。

提示几个关键点：用 `QMap<QString, std::unique_ptr<Room>>` 管理多个房间；用 `BufferedMessageQueue` 处理离线消息；踢人功能通过遍历房间找到目标连接后 `close()` 关闭。

## 6. 官方文档参考链接

[Qt 文档 · QWebSocketServer](https://doc.qt.io/qt-6/qwebsocketserver.html) -- WebSocket 服务端核心类

[Qt 文档 · QWebSocket](https://doc.qt.io/qt-6/qwebsocket.html) -- WebSocket 连接管理

[Qt 文档 · QJsonDocument](https://doc.qt.io/qt-6/qjsondocument.html) -- JSON 解析与序列化

---

到这里 WebSocket 服务端的进阶模式就拆完了。房间管理、发布/订阅、消息队列缓冲——这是构建实时通信应用的三个核心模式。把它们组合起来，足以支撑聊天室、实时协作、多人游戏等场景。如果需要更高阶的功能（消息持久化到数据库、集群化的房间同步、WebSocket 集群负载均衡），那就是分布式系统的话题了，超出了 Qt 单机服务的范畴。
