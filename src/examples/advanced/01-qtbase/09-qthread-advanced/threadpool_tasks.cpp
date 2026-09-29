/// @file    threadpool_tasks.cpp
/// @brief   QThreadPool 任务提交与调优演示的实现。
///
/// 对应教程：进阶层 01-QtBase/09-多线程。

#include "threadpool_tasks.h"

// ============================================================================
// ComputeTask
// ============================================================================

ComputeTask::ComputeTask(int taskId, int iterations)
    : m_taskId(taskId), m_iterations(iterations)
{
    // 设置自动删除：任务执行完毕后由线程池自动 delete
    setAutoDelete(true);
}

void ComputeTask::run()
{
    qDebug() << "[ComputeTask] 任务" << m_taskId
             << "在线程" << QThread::currentThreadId() << "启动";

    // 模拟计算密集型工作：累加运算
    double result = 0.0;
    for (int i = 0; i < m_iterations; ++i) {
        result += static_cast<double>(i) * 0.0001;
    }

    qDebug() << "[ComputeTask] 任务" << m_taskId
             << "完成，计算结果:" << result;
}

// ============================================================================
// DataProcessingTask
// ============================================================================

DataProcessingTask::DataProcessingTask(int taskId, const QString& dataName)
    : m_taskId(taskId), m_dataName(dataName)
{
    setAutoDelete(true);
}

void DataProcessingTask::run()
{
    qDebug() << "[DataProcessingTask] 数据处理任务" << m_taskId
             << "（" << m_dataName << "）"
             << "在线程" << QThread::currentThreadId() << "启动";

    // 模拟数据处理延迟（50~200ms 随机）
    int processingTime = QRandomGenerator::global()->bounded(50, 200);
    QThread::msleep(processingTime);

    qDebug() << "[DataProcessingTask] 数据处理任务" << m_taskId
             << "（" << m_dataName << "）"
             << "完成，耗时" << processingTime << "ms";
}

// ============================================================================
// demoBasicThreadPoolUsage
// ============================================================================

/// @brief 演示 QThreadPool 基础用法。
void demoBasicThreadPoolUsage()
{
    qDebug() << "\n--- QThreadPool 基础用法演示 ---";

    QThreadPool* pool = QThreadPool::globalInstance();
    qDebug() << "[线程池] 最大线程数:" << pool->maxThreadCount();
    qDebug() << "[线程池] 活跃线程数:" << pool->activeThreadCount();

    // 提交 8 个计算任务到全局线程池
    qDebug() << "[线程池] 提交 8 个计算任务...";
    for (int i = 0; i < 8; ++i) {
        pool->start(new ComputeTask(i + 1));
    }

    // waitForDone() 阻塞当前线程直到所有任务执行完毕
    pool->waitForDone();
    qDebug() << "[线程池] 所有计算任务已完成";
}

// ============================================================================
// demoThreadPoolTuning
// ============================================================================

/// @brief 演示 QThreadPool 线程数调优。
void demoThreadPoolTuning()
{
    qDebug() << "\n--- QThreadPool 线程数调优演示 ---";

    QThreadPool pool;  // 创建独立线程池（非全局）

    qDebug() << "[调优] CPU 核心数:" << QThread::idealThreadCount();
    qDebug() << "[调优] 默认最大线程数:" << pool.maxThreadCount();

    // 场景一：限制为 2 个线程，多余任务排队等待
    pool.setMaxThreadCount(2);
    qDebug() << "[调优] 调整最大线程数为 2";

    QElapsedTimer timer;
    timer.start();

    for (int i = 0; i < 6; ++i) {
        pool.start(new DataProcessingTask(i + 1, QString("数据集-%1").arg(i + 1)));
    }

    pool.waitForDone();
    qDebug() << "[调优] 2 线程模式下总耗时:" << timer.elapsed() << "ms";

    // 场景二：增加为 4 个线程，提高并行度
    pool.setMaxThreadCount(4);
    qDebug() << "[调优] 调整最大线程数为 4";

    timer.restart();
    for (int i = 0; i < 6; ++i) {
        pool.start(new DataProcessingTask(i + 7, QString("数据集-%1").arg(i + 7)));
    }

    pool.waitForDone();
    qDebug() << "[调优] 4 线程模式下总耗时:" << timer.elapsed() << "ms";
    qDebug() << "[调优] 更多线程通常意味着更快的并行处理（IO 密集型场景）";
}
