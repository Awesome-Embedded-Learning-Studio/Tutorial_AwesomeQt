/// @file    tcp_server.h
/// @brief   Multi-client TCP server with heartbeat detection.
///
/// @details 对应教程：进阶层 04-QtNetwork/01-TCP 高级。
///          管理多个 QTcpSocket 客户端，为每个连接分配 FrameParser，
///          定时发送心跳帧，超时未响应则自动断开。

#pragma once

#include "protocol_frame.h"

#include <QMap>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

/// @brief Multi-client TCP server with length-prefixed protocol and heartbeat.
class TcpServer : public QObject
{
    Q_OBJECT

public:
    /// @brief Construct the server (does not start listening yet).
    /// @param[in] parent Owner QObject.
    explicit TcpServer(QObject* parent = nullptr);

    /// @brief Start listening on the specified port.
    /// @param[in] port TCP port number to bind.
    /// @return True if the server is now listening successfully.
    bool listen(quint16 port);

    /// @brief Send a framed payload to a specific client.
    /// @param[in] clientId  Server-assigned client identifier.
    /// @param[in] payload   Application data to send.
    /// @note Silently ignores unknown client IDs.
    void sendToClient(int clientId, const QByteArray& payload);

    /// @brief Broadcast a framed payload to all connected clients.
    /// @param[in] payload Application data to send.
    void broadcast(const QByteArray& payload);

    /// @brief Get the number of currently connected clients.
    /// @return Client count.
    int clientCount() const;

signals:
    /// @brief Emitted when a new client connects.
    /// @param[in] id Server-assigned ID for the new client.
    void clientConnected(int id);

    /// @brief Emitted when a client disconnects (voluntarily or timeout).
    /// @param[in] id ID of the disconnected client.
    void clientDisconnected(int id);

    /// @brief Emitted when a complete frame is received from a client.
    /// @param[in] id      Client ID that sent the frame.
    /// @param[in] payload Decoded application payload.
    void frameReceived(int id, const QByteArray& payload);

private slots:
    /// @brief Handle a new incoming connection.
    void onNewConnection();

    /// @brief Handle ready-read from any client socket.
    void onReadyRead();

    /// @brief Handle client disconnection.
    void onClientDisconnected();

    /// @brief Periodic heartbeat: send ping, check timeouts.
    void onHeartbeatTick();

private:
    /// @brief Assign metadata for a newly connected socket.
    /// @param[in] socket The raw socket (TcpServer does NOT take ownership via
    ///                   QObject parent; we connect disconnected() to clean up).
    void registerClient(QTcpSocket* socket);

    /// @brief Remove a client and emit clientDisconnected().
    /// @param[in] socket The socket being removed.
    void unregisterClient(QTcpSocket* socket);

    /// @brief Look up the client ID for a given socket pointer.
    /// @param[in] socket Socket to search.
    /// @return Client ID or -1 if not found.
    int idForSocket(QTcpSocket* socket) const;

    QTcpServer* m_server{nullptr};  ///< TCP listener (child of this)

    int m_nextId{1};  ///< Monotonically increasing client ID counter

    /// @brief Per-client bookkeeping stored in a QMap keyed by client ID.
    struct ClientInfo
    {
        QTcpSocket* socket;       ///< Raw socket (not owned by QObject tree)
        FrameParser* parser;      ///< Per-client frame parser (child of TcpServer)
        qint64 lastSeenEpoch;     ///< Timestamp (msec since epoch) of last activity
    };

    QMap<int, ClientInfo> m_clients;  ///< Connected clients indexed by ID

    QTimer* m_heartbeatTimer{nullptr};  ///< Periodic heartbeat / timeout timer

    static constexpr int kHeartbeatIntervalMs = 5000;   ///< Send heartbeat every 5 s
    static constexpr int kTimeoutMs = 15000;            ///< Disconnect after 15 s silence
};
