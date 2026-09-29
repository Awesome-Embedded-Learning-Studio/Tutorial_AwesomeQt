/// @file    main.cpp
/// @brief   程序入口——演示 WebSocket 聊天服务器的房间广播与路由。
///
/// 对应教程：进阶层 05-其他模块/19-QtWebSockets Server。
/// 启动服务器后创建 3 个测试客户端，验证房间隔离和消息路由。

#include "chat_server.h"

#include <QCoreApplication>
#include <QTimer>
#include <QWebSocket>

#include <cstdio>

/// @brief 创建一个测试客户端并连接到服务器。
/// @param[in] url    WebSocket 服务器地址。
/// @param[in] parent 父对象。
/// @return 指向新创建的 QWebSocket 的裸指针（父对象持有所有权）。
static QWebSocket* createClient(const QUrl& url, QObject* parent)
{
    // QWebSocket 构造函数要求 origin、version、parent 三个参数
    auto* socket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest,
                                  parent);
    socket->open(url);
    return socket;
}

/// @brief 构造并发送一条 JSON 协议消息。
/// @param[in] socket 目标 WebSocket 连接。
/// @param[in] json   要发送的 JSON 文本。
static void sendJson(QWebSocket* socket, const QString& json)
{
    socket->sendTextMessage(json);
}

/// @brief 将服务器日志信号输出到 stderr，保证即时可见。
/// @param[in] msg 日志内容。
static void logServer(const QString& msg)
{
    std::fprintf(stderr, "%s\n", msg.toUtf8().constData());
    std::fflush(stderr);
}

/// @brief 将客户端收到的消息输出到 stderr。
/// @param[in] tag  客户端标识（如 "Client1"）。
/// @param[in] msg  收到的消息。
static void logClient(const char* tag, const QString& msg)
{
    std::fprintf(stderr, "[%s RX] %s\n", tag, msg.toUtf8().constData());
    std::fflush(stderr);
}

/// @brief 输出阶段分隔符。
/// @param[in] text 阶段描述。
static void logPhase(const char* text)
{
    std::fprintf(stderr, "\n=== %s ===\n", text);
    std::fflush(stderr);
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    // -- 1. 启动聊天服务器 ------------------------------------------------
    const quint16 kPort = 12345;
    ChatServer server;
    server.start(kPort);

    // 将服务器日志输出到 stderr
    QObject::connect(&server, &ChatServer::serverInfo, &logServer);

    // -- 2. 创建 3 个测试客户端 -------------------------------------------
    const QUrl serverUrl(QStringLiteral("ws://localhost:%1").arg(kPort));

    auto* client1 = createClient(serverUrl, &app);
    auto* client2 = createClient(serverUrl, &app);
    auto* client3 = createClient(serverUrl, &app);

    // 记录各客户端收到的消息
    QObject::connect(client1, &QWebSocket::textMessageReceived,
                     [](const QString& msg)
                     { logClient("Client1", msg); });
    QObject::connect(client2, &QWebSocket::textMessageReceived,
                     [](const QString& msg)
                     { logClient("Client2", msg); });
    QObject::connect(client3, &QWebSocket::textMessageReceived,
                     [](const QString& msg)
                     { logClient("Client3", msg); });

    // -- 3. 编排演示时序 --------------------------------------------------
    // 所有客户端先等待连接建立（WebSocket 握手是异步的），
    // 然后依次加入房间、发消息、验证路由。

    // 阶段 1 (t=500ms): 客户端加入房间
    QTimer::singleShot(500, [&]()
    {
        logPhase("Phase 1: Clients join rooms");

        // 客户端 1 加入 "general" 房间
        sendJson(client1,
                 QStringLiteral(R"({"type":"join","room":"general"})"));

        // 客户端 2 加入 "general" 房间
        sendJson(client2,
                 QStringLiteral(R"({"type":"join","room":"general"})"));

        // 客户端 3 加入 "random" 房间
        sendJson(client3,
                 QStringLiteral(R"({"type":"join","room":"random"})"));
    });

    // 阶段 2 (t=1500ms): 客户端 1 在 general 发消息
    // 预期：客户端 1 和 2 收到，客户端 3 收不到（不同房间）
    QTimer::singleShot(1500, [&]()
    {
        logPhase("Phase 2: Client1 sends to 'general'");
        sendJson(client1,
                 QStringLiteral(
                     R"({"type":"msg","text":"Hello from client1"})"));
    });

    // 阶段 3 (t=2500ms): 客户端 3 在 random 发消息
    // 预期：只有客户端 3 自己收到（random 房间只有它一人）
    QTimer::singleShot(2500, [&]()
    {
        logPhase("Phase 3: Client3 sends to 'random' (alone)");
        sendJson(client3,
                 QStringLiteral(
                     R"({"type":"msg","text":"Hello from client3"})"));
    });

    // 阶段 4 (t=3500ms): 在 general 房间发第二条消息，确认房间隔离
    QTimer::singleShot(3500, [&]()
    {
        logPhase("Phase 4: Client2 sends to 'general'");
        sendJson(client2,
                 QStringLiteral(
                     R"({"type":"msg","text":"Hello from client2"})"));
    });

    // 阶段 5 (t=5000ms): 演示结束，自动退出
    QTimer::singleShot(5000, [&]()
    {
        logPhase("Demo complete. Quitting...");
        QCoreApplication::quit();
    });

    return app.exec();
}
