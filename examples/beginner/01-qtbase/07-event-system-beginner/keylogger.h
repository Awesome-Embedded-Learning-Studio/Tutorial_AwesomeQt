#ifndef KEYLOGGER_H
#define KEYLOGGER_H

#include <QObject>
#include <QEvent>
#include <QKeyEvent>
#include <iostream>
#include <string>

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

#endif // KEYLOGGER_H
