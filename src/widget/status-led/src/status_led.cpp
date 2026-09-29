/**
 * @file status_led.cpp
 * @brief StatusLED 控件实现——颜色过渡动画 + 呼吸 + 自绘圆盘
 * @copyright Copyright (c) 2026
 */

#include "status_led.h"

#include <algorithm>

#include <QEasingCurve>
#include <QPainter>
#include <QPaintEvent>
#include <QPropertyAnimation>
#include <QRadialGradient>
#include <QTimer>
#include <QVariantAnimation>

namespace AwesomeQt {

// ============================================================================
// 状态代表色（替代裸数组索引，与枚举解耦）
// ============================================================================
QColor StatusLED::statusColor(Status s) const {
    switch (s) {
        case Status::NORMAL:  return QColor(0, 200, 0);     // 绿
        case Status::WARNING: return QColor(255, 180, 0);   // 琥珀
        case Status::ERROR:   return QColor(255, 40, 40);   // 红
        case Status::OFFLINE: return QColor(160, 160, 160); // 灰
    }
    return QColor(160, 160, 160); // 兜底（理论上不可达）
}

// ============================================================================
// 构造
// ============================================================================
StatusLED::StatusLED(QWidget* parent) : QWidget(parent) {
    initColorAnimation();
    initBreathingAnimation();
    initBlinkTimer();
    current_color_ = statusColor(status_);
}

StatusLED::StatusLED(Status default_status, QWidget* parent)
    : QWidget(parent), status_(default_status) {
    initColorAnimation();
    initBreathingAnimation();
    initBlinkTimer();
    current_color_ = statusColor(status_);
}

// ============================================================================
// 动画对象初始化（parent=this，对象树托管释放）
// ============================================================================
void StatusLED::initColorAnimation() {
    // 持久指针：stop()/重配/start() 复用，不用 DeleteWhenStopped（防连切悬空/叠加）
    color_anim_ = new QPropertyAnimation(this, "color", this);
    color_anim_->setDuration(300);
    color_anim_->setEasingCurve(QEasingCurve::OutCubic);
}

void StatusLED::initBreathingAnimation() {
    // 0 → 1 → 0 的正弦呼吸（InOutSine 缓动 + 中点置 1.0 + 无限循环）
    breathing_anim_ = new QVariantAnimation(this);
    breathing_anim_->setDuration(1400);
    breathing_anim_->setStartValue(0.0);
    breathing_anim_->setEndValue(0.0);
    breathing_anim_->setKeyValueAt(0.5, 1.0);
    breathing_anim_->setEasingCurve(QEasingCurve::InOutSine);
    breathing_anim_->setLoopCount(-1); // 无限
    connect(breathing_anim_, &QVariantAnimation::valueChanged, this,
            [this](const QVariant& value) {
                breathing_factor_ = value.toDouble();
                update();
            });
}

void StatusLED::initBlinkTimer() {
    onoff_timer_ = new QTimer(this);
    onoff_timer_->setInterval(500);
    connect(onoff_timer_, &QTimer::timeout, this, [this]() {
        onoff_visible_ = !onoff_visible_;
        update();
    });
}

// ============================================================================
// 状态切换（业务入口）：触发颜色过渡
// ============================================================================
void StatusLED::setStatus(Status new_status) {
    if (status_ == new_status) {
        return;
    }
    status_ = new_status;
    emit statusChanged(new_status);

    // 从当前显示色（可能是过渡中间值）接力到新目标，避免跳回旧目标造成跳变
    color_anim_->stop();
    color_anim_->setStartValue(current_color_);
    color_anim_->setEndValue(statusColor(new_status));
    color_anim_->start();
}

StatusLED::Status StatusLED::status() const {
    return status_;
}

// ============================================================================
// Q_PROPERTY(color) 回调：动画每帧驱动写这里
// ============================================================================
void StatusLED::setAnimatedColor(const QColor& color) {
    if (current_color_ == color) {
        return;
    }
    current_color_ = color;
    emit colorChanged(color);
    update(); // 异步请求重绘，不立即触发 paintEvent
}

QColor StatusLED::color() const {
    return current_color_;
}

// ============================================================================
// 闪烁模式
// ============================================================================
void StatusLED::setBlinkMode(BlinkMode mode) {
    if (blink_mode_ == mode) {
        return;
    }
    blink_mode_ = mode;
    emit blinkModeChanged(mode);

    // 先停掉所有闪烁动画并复位相位
    onoff_timer_->stop();
    breathing_anim_->stop();
    onoff_visible_ = true;
    breathing_factor_ = 0.0;

    switch (mode) {
        case BlinkMode::None:
            break;
        case BlinkMode::OnOff:
            onoff_timer_->start();
            break;
        case BlinkMode::Breathing:
            breathing_anim_->start();
            break;
    }
    update();
}

StatusLED::BlinkMode StatusLED::blinkMode() const {
    return blink_mode_;
}

void StatusLED::setBlinking(bool enabled) {
    setBlinkMode(enabled ? BlinkMode::OnOff : BlinkMode::None);
}

bool StatusLED::isBlinking() const {
    return blink_mode_ != BlinkMode::None;
}

// ============================================================================
// 尺寸
// ============================================================================
void StatusLED::setLedSize(int diameter) {
    if (led_size_ != diameter && diameter > 0) {
        led_size_ = diameter;
        updateGeometry();
        update();
        emit ledSizeChanged(diameter);
    }
}

int StatusLED::ledSize() const {
    return led_size_;
}

QSize StatusLED::sizeHint() const {
    return QSize(led_size_, led_size_);
}

QSize StatusLED::minimumSizeHint() const {
    const int min_side = std::max(8, led_size_ / 2);
    return QSize(min_side, min_side);
}

// ============================================================================
// 显示色变换：把过渡产物 current_color_ 按闪烁模式调制
// 过渡色与呼吸因子在此解耦合成——二者正交、可并行（不会打架）
// ============================================================================
QColor StatusLED::applyDisplayTransform(const QColor& base) const {
    if (blink_mode_ == BlinkMode::OnOff && !onoff_visible_) {
        return base.darker(400); // 熄灭，沿用旧视觉
    }
    if (blink_mode_ == BlinkMode::Breathing) {
        // 在 dim(暗) ↔ bright(亮) 间按 breathing_factor_ 做线性插值
        const QColor dim = base.darker(280);
        const QColor bright = base.lighter(140);
        const double t = breathing_factor_; // 0..1
        auto lerp = [](int a, int b, double f) {
            return static_cast<int>(a + (b - a) * f);
        };
        return QColor(lerp(dim.red(), bright.red(), t),
                      lerp(dim.green(), bright.green(), t),
                      lerp(dim.blue(), bright.blue(), t));
    }
    return base;
}

// ============================================================================
// 自绘：径向渐变圆盘（高光 → 原色 → 边缘暗）
// ============================================================================
void StatusLED::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QColor c = applyDisplayTransform(current_color_);

    // 兜底：w/h 极小时半径可能为负/0，clamp 到 1（见踩坑⑥）
    const int r = std::max(1, std::min(width(), height()) / 2 - 1);

    QRadialGradient g(rect().center(), r);
    g.setColorAt(0.0, c.lighter(160));
    g.setColorAt(0.6, c);
    g.setColorAt(1.0, c.darker(150));

    p.setPen(Qt::NoPen);
    p.setBrush(g);
    p.drawEllipse(rect().center(), r, r);
}

} // namespace AwesomeQt
