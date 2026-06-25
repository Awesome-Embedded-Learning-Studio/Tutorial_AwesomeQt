/**
 * @file speed_meter.cpp
 * @brief SpeedMeter 控件实现——动画指针 + 刻度 + 数字读数
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "speed_meter.h"

#include <algorithm>
#include <cmath>

#include <QEasingCurve>
#include <QFontMetrics>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QPolygonF>
#include <QPropertyAnimation>

namespace AwesomeQt {

namespace {
// —— 仪表弧角度约定 ——
// 用「屏幕角 β」统一描述：3 点钟为 0°、顺时针为正（与 QPainter::rotate 及
// cos/sin 在 y 朝下的屏幕坐标系完全一致）。整条弧：
//   value=0   → 135°（左下 7:30）
//   value=max → 45°（右下 4:30，= 135°+270° 取模 360）
//   value=mid → 270°（顶部 12:00）
// 即 β(v) = kStartScreen + (v/max)*kSweep，开口朝下（6 点钟方向无刻度）。
// drawArc 用 Qt 自带约定（0°=3 点、逆时针为正），与 β 差一个 y 翻转：
//   β=135°（左下）↔ drawArc 起始角 225°，扫角取负（顺时针铺开）。
constexpr qreal kStartScreen = 135.0; // β(v=0)：左下 7:30
constexpr qreal kSweep = 270.0;       // 顺时针扫角
constexpr int kArcStart16 = 225 * 16; // drawArc 起始角（= -β(0) mod 360，左下）
constexpr int kArcSpan16 = -270 * 16; // drawArc 扫角（负=顺时针）
constexpr int kMajorTickCount = 11;   // 主刻度条数（含两端，量程 10 等分）

constexpr qreal degToRad(qreal deg) {
    return deg * M_PI / 180.0;
}
} // namespace

// ============================================================================
// 构造
// ============================================================================
SpeedMeter::SpeedMeter(QWidget* parent) : QWidget(parent) {
    initAnimation();
    needle_angle_ = angleForValue(value_); // 初值与 value 对齐，不在动画中
}

// ============================================================================
// 动画对象初始化（parent=this，对象树托管释放）
// ============================================================================
void SpeedMeter::initAnimation() {
    // 持久指针：stop()/重配/start() 复用，不用 DeleteWhenStopped（防连切悬空/叠加）
    needle_anim_ = new QPropertyAnimation(this, "needleAngle", this);
    needle_anim_->setDuration(400);
    needle_anim_->setEasingCurve(QEasingCurve::OutCubic);
}

// ============================================================================
// value → 屏幕角 β 映射
// ============================================================================
qreal SpeedMeter::angleForValue(int v) const {
    const int max = std::max(1, max_value_);          // 防 max_value_<=0 除零
    const int clamped = std::clamp(v, 0, max_value_); // 夹到 [0, max]
    // β(v) = 135° + (v/max)*270°：v=0 左下、v=max 右下、mid 顶部，开口朝下
    return kStartScreen + (static_cast<qreal>(clamped) / max) * kSweep;
}

// ============================================================================
// 业务入口：触发指针接力旋转
// ============================================================================
void SpeedMeter::setValue(int value) {
    if (value_ == value) {
        return;
    }
    value_ = value;
    emit valueChanged(value);

    // 从当前显示角度（可能是动画中间值）接力到新目标，避免跳回旧目标造成跳变
    needle_anim_->stop();
    needle_anim_->setStartValue(needle_angle_);
    needle_anim_->setEndValue(angleForValue(value));
    needle_anim_->start();
}

int SpeedMeter::value() const {
    return value_;
}

// ============================================================================
// Q_PROPERTY(needleAngle) 回调：动画每帧驱动写这里（纯赋值 + emit + update）
// ============================================================================
void SpeedMeter::setNeedleAngle(qreal angle) {
    if (qFuzzyCompare(needle_angle_, angle)) {
        return;
    }
    needle_angle_ = angle;
    emit needleAngleChanged(angle);
    update(); // 异步请求重绘，不立即触发 paintEvent
}

qreal SpeedMeter::needleAngle() const {
    return needle_angle_;
}

// ============================================================================
// 量程 / 颜色 setter
// ============================================================================
void SpeedMeter::setMaxValue(int maxValue) {
    if (max_value_ == maxValue || maxValue <= 0) {
        return;
    }
    max_value_ = maxValue;
    emit maxValueChanged(maxValue);

    // 量程变了，指针角度按新映射重算并接力（不直接跳变）
    needle_anim_->stop();
    needle_anim_->setStartValue(needle_angle_);
    needle_anim_->setEndValue(angleForValue(value_));
    needle_anim_->start();
    update();
}

int SpeedMeter::maxValue() const {
    return max_value_;
}

void SpeedMeter::setNeedleColor(const QColor& color) {
    if (needle_color_ == color) {
        return;
    }
    needle_color_ = color;
    update();
    emit needleColorChanged(color);
}

QColor SpeedMeter::needleColor() const {
    return needle_color_;
}

void SpeedMeter::setTickColor(const QColor& color) {
    if (tick_color_ == color) {
        return;
    }
    tick_color_ = color;
    update();
    emit tickColorChanged(color);
}

QColor SpeedMeter::tickColor() const {
    return tick_color_;
}

void SpeedMeter::setGaugeColor(const QColor& color) {
    if (gauge_color_ == color) {
        return;
    }
    gauge_color_ = color;
    update();
    emit gaugeColorChanged(color);
}

QColor SpeedMeter::gaugeColor() const {
    return gauge_color_;
}

// ============================================================================
// 尺寸
// ============================================================================
QSize SpeedMeter::sizeHint() const {
    return QSize(200, 200);
}

QSize SpeedMeter::minimumSizeHint() const {
    return QSize(100, 100);
}

// ============================================================================
// 自绘
// ============================================================================
void SpeedMeter::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // —— 几何：所有半径对 0/负值 clamp，防控件被压极小时行为未定义 ——
    const qreal side = std::max(1, std::min(width(), height()));
    const QPointF center(width() / 2.0, height() / 2.0);
    const qreal outer_r = side / 2.0 - 4.0;                       // 背景弧半径
    const qreal gauge_r = std::max(1.0, outer_r);                 // clamp
    const qreal tick_outer = gauge_r;                             // 主刻度外端贴弧
    const qreal tick_major_inner = std::max(1.0, gauge_r - 14.0); // 主刻度内端
    const qreal tick_minor_inner = std::max(1.0, gauge_r - 7.0);  // 次刻度内端（更短）
    const qreal needle_len = std::max(1.0, gauge_r - 22.0);       // 指针尖到中心距离

    // —— 背景弧：drawArc 用 Qt 约定（0°=3点、逆时针为正、单位 1/16°）。
    //    kArcStart16/kArcSpan16 已按屏幕角 β 换算好（见顶部约定注释），
    //    与下方刻度/指针描述同一物理弧（左下起、顺时针铺到右下，开口朝下）。
    {
        QPen pen(gauge_color_);
        pen.setWidthF(std::max(1.0, gauge_r * 0.06)); // 粗弧，随尺寸缩放但至少 1px
        pen.setCapStyle(Qt::RoundCap);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        const QRectF arc_rect(center.x() - gauge_r, center.y() - gauge_r, gauge_r * 2, gauge_r * 2);
        p.drawArc(arc_rect, kArcStart16, kArcSpan16);
    }

    // —— 主刻度（kMajorTickCount 条）+ 数字标签 ——
    const QFontMetrics fm(p.font());

    // —— 标签挤压检测：相邻主刻度数字标签的弧距 < 标签宽 + 留白时，整组隐藏 ——
    // 控件被布局压小时，11 个数字标签会挤成一团；藏掉比挤着更易读。
    // 弧距 = label_r × 相邻主刻度夹角(弧度)；最宽标签按量程上限的位数估。
    const qreal kLabelGap = 4.0;
    const qreal label_r = std::max(1.0, tick_major_inner - 14.0);
    const qreal label_arc_step = label_r * degToRad(kSweep / (kMajorTickCount - 1));
    const qreal widest_label_w = fm.horizontalAdvance(QString::number(max_value_));
    const bool show_tick_labels = label_arc_step >= widest_label_w + kLabelGap;

    p.setPen(QPen(tick_color_, 2));
    for (int i = 0; i < kMajorTickCount; ++i) {
        // 第 i 个主刻度对应的屏幕角 β（与 angleForValue 同映射）
        const qreal t = static_cast<qreal>(i) / (kMajorTickCount - 1);
        const qreal ang = kStartScreen + t * kSweep;
        const qreal rad = degToRad(ang);
        const QPointF outer(center.x() + tick_outer * std::cos(rad),
                            center.y() + tick_outer * std::sin(rad));
        const QPointF inner(center.x() + tick_major_inner * std::cos(rad),
                            center.y() + tick_major_inner * std::sin(rad));
        p.drawLine(inner, outer);

        // 数字标签：在主刻度内端再往内一点，标该刻度对应的 value
        // （控件太小时 show_tick_labels=false，整组藏掉防挤压）
        if (show_tick_labels) {
            const QPointF label_pos(center.x() + label_r * std::cos(rad),
                                    center.y() + label_r * std::sin(rad));
            const int tick_value = static_cast<int>(std::round(t * max_value_)); // 该刻度代表的数值
            const QString label_text = QString::number(tick_value);
            p.setPen(tick_color_);
            p.drawText(QRectF(label_pos.x() - fm.horizontalAdvance(label_text) / 2.0,
                              label_pos.y() - fm.height() / 2.0, fm.horizontalAdvance(label_text),
                              fm.height()),
                       Qt::AlignCenter, label_text);
            p.setPen(QPen(tick_color_, 2)); // 复位为刻度线画笔
        }
    }

    // —— 次刻度：每两个主刻度间 5 等分（4 条次刻度） ——
    p.setPen(QPen(tick_color_, 1));
    for (int i = 0; i < kMajorTickCount - 1; ++i) {
        for (int j = 1; j < 5; ++j) {
            const qreal t =
                (static_cast<qreal>(i) + static_cast<qreal>(j) / 5.0) / (kMajorTickCount - 1);
            const qreal ang = kStartScreen + t * kSweep;
            const qreal rad = degToRad(ang);
            const QPointF outer(center.x() + tick_outer * std::cos(rad),
                                center.y() + tick_outer * std::sin(rad));
            const QPointF inner(center.x() + tick_minor_inner * std::cos(rad),
                                center.y() + tick_minor_inner * std::sin(rad));
            p.drawLine(inner, outer);
        }
    }

    // —— 指针：用 translate(center)+rotate 画根粗尖细的多边形 ——
    {
        // needle_angle_ 就是屏幕角 β（顺时针为正）。rotate(β) 把指针（默认指 +x=3 点钟）
        // 直接转到目标方向：v=0(135°)→左下、v=max(45°)→右下，与刻度/标签同映射。
        p.save();
        p.translate(center);
        p.rotate(needle_angle_);

        // 多边形：根在中心附近(宽)，尖往内半径方向(窄)，水平指向 +x 方向
        const qreal base_w = std::max(1.5, needle_len * 0.04); // 根部半宽
        const qreal tip_w = std::max(0.5, needle_len * 0.015); // 尖部半宽
        const qreal tail = needle_len * 0.12;                  // 尾端伸出中心一点
        QPolygonF needle;
        needle << QPointF(-tail, 0.0)         // 尾端中点
               << QPointF(-tail, base_w)      // 根部上
               << QPointF(needle_len, tip_w)  // 尖部上
               << QPointF(needle_len, -tip_w) // 尖部下
               << QPointF(-tail, -base_w);    // 根部下
        p.setPen(Qt::NoPen);
        p.setBrush(needle_color_);
        p.drawPolygon(needle);
        p.restore();
    }

    // —— 中心轴帽：小实心圆 ——
    {
        const qreal cap_r = std::max(1.0, needle_len * 0.08);
        p.setPen(Qt::NoPen);
        p.setBrush(needle_color_.darker(130));
        p.drawEllipse(center, cap_r, cap_r);
    }

    // —— 底部数字读数：当前 value 居中 ——
    {
        const QString text = QString::number(value_);
        QFont f = p.font();
        f.setBold(true);
        f.setPointSizeF(std::max(8.0, side * 0.08)); // 字号随尺寸缩放但至少 8pt
        p.setFont(f);
        const QFontMetrics fm2(f);
        const qreal readout_y = center.y() + gauge_r * 0.55; // 底部偏上一点（开口下方）
        const QRectF text_rect(center.x() - fm2.horizontalAdvance(text) / 2.0,
                               readout_y - fm2.height() / 2.0, fm2.horizontalAdvance(text),
                               fm2.height());
        p.setPen(needle_color_);
        p.drawText(text_rect, Qt::AlignCenter, text);
    }
}

} // namespace AwesomeQt
