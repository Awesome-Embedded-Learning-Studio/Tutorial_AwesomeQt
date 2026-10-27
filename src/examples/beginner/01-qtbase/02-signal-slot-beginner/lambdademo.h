#ifndef LAMBDADEMO_H
#define LAMBDADEMO_H

#include <QObject>
#include <QTimer>
#include <QDebug>
#include <QString>

// 演示 Lambda 捕获上下文
class LambdaDemo : public QObject {
    Q_OBJECT

public:
    LambdaDemo(QObject *parent = nullptr) : QObject(parent), m_counter(0) {}

    void setupConnections(QTimer *timer) {
        // Lambda 作为槽：捕获 this 指针访问成员变量
        connect(timer, &QTimer::timeout, this, [this]() {
            m_counter++;
            qDebug() << "[Lambda] 定时器触发，计数:" << m_counter;
        });

        // Lambda 捕获局部变量
        QString prefix = "定时器";
        connect(timer, &QTimer::timeout, [prefix]() {
            qDebug() << "[Lambda]" << prefix << "持续运行中...";
        });
    }

private:
    int m_counter;
};

#endif // LAMBDADEMO_H
