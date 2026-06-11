/// @file    tcp_server.cpp
/// @brief   Implementation of TcpServer — multi-client TCP with heartbeat.

#include "tcp_server.h"

#include <QDateTime>
#include <QHostAddress>

// ---------------------------------------------------------------------------
// Construction / listening
// ---------------------------------------------------------------------------

TcpServer::TcpServer(QObject* parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
    , m_heartbeatTimer(new QTimer(this))
{
    connect(m_server, &QTcpServer::newConnection,
            this,     &TcpServer::onNewConnection);

    // @note Heartbeat timer drives both outgoing pings and timeout detection.
    connect(m_heartbeatTimer, &QTimer::timeout,
            this,             &TcpServer::onHeartbeatTick);
}

bool TcpServer::listen(quint16 port)
{
    if (!m_server->listen(QHostAddress::Any, port)) {
        return false;
    }
    m_heartbeatTimer->start(kHeartbeatIntervalMs);
    return true;
}

// ---------------------------------------------------------------------------
// Public helpers
// ---------------------------------------------------------------------------

void TcpServer::sendToClient(int clientId, const QByteArray& payload)
{
    auto it = m_clients.find(clientId);
    if (it == m_clients.end()) {
        return;
    }
    const QByteArray frame = ProtocolFrame::encode(payload);
    it->socket->write(frame);
}

void TcpServer::broadcast(const QByteArray& payload)
{
    const QByteArray frame = ProtocolFrame::encode(payload);
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        it->socket->write(frame);
    }
}

int TcpServer::clientCount() const
{
    return static_cast<int>(m_clients.size());
}

// ---------------------------------------------------------------------------
// Private slots
// ---------------------------------------------------------------------------

void TcpServer::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QTcpSocket* socket = m_server->nextPendingConnection();
        registerClient(socket);
    }
}

void TcpServer::onReadyRead()
{
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) {
        return;
    }

    int id = idForSocket(socket);
    if (id < 0) {
        return;
    }

    // Refresh the last-seen timestamp so heartbeat timeout resets.
    m_clients[id].lastSeenEpoch = QDateTime::currentMSecsSinceEpoch();

    // Feed all available bytes into the per-client parser.
    QByteArray data = socket->readAll();
    m_clients[id].parser->parse(data);
}

void TcpServer::onClientDisconnected()
{
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        unregisterClient(socket);
    }
}

void TcpServer::onHeartbeatTick()
{
    const qint64 now = QDateTime::currentMSecsSinceEpoch();

    // Build the list of IDs to evict first so we don't mutate the map
    // while iterating it.
    QList<int> toEvict;
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        if ((now - it->lastSeenEpoch) >= kTimeoutMs) {
            toEvict.append(it.key());
        }
    }

    for (int id : toEvict) {
        auto it = m_clients.find(id);
        if (it != m_clients.end()) {
            qInfo("Heartbeat timeout, disconnecting client %d", id);
            it->socket->disconnectFromHost();
            // onClientDisconnected() will clean up the map entry.
        }
    }

    // Send a heartbeat ping to every remaining client.
    QByteArray heartbeatPayload = "HEARTBEAT";
    broadcast(heartbeatPayload);
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void TcpServer::registerClient(QTcpSocket* socket)
{
    int id = m_nextId++;

    // @note FrameParser is created as a child of TcpServer so it is
    //       destroyed automatically when the server is destroyed.
    FrameParser* parser = new FrameParser(this);

    ClientInfo info;
    info.socket = socket;
    info.parser = parser;
    info.lastSeenEpoch = QDateTime::currentMSecsSinceEpoch();
    m_clients.insert(id, info);

    // Wire up socket signals.
    connect(socket, &QTcpSocket::readyRead,
            this,   &TcpServer::onReadyRead);
    connect(socket, &QTcpSocket::disconnected,
            this,   &TcpServer::onClientDisconnected);

    // Forward parsed frames as server-level signals.
    connect(parser, &FrameParser::frameReady,
            this,   [this, id](const QByteArray& payload) {
                emit frameReceived(id, payload);
            });

    qInfo("Client %d connected (%s:%d)",
          id,
          socket->peerAddress().toString().toUtf8().constData(),
          socket->peerPort());

    emit clientConnected(id);
}

void TcpServer::unregisterClient(QTcpSocket* socket)
{
    int id = idForSocket(socket);
    if (id < 0) {
        return;
    }

    qInfo("Client %d disconnected", id);

    // @note We delete the parser here instead of relying on parent-child
    //       destruction to release per-client memory promptly.
    auto it = m_clients.find(id);
    delete it->parser;
    it->socket->deleteLater();

    m_clients.erase(it);
    emit clientDisconnected(id);
}

int TcpServer::idForSocket(QTcpSocket* socket) const
{
    for (auto it = m_clients.constBegin(); it != m_clients.constEnd(); ++it) {
        if (it->socket == socket) {
            return it.key();
        }
    }
    return -1;
}
