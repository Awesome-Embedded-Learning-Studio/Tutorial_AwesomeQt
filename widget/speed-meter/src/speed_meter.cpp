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
// —— 角度约定（整套在 paintEvent 自洽，与 drawArc 角度分别处理）——
// 数学角度（三角函数用）：0°=3 点钟方向，逆时针为正，Y 轴朝下时视觉上顺时针。
//   表盘弧从 225°（左下）顺时针扫 270° 到 -45°（右下），开口朝下。
//   value=0   → 225°（左下端）
//   value=max → -45°（=315°，右下端）
constexpr qreal kStartAngle = 225.0; // 最小值角度（左下）
constexpr qreal kSweepAngle = 270.0; // 顺时针扫角
constexpr int kMajorTickCount = 11;  // 主刻度条数（含两端，把量程 10 等分）

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
// value → 数学角度映射
// ============================================================================
qreal SpeedMeter::angleForValue(int v) const {
    const int max = std::max(1, max_value_);          // 防 max_value_<=0 除零
    const int clamped = std::clamp(v, 0, max_value_); // 夹到 [0, max]
    // 从起始角 225° 顺时针扫：角度随 value 减小（225 → -45）
    return kStartAngle - (static_cast<qreal>(clamped) / max) * kSweepAngle;
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

    // —— 背景弧：drawArc 角度是 1/16°、0°=3点、正值逆时针。
    //    我们要从 225° 顺时针扫到 -45°（视觉顺时针铺开 270°）。
    //    drawArc 起始角 = 225°*16，扫角 = -270°*16（负=顺时针）。
    {
        QPen pen(gauge_color_);
        pen.setWidthF(std::max(1.0, gauge_r * 0.06)); // 粗弧，随尺寸缩放但至少 1px
        pen.setCapStyle(Qt::RoundCap);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        // drawArc 参数：x,y,w,h, 起始角(1/16°), 扫角(1/16°)；负扫角=顺时针
        const QRectF arc_rect(center.x() - gauge_r, center.y() - gauge_r, gauge_r * 2, gauge_r * 2);
        p.drawArc(arc_rect, static_cast<int>(kStartAngle * 16),
                  static_cast<int>(-kSweepAngle * 16));
    }

    // —— 主刻度（kMajorTickCount 条）+ 数字标签 ——
    const QFontMetrics fm(p.font());
    p.setPen(QPen(tick_color_, 2));
    for (int i = 0; i < kMajorTickCount; ++i) {
        // 第 i 个主刻度对应的数学角度（与 angleForValue 同映射）
        const qreal t = static_cast<qreal>(i) / (kMajorTickCount - 1);
        const qreal ang = kStartAngle - t * kSweepAngle;
        const qreal rad = degToRad(ang);
        const QPointF outer(center.x() + tick_outer * std::cos(rad),
                            center.y() + tick_outer * std::sin(rad));
        const QPointF inner(center.x() + tick_major_inner * std::cos(rad),
                            center.y() + tick_major_inner * std::sin(rad));
        p.drawLine(inner, outer);

        // 数字标签：在主刻度内端再往内一点，标该刻度对应的 value
        const qreal label_r = std::max(1.0, tick_major_inner - 14.0);
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

    // —— 次刻度：每两个主刻度间 5 等分（4 条次刻度） ——
    p.setPen(QPen(tick_color_, 1));
    for (int i = 0; i < kMajorTickCount - 1; ++i) {
        for (int j = 1; j < 5; ++j) {
            const qreal t =
                (static_cast<qreal>(i) + static_cast<qreal>(j) / 5.0) / (kMajorTickCount - 1);
            const qreal ang = kStartAngle - t * kSweepAngle;
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
        // rotate 以 12 点钟（数学 -90°）为 0、顺时针为正。我们的数学角度 0°=3 点钟，
        // 故 painter 旋转角 = needle_angle_ + 90°（把 0° 摆正到 3 点，再 +90° 适配 rotate 基准）。
        // 经验证：value=0(225°)→rotate 315° 指向左下，value=max(-45°)→rotate 45° 指向右下。
        p.save();
        p.translate(center);
        p.rotate(needle_angle_ + 90.0);

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
