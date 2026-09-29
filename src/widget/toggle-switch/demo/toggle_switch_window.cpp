/**
 * @file toggle_switch_window.cpp
 * @brief ToggleSwitch 演示主窗口实现
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "toggle_switch_window.h"
#include "toggle_switch.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

ToggleSwitchWindow::ToggleSwitchWindow(QWidget* parent) : QMainWindow(parent) {
    setup_ui();
}

QWidget* ToggleSwitchWindow::setup_basic_layout() {
    auto* group = new QGroupBox("基本开关：关 / 开");
    auto* layout = new QHBoxLayout(group);

    auto* off_switch = new AwesomeQt::ToggleSwitch(false, group);
    auto* off_label = new QLabel("关", group);
    off_label->setAlignment(Qt::AlignCenter);

    auto* on_switch = new AwesomeQt::ToggleSwitch(true, group);
    auto* on_label = new QLabel("开", group);
    on_label->setAlignment(Qt::AlignCenter);

    layout->addWidget(off_switch);
    layout->addWidget(off_label);
    layout->addStretch();
    layout->addWidget(on_switch);
    layout->addWidget(on_label);
    return group;
}

QWidget* ToggleSwitchWindow::setup_interactive_layout() {
    auto* group = new QGroupBox("交互：点击或拖动切换");
    auto* layout = new QHBoxLayout(group);

    auto* sw = new AwesomeQt::ToggleSwitch(group);
    auto* state_label = new QLabel("当前：关", group);
    // toggled 信号驱动状态标签——证明信号链可用
    QObject::connect(sw, &AwesomeQt::ToggleSwitch::toggled, group,
                     [state_label](bool checked) {
                         state_label->setText(checked ? "当前：开" : "当前：关");
                     });

    auto* program_btn = new QPushButton("程序化 setChecked(true)", group);
    QObject::connect(program_btn, &QPushButton::clicked, group,
                     [sw]() { sw->setChecked(true); });

    layout->addWidget(sw);
    layout->addWidget(state_label);
    layout->addStretch();
    layout->addWidget(program_btn);
    return group;
}

QWidget* ToggleSwitchWindow::setup_custom_layout() {
    auto* group = new QGroupBox("自定义配色（Q_PROPERTY）");
    auto* layout = new QHBoxLayout(group);

    // 蓝色主题
    auto* blue = new AwesomeQt::ToggleSwitch(group);
    blue->setTrackColorOn(QColor(0, 120, 215));

    // 红色主题
    auto* red = new AwesomeQt::ToggleSwitch(true, group);
    red->setTrackColorOn(QColor(220, 60, 60));

    // 紫色主题
    auto* purple = new AwesomeQt::ToggleSwitch(group);
    purple->setTrackColorOn(QColor(140, 70, 200));

    auto add = [&](AwesomeQt::ToggleSwitch* s, const QString& name) {
        auto* col = new QVBoxLayout();
        col->addWidget(s, 0, Qt::AlignCenter);
        auto* label = new QLabel(name, group);
        label->setAlignment(Qt::AlignCenter);
        col->addWidget(label);
        layout->addLayout(col);
    };
    add(blue, "蓝");
    add(red, "红");
    add(purple, "紫");
    return group;
}

void ToggleSwitchWindow::setup_ui() {
    auto* central = new QWidget(this);
    setCentralWidget(central);

    auto* layout = new QVBoxLayout(central);
    layout->addWidget(setup_basic_layout());
    layout->addWidget(setup_interactive_layout());
    layout->addWidget(setup_custom_layout());

    setWindowTitle("ToggleSwitch Widget Demo");
    resize(420, 220);
}
