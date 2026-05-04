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

#include <QAbstractSocket>
#include <QWebSocket>

/// @brief 向所有已连接的客户端广播文本消息
/// @param clients 当前在线的客户端列表
/// @param message 要广播的文本消息
void broadcast_message(const QList<QWebSocket*> &clients,
                        const QString &message)
{
    for (QWebSocket* client : clients) {
        // 跳过已断开的连接（disconnected 信号可能有延迟）
        if (client->state() == QAbstractSocket::ConnectedState) {
            client->sendTextMessage(message);
        }
    }
}
