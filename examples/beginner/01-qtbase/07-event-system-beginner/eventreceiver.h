#ifndef EVENTRECEIVER_H
#define EVENTRECEIVER_H

#include <QObject>
#include <QEvent>
#include <QDateTime>
#include <iostream>

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

#endif // EVENTRECEIVER_H
