/// @file    channel_server.h
/// @brief   WebSocket transport + QWebChannel server for remote C++ IPC.
///
/// Provides a QWebChannelAbstractTransport subclass that wraps each
/// QWebSocket, and a ChannelServer that owns the QWebSocketServer, QWebChannel,
/// and ChatBackend. This is the standard pattern for using QWebChannel without
/// QtWebEngine — any WebSocket client (browser JS, another Qt app) can connect.

#pragma once

#include <QHash>
#include <QObject>
#include <QWebChannelAbstractTransport>

QT_BEGIN_NAMESPACE
class QWebSocket;
class QWebSocketServer;
QT_END_NAMESPACE

class QWebChannel;
class ChatBackend;

/// @brief Transport bridge between a single QWebSocket and QWebChannel.
///
/// Subclasses QWebChannelAbstractTransport as required by QWebChannel::connectTo().
/// Incoming WebSocket text frames are parsed as JSON and emitted via the
/// messageReceived signal; outgoing QJsonObject messages are serialized and
/// sent as text frames.
class WebSocketTransport : public QWebChannelAbstractTransport
{
    Q_OBJECT

public:
    /// @brief Wraps a connected WebSocket as a WebChannel transport.
    /// @param[in] socket  A live QWebSocket; becomes the transport's parent.
    /// @note The socket is parented to this transport and will be deleted
    ///       when the transport is destroyed.
    explicit WebSocketTransport(QWebSocket* socket);

    /// @brief Destructor — schedules the wrapped socket for deletion.
    ~WebSocketTransport() override;

    /// @brief Serializes a JSON message and sends it as a text frame.
    /// @param[in] message  The QJsonObject to send to the remote client.
    void sendMessage(const QJsonObject& message) override;

private slots:
    /// @brief Parses an incoming text frame and emits messageReceived.
    /// @param[in] messageData  Raw JSON text from the WebSocket.
    void onTextMessageReceived(const QString& messageData);

private:
    QWebSocket* m_socket;  ///< The wrapped WebSocket connection (owned via parentship).
};

// ---------------------------------------------------------------------------

/// @brief Server that hosts QWebChannel over WebSocket transport.
///
/// Owns the QWebSocketServer, QWebChannel, and ChatBackend. Each incoming
/// WebSocket connection is wrapped in a WebSocketTransport and registered
/// with the QWebChannel so the client can access the published backend.
class ChannelServer : public QObject
{
    Q_OBJECT

public:
    /// @brief Constructs the server with all internal objects wired up.
    /// @param[in] parent  Parent QObject for lifecycle management.
    explicit ChannelServer(QObject* parent = nullptr);

    /// @brief Starts listening for WebSocket connections.
    /// @param[in] port  The TCP port to bind to.
    /// @return true if the server started successfully, false on error.
    bool start(quint16 port);

    /// @brief Returns the port the server is currently listening on.
    /// @return The port number, or 0 if not listening.
    quint16 serverPort() const;

    /// @brief Returns the ChatBackend pointer for direct C++ interaction.
    ChatBackend* backend() const;

signals:
    /// @brief Emitted when a new client connects via WebSocket.
    void clientConnected();

    /// @brief Emitted when a client disconnects.
    void clientDisconnected();

private:
    QWebSocketServer* m_server;    ///< WebSocket server (owned).
    QWebChannel* m_channel;        ///< WebChannel bridge (owned).
    ChatBackend* m_backend;        ///< Published backend object (owned by channel).
};
