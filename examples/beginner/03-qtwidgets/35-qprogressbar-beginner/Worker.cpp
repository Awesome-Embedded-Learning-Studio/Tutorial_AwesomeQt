#include "Worker.h"

#include <QThread>

/// @brief 执行耗时操作（在 QThread 中运行）
void Worker::doWork()
{
    m_cancelRequested.store(false);

    for (int i = 0; i <= m_totalCount; ++i) {
        // 检查取消标志
        if (m_cancelRequested.load()) {
            emit canceled(i);
            return;
        }

        // 模拟耗时操作
        QThread::msleep(80);

        // 发送进度信号（百分比）
        int percent = static_cast<int>(
            100.0 * i / m_totalCount);
        emit progressChanged(percent);

        // 发送详细进度（当前值 / 总值）
        emit detailChanged(i, m_totalCount);
    }

    emit finished();
}
