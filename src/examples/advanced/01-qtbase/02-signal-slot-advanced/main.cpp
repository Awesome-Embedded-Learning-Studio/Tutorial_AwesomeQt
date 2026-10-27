/// @file    main.cpp
/// @brief   信号与槽高级用法演示程序入口。
///
/// 演示 DirectConnection / QueuedConnection / BlockingQueuedConnection
/// 的线程行为差异、Lambda 捕获陷阱、QMetaObject::Connection 手动断开、
/// UniqueConnection 防止重复连接。
///
/// 对应教程：进阶层 01-QtBase/02-信号与槽工程级深度剖析。

#include "connection_demo.h"
#include "lambda_trap.h"

#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QTimer>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    // 演示 1: DirectConnection vs QueuedConnection
    qDebug() << "\n[演示 1] DirectConnection vs QueuedConnection";
    qDebug() << "----------------------------------------------";

    demoDirectConnection();
    demoQueuedConnection();

    // 演示 2: BlockingQueuedConnection 跨线程同步
    qDebug() << "\n[演示 2] BlockingQueuedConnection 跨线程同步";
    qDebug() << "----------------------------------------------";
    qDebug() << "注意: BlockingQueuedConnection 要求 sender 和 receiver";
    qDebug() << "      在不同线程，否则会死锁（无超时、无错误提示）!";
    qDebug() << "";

    demoBlockingQueuedConnection();

    // 演示 3: Lambda 捕获陷阱——裸指针 vs QPointer
    qDebug() << "\n[演示 3] Lambda 捕获陷阱";
    qDebug() << "----------------------------------------------";

    demoRawPointerTrap();
    demoQPointerSafeCapture();

    // 演示 4: QMetaObject::Connection 手动断开连接
    qDebug() << "\n[演示 4] QMetaObject::Connection 手动断开";
    qDebug() << "----------------------------------------------";

    {
        QTimer timer;
        int count = 0;

        // connect 返回 QMetaObject::Connection 对象
        QMetaObject::Connection conn = QObject::connect(
            &timer, &QTimer::timeout,
            [&count]() {
                count++;
                qDebug() << "  槽函数被调用，count:" << count;
            }
        );

        qDebug() << "连接是否有效:" << static_cast<bool>(conn);

        timer.start(50);

        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        qDebug() << "两次触发后 count:" << count;

        // 手动断开这条特定连接
        QObject::disconnect(conn);
        qDebug() << "手动 disconnect 后，连接是否有效:"
                 << static_cast<bool>(conn);

        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        qDebug() << "断开后 count 仍然是:" << count;

        timer.stop();
    }
    qDebug() << "";

    // 演示 5: Qt::UniqueConnection 防止重复连接
    qDebug() << "\n[演示 5] Qt::UniqueConnection 防止重复连接";
    qDebug() << "----------------------------------------------";

    {
        QTimer timer;
        int count = 0;

        // 普通连接：connect 两次，槽被调用两次
        QObject::connect(&timer, &QTimer::timeout, [&count]() {
            count++;
        });
        QObject::connect(&timer, &QTimer::timeout, [&count]() {
            count++;
        });

        timer.start(50);
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        timer.stop();
        qDebug() << "普通连接 connect 两次，一次触发后 count:" << count
                 << "(每次触发调用 2 次)";

        // UniqueConnection：已存在相同连接则跳过
        count = 0;
        QObject::connect(&timer, &QTimer::timeout, &timer, [&count]() {
            count++;
        }, Qt::UniqueConnection);
        QObject::connect(&timer, &QTimer::timeout, &timer, [&count]() {
            count++;
        }, Qt::UniqueConnection);

        timer.start(50);
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        timer.stop();
        qDebug() << "UniqueConnection connect 两次，一次触发后 count:" << count
                 << "(只建立了 1 条连接)";
    }
    qDebug() << "";

    // 总结
    qDebug() << "========================================";
    qDebug() << "信号槽高级用法演示结束";
    qDebug() << "要点回顾:";
    qDebug() << "  1. DirectConnection 跨线程操作 GUI 可能崩溃";
    qDebug() << "  2. QueuedConnection 需要事件循环运行";
    qDebug() << "  3. BlockingQueuedConnection 同线程会死锁";
    qDebug() << "  4. Lambda 捕获裸指针是定时炸弹";
    qDebug() << "  5. QMetaObject::Connection 可精确断开";
    qDebug() << "  6. UniqueConnection 防止重复连接";
    qDebug() << "========================================";

    return 0;
}
