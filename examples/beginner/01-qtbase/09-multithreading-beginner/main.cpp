/**
 * Qt 多线程基础示例
 *
 * 本示例演示 Qt 中四种多线程使用方式：
 * 1. QThread + moveToThread（推荐方式）
 * 2. QThreadPool 线程池
 * 3. QtConcurrent::run 简单异步任务
 * 4. QFuture + QFutureWatcher 异步结果监听
 *
 * 核心要点：
 * - GUI 操作必须在主线程
 * - 跨线程通信用信号槽
 * - 不要在主线程阻塞等待异步结果
 */

#include <QCoreApplication>      // 应用程序核心类
#include <QThread>               // 线程类
#include <QThreadPool>           // 线程池类
#include <QtConcurrent>          // 并发算法
#include <QtConcurrentRun>       // QtConcurrent::run 函数
#include <QFuture>               // 异步结果
#include <QFutureWatcher>        // 异步结果监听器
#include <QDebug>                // 调试输出
#include <QMutex>                // 互斥锁
#include <QMutexLocker>          // RAII 风格锁管理器
#include <atomic>                // 原子类型

// ========================================
// 示例 1: Worker + moveToThread（推荐方式）
// ========================================

/**
 * Worker 类：在后台线程执行耗时任务
 *
 * 设计要点：
 * - 继承 QObject（不是 QThread）
 * - 用信号槽进行跨线程通信
 * - 任务完成后发送信号通知主线程
 */
class Worker : public QObject
{
    Q_OBJECT  // 启用 Qt 信号槽机制

public:
    explicit Worker(QObject *parent = nullptr) : QObject(parent) {}

    /**
     * 执行耗时任务的槽函数
     * 这个函数会在 moveToThread 后的新线程中执行
     */
public slots:
    void doWork(const QString &taskName) {
        qDebug() << "[" << QThread::currentThreadId() << "] Worker开始任务:" << taskName;

        // 模拟耗时操作（5秒）
        for (int i = 1; i <= 5; ++i) {
            QThread::sleep(1);  // 休眠1秒

            // 发送进度信号（跨线程，线程安全）
            emit progressChanged(taskName, i * 20);
        }

        // 任务完成，发送结果信号
        emit workFinished(taskName, "任务完成！");
    }

signals:
    /**
     * 进度变化信号
     * 跨线程发送时，Qt 会自动使用队列连接，保证线程安全
     */
    void progressChanged(const QString &taskName, int percent);

    /**
     * 任务完成信号
     * 可以携带结果数据，Qt 会自动拷贝参数到事件队列
     */
    void workFinished(const QString &taskName, const QString &result);
};

/**
 * 演示 QThread + moveToThread 用法
 *
 * 这是 Qt 中使用线程的推荐方式：
 * 1. 创建 Worker 对象（QObject）
 * 2. 创建 QThread 对象
 * 3. 将 Worker 移动到新线程（moveToThread）
 * 4. 通过信号槽启动任务和获取结果
 */
void demoMoveToThread()
{
    qDebug() << "\n=== 示例 1: QThread + moveToThread ===";

    QThread *thread = new QThread;
    Worker *worker = new Worker;

    // 将 worker 移动到新线程
    // 关键操作：之后 worker 的槽函数都会在新线程执行
    worker->moveToThread(thread);

    // 连接信号：线程启动时开始工作
    QObject::connect(thread, &QThread::started, [worker]() {
        worker->doWork("后台任务1");
    });

    // 连接信号：监听进度（主线程接收，安全更新 GUI）
    QObject::connect(worker, &Worker::progressChanged, [](const QString &name, int percent) {
        qDebug() << "[" << QThread::currentThreadId() << "] 主线程收到进度:" << name << percent << "%";
    });

    // 连接信号：任务完成后退出线程
    QObject::connect(worker, &Worker::workFinished, [thread](const QString &name, const QString &result) {
        qDebug() << "[" << QThread::currentThreadId() << "] 主线程收到结果:" << name << result;

        // 退出线程的事件循环
        thread->quit();
    });

    // 连接信号：线程结束后清理资源
    QObject::connect(thread, &QThread::finished, [thread, worker]() {
        qDebug() << "线程结束，清理资源";
        thread->deleteLater();  // 延迟删除（安全删除 QObject）
        worker->deleteLater();
    });

    // 启动线程
    thread->start();

    // 等待线程完成（仅用于示例，实际应用中不要阻塞主线程）
    thread->wait();
    qDebug() << "示例 1 完成\n";
}

