/*
 * 02-signal-slot-beginner 示例
 *
 * 演示 Qt 信号与槽的基础用法：
 * 1. 新式 connect 语法（函数指针）
 * 2. Lambda 表达式作为槽
 * 3. 信号槽的同步与异步调用
 * 4. 信号的参数传递
 * 5. 连接的管理与断开
 */

#include <QCoreApplication>      // 应用程序核心类
#include <QTimer>                // 定时器类
#include <QDebug>                // 调试输出
#include <QThread>               // 线程类
#include <QMetaObject>           // 元对象类

// 自定义计数器类：演示信号声明和发射
class Counter : public QObject {
    Q_OBJECT  // 必须有此宏才能使用信号槽

public:
    Counter(QObject *parent = nullptr) : QObject(parent), m_value(0) {}

    // 增加计数值并发射信号
    void increment() {
        ++m_value;
        // emit 关键字发射信号，传递新值
        emit valueChanged(m_value);
    }

    // 重置计数值
    void reset() {
        m_value = 0;
        emit valueChanged(m_value);
    }

    // 获取当前值
    int value() const { return m_value; }

signals:
    // 信号声明：告诉系统这个类可能发出这个信号
    // 信号只需要声明，不需要实现
    void valueChanged(int newValue);  // 参数：新值

    // 无参数信号示例
    void resetSignal();

private:
    int m_value;  // 计数值
};

// 演示槽函数的普通成员函数
class SlotReceiver : public QObject {
    Q_OBJECT

public:
    SlotReceiver(QObject *parent = nullptr) : QObject(parent), m_callCount(0) {}

public slots:
    // 槽函数：可以被信号连接调用
    void onValueChanged(int newValue) {
        m_callCount++;
        qDebug() << "[槽函数调用] 值改变为:" << newValue
                 << "(第" << m_callCount << "次)";
    }

    void onReset() {
        qDebug() << "[槽函数调用] 收到重置信号";
    }

    // 无参数槽函数
    void simpleSlot() {
        qDebug() << "[槽函数调用] 简单槽被调用";
    }

private:
    int m_callCount;  // 调用计数
};

// 演示跨线程信号槽的工作类
class Worker : public QObject {
    Q_OBJECT

public:
    Worker(QObject *parent = nullptr) : QObject(parent) {}

public slots:
    void doWork() {
        QThread *thread = QThread::currentThread();
        qDebug() << "[工作线程] doWork 在线程" << thread << "中执行";
        emit workFinished("工作完成！");
    }

signals:
    void workFinished(const QString &result);
};

// 演示 Lambda 捕获上下文
class LambdaDemo : public QObject {
    Q_OBJECT

public:
    LambdaDemo(QObject *parent = nullptr) : QObject(parent), m_counter(0) {}

