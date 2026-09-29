/// @file    echo_server.cpp
/// @brief   WebSocket Echo 服务器的实现。
///
/// @details 监听指定端口，接受连接并将收到的文本消息原样回传。
///          对应教程：进阶层 04-QtNetwork/04-WebSocket。

#include "echo_server.h"

#include <QDebug>
#include <QWebSocket>
#include <QWebSocketServer>

EchoServer::EchoServer(QObject* parent)
    : QObject(parent)
    , m_server(new QWebSocketServer(QStringLiteral("EchoServer"),
                                    QWebSocketServer::NonSecureMode,
                                    this))
{
    connect(m_server, &QWebSocketServer::newConnection,
            this, &EchoServer::onNewConnection);
}

EchoServer::~EchoServer()
{
    stop();
}

bool EchoServer::start(quint16 port)
{
    if (m_server->isListening())
    {
        return false;
    }

    const bool ok = m_server->listen(QHostAddress::Any, port);
    if (ok)
    {
        qDebug("EchoServer: listening on port %u", static_cast<unsigned>(port));
    }
    else
    {
        qDebug("EchoServer: failed to listen on port %u - %s",
               static_cast<unsigned>(port),
               qPrintable(m_server->errorString()));
    }

    return ok;
}

void EchoServer::stop()
{
    if (!m_server->isListening())
    {
        return;
    }

    // @note 必须先逐个关闭客户端连接，再关闭服务器，
    //       否则客户端可能收不到正确的 close frame
    for (auto* client : m_clients)
    {
        client->close(QWebSocketProtocol::CloseCodeNormal,
                      QLatin1String("Server shutting down"));
        client->deleteLater();
    }
    m_clients.clear();

    m_server->close();
    qDebug("EchoServer: stopped");
}

bool EchoServer::isRunning() const
{
    return m_server->isListening();
}

int EchoServer::clientCount() const
{
    return m_clients.size();
}

void EchoServer::onNewConnection()
{
    QWebSocket* client = m_server->nextPendingConnection();
    if (!client)
    {
        return;
    }

    // @note 设置父对象为 nullptr，由本类通过 deleteLater() 管理
    m_clients.append(client);

    connect(client, &QWebSocket::textMessageReceived,
            this, &EchoServer::onTextMessageReceived);
    connect(client, &QWebSocket::disconnected,
            this, &EchoServer::onClientDisconnected);

    qDebug("EchoServer: client connected, total %lld clients",
           static_cast<long long>(m_clients.size()));
    emit clientConnected();
}

void EchoServer::onTextMessageReceived(const QString& message)
{
    // @note qobject_cast 安全获取发送者； sender() 返回信号发射对象
    auto* client = qobject_cast<QWebSocket*>(sender());
    if (!client)
    {
        return;
    }

    // Echo: 将收到的消息原样回传
    client->sendTextMessage(message);
    emit messageEchoed(message);
}

void EchoServer::onClientDisconnected()
{
    auto* client = qobject_cast<QWebSocket*>(sender());
    if (!client)
    {
        return;
    }

    m_clients.removeOne(client);
    client->deleteLater();

    qDebug("EchoServer: client disconnected, total %lld clients",
           static_cast<long long>(m_clients.size()));
    emit clientDisconnected(client);
}
