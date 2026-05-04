# 现代Qt开发教程（新手篇）5.3——QtCharts 图表基础

## 1. 前言：数据可视化不是锦上添花

做应用做到一定程度，一定会遇到"把数据画出来"的需求。你可能觉得画图表是锦上添花的事情，但事实上，一行数字和一张折线图给人的信息量完全不在一个量级。当你的程序需要展示销售趋势、系统监控、性能分析这类数据的时候，图表不是可选的，是必须的。

Qt 从 5.7 开始把 QtCharts 纳入了官方模块，Qt 6 里它已经是一个非常成熟的图表方案了。折线图、柱状图、饼图、散点图、面积图——常见的图表类型全覆盖，而且自带主题切换和动画效果。这篇我们要做的是把 QtCharts 的核心 API 摸清楚，从最简单的折线图开始，一步步加上坐标轴、多系列、主题和交互。

## 2. 环境说明

本篇基于 Qt 6.9.1，需要引入 `Qt6::Charts` 和 `Qt6::Widgets` 两个模块。QtCharts 在 Qt 6 中属于独立模块，CMake 配置需要 `find_package(Qt6 REQUIRED COMPONENTS Charts Widgets)`。编译工具链方面，MSVC 2019+、GCC 11+ 均可，C++17 标准，CMake 3.26+ 构建系统。QtCharts 底层使用 Qt 的 Graphics View Framework 渲染，所以它的性能对于几千个数据点以内是完全够用的。如果你的数据量达到十万级以上，那需要考虑 Qt Graphs（Qt 6.8+ 引入的新模块，基于 GPU 渲染），但那是进阶话题了。

## 3. 核心概念讲解

### 3.1 第一张图表——QChart + QChartView

QtCharts 的核心架构非常清晰：`QChart` 是图表的数据容器和绘制引擎，`QChartView` 是用来显示 `QChart` 的 Widget。你可以把 `QChart` 理解成画布，`QChartView` 理解成画框——数据都挂在画布上，画框负责把画布放到你的界面里。

先来画一张最简单的折线图：

```cpp
#include <QChartView>
#include <QLineSeries>

// 创建数据系列
QLineSeries *series = new QLineSeries();
series->append(0, 6);
series->append(1, 4);
series->append(2, 8);
series->append(3, 5);
series->append(4, 9);
series->append(5, 7);

// 创建图表并添加系列
QChart *chart = new QChart();
chart->addSeries(series);
chart->setTitle("简单的折线图");

// 让 Qt 自动创建默认坐标轴
chart->createDefaultAxes();

// 创建视图并显示
QChartView *chartView = new QChartView(chart);
chartView->setRenderHint(QPainter::Antialiasing);  // 抗锯齿
chartView->show();
```

这段代码会在窗口中显示一张折线图，X 轴从 0 到 5，Y 轴根据数据自动调整范围。`setRenderHint(QPainter::Antialiasing)` 开启抗锯齿渲染，线条会更平滑——不开的话折线图会出现锯齿，看着很粗糙。这个设置几乎是必须的，建议每次创建 `QChartView` 都加上。

`createDefaultAxes()` 是一个便捷方法，它会根据已有数据系列的 X 值和 Y 值范围自动创建 `QValueAxis` 作为坐标轴。对于快速原型开发很方便，但对于正式产品，你通常需要手动创建坐标轴以获得更精确的控制——设置刻度间隔、标签格式、范围等等。后面我们会详细讲手动配置坐标轴。

### 3.2 三种基础图表类型

QtCharts 提供了多种 Series 类型，最常用的有三种：`QLineSeries` 折线图、`QBarSeries` 柱状图、`QPieSeries` 饼图。

折线图适合展示趋势变化，比如一周内的温度走势、一个月内的销售额变化。数据点之间会用直线连接，视觉上强调"连续性"：

```cpp
QLineSeries *series = new QLineSeries();
series->setName("月度销售额");
series->append(1, 120);   // 1 月，120 万
series->append(2, 135);
series->append(3, 148);
series->append(4, 162);
series->append(5, 155);
series->append(6, 178);
```

