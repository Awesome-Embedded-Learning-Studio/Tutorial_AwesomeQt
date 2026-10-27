/**
 * Qt TCP Socket 基础示例
 *
 * 本示例演示 TCP 服务端与客户端的完整通信流程：
 * 1. QTcpServer 监听端口并接受连接
 * 2. QTcpSocket 客户端连接服务器
 * 3. 双向数据收发（write / readyRead）
 * 4. disconnected 信号处理断线检测
 *
 * 核心要点：
 * - TCP 是面向连接的字节流协议
 * - Qt 网络模块全部基于信号槽的异步模型
 * - 服务端每个连接对应一个独立的 QTcpSocket
 */

#include <QCoreApplication>
#include <QTimer>
#include <QDebug>

#include "echoserver.h"
#include "tcpclient.h"

// ========================================
// 主函数：演示服务端 + 客户端通信
// ========================================

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "=== Qt TCP Socket Basic Example ===\n";

    // 第一步：启动回声服务器
    EchoServer server;

    // 第二步：创建客户端并连接
    TcpClient client;
    client.connectToServer(QHostAddress::LocalHost, 12345);

    // 第三步：模拟多轮通信
    // 延迟 500ms 后发送第二条消息（等连接建立完成）
    QTimer::singleShot(500, [&client]() {
        client.sendMessage("This is the second message.");
    });

    // 延迟 1000ms 后发送第三条消息
    QTimer::singleShot(1000, [&client]() {
        client.sendMessage("Third message coming through.");
    });

    // 延迟 2000ms 后客户端主动断开
    QTimer::singleShot(2000, [&client]() {
        qDebug() << "\n[Client] Disconnecting gracefully...";
        client.disconnectFromServer();
    });

    // 延迟 3000ms 后退出事件循环
    QTimer::singleShot(3000, [&app, &server]() {
        qDebug() << "\n=== Summary ===";
        qDebug() << "Server handled connections:"
                 << server.clientCount() << "active";
        qDebug() << "Demo finished.";
        QCoreApplication::quit();
    });

    return app.exec();
}
