/// @file    elapsed_timer_demo.cpp
/// @brief   elapsed_timer_demo.h 中所有函数和方法的实现。
///
/// 对应教程：进阶层 01-QtBase/11-QTimer 高级。

#include "elapsed_timer_demo.h"

#include <QDebug>
#include <QDeadlineTimer>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QThread>
#include <QTimerEvent>

void demoElapsedTimer()
{
    qDebug() << "\n--- QElapsedTimer 纳秒级性能计时演示 ---";

    QElapsedTimer timer;

    // 基础用法：start() / elapsed()
    timer.start();
    qDebug() << "[ElapsedTimer] 启动计时器";

    // 模拟耗时操作 1：忙等待
    QThread::msleep(100);
    qint64 elapsed1 = timer.elapsed();
    qDebug() << "[ElapsedTimer] 100ms sleep 后 elapsed():" << elapsed1 << "ms";

    // 纳秒精度：nsecsElapsed()
    qint64 nsecs = timer.nsecsElapsed();
    qDebug() << "[ElapsedTimer] 纳秒级计时:" << nsecs << "ns"
             << "（约" << (nsecs / 1000000.0) << "ms）";

    // restart(): 重置并返回上次的经过时间
    qint64 previousMs = timer.restart();
    qDebug() << "[ElapsedTimer] restart() 返回上次经过:" << previousMs << "ms";

    // 模拟耗时操作 2
    QThread::msleep(50);
    qint64 elapsed2 = timer.elapsed();
    qDebug() << "[ElapsedTimer] restart 后 50ms sleep:" << elapsed2 << "ms";

    // hasExpired(): 检查是否超过指定时间
    qDebug() << "[ElapsedTimer] 是否超过 40ms:" << timer.hasExpired(40);    // true
    qDebug() << "[ElapsedTimer] 是否超过 200ms:" << timer.hasExpired(200);  // false

    // 性能测量示例：测量循环执行时间
    qDebug() << "\n[ElapsedTimer] 性能测量示例";
    timer.restart();

    // volatile 防止编译器优化掉循环
    volatile int sum = 0;
    for (int i = 0; i < 1000000; ++i) {
        sum += i;
    }

    qint64 loopNsecs = timer.nsecsElapsed();
    qDebug() << "[ElapsedTimer] 100 万次累加:"
             << loopNsecs << "ns"
             << "（约" << (loopNsecs / 1000.0) << "us）"
             << "结果:" << sum;
}

void demoDeadlineTimer()
{
    qDebug() << "\n--- QDeadlineTimer 截止时间计时器演示 ---";

    // 创建 500ms 的截止时间（从当前时间起）
    QDeadlineTimer deadline(500);
    qDebug() << "[DeadlineTimer] 创建 500ms 截止时间";
    qDebug() << "[DeadlineTimer] 剩余时间:" << deadline.remainingTime() << "ms";
    qDebug() << "[DeadlineTimer] 是否已过期:" << deadline.hasExpired();

    // 模拟在循环中处理任务，每次检查是否超时
    qDebug() << "\n[DeadlineTimer] 模拟带超时的任务处理循环";
    int taskCount = 0;
    while (!deadline.hasExpired()) {
        // 模拟一个处理步骤
        QThread::msleep(100);
        taskCount++;
        qint64 remaining = deadline.remainingTime();

        if (remaining > 0) {
            qDebug() << "[DeadlineTimer] 完成任务" << taskCount
                     << "，剩余时间:" << remaining << "ms";
        } else {
            qDebug() << "[DeadlineTimer] 截止时间已到，停止处理";
            break;
        }
    }

    qDebug() << "[DeadlineTimer] 在截止时间前完成了" << taskCount << "个任务";

    // 使用 Forever 常量创建永不超时的 deadline
    QDeadlineTimer foreverDeadline(QDeadlineTimer::Forever);
    qDebug() << "\n[DeadlineTimer] Forever 模式剩余时间:"
             << foreverDeadline.remainingTime() << "ms（-1 表示永不超时）";

    // 使用绝对截止时间（设置 200ms 后的截止线）
    QDeadlineTimer absoluteDeadline;
    absoluteDeadline.setRemainingTime(200);  // 200ms 后到期
    qDebug() << "[DeadlineTimer] 绝对截止时间 200ms，是否过期:"
             << absoluteDeadline.hasExpired();

    QThread::msleep(250);
    qDebug() << "[DeadlineTimer] 250ms 后是否过期:" << absoluteDeadline.hasExpired();
}

BasicTimerObject::BasicTimerObject(QObject* parent)
    : QObject(parent)
    , m_counter(0)
{
}

void BasicTimerObject::startTimers()
{
    qDebug() << "[BasicTimer] 启动两个定时器";

    // 定时器 1：200ms 间隔
    m_timer1.start(200, Qt::PreciseTimer, this);

    // 定时器 2：500ms 间隔
    m_timer2.start(500, Qt::PreciseTimer, this);
}

void BasicTimerObject::stopTimers()
{
    m_timer1.stop();
    m_timer2.stop();
    qDebug() << "[BasicTimer] 所有定时器已停止";
}

bool BasicTimerObject::isTimer1Active() const
{
    return m_timer1.isActive();
}

void BasicTimerObject::timerEvent(QTimerEvent* event)
{
    // 通过 timerId 判断是哪个定时器触发的
    if (event->timerId() == m_timer1.timerId()) {
        qDebug() << "[BasicTimer] 定时器 1 触发（200ms）"
                 << "计数:" << ++m_counter;
    } else if (event->timerId() == m_timer2.timerId()) {
        qDebug() << "[BasicTimer] 定时器 2 触发（500ms）"
                 << "计时:" << (m_counter * 200) << "ms";
    }

    // 达到 5 次后停止所有定时器
    if (m_counter >= 5) {
        stopTimers();
        emit allDone();
    }
}

void demoBasicTimer()
{
    qDebug() << "\n--- QBasicTimer 轻量级定时器演示 ---";

    QEventLoop loop;
    BasicTimerObject obj;

    // 连接完成信号到事件循环退出
    QObject::connect(&obj, &BasicTimerObject::allDone, &loop, &QEventLoop::quit);

    // 启动定时器
    obj.startTimers();
    qDebug() << "[BasicTimer] 定时器是否活跃:" << obj.isTimer1Active();

    // 进入事件循环等待
    loop.exec();

    qDebug() << "[BasicTimer] 演示结束";
    qDebug() << "[BasicTimer] QBasicTimer 优点：无信号/槽开销，适合大量定时器";
}
