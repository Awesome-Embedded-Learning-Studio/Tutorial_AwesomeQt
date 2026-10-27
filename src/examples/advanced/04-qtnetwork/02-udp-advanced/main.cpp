/// @file    main.cpp
/// @brief   UDP 组播分片传输示例的入口程序。
///
/// @details 对应教程：进阶层 04-QtNetwork/02-UDP 高级用法。
///          创建发送端和接收端两个 MulticastManager 实例，接收端绑定组播端口，
///          发送端绑定另一端口但向组播端口发送数据。发送端将一条 3000 字节的消息
///          分片后通过组播发送，接收端收齐后重组并打印对比结果，演示完毕后自动退出。

#include "fragment_handler.h"
#include "multicast_manager.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>

/// @brief 组播组地址，属于本地网络管理范围（239.0.0.0/8）。
static const QHostAddress kMulticastGroup("239.255.43.21");

/// @brief 接收端绑定的端口，也是组播发送的目标端口。
static constexpr quint16 kMulticastPort = 45454;

/// @brief 发送端本地绑定的端口（仅用于出站，不接收组播数据）。
static constexpr quint16 kSenderBindPort = 45455;

/// @brief 演示用的大数据块大小（字节）。
static constexpr int kDemoDataSize = 3000;

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    // --- 接收端 ---
    // bindPort == multicastPort：接收端绑定组播端口以接收组播数据
    MulticastManager receiver(kMulticastPort, kMulticastPort);
    receiver.joinGroup(kMulticastGroup);

    FragmentHandler reassembler;

    // 接收端收到组播数据报后，逐片送入重组器
    QObject::connect(&receiver, &MulticastManager::messageReceived,
                     &reassembler, [&reassembler](const QHostAddress& /*sender*/,
                                                  const QByteArray& data) {
                         reassembler.processFragment(data);
                     });

    // 重组完成后打印对比结果
    QObject::connect(&reassembler, &FragmentHandler::messageAssembled,
                     [&app](const QByteArray& completeData) {
                         qDebug() << "=== Reassembly complete ===";
                         qDebug() << "Reassembled size:" << completeData.size() << "bytes";

                         // 验证内容：发送端用 "A" ~ "Z" 循环填充，逐字节校验
                         bool match = true;
                         const char pattern[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
                         const int patternLen = 26;
                         for (int i = 0; i < completeData.size(); ++i) {
                             const char expected = pattern[i % patternLen];
                             if (completeData.at(i) != expected) {
                                 match = false;
                                 qDebug() << "Mismatch at byte" << i
                                          << ": expected" << QChar(expected)
                                          << "got" << QChar(completeData.at(i));
                                 break;
                             }
                         }

                         if (match) {
                             qDebug() << "Data integrity check: PASSED";
                         } else {
                             qDebug() << "Data integrity check: FAILED";
                         }

                         // 演示结束，延迟退出让事件循环处理完剩余事件
                         QTimer::singleShot(100, &app, &QCoreApplication::quit);
                     });

    // --- 发送端 ---
    // bindPort != multicastPort：发送端绑定不同端口，但向组播端口发送
    MulticastManager sender(kSenderBindPort, kMulticastPort);
    sender.joinGroup(kMulticastGroup);

    // 构造 3000 字节的演示数据，用 A~Z 循环填充便于验证
    QByteArray originalData(kDemoDataSize, Qt::Initialization::Uninitialized);
    const char pattern[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const int patternLen = 26;
    for (int i = 0; i < kDemoDataSize; ++i) {
        originalData[i] = pattern[i % patternLen];
    }

    qDebug() << "=== Fragmentation + Multicast Demo ===";
    qDebug() << "Original data size:" << originalData.size() << "bytes";

    // 将原始数据分片
    FragmentHandler fragmenter;
    QVector<QByteArray> fragments = fragmenter.fragment(originalData);

    qDebug() << "Fragment count:" << fragments.size();

    // 延迟 500ms 后逐片发送，给接收端留出就绪时间
    // @note 使用QTimer逐片发送，避免瞬间发出导致本地缓冲区丢包
    int fragmentIndex = 0;
    QTimer* sendTimer = new QTimer(&app);
    sendTimer->setInterval(50);    // 每片间隔 50ms

    QObject::connect(sendTimer, &QTimer::timeout, sendTimer,
                     [&sender, &fragments, &fragmentIndex, sendTimer]() {
                         if (fragmentIndex < fragments.size()) {
                             qint64 written = sender.sendMessage(fragments[fragmentIndex]);
                             qDebug() << "Sent fragment" << fragmentIndex
                                      << "-" << written << "bytes";
                             ++fragmentIndex;
                         } else {
                             sendTimer->stop();
                             qDebug() << "All fragments sent.";
                         }
                     });

    // 延迟 500ms 启动发送，让 receiver 完成组播组加入
    QTimer::singleShot(500, sendTimer, static_cast<void (QTimer::*)()>(&QTimer::start));

    return app.exec();
}
