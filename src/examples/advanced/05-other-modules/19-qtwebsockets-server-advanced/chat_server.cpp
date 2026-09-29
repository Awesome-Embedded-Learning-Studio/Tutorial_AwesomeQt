/// @file    chat_server.cpp
/// @brief   ChatServer 类的实现——WebSocket 聊天服务器核心逻辑。
///
/// 对应教程：进阶层 05-其他模块/19-QtWebSockets Server。

#include "chat_server.h"

#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>

// ============================================================================
// Construction / Destruction
// ============================================================================

ChatServer::ChatServer(QObject* parent)
    : QObject(parent)
    , m_server(std::make_unique<QWebSocketServer>(
          QStringLiteral("ChatServer"), QWebSocketServer::NonSecureMode, this))
{
    // newConnection 是 QWebSocketServer 的标准信号，表示有客户端完成握手
    connect(m_server.get(), &QWebSocketServer::newConnection, this,
            &ChatServer::onNewConnection);
}

ChatServer::~ChatServer()
{
    // 逐个关闭客户端连接，避免资源泄漏
    for (auto* client : m_clients)
    {
        if (client && client->socket)
        {
            client->socket->close();
        }
        delete client;
    }
    m_clients.clear();
    m_server->close();
}

// ============================================================================
// Public API
// ============================================================================

bool ChatServer::start(quint16 port)
{
    // listen() 返回 false 说明端口被占用或权限不足
    if (!m_server->listen(QHostAddress::Any, port))
    {
        emit serverInfo(
            QStringLiteral("[ERROR] Failed to listen on port %1: %2")
                .arg(port)
                .arg(m_server->errorString()));
        return false;
    }

    emit serverInfo(
        QStringLiteral("[SERVER] Listening on ws://localhost:%1/").arg(port));
    return true;
}

QStringList ChatServer::rooms() const
{
    QStringList result;
    for (const auto* client : m_clients)
    {
        if (!client->room.isEmpty() && !result.contains(client->room))
        {
            result.append(client->room);
        }
    }
    return result;
}

int ChatServer::clientCount(const QString& room) const
{
    return static_cast<int>(
        std::count_if(m_clients.cbegin(), m_clients.cend(),
                      [&room](const ClientInfo* c)
                      { return c->room == room; }));
}

// ============================================================================
// Private Slots
// ============================================================================

void ChatServer::onNewConnection()
{
    // nextPendingConnection() 返回已完成 HTTP 升级握手的 WebSocket
    QWebSocket* socket = m_server->nextPendingConnection();
    if (!socket)
    {
        return;
    }

    auto* info = new ClientInfo;
    info->socket = socket;
    // 用 socket 指针的低 16 位作为默认用户名，便于调试辨识
    info->username =
        QStringLiteral("User_%1").arg(reinterpret_cast<quintptr>(socket) & 0xFFFF,
                                      4, 16, QLatin1Char('0'));
    info->room = QString();

    emit serverInfo(
        QStringLiteral("[CONNECT] %1 joined (no room yet)").arg(info->username));

    // 文本消息走 JSON 协议；二进制帧本示例不处理
    connect(socket, &QWebSocket::textMessageReceived, this,
            &ChatServer::onTextMessageReceived);
    // disconnected 信号触发资源清理
    connect(socket, &QWebSocket::disconnected, this,
            &ChatServer::onClientDisconnected);

    m_clients.append(info);
}

void ChatServer::onTextMessageReceived(const QString& message)
{
    auto* socket = qobject_cast<QWebSocket*>(sender());
    if (!socket)
    {
        return;
    }

    ClientInfo* client = findClient(socket);
    if (!client)
    {
        return;
    }

    // 解析 JSON 协议帧
    QJsonParseError parseError;
    const QJsonDocument doc =
        QJsonDocument::fromJson(message.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        sendJson(socket, QStringLiteral("error"),
                 {{QStringLiteral("text"),
                   QStringLiteral("Invalid JSON: %1").arg(parseError.errorString())}});
        return;
    }

    const QJsonObject obj = doc.object();
    const QString type = obj.value(QStringLiteral("type")).toString();

    if (type == QLatin1String("join"))
    {
        const QString newRoom = obj.value(QStringLiteral("room")).toString();
        if (newRoom.isEmpty())
        {
            sendJson(socket, QStringLiteral("error"),
                     {{QStringLiteral("text"), QStringLiteral("Room name is empty")}});
            return;
        }

        const QString oldRoom = client->room;
        client->room = newRoom;

        // 先向旧房间通知有人离开，再向新房通知有人加入
        if (!oldRoom.isEmpty())
        {
            broadcastToRoom(nullptr,
                            QStringLiteral("[System] %1 left room '%2'")
                                .arg(client->username, oldRoom));
            broadcastRoomCount(oldRoom);
        }

        broadcastToRoom(nullptr,
                        QStringLiteral("[System] %1 joined room '%2'")
                            .arg(client->username, newRoom));
        broadcastRoomCount(newRoom);

        sendJson(socket, QStringLiteral("joined"),
                 {{QStringLiteral("room"), newRoom}});

        emit serverInfo(QStringLiteral("[JOIN] %1 -> room '%2'")
                            .arg(client->username, newRoom));
    }
    else if (type == QLatin1String("msg"))
    {
        if (client->room.isEmpty())
        {
            sendJson(socket, QStringLiteral("error"),
                     {{QStringLiteral("text"),
                       QStringLiteral("You must join a room first")}});
            return;
        }
        const QString text =
            obj.value(QStringLiteral("text")).toString();
        const QString formatted =
            QStringLiteral("[%1] %2").arg(client->username, text);

        broadcastToRoom(socket, formatted);
        // 发送者也收到自己的消息（echo），确认消息已广播
        sendJson(socket, QStringLiteral("broadcast"),
                 {{QStringLiteral("text"), formatted}});

        emit serverInfo(
            QStringLiteral("[MSG] room='%1' from=%2 : %3")
                .arg(client->room, client->username, text));
    }
    else if (type == QLatin1String("pm"))
    {
        const QString targetName =
            obj.value(QStringLiteral("to")).toString();
        const QString text =
            obj.value(QStringLiteral("text")).toString();

        if (!sendPrivateMessage(socket, targetName, text))
        {
            sendJson(socket, QStringLiteral("error"),
                     {{QStringLiteral("text"),
                       QStringLiteral("User '%1' not found in your room")
                           .arg(targetName)}});
        }
    }
    else if (type == QLatin1String("leave"))
    {
        if (client->room.isEmpty())
        {
            sendJson(socket, QStringLiteral("error"),
                     {{QStringLiteral("text"),
                       QStringLiteral("You are not in any room")}});
            return;
        }

        const QString oldRoom = client->room;
        broadcastToRoom(nullptr,
                        QStringLiteral("[System] %1 left room '%2'")
                            .arg(client->username, oldRoom));
        client->room.clear();
        broadcastRoomCount(oldRoom);

        sendJson(socket, QStringLiteral("left"),
                 {{QStringLiteral("room"), oldRoom}});

        emit serverInfo(
            QStringLiteral("[LEAVE] %1 left room '%2'")
                .arg(client->username, oldRoom));
    }
    else
    {
        sendJson(socket, QStringLiteral("error"),
                 {{QStringLiteral("text"),
                   QStringLiteral("Unknown message type: %1").arg(type)}});
    }
}

