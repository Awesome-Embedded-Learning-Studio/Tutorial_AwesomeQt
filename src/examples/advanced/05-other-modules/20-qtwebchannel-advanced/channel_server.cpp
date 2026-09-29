/// @file    channel_server.cpp
/// @brief   Implementation of WebSocketTransport and ChannelServer.
///
/// Corresponds to tutorial: advanced 05-other-modules/20-qtwebchannel.
///
/// Design note: QWebChannel is transport-agnostic. It communicates through
/// QWebChannelAbstractTransport instances. WebSocketTransport adapts each
/// QWebSocket to this interface, translating between text frames and the
/// QJsonObject messages that the channel expects.
///
/// The connection pattern follows Qt's official standalone example: each
/// incoming WebSocket is wrapped in a WebSocketTransport, and the transport
/// is passed to QWebChannel::connectTo() via a direct signal-slot connection.

#include "channel_server.h"
#include "chat_backend.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

#include <QWebChannel>
#include <QWebSocket>
#include <QWebSocketServer>

// ---------------------------------------------------------------------------
// WebSocketTransport
// ---------------------------------------------------------------------------

WebSocketTransport::WebSocketTransport(QWebSocket* socket)
    : QWebChannelAbstractTransport(socket)
    , m_socket(socket)
{
    // The socket is parented to this transport so it is freed automatically.
    // Use DirectConnection to ensure messages are forwarded to QWebChannel
    // immediately in the same event loop iteration they arrive.
    connect(m_socket, &QWebSocket::textMessageReceived,
            this, &WebSocketTransport::onTextMessageReceived,
            Qt::DirectConnection);

    // When the socket drops, self-destruct so the channel removes this transport.
    connect(m_socket, &QWebSocket::disconnected,
            this, &WebSocketTransport::deleteLater);
}

WebSocketTransport::~WebSocketTransport()
{
    m_socket->deleteLater();
}

void WebSocketTransport::sendMessage(const QJsonObject& message)
{
    QJsonDocument doc(message);
    m_socket->sendTextMessage(
        QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
}

void WebSocketTransport::onTextMessageReceived(const QString& messageData)
{
    qDebug() << "[WebSocketTransport] received:" << messageData.left(200);

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(messageData.toUtf8(), &error);

    if (error.error) {
        qWarning() << "[WebSocketTransport] JSON parse error:" << error.errorString()
                   << "\n  Raw data:" << messageData;
        return;
    }

    if (!doc.isObject()) {
        qWarning() << "[WebSocketTransport] expected JSON object, got:" << messageData;
        return;
    }

    // The second argument must be `this` so QWebChannel knows which transport
    // delivered the message — it uses this to route responses correctly.
    emit messageReceived(doc.object(), this);
}

// ---------------------------------------------------------------------------
// ChannelServer
// ---------------------------------------------------------------------------

ChannelServer::ChannelServer(QObject* parent)
    : QObject(parent)
    , m_server(nullptr)
    , m_channel(nullptr)
    , m_backend(nullptr)
{
    // NonSecureMode is fine for localhost demonstrations. Production code
    // should use QWebSocketServer::SecureMode with TLS certificates.
    m_server = new QWebSocketServer(
        "WebChannelServer", QWebSocketServer::NonSecureMode, this);

    m_channel = new QWebChannel(this);

    // The backend is registered under the key "backend" so remote clients
    // access it as channel.objects.backend in the WebChannel protocol.
    m_backend = new ChatBackend(m_channel);
    m_channel->registerObject("backend", m_backend);

    // Follow the official Qt standalone WebChannel example pattern.
    connect(m_server, &QWebSocketServer::newConnection,
            this, [this]() {
                QWebSocket* socket = m_server->nextPendingConnection();
                if (!socket) return;

                qDebug() << "[ChannelServer] new connection from"
                         << socket->peerAddress().toString() << ":"
                         << socket->peerPort();

                auto* transport = new WebSocketTransport(socket);
                m_channel->connectTo(transport);

                connect(transport, &QObject::destroyed, this, [this]() {
                    qDebug() << "[ChannelServer] client transport destroyed";
                    emit clientDisconnected();
                });

                emit clientConnected();
            });
}

bool ChannelServer::start(quint16 port)
{
    if (!m_server->listen(QHostAddress::Any, port)) {
        qWarning() << "[ChannelServer] failed to listen on port" << port
                   << "-" << m_server->errorString();
        return false;
    }

    qDebug() << "[ChannelServer] listening on ws://localhost:"
             << m_server->serverPort();
    return true;
}

quint16 ChannelServer::serverPort() const
{
    return m_server->serverPort();
}

ChatBackend* ChannelServer::backend() const
{
    return m_backend;
}
