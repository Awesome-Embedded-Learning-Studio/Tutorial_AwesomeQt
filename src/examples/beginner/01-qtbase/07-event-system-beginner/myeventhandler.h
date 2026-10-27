#ifndef MYEVENTHANDLER_H
#define MYEVENTHANDLER_H

#include <QObject>
#include <QEvent>
#include <QKeyEvent>
#include <QString>
#include <iostream>
#include <string>

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

#endif // MYEVENTHANDLER_H
