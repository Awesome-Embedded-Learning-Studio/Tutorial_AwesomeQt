/**
 * @file range_slider_window.cpp
 * @brief RangeSlider 演示主窗口实现
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "range_slider_window.h"
#include "range_slider.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

RangeSliderWindow::RangeSliderWindow(QWidget* parent) : QMainWindow(parent) {
    setupUi();
}

QWidget* RangeSliderWindow::setupStaticLayout() {
    auto* group = new QGroupBox("静态区间：lower=20 / upper=80");
    auto* layout = new QVBoxLayout(group);

    auto* slider = new AwesomeQt::RangeSlider(group);
    slider->setRange(0, 100);
    slider->setLowerValue(20);
    slider->setUpperValue(80);

    auto* info = new QLabel("lower=20  upper=80", group);
    info->setAlignment(Qt::AlignCenter);

    layout->addWidget(slider);
    layout->addWidget(info);
    return group;
}

QWidget* RangeSliderWindow::setupInteractiveLayout() {
    auto* group = new QGroupBox("交互：拖动任一手柄，实时显示当前值");
    auto* layout = new QVBoxLayout(group);

    auto* slider = new AwesomeQt::RangeSlider(group);
    slider->setRange(0, 100);
    slider->setLowerValue(30);
    slider->setUpperValue(70);

    auto* state = new QLabel("lower=30  upper=70", group);
    state->setAlignment(Qt::AlignCenter);

    // rangeChanged 拖动时每步都发，标签即时跟手
    QObject::connect(slider, &AwesomeQt::RangeSlider::rangeChanged, group,
                     [state](int lower, int upper) {
                         state->setText(QString("lower=%1  upper=%2").arg(lower).arg(upper));
                     });

    layout->addWidget(slider);
    layout->addWidget(state);
    return group;
}

QWidget* RangeSliderWindow::setupProgrammaticLayout() {
    auto* group = new QGroupBox("程序化：按钮移动手柄");
    auto* layout = new QVBoxLayout(group);

    auto* slider = new AwesomeQt::RangeSlider(group);
    slider->setRange(0, 100);
    slider->setLowerValue(25);
    slider->setUpperValue(75);

    auto* state = new QLabel("lower=25  upper=75", group);
    state->setAlignment(Qt::AlignCenter);
    QObject::connect(slider, &AwesomeQt::RangeSlider::rangeChanged, group,
                     [state](int lower, int upper) {
                         state->setText(QString("lower=%1  upper=%2").arg(lower).arg(upper));
                     });

    auto* btn_row = new QHBoxLayout();
    auto* lower_btn = new QPushButton("setLowerValue(10)", group);
    auto* upper_btn = new QPushButton("setUpperValue(90)", group);
    // 验证程序化接口仍受 lower<=upper 约束保护
    QObject::connect(lower_btn, &QPushButton::clicked, group,
                     [slider]() { slider->setLowerValue(10); });
    QObject::connect(upper_btn, &QPushButton::clicked, group,
                     [slider]() { slider->setUpperValue(90); });
    btn_row->addWidget(lower_btn);
    btn_row->addWidget(upper_btn);

    layout->addWidget(slider);
    layout->addWidget(state);
    layout->addLayout(btn_row);
    return group;
}

QWidget* RangeSliderWindow::setupThemedLayout() {
    auto* group = new QGroupBox("自定义配色（Q_PROPERTY）");
    auto* layout = new QHBoxLayout(group);

    // 蓝绿主题
    auto* teal = new AwesomeQt::RangeSlider(group);
    teal->setRangeColor(QColor(0, 150, 136));
    teal->setHandleColor(QColor(255, 255, 255));

    // 橙色主题
    auto* orange = new AwesomeQt::RangeSlider(group);
    orange->setRangeColor(QColor(255, 120, 50));
    orange->setLowerValue(40);
    orange->setUpperValue(60);

    auto add = [&](AwesomeQt::RangeSlider* s, const QString& name) {
        auto* col = new QVBoxLayout();
        col->addWidget(s, 0, Qt::AlignCenter);
        auto* label = new QLabel(name, group);
        label->setAlignment(Qt::AlignCenter);
        col->addWidget(label);
        layout->addLayout(col);
    };
    add(teal, "蓝绿");
    add(orange, "橙");
    return group;
}

void RangeSliderWindow::setupUi() {
    auto* central = new QWidget(this);
    setCentralWidget(central);

    auto* layout = new QVBoxLayout(central);
    layout->addWidget(setupStaticLayout());
    layout->addWidget(setupInteractiveLayout());
    layout->addWidget(setupProgrammaticLayout());
    layout->addWidget(setupThemedLayout());

    setWindowTitle("RangeSlider Widget Demo");
    resize(420, 380);
}
