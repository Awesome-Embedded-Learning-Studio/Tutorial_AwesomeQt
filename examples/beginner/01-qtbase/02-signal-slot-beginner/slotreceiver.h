#ifndef SLOTRECEIVER_H
#define SLOTRECEIVER_H

#include <QObject>
#include <QDebug>

// 演示槽函数的普通成员函数
class SlotReceiver : public QObject {
    Q_OBJECT

public:
    SlotReceiver(QObject *parent = nullptr) : QObject(parent), m_callCount(0) {}

public slots:
    // 槽函数：可以被信号连接调用
    void onValueChanged(int newValue) {
        m_callCount++;
        qDebug() << "[槽函数调用] 值改变为:" << newValue
                 << "(第" << m_callCount << "次)";
    }

    void onReset() {
        qDebug() << "[槽函数调用] 收到重置信号";
    }

    // 无参数槽函数
    void simpleSlot() {
        qDebug() << "[槽函数调用] 简单槽被调用";
    }

private:
    int m_callCount;  // 调用计数
};

#endif // SLOTRECEIVER_H
