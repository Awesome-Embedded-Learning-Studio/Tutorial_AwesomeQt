#include "worker.h"

#include <QElapsedTimer>
#include <QThread>

// Worker::doWork
void Worker::doWork()
{
    QElapsedTimer timer;
    timer.start();

    for (int i = 0; i <= m_totalSteps; ++i) {
        if (m_cancel.load()) {
            emit finished(false,
                          timer.elapsed());
            return;
        }

        // 根据阶段发射不同的描述文字
        QString label;
        if (i <= 50) {
            label = "正在读取数据...";
        } else if (i <= 120) {
            label = "正在解析数据...";
        } else if (i <= 180) {
            label = "正在计算结果...";
        } else {
            label = "正在写入...";
        }

        emit labelChanged(label);
        emit progressChanged(i);

        // 模拟每步耗时 30ms
        QThread::msleep(30);
    }

    emit finished(true, timer.elapsed());
}
