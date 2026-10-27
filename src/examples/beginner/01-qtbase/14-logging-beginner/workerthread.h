#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QThread>
#include <QDebug>
#include <QLoggingCategory>
#include <QString>

// 主应用日志类别
Q_DECLARE_LOGGING_CATEGORY(mainLog)

/**
 * 工作线程类：演示线程安全的日志输出
 */
class WorkerThread : public QThread {
    Q_OBJECT

public:
    WorkerThread(const QString &name, QObject *parent = nullptr)
        : QThread(parent), m_name(name) {}

protected:
    void run() override {
        qCDebug(mainLog) << "工作线程" << m_name << "启动，线程ID:"
                         << QString::number(quintptr(QThread::currentThreadId()), 16);

        // 在循环中输出日志
        for (int i = 0; i < 3; ++i) {
            qCDebug(mainLog) << "[" << m_name << "] 处理任务" << (i + 1);
            QThread::msleep(50);
        }

        qCDebug(mainLog) << "工作线程" << m_name << "完成";
    }

private:
    QString m_name;
};

#endif // WORKERTHREAD_H
