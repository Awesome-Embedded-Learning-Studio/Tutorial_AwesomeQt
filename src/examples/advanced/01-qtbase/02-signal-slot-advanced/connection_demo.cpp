/// @file    connection_demo.cpp
/// @brief   Qt::ConnectionType 演示函数实现。
///
/// 对应教程：进阶层 01-QtBase/02-信号与槽工程级深度剖析。

#include "connection_demo.h"
#include "worker.h"

#include <QDebug>
#include <QThread>

void demoDirectConnection()
{
    qDebug() << "--- DirectConnection ---";
    qDebug() << "主线程:" << QThread::currentThread();

    QThread workerThread;
    Worker worker;
    worker.moveToThread(&workerThread);

    SignalHost host;

    // DirectConnection：无视线程亲和性，槽在信号发射的线程同步执行
    QObject::connect(&host, &SignalHost::triggerWork,
                     &worker, &Worker::doWork,
                     Qt::DirectConnection);

    workerThread.start();
    QThread::msleep(50);

    emit host.triggerWork();
    qDebug() << "  注意: doWork 在主线程执行，而非 Worker 所在的工作线程";

    workerThread.quit();
    workerThread.wait();
    qDebug() << "";
}

void demoQueuedConnection()
{
    qDebug() << "--- QueuedConnection ---";
    qDebug() << "主线程:" << QThread::currentThread();

    QThread workerThread;
    Worker worker;
    worker.moveToThread(&workerThread);

    SignalHost host;

    // QueuedConnection：信号发射后立刻返回，槽在接收者线程异步执行
    // 要求接收者线程的事件循环必须正在运行
    QObject::connect(&host, &SignalHost::triggerWork,
                     &worker, &Worker::doWork,
                     Qt::QueuedConnection);

    workerThread.start();
    QThread::msleep(50);

    emit host.triggerWork();
    qDebug() << "  信号已发射（主线程不阻塞），等待 Worker 线程处理...";

    QThread::msleep(200);

    workerThread.quit();
    workerThread.wait();
    qDebug() << "";
}

void demoBlockingQueuedConnection()
{
    qDebug() << "--- BlockingQueuedConnection ---";
    qDebug() << "主线程:" << QThread::currentThread();

    QThread workerThread;
    Worker worker;
    worker.moveToThread(&workerThread);

    SignalHost host;

    // BlockingQueuedConnection：信号发射后阻塞发送者线程，
    // 直到接收者线程执行完槽函数
    QObject::connect(&host, &SignalHost::triggerWork,
                     &worker, &Worker::doWork,
                     Qt::BlockingQueuedConnection);

    workerThread.start();
    QThread::msleep(50);

    qDebug() << "  发射信号（主线程将阻塞直到槽执行完毕）...";
    emit host.triggerWork();
    qDebug() << "  槽已执行完毕，主线程解除阻塞";

    workerThread.quit();
    workerThread.wait();
    qDebug() << "";
}
