/**
 * Qt 事件系统入门示例
 *
 * 本示例演示 Qt 事件系统的核心概念：
 * 1. 重写 event() 处理事件
 * 2. 使用事件过滤器 (installEventFilter + eventFilter)
 * 3. 事件的 accept() 与 ignore()
 * 4. postEvent vs sendEvent 的区别
 */

#include <QCoreApplication>
#include <QEvent>
#include <QKeyEvent>
#include <QTimer>
#include <QDateTime>
#include <iostream>
#include <string>

#include "myeventhandler.h"
#include "keylogger.h"
#include "eventreceiver.h"

// 自定义事件类型，用于演示
class CustomEvent : public QEvent {
public:
    CustomEvent(const QString &message)
        : QEvent(QEvent::User), m_message(message) {}

    QString message() const { return m_message; }

private:
    QString m_message;
};

// ============================================================================
// 演示函数
// ============================================================================

/**
 * 演示基础事件处理
 */
void demoBasicEventHandling() {
    std::cout << "\n=== 示例 1: 基础事件处理 ===" << std::endl;

    MyEventHandler handler;
    handler.setObjectName("MyHandler");

    // 模拟键盘事件 - 使用 sendEvent() 同步发送
    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier, "A");
    QCoreApplication::sendEvent(&handler, &keyEvent);

    // 演示 ignore() 的效果
    handler.setAcceptEvents(false);
    QKeyEvent keyEvent2(QEvent::KeyPress, Qt::Key_B, Qt::NoModifier, "B");
    QCoreApplication::sendEvent(&handler, &keyEvent2);

    // 演示 KeyRelease 事件
    handler.setAcceptEvents(true);
    QKeyEvent keyEvent3(QEvent::KeyRelease, Qt::Key_A, Qt::NoModifier, "A");
    QCoreApplication::sendEvent(&handler, &keyEvent3);
}

/**
 * 演示事件过滤器
 */
void demoEventFilter() {
    std::cout << "\n=== 示例 2: 事件过滤器 ===" << std::endl;

    MyEventHandler target;
    target.setObjectName("TargetObject");
    KeyLogger logger;

    // 安装事件过滤器 - logger 将监听 target 的所有事件
    target.installEventFilter(&logger);
    std::cout << "已安装事件过滤器，TargetObject 的事件将被拦截" << std::endl;

    // 发送一些键盘事件
    QKeyEvent eventA(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier, "A");
    QCoreApplication::sendEvent(&target, &eventA);

    // 发送 ESC 键 - 会被过滤器拦截
    QKeyEvent eventEsc(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    QCoreApplication::sendEvent(&target, &eventEsc);

    // 禁用过滤器后再试一次
    logger.setEnabled(false);
    std::cout << "\n过滤器已禁用，事件将正常通过" << std::endl;
    QKeyEvent eventB(QEvent::KeyPress, Qt::Key_B, Qt::NoModifier, "B");
    QCoreApplication::sendEvent(&target, &eventB);
}

/**
 * 演示 postEvent vs sendEvent
 */
void demoPostVsSendEvent() {
    std::cout << "\n=== 示例 3: postEvent vs sendEvent ===" << std::endl;

    EventReceiver receiver;
    receiver.setObjectName("AsyncReceiver");

    std::cout << "当前时间: "
             << QDateTime::currentDateTime().toString("hh:mm:ss.zzz").toStdString() << std::endl;

    // sendEvent() - 同步，立即处理
    std::cout << "\n[sendEvent] 同步发送事件" << std::endl;
    CustomEvent syncEvent("同步事件");
    QCoreApplication::sendEvent(&receiver, &syncEvent);
    std::cout << "[sendEvent] 返回，事件已处理" << std::endl;

    // postEvent() - 异步，加入队列
    std::cout << "\n[postEvent] 异步发送事件" << std::endl;
    // 注意：postEvent 只接受堆上分配的事件，Qt 会自动删除
    CustomEvent *asyncEvent = new CustomEvent("异步事件");
    QCoreApplication::postEvent(&receiver, asyncEvent);
    std::cout << "[postEvent] 返回，事件已加入队列，稍后处理" << std::endl;

    // 启动事件循环处理队列中的事件
    QTimer::singleShot(100, []() {
        std::cout << "[Timer] 事件循环处理完毕，退出" << std::endl;
        QCoreApplication::quit();
    });
}

// ============================================================================
// 主函数
// ============================================================================

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    // 运行各个示例
    demoBasicEventHandling();
    demoEventFilter();

    // postEvent 演示需要事件循环
    demoPostVsSendEvent();

    // 启动事件循环
    // 注意：在控制台应用中，事件循环需要 QTimer 或其他方式触发退出
    return app.exec();
}
