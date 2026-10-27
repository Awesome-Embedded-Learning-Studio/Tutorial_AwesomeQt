/// @file    threadpool_tasks.h
/// @brief   定义 QRunnable 子类，演示 QThreadPool 的任务提交机制。
///
/// 对应教程：进阶层 01-QtBase/09-多线程。
/// QRunnable 是 Qt 线程池的任务单元，通过重写 run() 定义任务逻辑，
/// 然后提交到 QThreadPool 执行，避免频繁创建/销毁线程的开销。

#pragma once

#include <QDebug>
#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QRunnable>
#include <QThreadPool>
#include <QThread>

/// @brief 计算密集型任务，模拟 CPU 密集型运算。
///
/// setAutoDelete(true) 使得任务执行完毕后由线程池自动释放，
/// 无需手动管理内存，适合大量一次性任务。
class ComputeTask : public QRunnable
{
public:
    /// @brief 构造计算任务。
    /// @param[in] taskId     任务编号，用于日志追踪。
    /// @param[in] iterations 迭代次数，控制计算量。
    explicit ComputeTask(int taskId, int iterations = 1000000);

    void run() override;

private:
    int m_taskId;      ///< 任务编号
    int m_iterations;  ///< 迭代次数
};

/// @brief 数据处理任务，模拟 IO 或数据处理操作。
///
/// 带有随机延迟以模拟真实场景中任务耗时的不确定性，
/// 例如批量文件转换、数据库查询、网络请求处理等。
class DataProcessingTask : public QRunnable
{
public:
    /// @brief 构造数据处理任务。
    /// @param[in] taskId   任务编号。
    /// @param[in] dataName 数据集名称。
    explicit DataProcessingTask(int taskId, const QString& dataName);

    void run() override;

private:
    int m_taskId;           ///< 任务编号
    QString m_dataName;     ///< 数据集名称
};

/// @brief 演示 QThreadPool 基础用法：提交多个任务并等待完成。
///
/// QThreadPool::start() 将任务提交到队列，线程池自动分配给空闲线程执行。
void demoBasicThreadPoolUsage();

/// @brief 演示 QThreadPool 线程数调优。
///
/// 计算密集型任务设为 CPU 核心数即可，IO 密集型可适当增大以提高吞吐量。
void demoThreadPoolTuning();
