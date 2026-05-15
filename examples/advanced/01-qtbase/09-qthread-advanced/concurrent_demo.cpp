/// @file    concurrent_demo.cpp
/// @brief   QtConcurrent::run() 与 QFutureWatcher 演示的实现。
///
/// 对应教程：进阶层 01-QtBase/09-多线程。

#include "concurrent_demo.h"

#include <QDebug>
#include <QThread>

/// @brief 演示 QtConcurrent::run() 基础异步执行。
///
/// 展示三种用法：无返回值任务、带返回值任务、带参数任务。
/// QFuture::result() 会阻塞当前线程直到异步结果就绪。
void demoConcurrentRun()
{
    qDebug() << "\n--- QtConcurrent::run() 异步执行演示 ---";

    // 示例 1：无返回值的异步任务
    qDebug() << "[Concurrent] 提交无返回值异步任务...";
    QFuture<void> future1 = QtConcurrent::run([]() {
        qDebug() << "[Concurrent] 无返回值任务在线程"
                 << QThread::currentThreadId() << "执行";
        QThread::msleep(100);
        qDebug() << "[Concurrent] 无返回值任务完成";
    });

    // 等待完成
    future1.waitForFinished();
    qDebug() << "[Concurrent] 无返回值任务已结束";

    // 示例 2：带返回值的异步任务
    qDebug() << "\n[Concurrent] 提交带返回值异步任务...";
    QFuture<int> future2 = QtConcurrent::run([]() -> int {
        qDebug() << "[Concurrent] 计算任务在线程"
                 << QThread::currentThreadId() << "执行";
        QThread::msleep(150);
        // 模拟复杂计算
        int result = 0;
        for (int i = 1; i <= 100; ++i) {
            result += i;
        }
        return result;  // 返回计算结果 5050
    });

    // 在等待期间可以检查状态
    qDebug() << "[Concurrent] 任务是否正在运行:" << future2.isRunning();
    qDebug() << "[Concurrent] 等待结果...";

    // result() 会阻塞直到结果就绪
    int result = future2.result();
    qDebug() << "[Concurrent] 异步计算结果:" << result
             << "（预期 5050）";

    // 示例 3：带参数的异步任务
    qDebug() << "\n[Concurrent] 提交带参数的异步任务...";
    auto multiplyFunc = [](int a, int b) -> QString {
        QThread::msleep(50);
        return QString("%1 * %2 = %3").arg(a).arg(b).arg(a * b);
    };

    QFuture<QString> future3 = QtConcurrent::run(multiplyFunc, 42, 7);
    future3.waitForFinished();
    qDebug() << "[Concurrent] 乘法结果:" << future3.result();
}

/// @brief 演示 QFutureWatcher 监控异步结果完成。
///
/// QFutureWatcher 将 QFuture 状态变化转为 Qt 信号（finished、resultReadyAt），
/// 配合局部事件循环实现非阻塞等待，避免冻结 GUI。
void demoFutureWatcher()
{
    qDebug() << "\n--- QFutureWatcher 异步监控演示 ---";

    QEventLoop loop;  // 局部事件循环，用于等待异步结果

    // 创建 QFutureWatcher 监控异步任务
    QFutureWatcher<QString> watcher;

    // 连接 finished 信号：任务完成时触发
    QObject::connect(&watcher, &QFutureWatcher<QString>::finished, [&]() {
        qDebug() << "[FutureWatcher] 收到 finished 信号";

        // 获取所有结果
        QList<QString> results = watcher.future().results();
        for (int i = 0; i < results.size(); ++i) {
            qDebug() << "[FutureWatcher] 结果" << (i + 1) << ":" << results[i];
        }

        loop.quit();  // 退出局部事件循环
    });

    // 连接 resultReadyAt 信号：每个结果就绪时逐个触发
    QObject::connect(&watcher, &QFutureWatcher<QString>::resultReadyAt,
        [](int index) {
            qDebug() << "[FutureWatcher] resultReadyAt 信号，索引:" << index;
        });

    // 提交异步计算任务
    qDebug() << "[FutureWatcher] 提交多个异步计算...";

    QFuture<QString> future = QtConcurrent::run([]() -> QString {
        QThread::msleep(200);
        return QString("异步计算完成，线程 ID: %1")
            .arg(reinterpret_cast<quintptr>(QThread::currentThreadId()));
    });

    watcher.setFuture(future);

    qDebug() << "[FutureWatcher] 进入事件循环，等待异步完成...";
    loop.exec();  // 进入事件循环，等待 finished 信号
    qDebug() << "[FutureWatcher] 监控完成";
}

/// @brief 演示多个并发任务的同时执行与等待。
///
/// 同时提交三个耗时任务，并行执行后逐个等待结果。
/// 总耗时接近最慢的单个任务，而非串行累加。
void demoMultipleConcurrent()
{
    qDebug() << "\n--- 多任务并发执行演示 ---";

    QElapsedTimer timer;
    timer.start();

    // 同时提交 3 个耗时任务
    QFuture<int> f1 = QtConcurrent::run([]() -> int {
        QThread::msleep(200);
        qDebug() << "[多任务] 任务 1 完成";
        return 100;
    });

    QFuture<int> f2 = QtConcurrent::run([]() -> int {
        QThread::msleep(150);
        qDebug() << "[多任务] 任务 2 完成";
        return 200;
    });

    QFuture<int> f3 = QtConcurrent::run([]() -> int {
        QThread::msleep(100);
        qDebug() << "[多任务] 任务 3 完成";
        return 300;
    });

    // 逐个等待结果（并行执行，总耗时接近最慢的那个）
    f1.waitForFinished();
    f2.waitForFinished();
    f3.waitForFinished();

    qint64 elapsed = timer.elapsed();
    qDebug() << "[多任务] 三个任务的结果:" << f1.result() << "+" << f2.result()
             << "+" << f3.result() << "=" << (f1.result() + f2.result() + f3.result());
    qDebug() << "[多任务] 并行总耗时:" << elapsed << "ms（串行需要 ~450ms）";
}
