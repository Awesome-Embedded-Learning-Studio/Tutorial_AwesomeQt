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

// ============================================================================
// 示例 1: 基础事件处理 - 重写 event() 方法
// ============================================================================

/**
 * MyEventHandler 演示如何重写 event() 方法处理事件
 * 这个类会处理键盘和定时器事件
 *
 * 注意：keyPressEvent() 和 keyReleaseEvent() 是 QWidget 的方法，
 * 对于纯 QObject 派生类，需要在 event() 中直接处理 KeyPress 事件
 */
class MyEventHandler : public QObject {
    Q_OBJECT  // 启用 Qt 元对象系统
public:
    explicit MyEventHandler(QObject *parent = nullptr) : QObject(parent) {
        // 设置一个标志，用于演示 accept() 和 ignore()
        m_acceptEvents = true;
    }

    /**
     * 重写通用事件处理函数 event()
     * 这是所有事件的入口点，根据事件类型分发到具体处理逻辑
     */
    bool event(QEvent *event) override {
        // 处理定时器事件
        if (event->type() == QEvent::Timer) {
            std::cout << "[event()] 捕获到定时器事件" << std::endl;
            return true;  // 返回 true 表示事件已处理
        }

        // 处理键盘按下事件
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            QString keyText = keyEvent->text();
            int key = keyEvent->key();

            std::cout << "[event() KeyPress] 按键: " << keyText.toStdString()
                     << " 键码: " << key
                     << " 修饰键: " << keyEvent->modifiers() << std::endl;

            // 演示 accept() 和 ignore() 的作用
            if (m_acceptEvents) {
                event->accept();
                std::cout << "  -> 事件已 accept()，处理结束" << std::endl;
            } else {
                event->ignore();
                std::cout << "  -> 事件已 ignore()，将继续传播" << std::endl;
            }
            return true;  // 已处理此事件
        }

        // 处理键盘释放事件
        if (event->type() == QEvent::KeyRelease) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            std::cout << "[event() KeyRelease] 释放按键: " << keyEvent->text().toStdString() << std::endl;
            event->accept();
            return true;
        }

        // 其他事件交给基类处理
        return QObject::event(event);
    }

    void setAcceptEvents(bool accept) { m_acceptEvents = accept; }

private:
    bool m_acceptEvents;
};

// ============================================================================
// 示例 2: 事件过滤器 - 监听其他对象的事件
// ============================================================================

/**
 * KeyLogger 演示如何用事件过滤器拦截其他对象的事件
 * 事件过滤器比继承更灵活，可以动态安装/卸载
 */
class KeyLogger : public QObject {
    Q_OBJECT
public:
    explicit KeyLogger(QObject *parent = nullptr) : QObject(parent), m_enabled(true) {}

    /**
     * eventFilter() 是事件过滤器的核心函数
     * @param watched 被监听的对象
     * @param event 发生的事件
     * @return true 表示拦截事件，false 表示继续传播
     */
    bool eventFilter(QObject *watched, QEvent *event) override {
        // 如果过滤器被禁用，直接让事件通过
        if (!m_enabled) {
            return QObject::eventFilter(watched, event);
        }

        // 只处理键盘按下事件
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

            std::cout << "[EventFilter] 拦截到来自 "
                     << watched->objectName().toStdString()
                     << " 的按键: " << keyEvent->text().toStdString() << std::endl;

            // 演示：拦截 ESC 键，不让被监听对象处理
            if (keyEvent->key() == Qt::Key_Escape) {
                std::cout << "  -> ESC 键被拦截，目标对象不会收到此事件" << std::endl;
                return true;  // 返回 true 拦截事件
            }

            // 其他键让被监听对象处理
            return false;  // 返回 false 继续传播
        }

        // 其他类型事件不做处理，交给基类
        return QObject::eventFilter(watched, event);
    }

    void setEnabled(bool enabled) { m_enabled = enabled; }

private:
    bool m_enabled;
};

// ============================================================================
// 示例 3: postEvent vs sendEvent 的区别
// ============================================================================

/**
 * EventReceiver 演示同步和异步事件处理的区别
 */
class EventReceiver : public QObject {
    Q_OBJECT
public:
    explicit EventReceiver(QObject *parent = nullptr) : QObject(parent) {
        setObjectName("EventReceiver");
    }

    bool event(QEvent *event) override {
        // 捕获自定义事件类型
        if (event->type() == QEvent::User) {
            std::cout << "[Receiver] 收到 User 事件，当前时间: "
                     << QDateTime::currentDateTime().toString("hh:mm:ss.zzz").toStdString() << std::endl;
            return true;
        }
        return QObject::event(event);
    }
};

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

// 由于使用了 Q_OBJECT 宏，这个文件需要通过 MOC 处理
// CMake 会自动处理（设置 CMAKE_AUTOMOC ON）
#include "main.moc"  // 必须包含，让 MOC 生成的代码能被正确链接
