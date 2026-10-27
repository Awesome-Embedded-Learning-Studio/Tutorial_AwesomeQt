/// @file    precision_demo.cpp
/// @brief   precision_demo.h 中所有函数的实现。
///
/// 对应教程：进阶层 01-QtBase/11-QTimer 高级。

#include "precision_demo.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QTimer>

TimerPrecisionResult testTimerPrecision(
    Qt::TimerType timerType, const QString& name,
    int intervalMs, int count)
{
    TimerPrecisionResult result;
    result.timerType = timerType;
    result.name = name;
    result.targetIntervalMs = intervalMs;

    QTimer timer;
    timer.setInterval(intervalMs);
    timer.setTimerType(timerType);  // 设置要测试的精度类型

    QElapsedTimer elapsed;
    elapsed.start();

    int triggerCount = 0;
    qint64 lastElapsed = 0;

    QEventLoop loop;

    // 连接 timeout 信号，记录每次触发的实际时间
    QObject::connect(&timer, &QTimer::timeout, &timer, [&]() {
        qint64 currentElapsed = elapsed.elapsed();

        // 从第二次触发开始记录间隔（第一次没有前一次作为参考）
        if (triggerCount > 0) {
            qint64 actualInterval = currentElapsed - lastElapsed;
            result.actualIntervals.append(actualInterval);
        }
        lastElapsed = currentElapsed;
        triggerCount++;

        if (triggerCount >= count + 1) {  // +1 因为第一次不记录
            timer.stop();
            loop.quit();
        }
    });

    timer.start();
    loop.exec();

    // 计算偏差统计
    if (!result.actualIntervals.isEmpty()) {
        qint64 totalDeviation = 0;
        result.maxDeviation = 0;
        for (qint64 actual : result.actualIntervals) {
            qint64 deviation = qAbs(actual - intervalMs);
            totalDeviation += deviation;
            result.maxDeviation = qMax(result.maxDeviation, deviation);
        }
        result.avgDeviation = totalDeviation / result.actualIntervals.size();
    }

    return result;
}

void printPrecisionResult(const TimerPrecisionResult& result)
{
    qDebug() << "\n[精度测试]" << result.name;
    qDebug() << "  目标间隔:" << result.targetIntervalMs << "ms";
    qDebug() << "  触发次数:" << result.actualIntervals.size();
    qDebug() << "  平均偏差:" << result.avgDeviation << "ms";
    qDebug() << "  最大偏差:" << result.maxDeviation << "ms";

    // 打印前 5 次的实际间隔（如果有的话）
    int printCount = qMin(5, result.actualIntervals.size());
    if (printCount > 0) {
        qDebug() << "  前" << printCount << "次实际间隔:"
                 << result.actualIntervals.mid(0, printCount);
    }
}

void demoTimerPrecisionComparison()
{
    qDebug() << "\n--- QTimer 三种精度对比演示 ---";
    qDebug() << "[精度对比] 使用 50ms 间隔，每种类型测试 10 次\n";

    // 测试 PreciseTimer（高精度）
    TimerPrecisionResult precise = testTimerPrecision(
        Qt::PreciseTimer, "PreciseTimer（高精度）", 50, 10);
    printPrecisionResult(precise);

    // 测试 CoarseTimer（默认精度）
    TimerPrecisionResult coarse = testTimerPrecision(
        Qt::CoarseTimer, "CoarseTimer（默认精度）", 50, 10);
    printPrecisionResult(coarse);

    // 测试 VeryCoarseTimer（极粗精度）
    TimerPrecisionResult veryCoarse = testTimerPrecision(
        Qt::VeryCoarseTimer, "VeryCoarseTimer（极粗精度）", 50, 10);
    printPrecisionResult(veryCoarse);

    // 总结对比
    qDebug() << "\n[精度对比] 偏差总结:";
    qDebug() << "  PreciseTimer  平均偏差:" << precise.avgDeviation << "ms";
    qDebug() << "  CoarseTimer   平均偏差:" << coarse.avgDeviation << "ms";
    qDebug() << "  VeryCoarseTimer 平均偏差:" << veryCoarse.avgDeviation << "ms";
    qDebug() << "\n[精度对比] 选择建议:";
    qDebug() << "  - 音视频/游戏/实时控制 -> PreciseTimer";
    qDebug() << "  - 一般 UI 刷新/后台任务 -> CoarseTimer（默认）";
    qDebug() << "  - 低频后台检查（如电池监控）-> VeryCoarseTimer";
}

void demoIntervalVsPrecision()
{
    qDebug() << "\n--- 不同间隔下的精度表现 ---";

    QList<int> intervals = {10, 50, 200, 1000};

    for (int interval : intervals) {
        qDebug() << "\n[间隔测试] 间隔:" << interval << "ms";

        TimerPrecisionResult result = testTimerPrecision(
            Qt::CoarseTimer,
            QString("CoarseTimer @ %1ms").arg(interval),
            interval, 8);

        qDebug() << "  平均偏差:" << result.avgDeviation << "ms"
                 << "（偏差率:" << (result.avgDeviation * 100.0 / interval) << "%）";
    }

    qDebug() << "\n[间隔测试] 结论：间隔越大，相对偏差率越小";
}
