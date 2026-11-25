---
title: "5.3 QtCharts/QtGraphs 进阶：实时数据更新与自定义 Axis"
description: "入门篇我们把 QtCharts 的基本图表创建跑通了——QLineSeries + QChart + QChartView，画一条静态折线图。做报表确实够用了。但如果你要做实时监控仪表盘——传感器数据每秒更新、曲线不停滚动、Y 轴自动缩放——入门篇的那套静态方案就扛不住了。"
---

# 现代Qt开发教程（进阶篇）5.3——QtCharts/QtGraphs 进阶：实时数据更新与自定义 Axis

## 1. 前言 / 从「静态报表」到「实时仪表盘」

入门篇我们把 QtCharts 的基本图表创建跑通了——QLineSeries + QChart + QChartView，画一条静态折线图。做报表确实够用了。但如果你要做实时监控仪表盘——传感器数据每秒更新、曲线不停滚动、Y 轴自动缩放——入门篇的那套静态方案就扛不住了。

实时图表的核心挑战是性能。如果你每次收到新数据点都调用 `chart->removeSeries()` 再 `chart->addSeries()`，整个图表会重新布局和渲染，CPU 和内存都会被吃光。正确做法是直接操作 Series 的数据，让 Chart 只做增量更新。

另一个挑战是坐标轴的自适应。静态图表可以预先算好 Y 轴范围，实时图表的数据范围是动态变化的。你需要一个策略来决定什么时候缩放 Y 轴、缩到什么范围、要不要留余量。

这篇我们一起来把实时数据流的高效更新、滚动时间轴、Y 轴自适应缩放这三个核心问题拆干净。

## 2. 环境说明

本文档基于 Qt 6.2+ 编写，使用 C++17 标准和 CMake 3.26+ 构建系统。本篇依赖 Qt6::Charts 模块，CMake 中通过 `find_package(Qt6 REQUIRED COMPONENTS Charts)` 引入。注意 Qt 6.7+ 推出了新的 QtGraphs 模块（基于 QML 的 GPU 加速图表），本篇以 QtCharts（Widgets 环境的经典方案）为主。

## 3. 核心概念讲解

### 3.1 高效更新 Series——不要重建，要追加

QtCharts 的 `QXYSeries`（`QLineSeries`、`QScatterSeries`、`QSplineSeries` 的基类）提供了两种更新数据的方式：`replace()` 全量替换和 `append()` / `remove()` 增量操作。对于实时数据流，增量操作的性能远优于全量替换。

```cpp
class RealtimeChart : public QWidget
{
    Q_OBJECT
public:
    RealtimeChart(QWidget *parent = nullptr) : QWidget(parent)
    {
        chart_ = new QChart();
        series_ = new QLineSeries();
        chart_->addSeries(series_);

        // 配置坐标轴
        axisX_ = new QValueAxis();
        axisX_->setTitleText("Time (s)");
        axisX_->setLabelFormat("%.1f");
        chart_->addAxis(axisX_, Qt::AlignBottom);
        series_->attachAxis(axisX_);

        axisY_ = new QValueAxis();
        axisY_->setTitleText("Value");
        axisY_->setLabelFormat("%.2f");
        chart_->addAxis(axisY_, Qt::AlignLeft);
        series_->attachAxis(axisY_);

        chart_->setTitle("Real-time Sensor Data");
        chart_->legend()->hide();

        auto *view = new QChartView(chart_, this);
        view->setRenderHint(QPainter::Antialiasing);

        auto *layout = new QVBoxLayout(this);
        layout->addWidget(view);

        // 模拟数据源：每 50ms 产生一个新数据点
        timer_.setInterval(50);
        connect(&timer_, &QTimer::timeout, this, &RealtimeChart::onNewData);
        timer_.start();
    }

private slots:
    void onNewData()
    {
        double x = elapsedTime_.elapsed() / 1000.0;
        double y = generateValue(x);  // 你的数据源

        series_->append(x, y);

        // 维护滑动窗口：只保留最近 60 秒的数据
        while (series_->count() > 0) {
            QPointF first = series_->at(0);
            if (first.x() < x - kWindowSeconds) {
                series_->remove(0);
            } else {
                break;
            }
        }

        // 更新 X 轴范围
        double xMin = qMax(0.0, x - kWindowSeconds);
        double xMax = x + 0.5;  // 留 0.5 秒的余量
        axisX_->setRange(xMin, xMax);

        // Y 轴自适应
        updateYAxisRange();
    }

private:
    void updateYAxisRange()
    {
        if (series_->count() == 0) return;

        double yMin = std::numeric_limits<double>::max();
        double yMax = std::numeric_limits<double>::lowest();

        for (int i = 0; i < series_->count(); ++i) {
            double y = series_->at(i).y();
            yMin = qMin(yMin, y);
            yMax = qMax(yMax, y);
        }

        // 加 10% 的余量
        double margin = (yMax - yMin) * 0.1;
        if (margin < 0.01) margin = 1.0;  // 避免范围太小
        axisY_->setRange(yMin - margin, yMax + margin);
    }

    QChart *chart_;
    QLineSeries *series_;
    QValueAxis *axisX_;
    QValueAxis *axisY_;
    QTimer timer_;
    QElapsedTimer elapsedTime_;

    static constexpr double kWindowSeconds = 60.0;
};
```

