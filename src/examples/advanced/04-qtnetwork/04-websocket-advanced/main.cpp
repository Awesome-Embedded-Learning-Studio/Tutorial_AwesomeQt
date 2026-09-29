/// @file    main.cpp
/// @brief   演示 WebSocket 指数退避重连和 Ping/Pong 心跳的完整流程。
///
/// @details 对应教程：进阶层 04-QtNetwork/04-WebSocket。
///          启动 EchoServer，创建 ReconnectClient 连接，模拟服务器重启，
///          观察客户端自动重连行为，演示结束后自动退出。

#include "echo_server.h"
#include "reconnect_client.h"

#include <QCoreApplication>
#include <QTimer>
#include <QUrl>

/// @brief 演示控制器，编排服务器启停和客户端消息发送的时序。
class DemoController : public QObject
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化服务器和客户端。
    /// @param[in] parent 父对象指针。
    explicit DemoController(QObject* parent = nullptr)
        : QObject(parent)
        , m_server(this)
        , m_client(this)
        , m_messageIndex(0)
        , m_reconnectObserved(false)
    {
        // 监听客户端状态变化
        connect(&m_client, &ReconnectClient::stateChanged,
                this, &DemoController::onClientStateChanged);

        // 监听客户端收到消息
        connect(&m_client, &ReconnectClient::messageReceived,
                this, &DemoController::onMessageReceived);

        // 监听服务器消息回传
        connect(&m_server, &EchoServer::messageEchoed,
                this, [this](const QString& msg) {
                    qDebug("  [Server] echoed: %s", qPrintable(msg));
                });

        connect(&m_server, &EchoServer::clientConnected,
                this, [this]() {
                    qDebug("  [Server] client count: %d", m_server.clientCount());
                });
    }

    /// @brief 启动演示流程。
    void run()
    {
        qDebug("=== WebSocket Reconnect + Heartbeat Demo ===\n");

        // 阶段 1: 启动服务器并连接
        qDebug("--- Phase 1: Start server and connect ---");
        if (!m_server.start(kServerPort))
        {
            qCritical("Failed to start echo server, aborting");
            QCoreApplication::quit();
            return;
        }

        QUrl url(QStringLiteral("ws://localhost:%1").arg(kServerPort));
        m_client.connectToServer(url);

        // 阶段 2: 连接成功后发送几条消息
        // @note 延迟 500ms 等待连接建立完成
        QTimer::singleShot(500, this, [this]() {
            qDebug("\n--- Phase 2: Send messages and receive echoes ---");
            m_client.sendMessage(QStringLiteral("Hello, WebSocket!"));
            m_client.sendMessage(QStringLiteral("Second message"));
            m_client.sendMessage(QStringLiteral("Third message"));
        });

        // 阶段 3: 模拟服务器宕机
        QTimer::singleShot(2000, this, [this]() {
            qDebug("\n--- Phase 3: Simulate server crash (stop server) ---");
            m_server.stop();
            qDebug("  Server stopped. Client should detect disconnection "
                   "and start reconnecting...");
        });

        // 阶段 4: 在客户端重连尝试期间发送一条消息（会被丢弃）
        QTimer::singleShot(4000, this, [this]() {
            qDebug("\n--- Phase 4: Try sending while disconnected ---");
            m_client.sendMessage(QStringLiteral("This will be dropped"));
            qDebug("  (Message dropped because client is not connected)");
        });

        // 阶段 5: 重启服务器，客户端应自动重连成功
        // @note 给客户端足够的时间经历几次指数退避
        QTimer::singleShot(8000, this, [this]() {
            qDebug("\n--- Phase 5: Restart server (client should reconnect) ---");
            m_reconnectObserved = true;
            if (!m_server.start(kServerPort))
            {
                qWarning("Failed to restart server");
            }
        });

        // 阶段 6: 重连成功后发送新消息
        QTimer::singleShot(10000, this, [this]() {
            qDebug("\n--- Phase 6: Send messages after reconnect ---");
            m_client.sendMessage(QStringLiteral("Reconnected! Hello again!"));
            m_client.sendMessage(QStringLiteral("Final message"));
        });

        // 阶段 7: 干净关闭，结束演示
        QTimer::singleShot(12000, this, [this]() {
            qDebug("\n--- Phase 7: Clean shutdown ---");
            m_client.disconnect();
            m_server.stop();

            qDebug("\n=== Demo complete. Quitting in 1 second... ===");
            QTimer::singleShot(1000, this, []() {
                QCoreApplication::quit();
            });
        });
    }

private slots:
    /// @brief 客户端状态变化回调。
    /// @param[in] state 新状态。
    void onClientStateChanged(ReconnectClient::State state)
    {
        const char* name = "Unknown";
        switch (state)
        {
        case ReconnectClient::State::kDisconnected:
            name = "Disconnected";
            break;
        case ReconnectClient::State::kConnecting:
            name = "Connecting";
            break;
        case ReconnectClient::State::kConnected:
            name = "Connected";
            break;
        case ReconnectClient::State::kReconnecting:
            name = "Reconnecting";
            break;
        }
        qDebug("  [Client] state -> %s", name);
    }

    /// @brief 客户端收到消息回调。
    /// @param[in] message 收到的消息。
    void onMessageReceived(const QString& message)
    {
        ++m_messageIndex;
        qDebug("  [Client] received echo #%d: %s", m_messageIndex, qPrintable(message));
    }

private:
    EchoServer m_server;               ///< Echo 服务器实例
    ReconnectClient m_client;          ///< 重连客户端实例
    int m_messageIndex;                ///< 收到的消息计数
    bool m_reconnectObserved;          ///< 是否已观察过重连阶段

    static constexpr quint16 kServerPort = 12345;  ///< 服务器监听端口
};

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    DemoController controller;
    // @note 延迟启动，确保事件循环已运行
    QTimer::singleShot(0, &controller, &DemoController::run);

    return app.exec();
}

#include "main.moc"
