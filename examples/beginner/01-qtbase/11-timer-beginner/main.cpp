/*
 * 11-timer-beginner 示例
 *
 * 演示 Qt 定时器的基础用法：
 * 1. QTimer 基础用法（重复定时器）
 * 2. 单次定时器（setSingleShot 和 singleShot 静态方法）
 * 3. QElapsedTimer 高精度计时
 * 4. 定时器生命周期管理
 * 5. 定时器在线程中的注意事项
 */

#include <QCoreApplication>       // 应用程序核心类
#include <QTimer>                 // 定时器类
#include <QElapsedTimer>          // 高精度计时器类
#include <QDebug>                 // 调试输出
#include <QThread>                // 线程类
#include <QEventLoop>             // 事件循环类

// ========== 示例 1: 基础重复定时器 ==========
void basicTimerExample() {
    qDebug() << "\n--- 示例 1: 基础重复定时器 ---";

    // 创建定时器，设置间隔为 500 毫秒
    QTimer *timer = new QTimer;
    timer->setInterval(500);  // 500 毫秒 = 0.5 秒

    // 计数器，用于限制执行次数
    int counter = 0;

    // 连接 timeout 信号到 Lambda 表达式
    QObject::connect(timer, &QTimer::timeout, [&]() {
        counter++;
        qDebug() << "[重复定时器] Tick" << counter;

        if (counter >= 5) {
            timer->stop();  // 5 次后停止
            qDebug() << "[重复定时器] 停止";
        }
    });

    // 启动定时器
    timer->start();

    // 等待定时器完成
    QEventLoop loop;
    QObject::connect(timer, &QTimer::timeout, &loop, [&]() {
        if (!timer->isActive()) {
            loop.quit();
        }
    });
    loop.exec();

    // 手动删除定时器（因为没有 parent）
    delete timer;
}

