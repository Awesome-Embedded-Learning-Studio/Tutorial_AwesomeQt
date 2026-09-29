/// @file    rwlock_demo.cpp
/// @brief   QReadWriteLock 读写锁演示的实现。
///
/// 对应教程：进阶层 01-QtBase/09-多线程。

#include "rwlock_demo.h"

// ============================================================================
// SharedDataSet
// ============================================================================

SharedDataSet::SharedDataSet() : m_value(0) {}

/// @brief 读取数据（读者操作），使用 lockForRead()，允许多个读者同时持有。
int SharedDataSet::readValue(const QString& readerName)
{
    m_lock.lockForRead();  // 获取读锁（允许多个读者同时持有）

    // 读取数据（此期间其他读者可以进入，但写者必须等待）
    int value = m_value;
    QThread::msleep(10);  // 模拟读取处理时间
    qDebug() << "[读者" << readerName << "] 读取到值:" << value
             << "（线程:" << QThread::currentThreadId() << "）";

    m_lock.unlock();  // 释放读锁
    return value;
}

/// @brief 写入数据（写者操作），使用 lockForWrite()，写者独占访问。
void SharedDataSet::writeValue(const QString& writerName, int newValue)
{
    m_lock.lockForWrite();  // 获取写锁（独占，阻塞所有读写）

    // 写入数据（此期间没有任何其他线程可以读写）
    m_value = newValue;
    QThread::msleep(30);  // 模拟写入处理时间
    qDebug() << "[写者" << writerName << "] 写入值:" << newValue
             << "（线程:" << QThread::currentThreadId() << "）";

    m_lock.unlock();  // 释放写锁
}

/// @brief 尝试非阻塞读取，如果锁不可用则立即返回 false。
bool SharedDataSet::tryReadValue(const QString& readerName, int& outValue)
{
    if (m_lock.tryLockForRead(50)) {  // 最多等待 50ms
        outValue = m_value;
        qDebug() << "[读者" << readerName << "] tryLock 读取:" << outValue;
        m_lock.unlock();
        return true;
    }
    qDebug() << "[读者" << readerName << "] tryLock 获取读锁超时";
    return false;
}

// ============================================================================
// ReaderTask
// ============================================================================

ReaderTask::ReaderTask(SharedDataSet& dataSet, const QString& name)
    : m_dataSet(dataSet), m_name(name)
{
    setAutoDelete(true);
}

void ReaderTask::run()
{
    // 每个读者执行 2 次读取
    for (int i = 0; i < 2; ++i) {
        m_dataSet.readValue(m_name);
        QThread::msleep(QRandomGenerator::global()->bounded(10, 50));
    }
}

// ============================================================================
// WriterTask
// ============================================================================

WriterTask::WriterTask(SharedDataSet& dataSet, const QString& name, int value)
    : m_dataSet(dataSet), m_name(name), m_value(value)
{
    setAutoDelete(true);
}

void WriterTask::run()
{
    m_dataSet.writeValue(m_name, m_value);
}

// ============================================================================
// demoReadWriteLock
// ============================================================================

/// @brief 演示 QReadWriteLock 多读者并发 vs 写者互斥。
void demoReadWriteLock()
{
    qDebug() << "\n--- QReadWriteLock 读写锁演示 ---";

    SharedDataSet dataSet;
    QThreadPool pool;
    pool.setMaxThreadCount(8);  // 足够多的线程支持并发

    QElapsedTimer timer;

    // 阶段一：纯读操作 - 多个读者并发执行
    qDebug() << "\n[读写锁] 阶段一：纯读操作（多读者并发）";
    timer.start();

    // 提交 4 个读者任务，它们可以同时持有读锁
    pool.start(new ReaderTask(dataSet, "R1"));
    pool.start(new ReaderTask(dataSet, "R2"));
    pool.start(new ReaderTask(dataSet, "R3"));
    pool.start(new ReaderTask(dataSet, "R4"));

    pool.waitForDone();
    qDebug() << "[读写锁] 纯读阶段耗时:" << timer.elapsed() << "ms";
    qDebug() << "[读写锁] 注意：4 个读者几乎同时执行，因为是并发读取";

    // 阶段二：纯写操作 - 写者之间互斥
    qDebug() << "\n[读写锁] 阶段二：纯写操作（写者互斥）";
    timer.restart();

    pool.start(new WriterTask(dataSet, "W1", 100));
    pool.start(new WriterTask(dataSet, "W2", 200));
    pool.start(new WriterTask(dataSet, "W3", 300));

    pool.waitForDone();
    qDebug() << "[读写锁] 纯写阶段耗时:" << timer.elapsed() << "ms";
    qDebug() << "[读写锁] 注意：写者必须串行执行，因为写锁是独占的";

    // 阶段三：读写混合 - 写者独占，读者等待
    qDebug() << "\n[读写锁] 阶段三：读写混合";
    timer.restart();

    // 先提交读者，再提交写者，观察写者的到来会让读者等待
    pool.start(new ReaderTask(dataSet, "R5"));
    pool.start(new WriterTask(dataSet, "W4", 999));
    pool.start(new ReaderTask(dataSet, "R6"));

    pool.waitForDone();
    qDebug() << "[读写锁] 读写混合阶段耗时:" << timer.elapsed() << "ms";
    qDebug() << "[读写锁] 注意：写者执行时读者被阻塞，写完后读者才能继续";

    // 演示 tryLockForRead 非阻塞读取
    qDebug() << "\n[读写锁] tryLock 非阻塞读取演示";
    int value = 0;
    if (dataSet.tryReadValue("tryR1", value)) {
        qDebug() << "[读写锁] tryLock 成功，读取值:" << value;
    }
}