`append()` 的两个参数分别是 X 值和 Y 值。你也可以用 `<<` 操作符：`*series << QPointF(1, 120) << QPointF(2, 135)`，效果完全一样，写起来更紧凑。

柱状图适合展示分类对比，比如各部门的人数、各季度的营收。`QBarSeries` 需要配合 `QBarSet` 使用——`QBarSet` 代表一组数据，`QBarSeries` 负责把这组数据画成柱子：

```cpp
QBarSet *set0 = new QBarSet("Engineering");
QBarSet *set1 = new QBarSet("Marketing");

*set0 << 10 << 12 << 15 << 14;  // 四个季度的数据
*set1 << 8 << 9 << 11 << 10;

QBarSeries *series = new QBarSeries();
series->append(set0);
series->append(set1);
```

多组 `QBarSet` 会自动并排显示。如果你想让它们堆叠显示（比如显示总量），用 `QStackedBarSeries` 代替 `QBarSeries` 就行，API 完全一样。

饼图适合展示占比关系，比如市场份额、支出构成。`QPieSeries` 的数据添加方式稍有不同——你添加的是"切片"（slice），每个切片有一个标签和一个数值：

```cpp
QPieSeries *series = new QPieSeries();
series->append("Engineering", 40);   // 标签 + 数值
series->append("Marketing", 25);
series->append("Sales", 20);
series->append("Support", 15);

// 突出显示某个切片
series->slices().at(0)->setExploded(true);  // Engineering 突出
```

`setExploded(true)` 会让某个切片从饼图中"弹出来"一段距离，通常用来强调关键数据。数值是原始数据，不是百分比——Qt 会自动计算每个切片占总和的比例。

### 3.3 坐标轴配置——QValueAxis 与 QBarCategoryAxis

前面用了 `createDefaultAxes()` 自动生成坐标轴，但实际项目中你需要更精细的控制。QtCharts 提供了几种坐标轴类型：`QValueAxis` 用于连续数值（折线图、散点图的 X/Y 轴），`QBarCategoryAxis` 用于分类标签（柱状图的 X 轴），`QCategoryAxis` 用于自定义范围的分类轴，`QDateTimeAxis` 用于时间轴。

手动创建坐标轴的方式：

```cpp
// Y 轴：数值轴
QValueAxis *axisY = new QValueAxis();
axisY->setRange(0, 200);        // 范围 0 到 200
axisY->setTickCount(5);         // 5 个刻度（含首尾，所以间隔是 50）
axisY->setLabelFormat("%d");    // 整数格式
axisY->setTitleText("销售额（万元）");

chart->addAxis(axisY, Qt::AlignLeft);
series->attachAxis(axisY);      // 把系列关联到坐标轴
```

两个步骤缺一不可：`chart->addAxis()` 把坐标轴添加到图表上（同时指定它靠哪一侧对齐），`series->attachAxis()` 把数据系列关联到坐标轴。如果你只做了第一步没做第二步，坐标轴会显示但数据系列的缩放不会跟它同步——图表可能出现数据跑到画面外面或者缩放不正确的情况。

柱状图的 X 轴通常需要分类标签：

```cpp
QBarCategoryAxis *axisX = new QBarCategoryAxis();
axisX->append(QStringList() << "Q1" << "Q2" << "Q3" << "Q4");
axisX->setTitleText("季度");

chart->addAxis(axisX, Qt::AlignBottom);
barSeries->attachAxis(axisX);
```

`QBarCategoryAxis` 会自动把柱子和分类标签对齐，每个分类对应一组柱子。

这里有个容易踩的坑：当你往 `QChart` 里添加多个系列时，每个系列都必须 attach 到对应的坐标轴。如果你添加了第二条折线但忘了 attach，新折线会使用默认的坐标轴映射，和第一条线的坐标轴不一致，图表就会变得一团糟。记住规则：有几个系列就 attach 几次，每添加一个新系列都要检查它是否关联了正确的坐标轴。

