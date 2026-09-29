/// @file    precision_demo.h
/// @brief   演示 QTimer 三种定时器精度类型，使用 QElapsedTimer 进行测量。
///
/// 对应教程：进阶层 01-QtBase/11-QTimer 高级。
///
/// Qt 提供三种定时器精度：
/// - Qt::PreciseTimer:    毫秒级精度，使用系统高精度定时器，耗电更多
/// - Qt::CoarseTimer:     默认精度，可能提前或延后约 5%，省电
/// - Qt::VeryCoarseTimer: 极粗精度，可能偏差达到几百毫秒，最省电

#pragma once

#include <QList>
#include <QString>
#include <Qt>

/// @brief 定时器精度测试结果，用于汇总单次测试的统计数据。
struct TimerPrecisionResult {
    Qt::TimerType timerType;         ///< 定时器类型
    QString name;                    ///< 类型名称
    qint64 targetIntervalMs;         ///< 目标间隔（ms）
    QList<qint64> actualIntervals;   ///< 实际间隔记录（ms）
    qint64 avgDeviation;             ///< 平均偏差（ms）
    qint64 maxDeviation;             ///< 最大偏差（ms）
};

/// @brief 执行单种定时器类型的精度测试。
///
/// 使用 QElapsedTimer 记录每次 timeout 信号的实际触发时间，
/// 然后与目标间隔进行比较，计算偏差统计信息。
///
/// @param[in] timerType   定时器精度类型。
/// @param[in] name        类型名称（用于输出）。
/// @param[in] intervalMs  目标间隔（毫秒）。
/// @param[in] count       测试次数。
/// @return 精度测试结果结构体。
TimerPrecisionResult testTimerPrecision(
    Qt::TimerType timerType, const QString& name,
    int intervalMs, int count);

/// @brief 打印精度测试结果到调试输出。
/// @param[in] result 待打印的精度测试结果。
void printPrecisionResult(const TimerPrecisionResult& result);

/// @brief 演示三种定时器精度对比。
///
/// 使用相同的间隔（50ms）分别测试三种精度类型，对比偏差。
/// 在桌面系统上差异可能不明显，但在嵌入式或低功耗场景下差异会更大。
void demoTimerPrecisionComparison();

/// @brief 演示不同间隔下的精度表现。
///
/// 在较长间隔（如 1000ms）下，偏差比例更小；
/// 在短间隔（如 10ms）下，CoarseTimer 和 VeryCoarseTimer 的偏差比例更明显。
void demoIntervalVsPrecision();
