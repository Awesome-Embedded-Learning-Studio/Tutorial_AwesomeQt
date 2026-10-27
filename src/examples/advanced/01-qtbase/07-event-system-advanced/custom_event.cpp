/// @file    custom_event.cpp
/// @brief   MessageEvent 与 EventReceiver 的实现，以及 sendEvent / postEvent 演示函数。
///
/// 对应教程：进阶层 01-QtBase/07-事件系统进阶。

#include "custom_event.h"

// ===========================================================================
// MessageEvent
// ===========================================================================

MessageEvent::MessageEvent(const QString& message, int priority)
    : QEvent(eventId), m_message(message), m_priority(priority)
{
}

QString MessageEvent::message() const
{
    return m_message;
}

int MessageEvent::priority() const
{
    return m_priority;
}

// ===========================================================================
// EventReceiver
// ===========================================================================

EventReceiver::EventReceiver(QObject* parent) : QObject(parent)
{
}

void EventReceiver::customEvent(QEvent* event)
{
    if (event->type() == MessageEvent::eventId) {
        auto* msgEvent = static_cast<MessageEvent*>(event);
        qDebug() << "  [customEvent] 收到消息:"
                 << msgEvent->message()
                 << "优先级:" << msgEvent->priority();
        event->accept();
    } else {
        QObject::customEvent(event);
    }
}

// ===========================================================================
// 演示函数
// ===========================================================================

void demoSendVsPost()
{
    qDebug() << "\n=== sendEvent vs postEvent 对比 ===";

    EventReceiver receiver;

    // sendEvent: 同步发送，立即处理，在同一线程中直接调用事件处理器
    qDebug() << "\n--- sendEvent（同步，立即处理）---";
    MessageEvent sendEvt("同步消息", 1);
    qDebug() << "发送前";
    QCoreApplication::sendEvent(&receiver, &sendEvt);
    qDebug() << "发送后（已被立即处理）";

    // postEvent: 异步投递，放入事件队列，下次事件循环时处理
    qDebug() << "\n--- postEvent（异步，事件队列）---";
    qDebug() << "投递前";
    QCoreApplication::postEvent(&receiver, new MessageEvent("异步消息", 2));
    qDebug() << "投递后（尚未处理，等待事件循环）";

    // 需要处理事件循环才能让 postEvent 的事件被处理
    QCoreApplication::processEvents();
    qDebug() << "processEvents() 后，异步消息已被处理";
}

void demoCrossThreadEvent()
{
    qDebug() << "\n=== 跨线程 postEvent ===";

    EventReceiver receiver;

    // postEvent 是线程安全的，可以从任何线程调用
    // 模拟从另一个线程投递事件
    qDebug() << "主线程投递事件到接收者...";
    QCoreApplication::postEvent(&receiver, new MessageEvent("来自主线程的消息", 3));
    QCoreApplication::postEvent(&receiver, new MessageEvent("另一条消息", 4));

    // 处理事件队列
    QCoreApplication::processEvents();
    qDebug() << "事件队列已处理";

    qDebug() << "\n关键要点:";
    qDebug() << "  - sendEvent 同步处理，必须与接收者在同一线程";
    qDebug() << "  - postEvent 异步处理，线程安全，适合跨线程通信";
    qDebug() << "  - postEvent 的事件在接收者所在线程的事件循环中处理";
}