void ChatServer::onClientDisconnected()
{
    auto* socket = qobject_cast<QWebSocket*>(sender());
    if (!socket)
    {
        return;
    }

    // 查找并移除客户端，同时通知房间内其他成员
    auto it = std::find_if(m_clients.begin(), m_clients.end(),
                           [socket](const ClientInfo* c)
                           { return c->socket == socket; });

    if (it != m_clients.end())
    {
        ClientInfo* client = *it;
        const QString username = client->username;
        const QString room = client->room;

        if (!room.isEmpty())
        {
            broadcastToRoom(nullptr,
                            QStringLiteral("[System] %1 disconnected").arg(username));
            broadcastRoomCount(room);
        }

        emit serverInfo(
            QStringLiteral("[DISCONNECT] %1").arg(username));
        m_clients.erase(it);
        delete client;
    }

    // QWebSocketServer 不自动删除已断开的 socket，需手动清理
    socket->deleteLater();
}

// ============================================================================
// Private Helpers
// ============================================================================

ClientInfo* ChatServer::findClient(QWebSocket* socket)
{
    auto it = std::find_if(m_clients.begin(), m_clients.end(),
                           [socket](const ClientInfo* c)
                           { return c->socket == socket; });
    return (it != m_clients.end()) ? *it : nullptr;
}

void ChatServer::broadcastToRoom(QWebSocket* sender, const QString& text)
{
    ClientInfo* senderClient = findClient(sender);

    for (const auto* client : m_clients)
    {
        // 跳过发送者自己（广播场景由调用方单独 echo）
        if (sender && client->socket == sender)
        {
            continue;
        }
        // 只发给同一房间的客户端
        if (senderClient && client->room != senderClient->room)
        {
            continue;
        }
        // 系统消息（sender==nullptr）发给房间内所有人
        if (!senderClient && client->room.isEmpty())
        {
            continue;
        }

        sendJson(client->socket, QStringLiteral("broadcast"),
                 {{QStringLiteral("text"), text}});
    }
}

bool ChatServer::sendPrivateMessage(QWebSocket* sender,
                                    const QString& targetName,
                                    const QString& text)
{
    ClientInfo* senderClient = findClient(sender);
    if (!senderClient || senderClient->room.isEmpty())
    {
        return false;
    }

    // 只在同一房间内查找目标用户，避免跨房间私信泄露
    for (const auto* client : m_clients)
    {
        if (client->username == targetName
            && client->room == senderClient->room)
        {
            // 给接收方发私信
            sendJson(client->socket, QStringLiteral("pm"),
                     {{QStringLiteral("from"), senderClient->username},
                      {QStringLiteral("text"), text}});
            // 给发送方发确认
            sendJson(sender, QStringLiteral("pm_sent"),
                     {{QStringLiteral("to"), targetName},
                      {QStringLiteral("text"), text}});
            return true;
        }
    }
    return false;
}

void ChatServer::sendJson(QWebSocket* socket, const QString& type,
                          const QHash<QString, QString>& data)
{
    QJsonObject obj;
    obj[QStringLiteral("type")] = type;
    for (auto it = data.cbegin(); it != data.cend(); ++it)
    {
        obj[it.key()] = it.value();
    }
    // QJsonDocument::Compact 去掉多余空白，减少传输量
    socket->sendTextMessage(
        QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void ChatServer::broadcastRoomCount(const QString& room)
{
    const int count = clientCount(room);
    const QString text =
        QStringLiteral("[System] Room '%1' has %2 user(s)").arg(room).arg(count);

    for (const auto* client : m_clients)
    {
        if (client->room == room)
        {
            sendJson(client->socket, QStringLiteral("room_count"),
                     {{QStringLiteral("room"), room},
                      {QStringLiteral("count"), QString::number(count)}});
        }
    }

    emit serverInfo(text);
}