// ========================================
// 示例 2: QThreadPool + QRunnable
// ========================================

/**
 * Runnable 任务类
 *
 * 用于 QThreadPool 执行短期任务
 * 注意：QRunnable 不是 QObject，不能发送信号
 */
class SimpleTask : public QRunnable
{
public:
    explicit SimpleTask(int id, const QString &name) : m_id(id), m_name(name) {}

    /**
     * 任务执行函数（会在线程池的线程中调用）
     */
    void run() override {
        qDebug() << "[" << QThread::currentThreadId() << "]"
                 << "任务" << m_id << "开始:" << m_name;

        // 模拟耗时操作
        QThread::sleep(1);

        qDebug() << "[" << QThread::currentThreadId() << "]"
                 << "任务" << m_id << "完成";

        // QRunnable 默认执行完后自动删除（由 QThreadPool 管理）
    }

private:
    int m_id;
    QString m_name;
};

/**
 * 演示 QThreadPool 线程池用法
 *
 * 线程池适合执行大量短期任务：
 * - 线程复用，减少创建开销
 * - 限制最大线程数，避免资源耗尽
 * - 任务完成后自动回收
 */
void demoThreadPool()
{
    qDebug() << "\n=== 示例 2: QThreadPool 线程池 ===";

    // 获取全局线程池（Qt 提供的默认线程池）
    QThreadPool *pool = QThreadPool::globalInstance();

    // 设置最大线程数（可选，默认通常是 CPU 核心数）
    pool->setMaxThreadCount(4);

    qDebug() << "线程池最大线程数:" << pool->maxThreadCount();

    // 提交多个任务到线程池
    for (int i = 1; i <= 8; ++i) {
        QString taskName = QString("任务%1").arg(i);
        // start() 会自动管理 QRunnable 的生命周期
        pool->start(new SimpleTask(i, taskName));
    }

    // 等待所有任务完成
    pool->waitForDone();
    qDebug() << "示例 2 完成\n";
}

// ========================================
// 示例 3: QtConcurrent::run 简单异步任务
// ========================================

/**
 * 耗时计算函数
 *
 * 这个函数会在后台线程执行
 */
int heavyCalculation(int n)
{
    qDebug() << "[" << QThread::currentThreadId() << "] 开始计算...";

    // 模拟耗时计算
    int result = 0;
    for (int i = 0; i < n; ++i) {
        result += i * i;
        QThread::msleep(100);  // 模拟计算耗时
    }

    qDebug() << "[" << QThread::currentThreadId() << "] 计算完成";
    return result;
}

/**
 * 演示 QtConcurrent::run 用法
 *
 * QtConcurrent 提供了高层 API，可以一行代码启动异步任务
 */
void demoQtConcurrent()
{
    qDebug() << "\n=== 示例 3: QtConcurrent::run ===";

    // 在后台线程运行函数
    QFuture<int> future = QtConcurrent::run([]() {
        return heavyCalculation(10);  // 计算平方和
    });

    // 方式1：等待结果（仅用于示例，实际不要在主线程阻塞！）
    // int result = future.result();
    // qDebug() << "计算结果:" << result;

    // 方式2：使用 QFutureWatcher（推荐，非阻塞）
    QFutureWatcher<int> *watcher = new QFutureWatcher<int>();

    QObject::connect(watcher, &QFutureWatcher<int>::finished, [watcher]() {
        // 任务完成，获取结果
        int result = watcher->result();
        qDebug() << "[" << QThread::currentThreadId() << "] 主线程收到结果:" << result;

        // 清理资源
        watcher->deleteLater();
    });

    // 监听进度（如果任务支持进度报告）
    QObject::connect(watcher, &QFutureWatcher<int>::progressValueChanged,
                     [](int value) {
        qDebug() << "进度:" << value;
    });

    // 设置要监听的 future
    watcher->setFuture(future);

    // 等待完成（仅用于示例）
    future.waitForFinished();
    qDebug() << "示例 3 完成\n";
}

// ========================================
// 示例 4: QMutex 线程安全
// ========================================

/**
 * 线程安全的计数器类
 *
 * 演示如何使用 QMutex 保护共享数据
 */
class SafeCounter
{
public:
    SafeCounter() : m_count(0) {}

