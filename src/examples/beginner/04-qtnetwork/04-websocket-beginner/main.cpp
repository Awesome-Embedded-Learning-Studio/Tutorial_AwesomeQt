/**
 * Qt WebSocket 基础示例
 *
 * 本示例演示 WebSocket 服务端与客户端的完整通信流程：
 * 1. QWebSocketServer 监听端口并接受连接
 * 2. QWebSocket 客户端连接服务器
 * 3. 文本消息与二进制消息的双向收发
 * 4. 心跳 ping/pong 机制
 * 5. 多客户端管理与消息广播
 *
 * 核心要点：
 * - WebSocket 是面向消息的全双工协议（不存在粘包问题）
 * - QtWebSockets 模块独立于 QtNetwork，需单独 find_package
 * - 服务端通过 newConnection 信号逐个获取客户端 QWebSocket
 * - 断开的客户端必须 deleteLater() 以防内存泄漏
 */

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

#include "chatserver.h"
#include "chatclient.h"

// ========================================
// 主函数：演示服务端 + 多客户端通信
// ========================================

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "=== Qt WebSocket Basic Example ===\n";

    // 第一步：启动聊天室服务端
    ChatServer server;

    // 第二步：创建第一个客户端并连接
    ChatClient client1;
    client1.connectToServer(QUrl("ws://localhost:12345"));

    // 延迟 500ms 后创建第二个客户端
    ChatClient *client2 = new ChatClient(&app);
    QTimer::singleShot(500, [&client2]() {
        client2->connectToServer(QUrl("ws://localhost:12345"));
    });

    // 延迟 1000ms 后 client1 发送文本消息
    QTimer::singleShot(1000, [&client1]() {
        client1.sendTextMessage("Hello from Client 1!");
    });

    // 延迟 1500ms 后 client2 发送文本消息
    QTimer::singleShot(1500, [&client2]() {
        client2->sendTextMessage("Hi there, this is Client 2!");
    });

    // 延迟 2000ms 后 client1 发送二进制消息
    QTimer::singleShot(2000, [&client1]() {
        QByteArray binaryData;
        for (int i = 0; i < 256; ++i) {
            binaryData.append(static_cast<char>(i));
        }
        client1.sendBinaryMessage(binaryData);
    });

    // 延迟 2500ms 后 client2 发送一条 JSON 消息
    QTimer::singleShot(2500, [&client2]() {
        QJsonObject json;
        json["type"] = "chat";
        json["from"] = "Client 2";
        json["content"] = "JSON message works!";
        json["timestamp"] = QDateTime::currentDateTime().toMSecsSinceEpoch();
        QByteArray jsonData =
            QJsonDocument(json).toJson(QJsonDocument::Compact);
        client2->sendTextMessage(QString::fromUtf8(jsonData));
    });

    // 延迟 4000ms 后客户端逐个断开
    QTimer::singleShot(4000, [&client1]() {
        qDebug() << "\n[Client1] Disconnecting...";
        client1.disconnectFromServer();
    });

    QTimer::singleShot(4500, [&client2, &server]() {
        qDebug() << "\n[Client2] Disconnecting...";
        client2->disconnectFromServer();
    });

    // 延迟 5000ms 后打印总结并退出
    QTimer::singleShot(5000, [&app, &server, &client1, &client2]() {
        qDebug() << "\n=== Summary ===";
        qDebug() << "Server remaining connections:"
                 << server.clientCount();
        qDebug() << "Client1 received messages:"
                 << client1.messageCount();
        qDebug() << "Client2 received messages:"
                 << client2->messageCount();
        qDebug() << "Demo finished.";
        client2->deleteLater();
        QCoreApplication::quit();
    });

    return app.exec();
}
