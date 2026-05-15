/// @file    main.cpp
/// @brief   Qt 高级线程示例程序入口。
///
/// 对应教程：进阶层 01-QtBase/09-多线程。
/// 依次演示：QThreadPool 任务提交与调优、QtConcurrent 异步执行、
/// QFutureWatcher 监控、QReadWriteLock 读写锁。

#include <QCoreApplication>
#include <QDebug>
#include <QTimer>

#include "concurrent_demo.h"
#include "rwlock_demo.h"
#include "threadpool_tasks.h"

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "========== Qt 高级线程示例 ==========";
    qDebug() << "Qt 版本:" << QT_VERSION_STR;
    qDebug() << "CPU 核心数:" << QThread::idealThreadCount();

    // [演示 1] QRunnable + QThreadPool 任务提交
    qDebug() << "\n[演示 1] QRunnable + QThreadPool 任务提交";
    demoBasicThreadPoolUsage();

    // [演示 2] QThreadPool 线程数调优
    qDebug() << "\n[演示 2] QThreadPool 线程数调优";
    demoThreadPoolTuning();

    // [演示 3] QtConcurrent::run() 异步执行
    qDebug() << "\n[演示 3] QtConcurrent::run() 异步执行";
    demoConcurrentRun();

    // [演示 4] QFutureWatcher 监控异步结果
    qDebug() << "\n[演示 4] QFutureWatcher 监控异步结果";
    demoFutureWatcher();

    // [演示 5] 多任务并发执行
    qDebug() << "\n[演示 5] 多任务并发执行";
    demoMultipleConcurrent();

    // [演示 6] QReadWriteLock 读写锁
    qDebug() << "\n[演示 6] QReadWriteLock 读写锁";
    demoReadWriteLock();

    qDebug() << "\n========== 所有演示执行完毕 ==========";

    // 使用 QTimer::singleShot 延迟退出，确保所有异步操作完成
    QTimer::singleShot(0, &app, &QCoreApplication::quit);

    return app.exec();
}
