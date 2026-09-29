/// 回声服务器：将客户端发来的数据原样返回
#pragma once

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>

class EchoServer : public QObject
{
    Q_OBJECT

public:
    explicit EchoServer(QObject *parent = nullptr);

    /// 返回当前连接的客户端数量
    int clientCount() const { return m_clients.size(); }

private:
    void handleNewConnection();

    QTcpServer *m_server;
    QList<QTcpSocket *> m_clients;
};
