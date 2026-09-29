/// @file    realtime_chart.h
/// @brief   实时滚动折线图控件，演示 QtCharts 的动态数据更新与自动缩放。
///
/// 对应教程：进阶层 05-其他模块/03-QtCharts 进阶。
/// 核心知识点：QLineSeries 滚动窗口、QValueAxis 自动缩放、QTimer 驱动模拟数据。

#pragma once

#include <QChartView>
#include <QLineSeries>
#include <QTimer>
#include <QValueAxis>

#include <chrono>

/// @brief 实时传感器数据可视化控件。
///
/// 以固定频率生成模拟传感器数据（正弦波 + 高斯噪声），使用滚动时间窗口
/// 展示最近 kMaxPoints 个采样点。Y 轴随数据自动缩放，X 轴显示已流逝秒数。
class RealtimeChart : public QChartView
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化图表、坐标轴和定时器。
    /// @param[in] parent 父控件指针，Qt 对象树自动管理生命周期。
    explicit RealtimeChart(QWidget* parent = nullptr);

    /// @brief 启动数据生成定时器。
    /// @note 调用后定时器每 kUpdateIntervalMs 毫秒触发一次，追加新数据点。
    void start();

    /// @brief 停止数据生成定时器。
    void stop();

    /// @brief 查询当前是否正在采集数据。
    /// @return true 表示定时器正在运行。
    [[nodiscard]] bool isRunning() const;

private:
    /// @brief 定时器回调，生成一个模拟传感器采样值并追加到序列中。
    /// @note 内部完成三件事：生成数据 → 滚动窗口裁剪 → Y 轴自动缩放。
    void appendDataPoint();

    /// @brief 根据当前可见数据重新计算 Y 轴范围。
    /// @note 在最大值之上留 10% 的余量，避免折线紧贴上边界导致视觉不适。
    void rescaleYAxis();

    /// @brief 生成模拟传感器值：正弦波 + 均匀分布噪声。
    /// @return 当前时刻的模拟采样值。
    /// @note 使用 std::chrono 稳定时钟驱动正弦相位，保证数据连续可重复。
    [[nodiscard]] double generateSensorValue() const;

private:
    // -- 图表组件 --
    QLineSeries* m_series;   ///< 折线数据序列
    QValueAxis*  m_axisX;    ///< X 轴（时间，单位：秒）
    QValueAxis*  m_axisY;    ///< Y 轴（传感器数值）
    QTimer*      m_timer;    ///< 数据生成定时器

    // -- 采集状态 --
    int  m_elapsedCount = 0;   ///< 已追加的数据点计数
    bool m_running      = false; ///< 定时器是否正在运行

    // -- 常量 --
    static constexpr int    kMaxPoints       = 100;   ///< 滚动窗口最大数据点数
    static constexpr int    kUpdateIntervalMs = 100;  ///< 定时器间隔（毫秒）
    static constexpr double kNoiseAmplitude   = 0.3;  ///< 噪声幅度
    static constexpr double kYPaddingRatio    = 0.1;  ///< Y 轴上界余量比例（10%）
};
