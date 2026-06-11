/// @file    main.cpp
/// @brief   Console demo that starts a TcpServer and a test client.
///
/// @details 对应教程：进阶层 04-QtNetwork/01-TCP 高级。
///          演示流程：启动服务器 → 客户端连接 → 发送 3 条消息 →
///          展示心跳机制 → 自动退出。

#include "protocol_frame.h"
#include "tcp_server.h"

#include <QCoreApplication>
#include <QHostAddress>
#include <QTcpSocket>
#include <QTimer>

/// @brief Sequenced demo client that sends framed messages and then exits.
class DemoClient : public QObject
{
    Q_OBJECT

public:
    /// @brief Construct the demo client.
    /// @param[in] parent Owner QObject.
    explicit DemoClient(QObject* parent = nullptr)
        : QObject(parent)
        , m_socket(new QTcpSocket(this))
        , m_step(0)
    {
        connect(m_socket, &QTcpSocket::connected,
                this,     &DemoClient::onConnected);

        connect(m_socket, &QTcpSocket::readyRead,
                this,     &DemoClient::onReadyRead);

        connect(m_socket, &QTcpSocket::errorOccurred,
                this,     [this](QAbstractSocket::SocketError error) {
                    qWarning("Client socket error: %d — %s",
                             static_cast<int>(error),
                             m_socket->errorString().toUtf8().constData());
                });
    }

    /// @brief Initiate connection to the server.
    /// @param[in] host Server host address.
    /// @param[in] port Server port number.
    void connectToServer(const QHostAddress& host, quint16 port)
    {
        m_socket->connectToHost(host, port);
    }

private slots:
    /// @brief Called when the TCP connection is established.
    void onConnected()
    {
        qInfo("Demo client connected to server");

        // Schedule the first message after a short delay so the event loop
        // has time to settle and the server-side clientConnected() signal
        // fires before we start sending data.
        QTimer::singleShot(100, this, &DemoClient::sendNextMessage);
    }

    /// @brief Display any frames received from the server (e.g. heartbeats).
    void onReadyRead()
    {
        QByteArray rawData = m_socket->readAll();

        // We reuse FrameParser on the client side too.
        if (!m_clientParser) {
            m_clientParser = new FrameParser(this);
            connect(m_clientParser, &FrameParser::frameReady,
                    this, [](const QByteArray& payload) {
                        qInfo("Client received frame: %s",
                              payload.constData());
                    });
        }
        m_clientParser->parse(rawData);
    }

private:
    /// @brief Send the next demo message in the sequence.
    void sendNextMessage()
    {
        static const char* kMessages[] = {
            "Hello from demo client!",
            "Second message: 42",
            "Final message: goodbye"
        };

        if (m_step < 3) {
            QByteArray payload(kMessages[m_step]);
            QByteArray frame = ProtocolFrame::encode(payload);
            m_socket->write(frame);
            qInfo("Client sent: %s", kMessages[m_step]);
            ++m_step;
            QTimer::singleShot(500, this, &DemoClient::sendNextMessage);
        } else {
            // All messages sent — wait a moment to see heartbeat exchange,
            // then quit the application.
            qInfo("All demo messages sent, waiting for heartbeat...");
            QTimer::singleShot(3000, qApp, &QCoreApplication::quit);
        }
    }

    QTcpSocket* m_socket;          ///< Client-side TCP socket
    FrameParser* m_clientParser{nullptr};  ///< Parses inbound frames from server
    int m_step;                    ///< Current message index in the demo sequence
};

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    static constexpr quint16 kServerPort = 12345;

    // --- Server ---
    TcpServer server;
    if (!server.listen(kServerPort)) {
        qCritical("Failed to start TCP server on port %d", kServerPort);
        return 1;
    }
    qInfo("TcpServer listening on port %d", kServerPort);

    // Log server-side events so the demo output is self-explanatory.
    QObject::connect(&server, &TcpServer::clientConnected,
                     [](int id) {
                         qInfo("[Server] clientConnected(id=%d)", id);
                     });

    QObject::connect(&server, &TcpServer::clientDisconnected,
                     [](int id) {
                         qInfo("[Server] clientDisconnected(id=%d)", id);
                     });

    QObject::connect(&server, &TcpServer::frameReceived,
                     [](int id, const QByteArray& payload) {
                         qInfo("[Server] frameReceived(id=%d, payload=\"%s\")",
                               id, payload.constData());
                     });

    // --- Client ---
    auto* client = new DemoClient(&app);
    // @note Small delay so the server is fully bound before the client dials.
    QTimer::singleShot(200, client, [client]() {
        client->connectToServer(QHostAddress::LocalHost, kServerPort);
    });

    return app.exec();
}

// @note Required because we use a Q_OBJECT in a .cpp file; AUTOMOC picks
//       this up via the #include "main.moc" or direct compilation unit scan.
#include "main.moc"
