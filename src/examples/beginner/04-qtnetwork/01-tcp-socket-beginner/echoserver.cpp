#include "echoserver.h"

#include <QDebug>

EchoServer::EchoServer(QObject *parent)
    : QObject(parent), m_server(new QTcpServer(this))
{
    // 监听所有网卡接口的 12345 端口
    if (!m_server->listen(QHostAddress::Any, 12345)) {
        qDebug() << "[Server] Failed to start:" << m_server->errorString();
        return;
    }

    qDebug() << "[Server] Listening on port"
             << m_server->serverPort() << "...";

    // 新客户端连接时触发
    connect(m_server, &QTcpServer::newConnection, this, [this]() {
        handleNewConnection();
    });
}

void EchoServer::handleNewConnection()
{
    // 取出待处理的客户端 Socket
    QTcpSocket *client = m_server->nextPendingConnection();
    m_clients.append(client);

    qDebug() << "[Server] Client connected:"
             << client->peerAddress().toString()
             << ":" << client->peerPort()
             << "(total:" << m_clients.size() << ")";

    // 客户端发来数据时回显
    connect(client, &QTcpSocket::readyRead, this, [client]() {
        // TCP 是字节流，readyRead 可能包含不完整的消息
        // 这里为简化演示直接 readAll，实际项目需要做消息分帧
        QByteArray data = client->readAll();
        qDebug() << "[Server] Received from"
                 << client->peerAddress().toString()
                 << ":" << data;

        // 回声：原样发回
        client->write(data);
        client->flush();
    });

    // 客户端断开时清理
    connect(client, &QTcpSocket::disconnected, this, [this, client]() {
        qDebug() << "[Server] Client disconnected:"
                 << client->peerAddress().toString();
        m_clients.removeOne(client);
        client->deleteLater();
    });

    // 网络错误处理
    connect(client, &QAbstractSocket::errorOccurred, this, [client](
                 QAbstractSocket::SocketError error) {
        qDebug() << "[Server] Client error:"
                 << error << client->errorString();
    });
}
