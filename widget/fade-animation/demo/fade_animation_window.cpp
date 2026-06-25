/**
 * @file fade_animation_window.cpp
 * @brief FadeWidget 控件演示主窗口实现
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "fade_animation_window.h"

#include "fade_animation.h"

#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>
#include <QWidget>

namespace AwesomeQt {

FadeAnimationWindow::FadeAnimationWindow(QWidget* parent) : QMainWindow(parent) {
    setup_ui();
}

void FadeAnimationWindow::setup_ui() {
    auto* central = new QWidget(this);
    auto* root = new QVBoxLayout(central);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(10);

    root->addWidget(build_fade_group(), /*stretch=*/1);
    root->addWidget(build_duration_group());
    root->addWidget(build_opacity_group());
    root->addStretch();

    setCentralWidget(central);
    setWindowTitle(QStringLiteral("FadeWidget Demo"));
    resize(420, 360);
}

QWidget* FadeAnimationWindow::build_fade_group() {
    auto* box = new QGroupBox(QStringLiteral("淡入 / 淡出"), this);
    auto* layout = new QVBoxLayout(box);

    // 待演示容器：内部放带色面板 + 文字，证明"内容跟着淡"。
    fade_widget_ = new FadeWidget(box);
    fade_widget_->setMinimumHeight(80);

    auto* content = new QFrame(fade_widget_);
    content->setStyleSheet(QStringLiteral("background-color:#2d7d9a; border-radius:6px;"));
    content->setFrameShape(QFrame::NoFrame);
    auto* content_layout = new QVBoxLayout(content);
    content_layout->setContentsMargins(12, 12, 12, 12);
    auto* label = new QLabel(
        QStringLiteral("这是一段会被一起淡入淡出的内容。\nQGraphicsOpacityEffect 挂在容器上。"),
        content);
    label->setStyleSheet(QStringLiteral("color:white; font-size:13px;"));
    label->setWordWrap(true);
    content_layout->addWidget(label);

    auto* container_layout = new QVBoxLayout(fade_widget_);
    container_layout->setContentsMargins(0, 0, 0, 0);
    container_layout->addWidget(content);

    layout->addWidget(fade_widget_);

    auto* btn_row = new QHBoxLayout();
    auto* fade_in_btn = new QPushButton(QStringLiteral("Fade In"), box);
    auto* fade_out_btn = new QPushButton(QStringLiteral("Fade Out"), box);
    btn_row->addWidget(fade_in_btn);
    btn_row->addWidget(fade_out_btn);
    layout->addLayout(btn_row);

    QObject::connect(fade_in_btn, &QPushButton::clicked, box,
                     [this]() { fade_widget_->fadeIn(fade_widget_->fadeDuration()); });
    QObject::connect(fade_out_btn, &QPushButton::clicked, box,
                     [this]() { fade_widget_->fadeOut(fade_widget_->fadeDuration()); });

    return box;
}

QWidget* FadeAnimationWindow::build_duration_group() {
    auto* box = new QGroupBox(QStringLiteral("fadeDuration（淡入淡出时长）"), this);
    auto* layout = new QHBoxLayout(box);

    auto* slider = new QSlider(Qt::Horizontal, box);
    slider->setRange(100, 1500);
    slider->setValue(fade_widget_->fadeDuration());

    auto* value_label = new QLabel(QStringLiteral("%1 ms").arg(fade_widget_->fadeDuration()), box);
    value_label->setMinimumWidth(70);

    layout->addWidget(slider, /*stretch=*/1);
    layout->addWidget(value_label);

    // 同步：滑块 → setFadeDuration → 回填标签。setFadeDuration 内部 clamp 兜底。
    QObject::connect(slider, &QSlider::valueChanged, box, [this, value_label](int value) {
        fade_widget_->setFadeDuration(value);
        value_label->setText(QStringLiteral("%1 ms").arg(fade_widget_->fadeDuration()));
    });

    return box;
}

QWidget* FadeAnimationWindow::build_opacity_group() {
    auto* box =
        new QGroupBox(QStringLiteral("瞬时 opacity（绕过动画，证明是真 Q_PROPERTY）"), this);
    auto* layout = new QHBoxLayout(box);

    auto* slider = new QSlider(Qt::Horizontal, box);
    // 滑块整型 0..100，映射到 0.0..1.0。
    slider->setRange(0, 100);
    slider->setValue(static_cast<int>(fade_widget_->opacity() * 100));

    auto* value_label = new QLabel(QString::number(fade_widget_->opacity(), 'f', 2), box);
    value_label->setMinimumWidth(50);

    layout->addWidget(slider, /*stretch=*/1);
    layout->addWidget(value_label);

    // 直接 setOpacity：纯赋值 + update，无动画，证明 opacity 是可外部直接驱动的 Q_PROPERTY。
    QObject::connect(slider, &QSlider::valueChanged, box, [this, value_label](int value) {
        fade_widget_->setOpacity(value / 100.0);
        value_label->setText(QString::number(fade_widget_->opacity(), 'f', 2));
    });

    // 反向同步：动画运行时也会改 opacity，标签跟着走。
    QObject::connect(fade_widget_, &FadeWidget::opacityChanged, box,
                     [slider, value_label](qreal opacity) {
                         value_label->setText(QString::number(opacity, 'f', 2));
                         const int pct = static_cast<int>(opacity * 100);
                         if (slider->value() != pct) {
                             QSignalBlocker blocker(slider); // 防止回灌触发再次 setOpacity
                             slider->setValue(pct);
                         }
                     });

    return box;
}

} // namespace AwesomeQt
