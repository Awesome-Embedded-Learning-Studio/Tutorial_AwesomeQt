/// @file    custom_event.h
/// @brief   自定义事件类型：携带消息数据的自定义事件，以及 sendEvent / postEvent 演示。
///
/// 对应教程：进阶层 01-QtBase/07-事件系统进阶。

#pragma once

#include <QCoreApplication>
#include <QDebug>
#include <QEvent>
#include <QString>

// ---------------------------------------------------------------------------
// 自定义事件类型：携带消息数据的自定义事件
// ---------------------------------------------------------------------------
/// @brief 携带消息和优先级的自定义事件。
class MessageEvent : public QEvent {
public:
    /// @brief MessageEvent 的全局唯一事件 ID，通过 registerEventType() 注册。
    /// @note QEvent::User 是自定义事件的起始 ID，+1 避免与基线冲突。
    static const QEvent::Type eventId;

    /// @brief 构造消息事件。
    /// @param[in] message  事件携带的消息文本。
    /// @param[in] priority 消息优先级，默认为 0。
    MessageEvent(const QString& message, int priority = 0);

    /// @brief 获取事件携带的消息文本。
    /// @return 消息字符串。
    QString message() const;

    /// @brief 获取消息优先级。
    /// @return 优先级整数值。
    int priority() const;

private:
    QString m_message;   ///< 事件携带的消息文本
    int m_priority;      ///< 消息优先级
};

// 使用 inline 避免多定义链接错误（头文件被多个编译单元包含时）
inline const QEvent::Type MessageEvent::eventId =
    static_cast<QEvent::Type>(QEvent::registerEventType(QEvent::User + 1));

// ---------------------------------------------------------------------------
// 自定义事件接收者：重写 customEvent() 处理自定义事件
// ---------------------------------------------------------------------------
/// @brief 通过重写 customEvent() 接收并处理自定义事件的对象。
class EventReceiver : public QObject {
    Q_OBJECT
public:
    /// @brief 构造事件接收者。
    /// @param[in] parent 父对象指针，Qt 对象树自动管理生命周期。
    explicit EventReceiver(QObject* parent = nullptr);

protected:
    /// @brief 重写 customEvent()，在事件循环中处理自定义事件。
    /// @param[in] event 待处理的事件指针。
    /// @note 只有自定义事件（ID >= QEvent::User）会进入此方法。
    void customEvent(QEvent* event) override;
};

// ---------------------------------------------------------------------------
// 演示函数
// ---------------------------------------------------------------------------

/// @brief 演示 sendEvent（同步）与 postEvent（异步）的行为差异。
/// @note sendEvent 在同一线程中直接调用事件处理器；
///       postEvent 将事件放入队列，需要事件循环处理。
void demoSendVsPost();

/// @brief 演示跨线程 postEvent 的线程安全特性。
/// @note postEvent 是线程安全的，可以从任何线程投递事件到接收者。
void demoCrossThreadEvent();
