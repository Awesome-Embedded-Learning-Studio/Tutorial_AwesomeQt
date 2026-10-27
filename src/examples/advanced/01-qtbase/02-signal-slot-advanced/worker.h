/// @file    worker.h
/// @brief   Worker 和 SignalHost 类声明——用于演示不同 Qt::ConnectionType 的线程行为。
///
/// Worker 槽函数打印当前线程 ID，便于观察它在哪个线程执行。
/// SignalHost 提供信号源，从主线程发射信号到 Worker。
///
/// 对应教程：进阶层 01-QtBase/02-信号与槽工程级深度剖析。

#pragma once

#include <QObject>

/// 工作线程对象，doWork 槽函数打印调用线程 ID。
class Worker : public QObject
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父对象指针。
    explicit Worker(QObject* parent = nullptr);

public slots:
    /// @brief 工作槽：打印调用线程 ID，区分 DirectConnection vs QueuedConnection。
    void doWork();
};

/// 信号源辅助类，从主线程发射信号到工作线程。
class SignalHost : public QObject
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父对象指针。
    explicit SignalHost(QObject* parent = nullptr);

signals:
    /// 主线程发射，Worker 在工作线程接收。
    void triggerWork();
};
