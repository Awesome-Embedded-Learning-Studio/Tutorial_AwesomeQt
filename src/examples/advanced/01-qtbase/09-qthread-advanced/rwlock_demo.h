/// @file    rwlock_demo.h
/// @brief   演示 QReadWriteLock 读者-写者同步机制。
///
/// 对应教程：进阶层 01-QtBase/09-多线程。
/// QReadWriteLock 允许多个线程同时读取，但写入时互斥，
/// 比 QMutex 更适合读多写少的场景。

#pragma once

#include <QDebug>
#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QReadWriteLock>
#include <QRunnable>
#include <QThreadPool>
#include <QThread>

/// @brief 共享数据集，使用 QReadWriteLock 保护读写一致性。
///
/// 读操作使用 lockForRead()，允许多线程并发读取；
/// 写操作使用 lockForWrite()，确保写入时没有其他读写。
class SharedDataSet
{
public:
    SharedDataSet();

    /// @brief 读取数据（读者操作），多个读者可同时持有读锁。
    /// @param[in] readerName 读者名称，用于日志追踪。
    /// @return 当前共享数据的值。
    int readValue(const QString& readerName);

    /// @brief 写入数据（写者操作），写者独占访问。
    /// @param[in] writerName 写者名称，用于日志追踪。
    /// @param[in] newValue   要写入的新值。
    void writeValue(const QString& writerName, int newValue);

    /// @brief 尝试非阻塞读取，最多等待 50ms。
    /// @param[in]  readerName 读者名称，用于日志追踪。
    /// @param[out] outValue   读取到的值（仅成功时有效）。
    /// @return true 表示成功获取读锁并读取，false 表示超时。
    bool tryReadValue(const QString& readerName, int& outValue);

private:
    QReadWriteLock m_lock;  ///< 读写锁保护下面的共享数据
    int m_value;            ///< 被保护的共享数据
};

/// @brief 读者任务，QRunnable 封装读操作。
class ReaderTask : public QRunnable
{
public:
    /// @brief 构造读者任务。
    /// @param[in] dataSet 共享数据集引用。
    /// @param[in] name    读者名称。
    ReaderTask(SharedDataSet& dataSet, const QString& name);

    void run() override;

private:
    SharedDataSet& m_dataSet;  ///< 共享数据集引用
    QString m_name;            ///< 读者名称
};

/// @brief 写者任务，QRunnable 封装写操作。
class WriterTask : public QRunnable
{
public:
    /// @brief 构造写者任务。
    /// @param[in] dataSet 共享数据集引用。
    /// @param[in] name    写者名称。
    /// @param[in] value   要写入的值。
    WriterTask(SharedDataSet& dataSet, const QString& name, int value);

    void run() override;

private:
    SharedDataSet& m_dataSet;  ///< 共享数据集引用
    QString m_name;            ///< 写者名称
    int m_value;               ///< 要写入的值
};

/// @brief 演示 QReadWriteLock 多读者并发 vs 写者互斥。
///
/// 实验分三阶段：纯读并发、纯写互斥、读写混合，最后演示 tryLock 非阻塞读取。
void demoReadWriteLock();