    /**
     * 增加计数（线程安全）
     *
     * 使用 QMutexLocker 实现 RAII 风格的锁管理
     * 构造时加锁，析构时自动解锁
     */
    void increment()
    {
        QMutexLocker locker(&m_mutex);  // 自动加锁
        ++m_count;
        // 作用域结束，locker 析构，自动解锁
    }

    /**
     * 获取当前值（线程安全）
     *
     * const 函数也需要加锁，因为读取时可能有其他线程在写入
     */
    int value() const
    {
        QMutexLocker locker(&m_mutex);  // mutable 允许在 const 函数加锁
        return m_count;
    }

private:
    mutable QMutex m_mutex;  // mutable 允许在 const 成员函数中使用
    int m_count;
};

/**
 * 并发增加计数器的任务
 */
class CounterTask : public QRunnable
{
public:
    explicit CounterTask(SafeCounter *counter, int increments)
        : m_counter(counter), m_increments(increments) {}

    void run() override
    {
        for (int i = 0; i < m_increments; ++i) {
            m_counter->increment();
        }
    }

private:
    SafeCounter *m_counter;
    int m_increments;
};

/**
 * 演示 QMutex 保护共享数据
 */
void demoMutex()
{
    qDebug() << "\n=== 示例 4: QMutex 线程安全 ===";

    SafeCounter counter;

    // 创建多个任务并发修改计数器
    QThreadPool pool;
    pool.setMaxThreadCount(4);

    // 启动 10 个任务，每个任务增加 1000 次
    // 预期结果：10 * 1000 = 10000
    for (int i = 0; i < 10; ++i) {
        pool.start(new CounterTask(&counter, 1000));
    }

    // 等待所有任务完成
    pool.waitForDone();

    int finalValue = counter.value();
    qDebug() << "最终计数:" << finalValue;

    if (finalValue == 10000) {
        qDebug() << "正确！多线程访问安全";
    } else {
        qDebug() << "错误！数据竞争导致计数异常";
    }

    qDebug() << "示例 4 完成\n";
}

// ========================================
// 示例 5: 跨线程通信的错误和正确做法
// ========================================

/**
 * 演示跨线程 GUI 操作的错误和正确方式
 *
 * 注意：本示例在控制台程序中演示概念
 * 实际 GUI 程序中，错误的直接操作会导致崩溃
 */
void demoCrossThreadCommunication()
{
    qDebug() << "\n=== 示例 5: 跨线程通信正确做法 ===";

    QThread *thread = new QThread;
    Worker *worker = new Worker;

    worker->moveToThread(thread);

    // 正确方式：通过信号槽跨线程通信
    QObject::connect(thread, &QThread::started, [worker]() {
        worker->doWork("跨线程测试");
    });

    // 主线程接收进度信号
    // 这是安全的：Qt 会自动将信号参数拷贝到事件队列
    // 在主线程的事件循环中调用槽函数
    QObject::connect(worker, &Worker::progressChanged,
                     [](const QString &name, int percent) {
        qDebug() << "主线程安全地收到进度更新:" << percent << "%";
    });

    QObject::connect(worker, &Worker::workFinished, [thread](const QString &, const QString &) {
        thread->quit();
    });

    QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    QObject::connect(thread, &QThread::finished, worker, &Worker::deleteLater);

    thread->start();
    thread->wait();

    qDebug() << "示例 5 完成\n";
}

// ========================================
// 主函数
// ========================================

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "主线程 ID:" << QThread::currentThreadId();
    qDebug() << "Qt 多线程基础示例开始...\n";

    // 运行各个示例
    demoMoveToThread();           // 示例 1: QThread + moveToThread
    demoThreadPool();             // 示例 2: QThreadPool 线程池
    demoQtConcurrent();           // 示例 3: QtConcurrent::run
    demoMutex();                  // 示例 4: QMutex 线程安全
    demoCrossThreadCommunication(); // 示例 5: 跨线程通信

    qDebug() << "\n所有示例运行完成！";
    qDebug() << "要点回顾：";
    qDebug() << "1. 使用 moveToThread 将 QObject 移到新线程";
    qDebug() << "2. 线程池适合短期任务，减少创建开销";
    qDebug() << "3. QtConcurrent 提供简洁的异步 API";
    qDebug() << "4. 共享数据用 QMutex 保护";
    qDebug() << "5. 跨线程通信用信号槽，GUI 必须在主线程操作";

    return 0;
}

#include "main.moc"  // 包含 MOC 生成的代码（因为 Q_OBJECT 宏）
