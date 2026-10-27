/**
 * @file circle_progress_window.cpp
 * @brief CircleProgress 演示主窗口实现
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "circle_progress_window.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>

#include "circle_progress.h"

CircleProgressWindow::CircleProgressWindow(QWidget* parent) : QMainWindow(parent) {
    setupUi();
}

QWidget* CircleProgressWindow::setupStaticLayout() {
    // 静态多档：25/50/75/100% 一排，初值直接给（不经动画，启动即定格）
    auto* group = new QGroupBox("Static Progress");
    auto* grid = new QGridLayout(group);

    const int values[] = {25, 50, 75, 100};
    for (int i = 0; i < 4; ++i) {
        auto* ring = new AwesomeQt::CircleProgress(values[i], group);
        auto* label = new QLabel(QString("%1%").arg(values[i]), group);
        label->setAlignment(Qt::AlignCenter);
        grid->addWidget(ring, 0, i, Qt::AlignCenter);
        grid->addWidget(label, 1, i, Qt::AlignCenter);
    }
    return group;
}

QWidget* CircleProgressWindow::setupInteractiveLayout() {
    // 大环 + Cycle 按钮（看 400ms 过渡）+ Slider 实时驱动
    auto* group = new QGroupBox("Interactive Ring");
    auto* layout = new QVBoxLayout(group);

    auto* ring = new AwesomeQt::CircleProgress(0, group);
    ring->setStrokeWidth(14);

    auto* value_label = new QLabel("0%", group);
    value_label->setAlignment(Qt::AlignCenter);
    auto font = value_label->font();
    font.setBold(true);
    value_label->setFont(font);

    // 业务值变了就更新标签（progress 是动画中间值，文字跟 value 走更稳）
    auto sync_label = [ring, value_label]() {
        value_label->setText(QString("%1%").arg(ring->value()));
    };
    QObject::connect(ring, &AwesomeQt::CircleProgress::valueChanged, group,
                     [value_label](int v) { value_label->setText(QString("%1%").arg(v)); });

    auto* cycle_btn = new QPushButton("Cycle +25%", group);
    QObject::connect(cycle_btn, &QPushButton::clicked, group, [ring, sync_label]() {
        ring->setValue((ring->value() + 25) % 125); // 0→25→50→75→100→0 循环
        sync_label();
    });

    auto* slider = new QSlider(Qt::Horizontal, group);
    slider->setRange(0, 100);
    slider->setValue(0);
    QObject::connect(slider, &QSlider::valueChanged, group, [ring](int v) {
        ring->setValue(v); // Slider 拖动实时驱动 setValue，看过渡是否流畅
    });

    layout->addWidget(ring, 0, Qt::AlignCenter);
    layout->addWidget(value_label);
    layout->addWidget(cycle_btn);
    layout->addWidget(slider);
    return group;
}

QWidget* CircleProgressWindow::setupVariantsLayout() {
    // 配色/线宽变体：证明 progressColor / ringColor / strokeWidth 是真 Q_PROPERTY
    auto* group = new QGroupBox("Variants");
    auto* layout = new QHBoxLayout(group);

    // 绿色细环
    auto* green = new AwesomeQt::CircleProgress(60, group);
    green->setProgressColor(QColor(0, 180, 90));
    green->setStrokeWidth(6);
    auto* green_label = new QLabel("green / thin", group);
    green_label->setAlignment(Qt::AlignCenter);

    // 橙色粗环 + 关文字
    auto* orange = new AwesomeQt::CircleProgress(70, group);
    orange->setProgressColor(QColor(255, 140, 0));
    orange->setRingColor(QColor(60, 60, 60));
    orange->setStrokeWidth(18);
    orange->setShowText(false);
    auto* orange_label = new QLabel("orange / thick / no text", group);
    orange_label->setAlignment(Qt::AlignCenter);

    // 紫环
    auto* purple = new AwesomeQt::CircleProgress(40, group);
    purple->setProgressColor(QColor(150, 80, 220));
    purple->setStrokeWidth(10);
    auto* purple_label = new QLabel("purple", group);
    purple_label->setAlignment(Qt::AlignCenter);

    auto add = [&](QWidget* w, QWidget* lbl) {
        auto* col = new QVBoxLayout();
        col->addWidget(w, 0, Qt::AlignCenter);
        col->addWidget(lbl, 0, Qt::AlignCenter);
        layout->addLayout(col);
    };
    add(green, green_label);
    add(orange, orange_label);
    add(purple, purple_label);
    return group;
}

void CircleProgressWindow::setupUi() {
    auto* central = new QWidget(this);
    setCentralWidget(central);

    auto* layout = new QVBoxLayout(central);
    layout->addWidget(setupStaticLayout());
    layout->addWidget(setupInteractiveLayout());
    layout->addWidget(setupVariantsLayout());

    setWindowTitle("CircleProgress Widget Demo");
    resize(560, 480);
}
