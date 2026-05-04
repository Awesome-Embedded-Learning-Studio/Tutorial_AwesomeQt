#ifndef COUNTER_H
#define COUNTER_H

#include <QObject>
#include <QString>

// 自定义计数器类：演示信号声明和发射
class Counter : public QObject {
    Q_OBJECT  // 必须有此宏才能使用信号槽

public:
    Counter(QObject *parent = nullptr) : QObject(parent), m_value(0) {}

    // 增加计数值并发射信号
    void increment() {
        ++m_value;
        // emit 关键字发射信号，传递新值
        emit valueChanged(m_value);
    }

    // 重置计数值
    void reset() {
        m_value = 0;
        emit valueChanged(m_value);
    }

    // 获取当前值
    int value() const { return m_value; }

signals:
    // 信号声明：告诉系统这个类可能发出这个信号
    // 信号只需要声明，不需要实现
    void valueChanged(int newValue);  // 参数：新值

    // 无参数信号示例
    void resetSignal();

private:
    int m_value;  // 计数值
};

#endif // COUNTER_H
