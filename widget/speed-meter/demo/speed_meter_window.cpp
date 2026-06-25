/**
 * @file speed_meter_window.cpp
 * @brief SpeedMeter 演示主窗口实现
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "speed_meter_window.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRandomGenerator>
#include <QSlider>
#include <QVBoxLayout>

#include "speed_meter.h"

namespace {
// 静态档位展示的取值
constexpr int kStaticSteps[] = {0, 60, 120, 180, 220};
} // namespace

SpeedMeterWindow::SpeedMeterWindow(QWidget* parent) : QMainWindow(parent) {
    setupUi();
}

QWidget* SpeedMeterWindow::setupStaticLayout() {
    auto* group = new QGroupBox("Static Speed Meters");
    auto* layout = new QHBoxLayout(group);

    for (int step : kStaticSteps) {
        auto* meter = new AwesomeQt::SpeedMeter(group);
        meter->setValue(step); // 初值，无动画
        auto* label = new QLabel(QString::number(step), group);
        label->setAlignment(Qt::AlignCenter);

        auto* col = new QVBoxLayout();
        col->addWidget(meter, 0, Qt::AlignCenter);
        col->addWidget(label, 0, Qt::AlignCenter);
        layout->addLayout(col);
    }

    return group;
}

QWidget* SpeedMeterWindow::setupInteractiveLayout() {
    auto* group = new QGroupBox("Cycle（看指针过渡）");
    auto* layout = new QHBoxLayout(group);

    auto* meter = new AwesomeQt::SpeedMeter(group);
    meter->setFixedSize(180, 180);

    auto* cycle_btn = new QPushButton("Cycle Value", group);
    // 在几档间循环，观察指针接力过渡
    static const int seq[] = {0, 60, 120, 180, 220, 160, 40};
    connect(cycle_btn, &QPushButton::clicked, group, [meter]() {
        static int idx = 0;
        idx = (idx + 1) % 7;
        meter->setValue(seq[idx]);
    });

    layout->addWidget(meter, 0, Qt::AlignCenter);
    layout->addWidget(cycle_btn);

    return group;
}

QWidget* SpeedMeterWindow::setupSliderLayout() {
    auto* group = new QGroupBox("Slider 驱动（0..220）");
    auto* layout = new QVBoxLayout(group);

    auto* meter = new AwesomeQt::SpeedMeter(group);
    meter->setFixedSize(160, 160);

    auto* slider = new QSlider(Qt::Horizontal, group);
    slider->setRange(0, 220);
    slider->setValue(0);

    // 拖动滑块逐次 setValue——每次都 stop()/接力，看指针连贯追踪
    connect(slider, &QSlider::valueChanged, meter, &AwesomeQt::SpeedMeter::setValue);

    layout->addWidget(meter, 0, Qt::AlignCenter);
    layout->addWidget(slider);

    return group;
}

QWidget* SpeedMeterWindow::setupRandomLayout() {
    auto* group = new QGroupBox("随机跳变（测 stop()/接力不跳变）");
    auto* layout = new QHBoxLayout(group);

    auto* meter = new AwesomeQt::SpeedMeter(group);
    meter->setFixedSize(160, 160);

    auto* random_btn = new QPushButton("Random Jump", group);
    // 连点多次随机跳：动画中途 stop()+重配 setStartValue(当前角度)，
    // 指针应平滑改向而不闪回旧目标（防跳变的关键）
    connect(random_btn, &QPushButton::clicked, group,
            [meter]() { meter->setValue(QRandomGenerator::global()->bounded(0, 221)); });

    layout->addWidget(meter, 0, Qt::AlignCenter);
    layout->addWidget(random_btn);

    return group;
}

void SpeedMeterWindow::setupUi() {
    auto* central = new QWidget(this);
    setCentralWidget(central);

    auto* layout = new QVBoxLayout(central);
    layout->addWidget(setupStaticLayout());
    layout->addWidget(setupInteractiveLayout());
    layout->addWidget(setupSliderLayout());
    layout->addWidget(setupRandomLayout());

    setWindowTitle("SpeedMeter Widget Demo");
    resize(640, 720);
}
