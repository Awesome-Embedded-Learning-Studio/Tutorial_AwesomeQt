/**
 * @file circle_progress.cpp
 * @brief CircleProgress 控件实现——背景环 + 进度弧 + 中心文字 + 平滑过渡
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "circle_progress.h"

#include <algorithm>

#include <QEasingCurve>
#include <QFont>
#include <QFontMetrics>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QPropertyAnimation>

namespace AwesomeQt {

namespace {
// —— drawArc 角度约定（1/16°，0°=3 点钟，正值逆时针）——
// 12 点钟 = 90° → 1440（1/16°）。进度顺时针铺开 = 扫角取负。
constexpr int kStartAngle16 = 90 * 16;  // 12 点钟起始角
constexpr int kFullCircle16 = 360 * 16; // 整圈 5760
} // namespace

// ============================================================================
// 构造
// ============================================================================
CircleProgress::CircleProgress(QWidget* parent) : QWidget(parent) {
    initAnimation();
    progress_ = std::clamp(value_, 0, 100) / 100.0; // 初值与 value 对齐，不在动画中
}

CircleProgress::CircleProgress(int value, QWidget* parent)
    : QWidget(parent), value_(std::clamp(value, 0, 100)) {
    initAnimation();
    progress_ = value_ / 100.0;
}

// ============================================================================
// 动画对象初始化（parent=this，对象树托管释放）
// ============================================================================
void CircleProgress::initAnimation() {
    // 持久指针：stop()/重配/start() 复用，不用 DeleteWhenStopped（防连切悬空/叠加）
    progress_anim_ = new QPropertyAnimation(this, "progress", this);
    progress_anim_->setDuration(400);
    progress_anim_->setEasingCurve(QEasingCurve::OutCubic);
}

// ============================================================================
// 业务入口：触发进度弧接力铺开
// ============================================================================
void CircleProgress::setValue(int value) {
    const int clamped = std::clamp(value, 0, 100);
    if (value_ == clamped) {
        return;
    }
    value_ = clamped;
    emit valueChanged(clamped);

    // 从当前显示进度（可能是动画中间值）接力到新目标，避免跳回旧目标造成跳变
    progress_anim_->stop();
    progress_anim_->setStartValue(progress_);
    progress_anim_->setEndValue(clamped / 100.0);
    progress_anim_->start();
}

int CircleProgress::value() const {
    return value_;
}

// ============================================================================
// Q_PROPERTY(progress) 回调：动画每帧驱动写这里（纯赋值 + emit + update）
// ============================================================================
void CircleProgress::setDisplayProgress(double progress) {
    const double clamped = std::clamp(progress, 0.0, 1.0);
    if (qFuzzyCompare(progress_, clamped)) {
        return;
    }
    progress_ = clamped;
    emit progressChanged(clamped);
    update(); // 异步请求重绘，不立即触发 paintEvent
}

double CircleProgress::progress() const {
    return progress_;
}

// ============================================================================
// 线宽 / 颜色 / 文字开关
// ============================================================================
void CircleProgress::setStrokeWidth(int width) {
    const int clamped = std::max(1, width);
    if (stroke_width_ == clamped) {
        return;
    }
    stroke_width_ = clamped;
    update();
    emit strokeWidthChanged(clamped);
}

int CircleProgress::strokeWidth() const {
    return stroke_width_;
}

void CircleProgress::setProgressColor(const QColor& color) {
    if (progress_color_ == color) {
        return;
    }
    progress_color_ = color;
    update();
    emit progressColorChanged(color);
}

QColor CircleProgress::progressColor() const {
    return progress_color_;
}

void CircleProgress::setRingColor(const QColor& color) {
    if (ring_color_ == color) {
        return;
    }
    ring_color_ = color;
    update();
    emit ringColorChanged(color);
}

QColor CircleProgress::ringColor() const {
    return ring_color_;
}

void CircleProgress::setShowText(bool enabled) {
    if (show_text_ == enabled) {
        return;
    }
    show_text_ = enabled;
    update();
    emit showTextChanged(enabled);
}

bool CircleProgress::showText() const {
    return show_text_;
}

// ============================================================================
// 尺寸
// ============================================================================
QSize CircleProgress::sizeHint() const {
    return QSize(100, 100);
}

QSize CircleProgress::minimumSizeHint() const {
    return QSize(40, 40);
}

// ============================================================================
// 自绘：背景整圈环 + 进度弧（12 点钟顺时针）+ 中心百分比文字
// ============================================================================
void CircleProgress::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // —— 几何：半径对 0/负值 clamp，防控件被压极小时 drawArc 行为未定义 ——
    const qreal stroke = std::max(1.0, static_cast<qreal>(stroke_width_));
    const qreal side = std::max(1, std::min(width(), height()));
    // 环半径 = 内切圆半径 - 半个线宽 - 2px 边距，clamp 到 >=1
    const qreal r = std::max(1.0, side / 2.0 - stroke / 2.0 - 2.0);
    const QRectF arc_rect(width() / 2.0 - r, height() / 2.0 - r, r * 2.0, r * 2.0);

    // —— 背景环：整圈，ringColor ——
    {
        QPen pen(ring_color_);
        pen.setWidthF(stroke);
        pen.setCapStyle(Qt::RoundCap);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        p.drawArc(arc_rect, 0, kFullCircle16);
    }

    // —— 进度弧：从 12 点钟顺时针铺开 progress 比例 ——
    if (progress_ > 0.0) {
        QPen pen(progress_color_);
        pen.setWidthF(stroke);
        pen.setCapStyle(Qt::RoundCap);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        // 扫角 = progress * 5760，取负 = 顺时针（从 12 点钟往 3 点钟方向铺开）
        const int span = static_cast<int>(std::round(progress_ * kFullCircle16));
        p.drawArc(arc_rect, kStartAngle16, -span);
    }

    // —— 中心百分比文字 ——
    if (show_text_) {
        const QString text = QString::number(static_cast<int>(std::round(progress_ * 100))) + "%";
        QFont f = p.font();
        f.setBold(true);
        f.setPointSizeF(std::max(8.0, side * 0.16)); // 字号随尺寸缩放但至少 8pt
        p.setFont(f);
        const QFontMetrics fm(f);
        const QRectF text_rect(width() / 2.0 - fm.horizontalAdvance(text) / 2.0,
                               height() / 2.0 - fm.height() / 2.0, fm.horizontalAdvance(text),
                               fm.height());
        p.setPen(progress_color_);
        p.drawText(text_rect, Qt::AlignCenter, text);
    }
}

} // namespace AwesomeQt