### 3.4 多系列叠加

一张图表上显示多条数据线是很常见的需求——比如同一个图表上画两年的销售趋势做对比。做法很简单，往同一个 `QChart` 里添加多个 `QLineSeries` 就行：

```cpp
QLineSeries *series2024 = new QLineSeries();
series2024->setName("2024 年");
series2024->append(0, 120);
series2024->append(1, 135);
// ...

QLineSeries *series2025 = new QLineSeries();
series2025->setName("2025 年");
series2025->append(0, 140);
series2025->append(1, 158);
// ...

chart->addSeries(series2024);
chart->addSeries(series2025);

// 手动创建坐标轴（多个系列共享）
QValueAxis *axisX = new QValueAxis();
axisX->setRange(0, 5);
QValueAxis *axisY = new QValueAxis();
axisY->setRange(0, 200);

chart->addAxis(axisX, Qt::AlignBottom);
chart->addAxis(axisY, Qt::AlignLeft);

series2024->attachAxis(axisX);
series2024->attachAxis(axisY);
series2025->attachAxis(axisX);
series2025->attachAxis(axisY);
```

`setName()` 设置的名称会出现在图例（Legend）里，Qt 默认会在图表上方显示图例。两条线会用不同的颜色自动区分，图例里也会显示对应的名称和颜色标记。点击图例可以切换该系列的显示/隐藏——这是 QtCharts 自带的交互功能，不需要你写代码。

### 3.5 图表主题与动画

QtCharts 内置了 8 种主题，通过 `chart->setTheme()` 设置。每种主题会统一调整图表的配色方案、字体、背景色和网格线样式：

```cpp
// 切换不同主题看看效果
chart->setTheme(QChart::ChartThemeLight);         // 浅色（默认）
chart->setTheme(QChart::ChartThemeDark);          // 深色
chart->setTheme(QChart::ChartThemeBlueCerulean);  // 蓝色
chart->setTheme(QChart::ChartThemeBrownSand);     // 棕色
chart->setTheme(QChart::ChartThemeHighContrast);  // 高对比度
```

主题的设置应该在添加数据之前或者紧接着创建 `QChart` 之后做，因为切换主题会重置一些样式属性。如果你在自定义了样式之后再切换主题，之前的自定义设置可能会被覆盖。

动画效果可以让图表的展示更加流畅。QtCharts 提供了三种动画类型：

```cpp
chart->setAnimationOptions(QChart::SeriesAnimations);  // 系列动画（数据点渐入）
chart->setAnimationOptions(QChart::GridAxisAnimations); // 坐标轴动画
chart->setAnimationOptions(QChart::AllAnimations);      // 全部动画
chart->setAnimationDuration(1000);  // 动画时长（毫秒）
```

`SeriesAnimations` 会让数据点从零渐变到实际值，视觉效果很好，适合首次展示数据的场景。`GridAxisAnimations` 在坐标轴范围变化时有过渡效果。`AllAnimations` 是两者的组合。动画时长默认 1000 毫秒，可以通过 `setAnimationDuration()` 调整。在数据频繁更新的场景（比如实时监控），建议关闭动画，否则每次更新数据都播一次动画会导致界面卡顿。

### 3.6 图例与交互

图例（Legend）默认显示在图表顶部，你可以控制它的位置、可见性和对齐方式：

```cpp
chart->legend()->setVisible(true);
chart->legend()->setAlignment(Qt::AlignBottom);  // 移到底部
chart->legend()->setMarkerShape(QLegend::MarkerShapeCircle);  // 圆形标记
```

`QChartView` 内置了一些交互功能：鼠标悬停时会高亮对应的数据点，滚轮可以缩放，左键拖拽可以平移。但这些交互默认是关闭的，需要手动启用：

```cpp
// 启用交互
chartView->setRubberBand(QChartView::RectangleRubberBand);  // 框选缩放
// 或者
chartView->setRubberBand(QChartView::HorizontalRubberBand);  // 水平缩放
```

