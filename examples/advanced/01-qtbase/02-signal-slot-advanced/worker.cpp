/// @file    worker.cpp
/// @brief   Worker 和 SignalHost 类实现。
///
/// 对应教程：进阶层 01-QtBase/02-信号与槽工程级深度剖析。

#include "worker.h"

#include <QDebug>
#include <QThread>

Worker::Worker(QObject* parent)
    : QObject(parent)
{
}

void Worker::doWork()
{
    qDebug() << "  [Worker::doWork] 执行线程:"
             << QThread::currentThread()
             << "(Worker 所在线程:" << this->thread() << ")";
}

SignalHost::SignalHost(QObject* parent)
    : QObject(parent)
{
}