    void setupConnections(QTimer *timer) {
        // Lambda 作为槽：捕获 this 指针访问成员变量
        connect(timer, &QTimer::timeout, this, [this]() {
            m_counter++;
            qDebug() << "[Lambda] 定时器触发，计数:" << m_counter;
        });

        // Lambda 捕获局部变量
        QString prefix = "定时器";
        connect(timer, &QTimer::timeout, [prefix]() {
            qDebug() << "[Lambda]" << prefix << "持续运行中...";
        });
    }

private:
    int m_counter;
};

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);  // 核心应用对象（非GUI）

    qDebug() << "========== Qt 信号槽基础示例 ==========";
    qDebug() << "Qt 版本:" << QT_VERSION_STR;
    qDebug() << "当前线程:" << QThread::currentThread();
    qDebug() << "";

    // ========== 示例 1: 基础信号槽连接 ==========
    qDebug() << "--- 示例 1: 基础信号槽连接 ---";

    Counter counter;
    SlotReceiver receiver;

    // 新式 connect 语法：使用函数指针
    // 连接 counter 的 valueChanged 信号到 receiver 的槽
    QObject::connect(&counter, &Counter::valueChanged,
                     &receiver, &SlotReceiver::onValueChanged);

    // 触发信号
    counter.increment();  // 输出: 值改变为: 1
    counter.increment();  // 输出: 值改变为: 2
    counter.reset();      // 输出: 值改变为: 0

    qDebug() << "";

    // ========== 示例 2: Lambda 表达式作为槽 ==========
    qDebug() << "--- 示例 2: Lambda 表达式作为槽 ---";

    Counter counter2;
    int lambdaCallCount = 0;  // Lambda 可以捕获局部变量

    // Lambda 直接捕获变量引用
    QObject::connect(&counter2, &Counter::valueChanged, [&](int newValue) {
        lambdaCallCount++;
        qDebug() << "[Lambda] 捕获到值变化:" << newValue
                 << "  (Lambda 调用次数:" << lambdaCallCount << ")";
    });

    counter2.increment();
    counter2.increment();

    qDebug() << "";

    // ========== 示例 3: 一个信号连接多个槽 ==========
    qDebug() << "--- 示例 3: 一个信号连接多个槽 ---";

    Counter counter3;
    SlotReceiver receiver1, receiver2;

    // 同一个信号连接到两个不同的槽
    QObject::connect(&counter3, &Counter::valueChanged, &receiver1, &SlotReceiver::onValueChanged);
    QObject::connect(&counter3, &Counter::valueChanged, &receiver2, &SlotReceiver::onValueChanged);
    QObject::connect(&counter3, &Counter::valueChanged, [](int value) {
        qDebug() << "[额外 Lambda] 也收到通知:" << value;
    });

    qDebug() << "触发一次信号，三个槽都会被调用:";
    counter3.increment();

    qDebug() << "";

    // ========== 示例 4: 连接的管理与断开 ==========
    qDebug() << "--- 示例 4: 连接的管理与断开 ---";

    Counter counter4;
    SlotReceiver receiver4;

    // 保存连接对象以便后续断开
    QMetaObject::Connection conn = QObject::connect(
        &counter4, &Counter::valueChanged,
        &receiver4, &SlotReceiver::onValueChanged
    );

    counter4.increment();  // 会触发槽
    qDebug() << "断开连接后再触发信号:";

    // 断开连接
    QObject::disconnect(conn);
    counter4.increment();  // 不会触发槽（已断开）

    qDebug() << "";

    // ========== 示例 5: 使用 QTimer 的信号槽 ==========
    qDebug() << "--- 示例 5: QTimer 定时器信号槽 ---";

    QTimer timer;
    timer.setInterval(500);  // 500毫秒

    int tickCount = 0;
    bool timerDone = false;

    // 使用 QTimer::singleShot 来演示信号槽
    // 单次定时器：500ms 后触发一次
    QTimer::singleShot(500, [&]() {
        tickCount++;
        qDebug() << "[定时器] 单次触发 Tick" << tickCount;
        timerDone = true;
    });

    // 等待定时器完成
    while (!timerDone) {
        QCoreApplication::processEvents();
    }

    qDebug() << "";

    // ========== 示例 6: 跨线程信号槽（异步） ==========
    qDebug() << "--- 示例 6: 跨线程信号槽 ---";

    QThread workerThread;
    Worker *worker = new Worker;

    // 将 worker 对象移动到新线程
    worker->moveToThread(&workerThread);

    // 连接跨线程信号槽：会自动使用 QueuedConnection
    QObject::connect(&workerThread, &QThread::started,
                     worker, &Worker::doWork);
    QObject::connect(worker, &Worker::workFinished, [](const QString &result) {
        qDebug() << "[主线程] 收到工作结果:" << result;
    });
    QObject::connect(worker, &Worker::workFinished,
                     &workerThread, &QThread::quit);

    workerThread.start();
    workerThread.wait();

    qDebug() << "";

    // ========== 示例 7: Lambda 捕获类成员 ==========
    qDebug() << "--- 示例 7: Lambda 捕获类成员 ---";

    QTimer timer2;
    timer2.setInterval(300);

    LambdaDemo demo;
    demo.setupConnections(&timer2);

    // 使用 singleShot 来演示
    QTimer::singleShot(300, [&]() {
        qDebug() << "[Lambda] 定时器触发完成";
    });

    QCoreApplication::processEvents();

    qDebug() << "";
    qDebug() << "========== 所有示例执行完毕 ==========";

    return 0;  // QCoreApplication 不需要事件循环持续运行
}

// 包含 MOC 生成的代码
#include "main.moc"
