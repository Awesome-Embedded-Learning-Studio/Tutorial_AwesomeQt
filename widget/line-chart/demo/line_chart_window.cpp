/**
 * @file line_chart_window.cpp
 * @brief LineChart 演示主窗口实现
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "line_chart_window.h"

#include <algorithm>
#include <cstdlib>

#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "line_chart.h"

LineChartWindow::LineChartWindow(QWidget* parent) : QMainWindow(parent) {
    setup_ui();
}

// —— 静态一组预设数据：展示基本折线 + Y auto-scale ——
QWidget* LineChartWindow::setup_static_layout() {
    auto* group = new QGroupBox("Static Data (Y auto-scale)", this);
    auto* layout = new QVBoxLayout(group);

    auto* chart = new AwesomeQt::LineChart(group);
    chart->setData({12, 28, 19, 45, 33, 52, 40, 61, 55, 48});
    // 演示线下填充：开 area，看折线下方淡色填充效果
    chart->setShowArea(true);

    layout->addWidget(chart);
    return group;
}

// —— 交互：追加随机点（看 Y auto-scale 跟着变）/ 重置 ——
QWidget* LineChartWindow::setup_dynamic_layout() {
    auto* group = new QGroupBox("Interactive (Append Random / Reset)", this);
    auto* layout = new QVBoxLayout(group);

    auto* chart = new AwesomeQt::LineChart(group);
    chart->setData({20, 35, 25, 50, 30});

    auto* ctrl_row = new QHBoxLayout();
    auto* append_btn = new QPushButton("Append Random Point", group);
    auto* reset_btn = new QPushButton("Reset", group);
    auto* clear_btn = new QPushButton("Clear", group);

    // 追加：取当前末值附近 ±随机，制造连续走势，auto-scale 会随值域变化重算 Y 轴
    connect(append_btn, &QPushButton::clicked, chart, [chart]() {
        const QVector<qreal>& data = chart->data();
        const qreal base = data.isEmpty() ? 50.0 : data.last();
        // 在 base ±20 区间抖动，并 clamp 到 [0, 100]，避免 Y 轴飞太远
        const qreal next = std::clamp(base + (std::rand() % 41 - 20), 0.0, 100.0);
        chart->appendPoint(next);
    });

    connect(reset_btn, &QPushButton::clicked, chart,
            [chart]() { chart->setData({20, 35, 25, 50, 30}); });

    connect(clear_btn, &QPushButton::clicked, chart, &AwesomeQt::LineChart::clear);

    ctrl_row->addWidget(append_btn);
    ctrl_row->addWidget(reset_btn);
    ctrl_row->addWidget(clear_btn);
    ctrl_row->addStretch();

    layout->addWidget(chart);
    layout->addLayout(ctrl_row);
    return group;
}

// —— 外观开关：showGrid / showDots / showArea，全部走 Q_PROPERTY ——
QWidget* LineChartWindow::setup_options_layout() {
    auto* group = new QGroupBox("Appearance Toggles", this);
    auto* layout = new QVBoxLayout(group);

    auto* chart = new AwesomeQt::LineChart(group);
    chart->setData({8, 22, 15, 40, 28, 60, 45, 70});
    chart->setLineColor(QColor(220, 80, 60));

    auto* toggles = new QHBoxLayout();
    auto* grid_cb = new QCheckBox("showGrid", group);
    grid_cb->setChecked(chart->showGrid());
    auto* dots_cb = new QCheckBox("showDots", group);
    dots_cb->setChecked(chart->showDots());
    auto* area_cb = new QCheckBox("showArea", group);
    area_cb->setChecked(chart->showArea());

    // 函数指针语法接 Q_PROPERTY 的 WRITE 经过的 NOTIFY 无关——直接绑定 setter
    connect(grid_cb, &QCheckBox::toggled, chart, &AwesomeQt::LineChart::setShowGrid);
    connect(dots_cb, &QCheckBox::toggled, chart, &AwesomeQt::LineChart::setShowDots);
    connect(area_cb, &QCheckBox::toggled, chart, &AwesomeQt::LineChart::setShowArea);

    toggles->addWidget(grid_cb);
    toggles->addWidget(dots_cb);
    toggles->addWidget(area_cb);
    toggles->addStretch();

    layout->addWidget(chart);
    layout->addLayout(toggles);
    return group;
}

void LineChartWindow::setup_ui() {
    auto* central = new QWidget(this);
    setCentralWidget(central);

    auto* layout = new QVBoxLayout(central);
    layout->addWidget(setup_static_layout());
    layout->addWidget(setup_dynamic_layout());
    layout->addWidget(setup_options_layout());

    setWindowTitle("LineChart Widget Demo");
    resize(560, 600);
}