这里有几个关键性能优化点。第一个是用 `series_->remove(0)` 而不是 `series_->replace()`——`remove` 只移除一个点，触发增量渲染；`replace` 需要重新构建整个数据集。第二个是限制 Series 中的数据点数量——滑动窗口 60 秒，每 50ms 一个点就是 1200 个点，QtCharts 处理这个量级毫无压力。如果你保留所有历史数据（几万个点），渲染性能就会急剧下降。

### 3.2 多序列同步与 QDateTimeAxis

如果你的图表需要同时显示多条曲线（比如温度、湿度、气压），每条曲线用独立的 `QLineSeries`，但共享同一套坐标轴。QDateTimeAxis 比 QValueAxis 更适合时间轴——它直接显示日期时间格式。

```cpp
// 用 QDateTime 作为 X 轴
auto *axisTime = new QDateTimeAxis();
axisTime->setFormat("HH:mm:ss");
axisTime->setTitleText("Time");
chart_->addAxis(axisTime, Qt::AlignBottom);

// Series 使用 QDateTime.toMSecsSinceEpoch() 作为 X 值
QDateTime now = QDateTime::currentDateTime();
series_->append(now.toMSecsSinceEpoch(), value);
```

`QDateTimeAxis` 内部自动把毫秒时间戳转换为可读的时间格式显示在刻度上。你不需要手动计算刻度标签。

### 3.3 自定义 Axis 标签和样式

`QValueAxis` 提供了丰富的自定义选项：刻度数量、标签格式、网格线样式、颜色等。

```cpp
// 精细控制坐标轴
axisY_->setTickCount(5);         // 5 个主刻度
axisY_->setMinorTickCount(1);    // 每个主刻度间 1 个副刻度
axisY_->setLabelFormat("%.1f");  // 一位小数
axisY_->setGridLineVisible(true);
axisY_->setGridLineColor(QColor(200, 200, 200));
axisY_->setLinePen(QPen(Qt::darkGray, 1));
```

如果你需要更复杂的标签（比如显示单位），可以继承 `QValueAxis` 并重写相关方法，或者使用 `QCategoryAxis` 自定义离散的分类标签。

现在有一道思考题。你的实时图表在数据量增长到 5000 个点后开始卡顿，每帧渲染耗时超过 50ms。但你的代码已经用了增量更新而不是 replace。问题可能出在哪里？

最可能的原因是 `updateYAxisRange()` 中的遍历。每次新数据点到达你都遍历整个 Series 来计算 Y 轴范围——5000 个点遍历一次不算多，但如果你的更新频率很高（比如每 10ms 一次），每秒就要遍历 5000 个点 100 次。解决方案是维护一个滑动窗口的 min/max 变量，每次追加新点时更新 max/min，移除旧点时如果恰好移除了当前极值才重新计算。

## 4. 踩坑预防

第一个坑是 `append()` 触发全量重绘。默认情况下，每调用一次 `append()` 就触发一次 Chart 的重绘。如果你一次要追加 100 个点（比如从文件批量加载数据），就会触发 100 次重绘。解决方案是用 `QXYSeries::replace()` 一次性替换整个数据集，或者在批量操作前用 `chart_->setAnimationOptions(QChart::NoAnimation)` 关闭动画，操作完再恢复。

第二个坑是 QDateTimeAxis 的时区问题。`QDateTime::currentDateTime()` 返回的是本地时间，但 `toMSecsSinceEpoch()` 返回的是 UTC 时间戳。如果你的应用需要跨时区使用，确保所有时间都用 UTC 或者明确指定时区（`QDateTime::setTimeZone(QTimeZone::utc())`）。

## 5. 练习项目

练习项目：三通道实时示波器。三个 Series 分别显示正弦波、方波和随机噪声，每 20ms 更新一次。X 轴用 QDateTimeAxis 显示实时时间。Y 轴自动缩放。提供暂停/继续按钮。支持切换显示/隐藏各个通道。

完成标准：三条曲线同时实时滚动、暂停时数据停止更新但已显示的数据保留、Y 轴平滑缩放（不会频繁跳动）、运行 5 分钟以上无明显卡顿。

提示几个关键点：用三个独立的 QLineSeries，共享同一个 QDateTimeAxis。暂停时停止 QTimer 但不清理数据。Y 轴缩放加一个平滑因子：`newRange = 0.9 * oldRange + 0.1 * calculatedRange`，避免频繁跳动。

## 6. 官方文档参考链接

[Qt 文档 · QChart](https://doc.qt.io/qt-6/qchart.html) -- 图表核心类，包含布局和坐标轴管理

[Qt 文档 · QLineSeries](https://doc.qt.io/qt-6/qlineseries.html) -- 折线序列，包含 append/replace/remove 数据操作

[Qt 文档 · QValueAxis](https://doc.qt.io/qt-6/qvalueaxis.html) -- 数值坐标轴，包含刻度和标签配置

[Qt 文档 · QDateTimeAxis](https://doc.qt.io/qt-6/qdatetimeaxis.html) -- 日期时间坐标轴，自动格式化时间标签

*（链接已验证，2026-06-11 可访问）*

---

到这里就大功告成了。增量更新、滑动窗口、自适应坐标轴——有了这三个核心技能，你的 QtCharts 就能从「静态报表」升级为「实时仪表盘」了。
