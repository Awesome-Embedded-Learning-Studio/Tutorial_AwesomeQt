#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QString>

#include <atomic>

// ============================================================================
// Worker: 后台执行耗时任务，发射进度信号
// ============================================================================
class Worker : public QObject
{
    Q_OBJECT

public:
    explicit Worker(int totalSteps)
        : m_totalSteps(totalSteps)
        , m_cancel(false)
    {
    }

public slots:
    void doWork();

    void cancel()
    {
        m_cancel.store(true);
    }

signals:
    void progressChanged(int value);
    void labelChanged(const QString &text);
    void finished(bool completed, qint64 elapsedMs);

private:
    int m_totalSteps;
    std::atomic<bool> m_cancel;
};

#endif // WORKER_H
