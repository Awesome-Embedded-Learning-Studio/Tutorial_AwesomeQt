/// WebSocket 客户端：连接服务器、收发消息、心跳保活
#pragma once

#include <QObject>
#include <QWebSocket>
#include <QTimer>

class ChatClient : public QObject
{
    Q_OBJECT

public:
    explicit ChatClient(QObject *parent = nullptr);

    ~ChatClient() override;

    /// 连接到 WebSocket 服务器
    void connectToServer(const QUrl &url);

    /// 发送文本消息
    void sendTextMessage(const QString &message);

    /// 发送二进制消息
    void sendBinaryMessage(const QByteArray &data);

    /// 主动断开连接
    void disconnectFromServer();

    /// 返回已接收的消息数量
    int messageCount() const { return m_messageCount; }

private:
    QWebSocket *m_ws;
    QTimer *m_heartbeatTimer;
    int m_messageCount;
};
