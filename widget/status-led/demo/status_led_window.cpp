/**
 * @file status_led_window.cpp
 * @brief StatusLED 演示主窗口实现
 * @copyright Copyright (c) 2026
 */

#include "status_led_window.h"

#include <QAbstractAnimation>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QVBoxLayout>

#include "status_led.h"

using Status = AwesomeQt::StatusLED::Status;

StatusLEDWindow::StatusLEDWindow(QWidget* parent) : QMainWindow(parent) {
    setup_ui();
}

QWidget* StatusLEDWindow::setup_static_layout() {
    auto* group = new QGroupBox("Static Status LEDs");
    auto* grid = new QGridLayout(group);

    const struct {
        Status status;
        QString name;
    } items[] = {
        {Status::NORMAL, "Normal"}, {Status::WARNING, "Warning"},
        {Status::ERROR, "Error"},   {Status::OFFLINE, "Offline"},
    };

    for (int i = 0; i < 4; ++i) {
        auto* led = new AwesomeQt::StatusLED(items[i].status, group);
        auto* label = new QLabel(items[i].name, group);
        label->setAlignment(Qt::AlignCenter);
        grid->addWidget(led, 0, i, Qt::AlignCenter);
        grid->addWidget(label, 1, i, Qt::AlignCenter);
    }

    return group;
}

QWidget* StatusLEDWindow::setup_dynamic_layout() {
    auto* group = new QGroupBox("Interactive LED");
    auto* layout = new QHBoxLayout(group);

    auto* led = new AwesomeQt::StatusLED(Status::NORMAL, group);

    auto* status_label = new QLabel("Normal", group);

    auto* cycle_btn = new QPushButton("Cycle Status", group);
    connect(cycle_btn, &QPushButton::clicked, this, [led]() {
        Status s = static_cast<Status>((static_cast<int>(led->status()) + 1) %
                                       (static_cast<int>(Status::OFFLINE) + 1));
        led->setStatus(s);
    });

    auto* blink_btn = new QPushButton("Toggle Blinking", group);
    connect(blink_btn, &QPushButton::clicked, this, [led, blink_btn]() {
        led->setBlinking(!led->isBlinking());
        blink_btn->setText(led->isBlinking() ? "Stop Blinking" : "Toggle Blinking");
    });

    connect(led, &AwesomeQt::StatusLED::statusChanged, this, [status_label](Status s) {
        static const char* names[] = {"Normal", "Warning", "Error", "Offline"};
        status_label->setText(names[static_cast<int>(s)]);
    });

    layout->addWidget(led);
    layout->addWidget(status_label);
    layout->addWidget(cycle_btn);
    layout->addWidget(blink_btn);

    return group;
}

QWidget* StatusLEDWindow::setup_sizes_layout() {
    auto* group = new QGroupBox("Different Sizes");
    auto* layout = new QHBoxLayout(group);

    const int sizes[] = {16, 24, 32};
    for (int sz : sizes) {
        auto* led = new AwesomeQt::StatusLED(Status::NORMAL, group);
        led->setLedSize(sz);
        auto* label = new QLabel(QString("%1px").arg(sz), group);
        label->setAlignment(Qt::AlignCenter);

        auto* col = new QVBoxLayout();
        col->addWidget(led, 0, Qt::AlignCenter);
        col->addWidget(label, 0, Qt::AlignCenter);
        layout->addLayout(col);
    }

    return group;
}

QWidget* StatusLEDWindow::setup_modes_layout() {
    auto* group = new QGroupBox("颜色过渡 · 闪烁模式 · 属性驱动");
    auto* layout = new QVBoxLayout(group);

    // —— 过渡可视化：大 LED + Cycle，观察 300ms OutCubic 颜色过渡 ——
    auto* transition_row = new QHBoxLayout();
    auto* big_led = new AwesomeQt::StatusLED(Status::NORMAL, group);
    big_led->setLedSize(48);
    auto* cycle_btn = new QPushButton("Cycle Status（看颜色过渡）", group);
    connect(cycle_btn, &QPushButton::clicked, group, [big_led]() {
        Status s = static_cast<Status>((static_cast<int>(big_led->status()) + 1) %
                                       (static_cast<int>(Status::OFFLINE) + 1));
        big_led->setStatus(s);
    });
    transition_row->addWidget(big_led, 0, Qt::AlignCenter);
    transition_row->addWidget(cycle_btn);
    layout->addLayout(transition_row);

    // —— BlinkMode 对比：OnOff 生硬明灭 vs Breathing 正弦呼吸 ——
    auto* blink_row = new QHBoxLayout();
    auto* onoff_led = new AwesomeQt::StatusLED(Status::WARNING, group);
    onoff_led->setLedSize(32);
    onoff_led->setBlinkMode(AwesomeQt::StatusLED::BlinkMode::OnOff);
    auto* onoff_label = new QLabel("OnOff（生硬明灭）", group);
    onoff_label->setAlignment(Qt::AlignCenter);

    auto* breath_led = new AwesomeQt::StatusLED(Status::WARNING, group);
    breath_led->setLedSize(32);
    breath_led->setBlinkMode(AwesomeQt::StatusLED::BlinkMode::Breathing);
    auto* breath_label = new QLabel("Breathing（正弦呼吸）", group);
    breath_label->setAlignment(Qt::AlignCenter);

    blink_row->addWidget(onoff_led);
    blink_row->addWidget(onoff_label);
    blink_row->addStretch();
    blink_row->addWidget(breath_led);
    blink_row->addWidget(breath_label);
    layout->addLayout(blink_row);

    // —— Q_PROPERTY 外部驱动：证明 color 属性对动画系统/Designer 可用 ——
    auto* drive_row = new QHBoxLayout();
    auto* drive_led = new AwesomeQt::StatusLED(Status::OFFLINE, group);
    drive_led->setLedSize(32);
    auto* drive_btn = new QPushButton("QPropertyAnimation 驱动 color", group);
    connect(drive_btn, &QPushButton::clicked, group, [drive_led]() {
        // 外部动画直接驱动 color 属性——不经过 setStatus，
        // 证明这条 Q_PROPERTY 链对外部（动画/Designer/State machine）可用
        auto* a = new QPropertyAnimation(drive_led, "color");
        a->setDuration(1200);
        a->setStartValue(QColor(0, 200, 200));
        a->setKeyValueAt(0.5, QColor(255, 0, 255));
        a->setEndValue(QColor(0, 200, 200));
        a->setLoopCount(3);
        a->start(QAbstractAnimation::DeleteWhenStopped);
    });
    drive_row->addWidget(drive_led, 0, Qt::AlignCenter);
    drive_row->addWidget(drive_btn);
    layout->addLayout(drive_row);

    return group;
}

void StatusLEDWindow::setup_ui() {
    auto* central = new QWidget(this);
    setCentralWidget(central);

    auto* layout = new QVBoxLayout(central);
    layout->addWidget(setup_static_layout());
    layout->addWidget(setup_dynamic_layout());
    layout->addWidget(setup_sizes_layout());
    layout->addWidget(setup_modes_layout());

    setWindowTitle("StatusLED Widget Demo");
    resize(500, 400);
}
