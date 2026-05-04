/// 聊天室服务端：接收任一客户端消息并广播给所有客户端
#pragma once

#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QList>

class ChatServer : public QObject
{
    Q_OBJECT

public:
    explicit ChatServer(QObject *parent = nullptr);

    /// 返回当前在线客户端数量
    int clientCount() const { return m_clients.size(); }

    ~ChatServer() override;

private:
    void handleNewConnection();

    /// 向所有已连接客户端广播文本消息
    void broadcastMessage(const QString &message);

    QWebSocketServer *m_server;
    QList<QWebSocket *> m_clients;
};
