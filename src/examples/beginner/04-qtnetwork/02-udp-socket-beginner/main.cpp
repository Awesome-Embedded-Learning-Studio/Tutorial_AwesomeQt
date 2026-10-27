/**
 * Qt UDP Socket 基础示例
 *
 * 本示例演示 QUdpSocket 的核心功能：
 * 1. bind() 绑定端口接收数据报
 * 2. writeDatagram() / readDatagram() 数据报收发
 * 3. QHostAddress::Broadcast 局域网广播
 * 4. UDP 不可靠性的演示（丢包、无序）
 *
 * 核心要点：
 * - UDP 是无连接的数据报协议，每次收发是完整消息
 * - readDatagram 需要先查询 pendingDatagramSize 分配缓冲区
 * - 广播只能在本子网内传播
 */

#include <QCoreApplication>
#include <QTimer>
#include <QDebug>

#include "udpreceiver.h"
#include "udpsender.h"

// ========================================
// 主函数：演示 UDP 单播、广播和数据报收发
// ========================================

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "=== Qt UDP Socket Basic Example ===\n";

    // 使用统一的端口
    const quint16 kPort = 23456;

    // 第一步：创建接收端（绑定端口监听）
    UdpReceiver receiver(kPort);

    // 第二步：创建发送端
    UdpSender sender;

    // 延迟 500ms 后发送单播消息给自己
    QTimer::singleShot(500, [&sender, kPort]() {
        sender.sendUnicast(
            "Hello via UDP unicast!",
            QHostAddress::LocalHost, kPort);
    });

    // 延迟 1500ms 后发送广播消息
    QTimer::singleShot(1500, [&sender, kPort]() {
        sender.sendBroadcast("DISCOVER", kPort);
    });

    // 延迟 2500ms 后发送多条消息（演示数据报边界）
    QTimer::singleShot(2500, [&sender, kPort]() {
        // 连续发送 3 个独立的数据报
        for (int i = 1; i <= 3; ++i) {
            QByteArray msg = QString("Message #%1").arg(i).toUtf8();
            sender.sendUnicast(msg, QHostAddress::LocalHost, kPort);
        }
        qDebug() << "\n[Sender] Sent 3 separate datagrams.";
        qDebug() << "[Sender] Each should arrive as a complete message"
                    "(UDP preserves datagram boundaries).";
    });

    // 延迟 4000ms 后退出
    QTimer::singleShot(4000, [&app]() {
        qDebug() << "\n=== Summary ===";
        qDebug() << "UDP datagrams preserve message boundaries.";
        qDebug() << "Each writeDatagram = one complete readDatagram.";
        qDebug() << "But delivery is NOT guaranteed (no ACK, no retry).";
        qDebug() << "Demo finished.";
        QCoreApplication::quit();
    });

    return app.exec();
}