// ========== 示例 2: 单次定时器 ==========
void singleShotTimerExample() {
    qDebug() << "\n--- 示例 2: 单次定时器 ---";

    // 方式一：使用 setSingleShot(true)
    QTimer *timer = new QTimer;
    timer->setSingleShot(true);  // 只触发一次
    timer->setInterval(1000);    // 1 秒后触发

    QObject::connect(timer, &QTimer::timeout, []() {
        qDebug() << "[单次定时器] 触发（只执行这一次）";
    });

    timer->start();

    // 等待触发
    QEventLoop loop;
    QObject::connect(timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    loop.exec();

    delete timer;

    // 方式二：使用静态方法 singleShot（更简洁）
    qDebug() << "[单次定时器] 使用静态方法，3 秒后执行...";
    QTimer::singleShot(3000, []() {
        qDebug() << "[单次定时器] 静态方法触发！";
    });

    // 等待静态方法完成
    QEventLoop loop2;
    QTimer::singleShot(3500, &loop2, &QEventLoop::quit);
    loop2.exec();
}

// ========== 示例 3: QElapsedTimer 高精度计时 ==========
void elapsedTimerExample() {
    qDebug() << "\n--- 示例 3: QElapsedTimer 高精度计时 ---";

    QElapsedTimer elapsedTimer;
    elapsedTimer.start();  // 启动计时器

    qDebug() << "[计时器] 开始计时...";

    // 模拟一些操作
    QThread::msleep(100);

    qint64 elapsed = elapsedTimer.elapsed();  // 获取经过的毫秒数
    qDebug() << "[计时器] 经过时间:" << elapsed << "毫秒";

    // 检查是否超时
    if (elapsedTimer.hasExpired(200)) {
        qDebug() << "[计时器] 已超过 200 毫秒";
    } else {
        qDebug() << "[计时器] 未超过 200 毫秒";
    }

    // restart() 重置并返回之前经过的时间
    qint64 previousElapsed = elapsedTimer.restart();
    qDebug() << "[计时器] 重置前经过:" << previousElapsed << "毫秒";

    QThread::msleep(50);
    qDebug() << "[计时器] 重置后经过:" << elapsedTimer.elapsed() << "毫秒";

    // elapsed() 返回纳秒精度（但实际精度取决于系统）
    qDebug() << "[计时器] 纳秒精度:" << elapsedTimer.nsecsElapsed() << "纳秒";
}

// ========== 示例 4: 定时器生命周期管理 ==========
void timerLifecycleExample() {
    qDebug() << "\n--- 示例 4: 定时器生命周期管理 ---";

    // 创建一个有 parent 的定时器
    QTimer *parentTimer = new QTimer;

    // 创建子定时器，parent 是 parentTimer
    QTimer *childTimer = new QTimer(parentTimer);

    // 当 parentTimer 被删除时，childTimer 也会被自动删除
    // 这是 Qt 对象树机制

    parentTimer->setInterval(1000);
    childTimer->setInterval(500);

    QObject::connect(parentTimer, &QTimer::timeout, []() {
        qDebug() << "[父定时器] 触发";
    });

    QObject::connect(childTimer, &QTimer::timeout, []() {
        qDebug() << "[子定时器] 触发";
    });

    parentTimer->start();
    childTimer->start();

    // 运行一段时间后删除父定时器
    QTimer::singleShot(2000, [&]() {
        qDebug() << "[生命周期] 删除父定时器，子定时器也会被删除";
        delete parentTimer;  // childTimer 也会被自动删除
    });

    // 等待完成
    QEventLoop loop;
    QTimer::singleShot(2500, &loop, &QEventLoop::quit);
    loop.exec();
}

// ========== 示例 5: 定时器状态查询 ==========
void timerStateExample() {
    qDebug() << "\n--- 示例 5: 定时器状态查询 ---";

    QTimer timer;
    timer.setInterval(1000);

    qDebug() << "[状态查询] 定时器是否活跃:" << timer.isActive();  // false

    timer.start();
    qDebug() << "[状态查询] 启动后是否活跃:" << timer.isActive();  // true

    qDebug() << "[状态查询] 定时器间隔:" << timer.interval() << "毫秒";

    // 获取剩余时间（可能不准确，仅供参考）
    qDebug() << "[状态查询] 剩余时间:" << timer.remainingTime() << "毫秒";

    // 设置定时器类型（CoarseTimer 精度较低但更省电）
    timer.setTimerType(Qt::CoarseTimer);
    qDebug() << "[状态查询] 定时器类型:" << timer.timerType();

    QThread::msleep(500);
    qDebug() << "[状态查询] 500ms 后剩余时间:" << timer.remainingTime() << "毫秒";

    timer.stop();
    qDebug() << "[状态查询] 停止后是否活跃:" << timer.isActive();  // false
    qDebug() << "[状态查询] 停止后剩余时间:" << timer.remainingTime();  // -1
}

// ========== 示例 6: 模拟秒表 ==========
class Stopwatch : public QObject {
    Q_OBJECT

public:
    Stopwatch(QObject *parent = nullptr) : QObject(parent), m_seconds(0), m_running(false) {}

    void start() {
        if (!m_running) {
            m_running = true;
            m_timer.start(1000);  // 每秒触发一次
            qDebug() << "[秒表] 启动";
        }
    }

    void pause() {
        if (m_running) {
            m_running = false;
            m_timer.stop();
            qDebug() << "[秒表] 暂停";
        }
    }

    void reset() {
        m_running = false;
        m_timer.stop();
        m_seconds = 0;
        qDebug() << "[秒表] 重置: 00:00";
    }

private slots:
    void onTick() {
        m_seconds++;
        int minutes = m_seconds / 60;
        int seconds = m_seconds % 60;
        qDebug() << "[秒表]" << QString("%1:%2")
                    .arg(minutes, 2, 10, QChar('0'))
                    .arg(seconds, 2, 10, QChar('0'));
    }

private:
    QTimer m_timer;
    int m_seconds;
    bool m_running;

    // 连接定时器信号
    friend void stopwatchExample();
};

void stopwatchExample() {
    qDebug() << "\n--- 示例 6: 模拟秒表 ---";

    Stopwatch stopwatch;

    // 连接定时器信号
    QObject::connect(&stopwatch.m_timer, &QTimer::timeout, &stopwatch, &Stopwatch::onTick);

    // 启动秒表
    stopwatch.start();

    // 运行 5 秒
    QEventLoop loop;
    QTimer::singleShot(5000, [&]() {
        stopwatch.pause();
        loop.quit();
    });
    loop.exec();

    // 重置
    qDebug() << "[秒表] 重置秒表";
    stopwatch.reset();
}

// ========== 示例 7: 定时器精度演示 ==========
void timerPrecisionExample() {
    qDebug() << "\n--- 示例 7: 定时器精度演示 ---";

    QElapsedTimer elapsedTimer;
    elapsedTimer.start();

    int expectedCount = 0;

    // 创建一个 50ms 间隔的定时器
    QTimer timer;
    timer.setInterval(50);

    QObject::connect(&timer, &QTimer::timeout, [&]() {
        expectedCount++;
        qint64 actualElapsed = elapsedTimer.elapsed();
        qint64 expectedElapsed = expectedCount * 50;
        qint64 drift = actualElapsed - expectedElapsed;

        qDebug() << "[精度测试] 第" << expectedCount << "次触发,"
                 << "实际经过:" << actualElapsed << "ms,"
                 << "预期经过:" << expectedElapsed << "ms,"
                 << "偏差:" << drift << "ms";

        if (expectedCount >= 10) {
            timer.stop();
        }
    });

    timer.start();

    // 等待完成
    QEventLoop loop;
    QObject::connect(&timer, &QTimer::timeout, &loop, [&]() {
        if (!timer.isActive()) {
            loop.quit();
        }
    });
    loop.exec();

    qDebug() << "[精度测试] 注意：偏差会累积，这是正常现象";
}

// ========== 主函数 ==========
int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    // 设置消息处理器，确保输出到控制台
    qSetMessagePattern("[%{type}] %{message}");

    qDebug() << "========== Qt 定时器基础示例 ==========";
    qDebug() << "Qt 版本:" << QT_VERSION_STR;
    qDebug() << "当前线程:" << QThread::currentThread();

    // 依次运行各个示例
    basicTimerExample();
    singleShotTimerExample();
    elapsedTimerExample();
    timerLifecycleExample();
    timerStateExample();
    stopwatchExample();
    timerPrecisionExample();

    qDebug() << "\n========== 所有示例执行完毕 ==========";

    return 0;
}

// 包含 MOC 生成的代码（因为我们使用了 Q_OBJECT）
#include "main.moc"
