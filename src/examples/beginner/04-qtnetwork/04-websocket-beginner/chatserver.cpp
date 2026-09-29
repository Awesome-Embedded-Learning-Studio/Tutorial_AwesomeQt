#include "chatserver.h"

#include <QDebug>

ChatServer::ChatServer(QObject *parent)
    : QObject(parent),
      m_server(new QWebSocketServer(
          "ChatServer", QWebSocketServer::NonSecureMode, this))
{
    if (!m_server->listen(QHostAddress::Any, 12345)) {
        qDebug() << "[Server] Failed to start:"
                 << m_server->errorString();
        return;
    }

    qDebug() << "[Server] Listening on port"
             << m_server->serverUrl().toString() << "...";

    connect(m_server, &QWebSocketServer::newConnection, this, [this]() {
        handleNewConnection();
    });
}

ChatServer::~ChatServer()
{
    // 关闭所有客户端连接
    for (QWebSocket *client : m_clients) {
        client->close();
        client->deleteLater();
    }
    m_clients.clear();
    m_server->close();
}

void ChatServer::handleNewConnection()
{
    QWebSocket *client = m_server->nextPendingConnection();
    m_clients.append(client);

    QString clientInfo = client->peerAddress().toString()
                         + ":" + QString::number(client->peerPort());
    qDebug() << "[Server] Client connected:" << clientInfo
             << "(total:" << m_clients.size() << ")";

    // 广播新客户端加入通知
    broadcastMessage(
        "[System] New client joined from " + clientInfo);

    // 收到文本消息时广播
    connect(client, &QWebSocket::textMessageReceived, this,
            [this, client](const QString &message) {
        qDebug() << "[Server] Text from"
                 << client->peerAddress().toString()
                 << ":" << message;
        broadcastMessage(message);
    });

    // 收到二进制消息时处理
    connect(client, &QWebSocket::binaryMessageReceived, this,
            [client](const QByteArray &data) {
        qDebug() << "[Server] Binary from"
                 << client->peerAddress().toString()
                 << ":" << data.size() << "bytes";
        // 回传确认收到二进制数据
        client->sendTextMessage(
            QString("[Server] Received binary: %1 bytes")
                .arg(data.size()));
    });

    // 客户端断开时清理
    connect(client, &QWebSocket::disconnected, this,
            [this, client]() {
        qDebug() << "[Server] Client disconnected:"
                 << client->peerAddress().toString();
        m_clients.removeOne(client);
        broadcastMessage(
            "[System] Client left. Remaining: "
            + QString::number(m_clients.size()));
        client->deleteLater();
    });

    // 发送欢迎消息
    client->sendTextMessage(
        "[Server] Welcome to the chat room! Online: "
        + QString::number(m_clients.size()));
}

void ChatServer::broadcastMessage(const QString &message)
{
    for (QWebSocket *client : m_clients) {
        client->sendTextMessage(message);
    }
}
