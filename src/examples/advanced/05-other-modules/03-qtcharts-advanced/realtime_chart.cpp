/// @file    realtime_chart.cpp
/// @brief   RealtimeChart 类实现——实时滚动折线图的数据生成与坐标轴缩放。
///
/// 对应教程：进阶层 05-其他模块/03-QtCharts 进阶。

#include "realtime_chart.h"

#include <chrono>
#include <cmath>

#include <QtCharts/QLegend>

// ---- 构造与析构 ----

RealtimeChart::RealtimeChart(QWidget* parent)
    : QChartView(parent)
    , m_series{new QLineSeries()}
    , m_axisX{new QValueAxis()}
    , m_axisY{new QValueAxis()}
    , m_timer{new QTimer(this)}
{
    // -- 图表基本配置 --
    auto* chart = this->chart();
    chart->addSeries(m_series);
    chart->setTitle(QStringLiteral("实时传感器数据"));
    chart->legend()->hide();
    chart->setTheme(QChart::ChartThemeDark);

    // -- X 轴：时间（秒），固定范围会在 appendDataPoint 中动态调整 --
    chart->addAxis(m_axisX, Qt::AlignBottom);
    m_axisX->setTitleText(QStringLiteral("时间 (s)"));
    m_axisX->setRange(0, kMaxPoints * kUpdateIntervalMs / 1000.0);
    m_series->attachAxis(m_axisX);

    // -- Y 轴：传感器数值，初始范围 -2 ~ 2，后续自动缩放 --
    chart->addAxis(m_axisY, Qt::AlignLeft);
    m_axisY->setTitleText(QStringLiteral("数值"));
    m_axisY->setRange(-2.0, 2.0);
    m_series->attachAxis(m_axisY);

    // -- 抗锯齿，提升折线渲染质量 --
    setRenderHint(QPainter::Antialiasing);

    // -- 定时器信号槽连接 --
    // @note 使用 Qt::ConnectionType 默认值（自动队列），因为定时器与控件在同一线程
    connect(m_timer, &QTimer::timeout, this, &RealtimeChart::appendDataPoint);
}

// ---- 公有接口 ----

void RealtimeChart::start()
{
    if (m_running) {
        return;
    }
    m_running = true;
    m_timer->start(kUpdateIntervalMs);
}

void RealtimeChart::stop()
{
    if (!m_running) {
        return;
    }
    m_running = false;
    m_timer->stop();
}

[[nodiscard]] bool RealtimeChart::isRunning() const
{
    return m_running;
}

// ---- 私有方法 ----

void RealtimeChart::appendDataPoint()
{
    const double value = generateSensorValue();
    const double elapsedSec = static_cast<double>(m_elapsedCount)
                              * kUpdateIntervalMs / 1000.0;

    m_series->append(elapsedSec, value);
    ++m_elapsedCount;

    // -- 滚动窗口：超出 kMaxPoints 时移除最早的数据点 --
    // @note QLineSeries::remove(0) 每次移除第 0 个点，时间复杂度 O(n)；
    //       对于 100 点规模完全可以接受，更大规模需考虑 replace 策略。
    if (m_series->count() > kMaxPoints) {
        m_series->remove(0);
    }

    // -- 更新 X 轴范围：跟随滚动窗口平移 --
    const auto pointCount = m_series->count();
    if (pointCount >= kMaxPoints) {
        // 窗口已满，X 轴左侧跟随最早数据点
        const double xMin = m_series->at(0).x();
        const double xMax = m_series->at(pointCount - 1).x();
        m_axisX->setRange(xMin, xMax);
    } else {
        // 窗口尚未填满，X 轴右侧继续扩展
        m_axisX->setRange(0.0, elapsedSec);
    }

    // -- Y 轴自动缩放 --
    rescaleYAxis();
}

void RealtimeChart::rescaleYAxis()
{
    if (m_series->count() == 0) {
        return;
    }

    double yMin = m_series->at(0).y();
    double yMax = m_series->at(0).y();

    // 遍历当前窗口内所有点，寻找极值
    const auto count = m_series->count();
    for (int i = 1; i < count; ++i) {
        const double y = m_series->at(i).y();
        if (y < yMin) {
            yMin = y;
        }
        if (y > yMax) {
            yMax = y;
        }
    }

    // 在最大值之上留 kYPaddingRatio 的余量
    const double padding = std::fabs(yMax) * kYPaddingRatio;
    const double lowerPadding = std::fabs(yMin) * kYPaddingRatio;
    m_axisY->setRange(yMin - lowerPadding, yMax + padding);
}

[[nodiscard]] double RealtimeChart::generateSensorValue() const
{
    // 使用稳态时钟的纳秒计数驱动正弦波，保证连续性
    const auto now = std::chrono::steady_clock::now();
    const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                       now.time_since_epoch())
                       .count();
    const double phase = static_cast<double>(ns) * 1e-9;

    // 正弦基波 + 均匀分布伪噪声
    // @note 使用简单的线性同余法生成伪随机数，避免引入 <random> 的开销；
    //       教学演示足够，生产环境请使用 std::mt19937 等引擎。
    const double noise = kNoiseAmplitude
                         * std::sin(phase * 137.5)  // 高频正弦模拟噪声
                         * std::cos(phase * 251.3);
    return std::sin(phase * 2.0) + noise;
}
