/**
 * QtWebSockets 聊天室服务端示例
 *
 * 本示例演示 QtWebSockets 模块的核心功能：
 * - QWebSocketServer 监听端口与接受连接
 * - 管理多个 QWebSocket 客户端连接
 * - 广播消息给所有连接的客户端
 * - 客户端连接/断开生命周期管理
 *
 * 启动后监听 12345 端口（ws://），提供聊天室功能：
 *   - 新客户端连接时广播加入通知
 *   - 收到消息后广播给所有在线客户端
 *   - 客户端断开时广播离开通知
 *
 * 测试方法：
 *   在浏览器控制台中执行：
 *     const ws = new WebSocket('ws://localhost:12345');
 *     ws.onmessage = (e) => console.log(e.data);
 *     ws.send('Hello from browser!');
 */

#include "chatserver.h"

#include <QCoreApplication>
#include <QDebug>
#include <QHostAddress>
#include <QList>
#include <QWebSocket>
#include <QWebSocketServer>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "QtWebSockets 聊天室服务端示例";
    qDebug() << "本示例演示 QWebSocketServer + 多客户端管理 + 广播消息";

    // 构造 WebSocket 服务器（非加密模式）
    QWebSocketServer server(
        QStringLiteral("ChatServer"), QWebSocketServer::NonSecureMode);

    // 监听所有网卡的 12345 端口
    if (!server.listen(QHostAddress::Any, 12345)) {
        qCritical() << "监听失败:" << server.errorString();
        return -1;
    }

    // 在线客户端列表
    QList<QWebSocket*> clients;

    // 处理新客户端连接
    QObject::connect(&server, &QWebSocketServer::newConnection, [&]() {
        // 获取已握手的 WebSocket 连接（所有权转移给调用者）
        QWebSocket* client = server.nextPendingConnection();
        clients.append(client);

        QString addr = client->peerAddress().toString();
        quint16 port = client->peerPort();
        qDebug() << "客户端连接:" << addr << "port:" << port
                 << "当前在线:" << clients.size();

        // 广播加入通知给所有客户端
        broadcast_message(clients,
            QStringLiteral("[系统] 新用户加入 (%1:%2)，当前在线: %3 人")
                .arg(addr)
                .arg(port)
                .arg(clients.size()));

        // 接收该客户端的文本消息并广播
        QObject::connect(client, &QWebSocket::textMessageReceived,
            [client, &clients](const QString &message) {
                QString addr = client->peerAddress().toString();
                quint16 port = client->peerPort();

                // 格式化消息：[IP:端口] 内容
                QString formatted = QStringLiteral("[%1:%2] %3")
                                        .arg(addr)
                                        .arg(port)
                                        .arg(message);

                qDebug() << "消息:" << formatted.left(80);
                broadcast_message(clients, formatted);
            });

        // 处理客户端断开连接
        QObject::connect(client, &QWebSocket::disconnected,
            [client, &clients]() {
                QString addr = client->peerAddress().toString();
                quint16 port = client->peerPort();

                // 从列表中移除并安全销毁
                clients.removeOne(client);
                client->deleteLater();

                qDebug() << "客户端断开:" << addr << "port:" << port
                         << "剩余在线:" << clients.size();

                // 广播离开通知
                broadcast_message(clients,
                    QStringLiteral("[系统] 用户离开 (%1:%2)，当前在线: %3 人")
                        .arg(addr)
                        .arg(port)
                        .arg(clients.size()));
            });
    });

    qDebug() << "WebSocket 聊天室服务已启动";
    qDebug() << "监听地址: ws://0.0.0.0:12345";
    qDebug() << "";
    qDebug() << "测试方法:";
    qDebug() << "  在浏览器控制台中执行:";
    qDebug() << "    const ws = new WebSocket('ws://localhost:12345');";
    qDebug() << "    ws.onmessage = (e) => console.log(e.data);";
    qDebug() << "    ws.send('Hello from browser!');";

    return app.exec();
}
