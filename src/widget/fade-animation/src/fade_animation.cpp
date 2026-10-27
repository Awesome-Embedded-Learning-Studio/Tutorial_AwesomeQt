/**
 * @file fade_animation.cpp
 * @brief 淡入淡出容器控件 FadeWidget 实现
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "fade_animation.h"

#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

namespace AwesomeQt {

FadeWidget::FadeWidget(QWidget* parent) : QWidget(parent) {
    // 透明度承载：构造期 new 并挂在自身，初值完全不透明。
    effect_ = new QGraphicsOpacityEffect(this);
    effect_->setOpacity(1.0);
    setGraphicsEffect(effect_);

    // 持久动画指针：驱动 effect 的 "opacity" 属性，parent=this 托管，循环复用。
    fade_anim_ = new QPropertyAnimation(effect_, "opacity", this);
    fade_anim_->setEasingCurve(QEasingCurve::OutCubic);
    QObject::connect(fade_anim_, &QPropertyAnimation::finished, this, [this]() {
        // fadeOut 走完后隐藏自身，营造"消失"语义；fadeIn 走完则保持可见。
        if (fading_out_) {
            hide();
        }
        emit fadeFinished(fading_out_);
    });
}

void FadeWidget::fadeIn(int duration_ms) {
    if (!isVisible()) {
        // 真正的淡入起点：先置全透明再显示，否则会"先蹦出来再淡"。
        effect_->setOpacity(0.0);
        show();
    }
    fading_out_ = false;
    runFade(1.0, duration_ms);
}

void FadeWidget::fadeOut(int duration_ms) {
    // 必须先可见才能看到淡出过程。
    if (!isVisible()) {
        show();
    }
    fading_out_ = true;
    runFade(0.0, duration_ms);
}

void FadeWidget::setFadeDuration(int ms) {
    if (ms < 1) {
        ms = 1; // 兜底，避免 0 或负时长导致动画不启动/除零语义混乱
    }
    if (ms == fade_duration_ms_) {
        return;
    }
    fade_duration_ms_ = ms;
    emit fadeDurationChanged(ms);
}

int FadeWidget::fadeDuration() const {
    return fade_duration_ms_;
}

qreal FadeWidget::opacity() const {
    return effect_->opacity();
}

void FadeWidget::setOpacity(qreal value) {
    if (value < 0.0) {
        value = 0.0;
    } else if (value > 1.0) {
        value = 1.0;
    }
    if (qFuzzyCompare(value, effect_->opacity())) {
        return;
    }
    effect_->setOpacity(value); // 纯赋值，effect 自带 update，不触发动画
    emit opacityChanged(value);
}

QSize FadeWidget::sizeHint() const {
    return QSize(200, 120);
}

void FadeWidget::runFade(qreal end, int duration_ms) {
    if (duration_ms < 1) {
        duration_ms = 1;
    }
    // 复用同一指针：先停再配起点（取当前实时 opacity，做到接力争不跳变）。
    fade_anim_->stop();
    fade_anim_->setDuration(duration_ms);
    fade_anim_->setStartValue(effect_->opacity());
    fade_anim_->setEndValue(end);
    fade_anim_->start();
}

} // namespace AwesomeQt
