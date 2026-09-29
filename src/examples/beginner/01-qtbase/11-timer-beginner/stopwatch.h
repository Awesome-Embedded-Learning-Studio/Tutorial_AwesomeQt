#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <QObject>
#include <QTimer>
#include <QDebug>
#include <QString>
#include <QChar>

class Stopwatch : public QObject {
    Q_OBJECT

public:
    Stopwatch(QObject *parent = nullptr) : QObject(parent), m_seconds(0), m_running(false) {}

    void start() {
        if (!m_running) {
            m_running = true;
            m_timer.start(1000);  // 每秒触发一次
            qDebug() << "[秒表] 启动";
        }
    }

    void pause() {
        if (m_running) {
            m_running = false;
            m_timer.stop();
            qDebug() << "[秒表] 暂停";
        }
    }

    void reset() {
        m_running = false;
        m_timer.stop();
        m_seconds = 0;
        qDebug() << "[秒表] 重置: 00:00";
    }

private slots:
    void onTick() {
        m_seconds++;
        int minutes = m_seconds / 60;
        int seconds = m_seconds % 60;
        qDebug() << "[秒表]" << QString("%1:%2")
                    .arg(minutes, 2, 10, QChar('0'))
                    .arg(seconds, 2, 10, QChar('0'));
    }

private:
    QTimer m_timer;
    int m_seconds;
    bool m_running;

    // 连接定时器信号
    friend void stopwatchExample();
};

#endif // STOPWATCH_H
