/// TCP 客户端：连接服务器并发送测试消息，支持自动重连
#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>

class TcpClient : public QObject
{
    Q_OBJECT

public:
    explicit TcpClient(QObject *parent = nullptr);

    /// 发起连接
    void connectToServer(const QHostAddress &host, quint16 port);

    /// 发送数据
    void sendMessage(const QByteArray &data);

    /// 主动断开连接
    void disconnectFromServer();

private:
    /// 延迟重连（指数退避：1s, 2s, 4s...）
    void attemptReconnect();

    QTcpSocket *m_socket;
    QHostAddress m_host;
    quint16 m_port;
    int m_retryCount;
    int m_maxRetries;
};
