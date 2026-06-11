/// @file    test_client.h
/// @brief   WebSocket test client that speaks the QWebChannel protocol.
///
/// Acts as a lightweight client that manually implements the WebChannel
/// handshake (init message) and method invocation protocol to demonstrate
/// that a non-Qt JavaScript or C++ client can interact with the server
/// backend purely over WebSocket JSON messages.

#pragma once

#include <QObject>
#include <QUrl>

class QWebSocket;

/// @brief Test client that drives the WebChannel demo end-to-end.
///
/// Connects to the WebSocket server, sends the "init" handshake expected
/// by QWebChannel, then invokes sendMessage and getMessageHistory on the
/// published "backend" object. All communication is raw JSON over WebSocket.
class TestClient : public QObject
{
    Q_OBJECT

public:
    /// @brief Constructs the client and immediately opens a WebSocket connection.
    /// @param[in] url  The WebSocket URL to connect to (e.g. ws://localhost:12345).
    /// @param[in] parent  Parent QObject for lifecycle management.
    explicit TestClient(const QUrl& url, QObject* parent = nullptr);

private slots:
    /// @brief Sends the WebChannel init handshake upon WebSocket connection.
    void onConnected();

    /// @brief Dispatches incoming JSON messages by WebChannel message type.
    /// @param[in] message  Raw text frame from the server.
    void onTextMessageReceived(const QString& message);

    /// @brief Logs the disconnection event.
    void onDisconnected();

private:
    /// @brief Serializes and sends a JSON object over the WebSocket.
    /// @param[in] obj  The JSON object to send.
    void sendJson(const QJsonObject& obj);

    /// @brief Invokes backend.sendMessage() via the WebChannel protocol.
    void sendTestMessage();

    /// @brief Invokes backend.getMessageHistory() via the WebChannel protocol.
    void requestHistory();

    /// @brief Prints a summary and quits the application.
    void quitApp();

    QWebSocket* m_socket;  ///< WebSocket connection to the channel server.
    int m_msgId = 0;       ///< Monotonic ID for request/response correlation.
    int m_step = 0;        ///< Tracks which demo phase is active (0, 1, 2).
};
