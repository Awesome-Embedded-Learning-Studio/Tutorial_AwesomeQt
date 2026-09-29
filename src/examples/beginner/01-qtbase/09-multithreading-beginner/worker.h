#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QString>

/**
 * Worker 类：在后台线程执行耗时任务
 *
 * 设计要点：
 * - 继承 QObject（不是 QThread）
 * - 用信号槽进行跨线程通信
 * - 任务完成后发送信号通知主线程
 */
class Worker : public QObject
{
    Q_OBJECT  // 启用 Qt 信号槽机制

public:
    explicit Worker(QObject *parent = nullptr) : QObject(parent) {}

    /**
     * 执行耗时任务的槽函数
     * 这个函数会在 moveToThread 后的新线程中执行
     */
public slots:
    void doWork(const QString &taskName) {
        qDebug() << "[" << QThread::currentThreadId() << "] Worker开始任务:" << taskName;

        // 模拟耗时操作（5秒）
        for (int i = 1; i <= 5; ++i) {
            QThread::sleep(1);  // 休眠1秒

            // 发送进度信号（跨线程，线程安全）
            emit progressChanged(taskName, i * 20);
        }

        // 任务完成，发送结果信号
        emit workFinished(taskName, "任务完成！");
    }

signals:
    /**
     * 进度变化信号
     * 跨线程发送时，Qt 会自动使用队列连接，保证线程安全
     */
    void progressChanged(const QString &taskName, int percent);

    /**
     * 任务完成信号
     * 可以携带结果数据，Qt 会自动拷贝参数到事件队列
     */
    void workFinished(const QString &taskName, const QString &result);
};

#endif // WORKER_H