框选缩放模式下，用户在图表上拖拽一个矩形区域，松开后图表会缩放到该区域。要恢复到原始视图，可以右键点击。这些内置交互对于数据探索非常有用，不需要你写任何事件处理代码。

## 4. 综合示例：多类型图表展示

把前面学的串起来，我们写一个包含折线图、柱状图和饼图的完整 GUI 程序。程序会创建三个 `QChartView`，分别展示不同类型的图表，全部使用手动坐标轴配置和主题设置：

```cpp
// 创建图表容器
QWidget *centralWidget = new QWidget;
QHBoxLayout *layout = new QHBoxLayout(centralWidget);

// 1. 折线图：月度趋势
QChart *lineChart = new QChart();
lineChart->setTitle("月度销售趋势");
lineChart->setTheme(QChart::ChartThemeBlueCerulean);
lineChart->setAnimationOptions(QChart::SeriesAnimations);

QLineSeries *line = new QLineSeries();
line->setName("销售额");
line->append(0, 120);
line->append(1, 135);
// ... 添加更多数据点
lineChart->addSeries(line);

QValueAxis *axisX = new QValueAxis();
QValueAxis *axisY = new QValueAxis();
// 配置坐标轴...
lineChart->addAxis(axisX, Qt::AlignBottom);
lineChart->addAxis(axisY, Qt::AlignLeft);
line->attachAxis(axisX);
line->attachAxis(axisY);

QChartView *lineView = new QChartView(lineChart);
layout->addWidget(lineView);

// 2. 柱状图和饼图类似方式添加...
```

## 5. 练习项目

练习项目：系统资源监控面板。

我们要做一个实时显示 CPU 和内存使用率的监控面板，使用折线图展示最近 30 秒的数据变化趋势。

完成标准是这样的：使用 `QTimer` 每秒更新一次数据，折线图始终展示最近 30 个数据点（滚动窗口）；两条折线分别表示 CPU 和内存使用率，使用不同颜色区分；Y 轴范围固定在 0-100%，X 轴标签显示时间（秒）；图表使用深色主题，带数据系列动画；鼠标悬停时显示当前数据点的值。

几个实现提示：用 `QLineSeries::replace()` 或 `removePoints()` + `append()` 维护滚动窗口；CPU 和内存数据可以用随机数模拟（`QRandomGenerator`）；悬停提示需要继承 `QChartView` 并重写 `mouseMoveEvent`，或者用 `QLineSeries` 的 `hovered` 信号；X 轴用 `QCategoryAxis` 或 `QValueAxis` 配合整数刻度。

## 6. 官方文档参考

[Qt 文档 · QtCharts 模块](https://doc.qt.io/qt-6/qtcharts-index.html) -- 图表模块总览

[Qt 文档 · QChart](https://doc.qt.io/qt-6/qchart.html) -- 图表核心类

[Qt 文档 · QLineSeries](https://doc.qt.io/qt-6/qlineseries.html) -- 折线图数据系列

[Qt 文档 · QBarSeries](https://doc.qt.io/qt-6/qbarseries.html) -- 柱状图数据系列

[Qt 文档 · QPieSeries](https://doc.qt.io/qt-6/qpieseries.html) -- 饼图数据系列

[Qt 文档 · QValueAxis](https://doc.qt.io/qt-6/qvalueaxis.html) -- 数值坐标轴

*（链接已验证，2026-04-23 可访问）*

---

到这里就大功告成了！QtCharts 的 API 设计得非常规整：数据用 Series 表示，坐标轴用 Axis 配置，图表用 QChart 管理，显示用 QChartView 做容器。掌握这个结构之后，不管你需要哪种图表类型，开发流程都是一样的——创建 Series、填充数据、配置坐标轴、扔进 QChart、用 QChartView 显示。后续如果你需要更复杂的定制（比如自定义工具提示、数据点点击事件、导出图片等），QtCharts 的信号槽机制和 `QChart` 的 Paint 接口都能满足需求。

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
