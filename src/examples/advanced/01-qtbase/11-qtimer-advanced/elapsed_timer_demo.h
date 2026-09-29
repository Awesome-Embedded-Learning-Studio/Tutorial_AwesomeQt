/// @file    elapsed_timer_demo.h
/// @brief   演示 QElapsedTimer、QDeadlineTimer 和 QBasicTimer 的高级用法。
///
/// 对应教程：进阶层 01-QtBase/11-QTimer 高级。
///
/// QElapsedTimer:  高精度计时器，用于测量代码执行时间
/// QDeadlineTimer: 带截止时间的计时器，用于超时控制
/// QBasicTimer:    轻量级定时器，直接通过 timerEvent 接收回调

#pragma once

#include <QBasicTimer>
#include <QObject>

/// @brief 演示 QElapsedTimer 纳秒级性能计时的各项 API。
///
/// 展示 start()、elapsed()、nsecsElapsed()、restart()、hasExpired()
/// 等方法的使用场景与输出含义。
void demoElapsedTimer();

/// @brief 演示 QDeadlineTimer 截止时间超时控制。
///
/// 演示相对截止时间、绝对截止时间、Forever 常量等用法，
/// 以及在循环中进行超时检查的典型模式。
void demoDeadlineTimer();

/// @brief 使用 QBasicTimer 的 QObject 子类，通过 timerEvent 接收回调。
///
/// QBasicTimer 不使用信号/槽机制，开销更低，适合大量定时器场景。
/// 重写 timerEvent() 并通过 timerId() 区分不同定时器。
class BasicTimerObject : public QObject {
    Q_OBJECT

public:
    /// @brief 构造函数，初始化内部计数器。
    /// @param[in] parent 父对象指针，Qt 对象树自动管理生命周期。
    explicit BasicTimerObject(QObject* parent = nullptr);

    /// @brief 启动两个不同间隔的 QBasicTimer。
    /// @note 定时器精度设为 Qt::PreciseTimer 以获得更准确的回调。
    void startTimers();

    /// @brief 停止所有正在运行的 QBasicTimer。
    void stopTimers();

    /// @brief 检查定时器 1 是否仍在活跃状态。
    /// @return true 表示定时器 1 仍在运行。
    bool isTimer1Active() const;

    /// @brief 重写 timerEvent 以接收 QBasicTimer 的定时回调。
    /// @param[in] event 定时器事件，通过 timerId() 区分来源。
    /// @note 达到 5 次触发后自动停止所有定时器并发射 allDone 信号。
    void timerEvent(QTimerEvent* event) override;

signals:
    /// @brief 所有定时器演示完成后发射此信号。
    void allDone();

private:
    QBasicTimer m_timer1;  ///< 轻量级定时器 1（200ms 间隔）
    QBasicTimer m_timer2;  ///< 轻量级定时器 2（500ms 间隔）
    int m_counter;          ///< 触发计数器
};

/// @brief 运行 QBasicTimer 完整演示流程。
///
/// 创建 BasicTimerObject 实例，启动定时器，进入事件循环
/// 等待 allDone 信号后退出。
void demoBasicTimer();
