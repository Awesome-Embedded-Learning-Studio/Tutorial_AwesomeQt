/**
 * @file range_slider.cpp
 * @brief RangeSlider 控件实现——双手柄 hit-test + 值/像素映射 + 自绘轨道/区间/手柄
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "range_slider.h"

#include <algorithm>

#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>

namespace AwesomeQt {

namespace {
constexpr int kHandleDiameter = 16; // 手柄直径（px），同时决定轨道高度
constexpr int kHitTolerance = 14;   // 点击离手柄圆心多少像素内算命中
constexpr int kDragThreshold = 4;   // 移动超过这个像素才算拖动，否则当点击
constexpr int kHandleGap = 0;       // 两手柄允许的最小间距（值单位），防完全重叠
} // namespace

RangeSlider::RangeSlider(QWidget* parent) : QWidget(parent) {
    setCursor(Qt::PointingHandCursor);
    setMouseTracking(false);
}

// ============================================================================
// 区间端点
// ============================================================================
int RangeSlider::minimum() const {
    return minimum_;
}

void RangeSlider::setMinimum(int min) {
    if (min > maximum_) {
        // 新最小值越过当前最大值：把最大值也顶上去，保 minimum<=maximum
        maximum_ = min;
    }
    if (min == minimum_) {
        return;
    }
    minimum_ = min;

    // 现有值夹到新区间，并维持 lower<=upper
    const int new_lower = std::clamp(lower_value_, minimum_, maximum_);
    const int new_upper = std::clamp(upper_value_, minimum_, maximum_);
    if (new_lower != lower_value_ || new_upper != upper_value_) {
        lower_value_ = new_lower;
        upper_value_ = new_upper;
        emit lowerValueChanged(lower_value_);
        emit upperValueChanged(upper_value_);
        emit rangeChanged(lower_value_, upper_value_);
    }
    emit minimumChanged(minimum_);
    update();
}

int RangeSlider::maximum() const {
    return maximum_;
}

void RangeSlider::setMaximum(int max) {
    if (max < minimum_) {
        minimum_ = max;
    }
    if (max == maximum_) {
        return;
    }
    maximum_ = max;

    const int new_lower = std::clamp(lower_value_, minimum_, maximum_);
    const int new_upper = std::clamp(upper_value_, minimum_, maximum_);
    if (new_lower != lower_value_ || new_upper != upper_value_) {
        lower_value_ = new_lower;
        upper_value_ = new_upper;
        emit lowerValueChanged(lower_value_);
        emit upperValueChanged(upper_value_);
        emit rangeChanged(lower_value_, upper_value_);
    }
    emit maximumChanged(maximum_);
    update();
}

void RangeSlider::setRange(int min, int max) {
    if (min > max) {
        std::swap(min, max); // 防止反着传，自动纠正是比断言更友好的契约
    }
    // 静默更新端点，再用统一夹值流程发信号，避免分别 set 时的中间非法态
    minimum_ = min;
    maximum_ = max;

    const int new_lower = std::clamp(lower_value_, minimum_, maximum_);
    const int new_upper = std::clamp(upper_value_, minimum_, maximum_);
    const bool value_changed = (new_lower != lower_value_) || (new_upper != upper_value_);
    lower_value_ = new_lower;
    upper_value_ = new_upper;

    emit minimumChanged(minimum_);
    emit maximumChanged(maximum_);
    if (value_changed) {
        emit lowerValueChanged(lower_value_);
        emit upperValueChanged(upper_value_);
        emit rangeChanged(lower_value_, upper_value_);
    }
    update();
}

// ============================================================================
// 手柄值
// ============================================================================
int RangeSlider::lowerValue() const {
    return lower_value_;
}

void RangeSlider::setLowerValue(int value) {
    // 夹到 [minimum, upper - gap]，保证 lower 始终不越过 upper
    const int clamped = std::clamp(value, minimum_, upper_value_ - kHandleGap);
    if (clamped == lower_value_) {
        return;
    }
    lower_value_ = clamped;
    emit lowerValueChanged(lower_value_);
    emit rangeChanged(lower_value_, upper_value_);
    update(); // 异步合并重绘，拖动跟手且不掉帧
}

int RangeSlider::upperValue() const {
    return upper_value_;
}

void RangeSlider::setUpperValue(int value) {
    const int clamped = std::clamp(value, lower_value_ + kHandleGap, maximum_);
    if (clamped == upper_value_) {
        return;
    }
    upper_value_ = clamped;
    emit upperValueChanged(upper_value_);
    emit rangeChanged(lower_value_, upper_value_);
    update();
}

// ============================================================================
// 配色
// ============================================================================
QColor RangeSlider::handleColor() const {
    return handle_color_;
}
void RangeSlider::setHandleColor(const QColor& c) {
    if (handle_color_ == c)
        return;
    handle_color_ = c;
    update();
    emit handleColorChanged(c);
}

QColor RangeSlider::trackColor() const {
    return track_color_;
}
void RangeSlider::setTrackColor(const QColor& c) {
    if (track_color_ == c)
        return;
    track_color_ = c;
    update();
    emit trackColorChanged(c);
}

QColor RangeSlider::rangeColor() const {
    return range_color_;
}
void RangeSlider::setRangeColor(const QColor& c) {
    if (range_color_ == c)
        return;
    range_color_ = c;
    update();
    emit rangeColorChanged(c);
}

// ============================================================================
// 尺寸
// ============================================================================
QSize RangeSlider::sizeHint() const {
    return {200, 24};
}
QSize RangeSlider::minimumSizeHint() const {
    return {120, 20};
}

// ============================================================================
// 几何映射
// ============================================================================
QRectF RangeSlider::trackRect() const {
    const qreal radius = kHandleDiameter / 2.0;
    // 左右各留半个手柄：让端点值(min/max)的圆心正好落在轨道两端
    return QRectF(rect()).adjusted(radius, 0, -radius, 0);
}

qreal RangeSlider::valueToX(int value) const {
    const QRectF tr = trackRect();
    const qreal span = static_cast<qreal>(maximum_ - minimum_);
    if (span <= 0.0) {
        return tr.left(); // min==max 时无行程，所有值都映射到左端
    }
    const qreal t = static_cast<qreal>(value - minimum_) / span;
    return tr.left() + t * tr.width();
}

int RangeSlider::xToValue(qreal x) const {
    const QRectF tr = trackRect();
    const qreal span = static_cast<qreal>(maximum_ - minimum_);
    if (span <= 0.0 || tr.width() <= 0.0) {
        return minimum_; // 除零兜底：控件过窄或区间为空
    }
    const qreal t = (x - tr.left()) / tr.width();
    return static_cast<int>(std::round(minimum_ + t * span));
}

RangeSlider::ActiveHandle RangeSlider::hitTestHandle(qreal x) const {
    const qreal lower_x = valueToX(lower_value_);
    const qreal upper_x = valueToX(upper_value_);
    const qreal d_lower = std::abs(x - lower_x);
    const qreal d_upper = std::abs(x - upper_x);
    // 离谁近抓谁；都在容差内才认，否则返回 kNone（不在任何手柄上点击）
    if (d_lower <= kHitTolerance && d_lower <= d_upper) {
        return ActiveHandle::kLower;
    }
    if (d_upper <= kHitTolerance) {
        return ActiveHandle::kUpper;
    }
    return ActiveHandle::kNone;
}

// ============================================================================
// 鼠标交互（照 toggle-switch 三事件 + 阈值拖动模式）
// ============================================================================
void RangeSlider::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }
    press_x_ = event->position().x();
    dragging_ = false;
    // 命中测试：点在手柄容差内就预选该手柄；实际抓取等拖过阈值再定（防误判点击）
    active_handle_ = hitTestHandle(press_x_);
    event->accept();
}

void RangeSlider::mouseMoveEvent(QMouseEvent* event) {
    if (!(event->buttons() & Qt::LeftButton)) {
        QWidget::mouseMoveEvent(event);
        return;
    }
    const qreal dx = event->position().x() - press_x_;
    // 超阈值才进入拖动：避免一次轻微抖动就被当成拖（toggle-switch 踩坑①）
    if (!dragging_ && (dx > kDragThreshold || dx < -kDragThreshold)) {
        dragging_ = true;
    }
    if (dragging_) {
        const int v = xToValue(event->position().x());
        if (active_handle_ == ActiveHandle::kLower) {
            setLowerValue(v);
        } else if (active_handle_ == ActiveHandle::kUpper) {
            setUpperValue(v);
        }
    }
    event->accept();
}

void RangeSlider::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
        QWidget::mouseReleaseEvent(event);
        return;
    }
    // 未拖动即纯点击：把最近手柄直接吸到点击位置（更贴自然的滑块手感）
    if (!dragging_ && active_handle_ != ActiveHandle::kNone) {
        const int v = xToValue(event->position().x());
        if (active_handle_ == ActiveHandle::kLower) {
            setLowerValue(v);
        } else {
            setUpperValue(v);
        }
    }
    active_handle_ = ActiveHandle::kNone;
    dragging_ = false;
    event->accept();
}

// ============================================================================
// 自绘：圆角轨道 + 选中区间高亮 + 两圆手柄
// ============================================================================
void RangeSlider::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const qreal radius = kHandleDiameter / 2.0;
    // 轨道条占控件垂直居中一带，高度比手柄略瘦，让手柄凸出可点
    const qreal bar_height = std::max<qreal>(2.0, kHandleDiameter * 0.55);
    const qreal top = (height() - bar_height) / 2.0;
    const QRectF bar_rect(0, top, width(), bar_height);

    // 底层轨道
    p.setPen(Qt::NoPen);
    p.setBrush(track_color_);
    p.drawRoundedRect(bar_rect, bar_height / 2.0, bar_height / 2.0);

    // 选中区间（lower→upper 之间）高亮覆盖
    const qreal x_lower = valueToX(lower_value_);
    const qreal x_upper = valueToX(upper_value_);
    // x_lower 可能因 gap 等于 x_upper，clamp 宽度防 drawRoundedRect 退化
    const qreal range_left = std::min(x_lower, x_upper);
    const qreal range_w = std::max<qreal>(0.0, std::abs(x_upper - x_lower));
    p.setBrush(range_color_);
    p.drawRoundedRect(QRectF(range_left, top, range_w, bar_height), bar_height / 2.0,
                      bar_height / 2.0);

    // 两手柄：圆心 x = valueToX，y 居中；给一圈描边提升辨识度
    const qreal cy = height() / 2.0;
    QPen outline(QColor(180, 180, 180), 1.0);
    p.setBrush(handle_color_);
    p.setPen(outline);
    p.drawEllipse(QPointF(x_lower, cy), radius, radius);
    p.drawEllipse(QPointF(x_upper, cy), radius, radius);
}

} // namespace AwesomeQt
