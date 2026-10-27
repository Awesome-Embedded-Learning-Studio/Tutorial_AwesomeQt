#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QString>

// 演示跨线程信号槽的工作类
class Worker : public QObject {
    Q_OBJECT

public:
    Worker(QObject *parent = nullptr) : QObject(parent) {}

public slots:
    void doWork() {
        QThread *thread = QThread::currentThread();
        qDebug() << "[工作线程] doWork 在线程" << thread << "中执行";
        emit workFinished("工作完成！");
    }

signals:
    void workFinished(const QString &result);
};

#endif // WORKER_H
