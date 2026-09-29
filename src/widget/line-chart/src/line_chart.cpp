/**
 * @file line_chart.cpp
 * @brief LineChart 控件实现——Y 轴自动缩放、网格/数据点/线下填充自绘
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "line_chart.h"

#include <algorithm>

#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>

namespace AwesomeQt {

// 边距常量（像素）：左留 Y 标签、下留 X 标签、上下右留少量内边
constexpr int kMarginLeft = 48;
constexpr int kMarginRight = 12;
constexpr int kMarginTop = 12;
constexpr int kMarginBottom = 28;
constexpr int kGridTickCount = 4;   // Y 轴刻度档数（横线数）
constexpr int kDotRadius = 3;       // 数据点半径
constexpr qreal kFlatPadding = 1.0; // min==max 时的展开余量，防除零

// ============================================================================
// 构造
// ============================================================================
LineChart::LineChart(QWidget* parent) : QWidget(parent) {
    // 背景默认浅色，避免空数据时一片灰白难辨
    setAutoFillBackground(true);
}

// ============================================================================
// 数据入口（业务 API，触发 update）
// ============================================================================
void LineChart::appendPoint(qreal value) {
    values_.append(value);
    update();
}

void LineChart::setData(const QVector<qreal>& values) {
    values_ = values;
    update();
}

void LineChart::clear() {
    if (values_.isEmpty()) {
        return;
    }
    values_.clear();
    update();
}

QVector<qreal> LineChart::data() const {
    return values_;
}

// ============================================================================
// Q_PROPERTY WRITE：纯赋值 + emit + update，不夹业务（无动画，故无栈溢出风险）
// ============================================================================
void LineChart::setLineColor(const QColor& color) {
    if (line_color_ == color) {
        return;
    }
    line_color_ = color;
    emit lineColorChanged(color);
    update();
}

QColor LineChart::lineColor() const {
    return line_color_;
}

void LineChart::setAxisColor(const QColor& color) {
    if (axis_color_ == color) {
        return;
    }
    axis_color_ = color;
    emit axisColorChanged(color);
    update();
}

QColor LineChart::axisColor() const {
    return axis_color_;
}

void LineChart::setGridColor(const QColor& color) {
    if (grid_color_ == color) {
        return;
    }
    grid_color_ = color;
    emit gridColorChanged(color);
    update();
}

QColor LineChart::gridColor() const {
    return grid_color_;
}

void LineChart::setShowGrid(bool on) {
    if (show_grid_ == on) {
        return;
    }
    show_grid_ = on;
    emit showGridChanged(on);
    update();
}

bool LineChart::showGrid() const {
    return show_grid_;
}

void LineChart::setShowDots(bool on) {
    if (show_dots_ == on) {
        return;
    }
    show_dots_ = on;
    emit showDotsChanged(on);
    update();
}

bool LineChart::showDots() const {
    return show_dots_;
}

void LineChart::setShowArea(bool on) {
    if (show_area_ == on) {
        return;
    }
    show_area_ = on;
    emit showAreaChanged(on);
    update();
}

bool LineChart::showArea() const {
    return show_area_;
}

// ============================================================================
// 尺寸
// ============================================================================
QSize LineChart::sizeHint() const {
    return QSize(300, 200);
}

QSize LineChart::minimumSizeHint() const {
    return QSize(120, 80);
}

// ============================================================================
// 自绘：auto-scale Y + 网格 + 坐标轴 + 折线 + 数据点 + 线下填充
// ============================================================================
void LineChart::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // plot 区：总宽高对 0/负值 clamp（布局压极小时仍能算出合理矩形）
    const int w = std::max(1, width());
    const int h = std::max(1, height());
    const qreal plot_left = kMarginLeft;
    const qreal plot_top = kMarginTop;
    const qreal plot_width = std::max(1.0, static_cast<qreal>(w - kMarginLeft - kMarginRight));
    const qreal plot_height = std::max(1.0, static_cast<qreal>(h - kMarginTop - kMarginBottom));
    const qreal plot_bottom = plot_top + plot_height;

    // —— Y 轴 auto-scale：取数据 min..max；单点/min==max 给 ±padding 防除零 ——
    qreal y_min = 0.0;
    qreal y_max = 0.0;
    const bool has_data = !values_.isEmpty();
    if (has_data) {
        // minmax_element 一次扫描同时拿最小最大（替代两次遍历）
        auto [min_it, max_it] = std::minmax_element(values_.constBegin(), values_.constEnd());
        y_min = *min_it;
        y_max = *max_it;
        if (qFuzzyCompare(y_min, y_max)) {
            // 常量/单点：上下各让一点，否则 value→像素分母为 0
            y_min -= kFlatPadding;
            y_max += kFlatPadding;
        }
    }
    const qreal y_range = y_max - y_min;

    // value → 像素 y：plotBottom - (v - yMin) / range * plotHeight
    auto to_y = [&](qreal v) { return plot_bottom - (v - y_min) / y_range * plot_height; };

    // —— 网格：Y 轴 kGridTickCount 档横线 + X 轴竖线（数据点数足够时画） ——
    if (show_grid_) {
        p.setPen(QPen(grid_color_, 1));
        for (int i = 0; i <= kGridTickCount; ++i) {
            const qreal y = plot_top + plot_height * i / kGridTickCount;
            p.drawLine(QPointF(plot_left, y), QPointF(plot_left + plot_width, y));
        }
        const int n = values_.size();
        if (n > 1) {
            for (int i = 0; i < n; ++i) {
                const qreal x = plot_left + plot_width * i / (n - 1);
                p.drawLine(QPointF(x, plot_top), QPointF(x, plot_bottom));
            }
        }
    }

    // —— 坐标轴：左 Y 轴 + 下 X 轴 ——
    p.setPen(QPen(axis_color_, 1));
    p.drawLine(QPointF(plot_left, plot_top), QPointF(plot_left, plot_bottom)); // 左 Y
    p.drawLine(QPointF(plot_left, plot_bottom),
               QPointF(plot_left + plot_width, plot_bottom)); // 下 X

    // —— Y 刻度数字：用 QFontMetrics 算文本宽，右对齐到 Y 轴左侧 ——
    p.setPen(axis_color_);
    QFontMetrics fm(p.font());
    for (int i = 0; i <= kGridTickCount; ++i) {
        // i=0 在顶部 → 对应 y_max；i=kGridTickCount 在底部 → 对应 y_min
        const qreal v = y_max - (y_max - y_min) * i / kGridTickCount;
        const qreal y = plot_top + plot_height * i / kGridTickCount;
        const QString text = QString::number(v, 'f', 1);
        const int text_w = fm.horizontalAdvance(text);
        // 右对齐到 Y 轴左侧、垂直大致居中（baseline 修正）
        p.drawText(QPointF(plot_left - text_w - 4, y + fm.ascent() / 2), text);
    }

    // —— 空数据：到此结束，不画折线（不崩） ——
    if (!has_data) {
        return;
    }

    // —— 折线 / 线下填充 / 数据点 ——
    QPolygonF poly;
    const int n = values_.size();
    for (int i = 0; i < n; ++i) {
        const qreal x = (n == 1) ? plot_left + plot_width / 2 // 单点居中
                                 : plot_left + plot_width * i / (n - 1);
        poly.append(QPointF(x, to_y(values_[i])));
    }

    // 线下填充：折线下方闭合到 plot bottom，淡色 fill
    if (show_area_ && n >= 2) {
        QPainterPath area;
        area.moveTo(poly.first().x(), plot_bottom);
        area.addPolygon(poly);
        area.lineTo(poly.last().x(), plot_bottom);
        area.closeSubpath();
        QColor fill = line_color_;
        fill.setAlpha(60); // 半透明填充，与折线同色系
        p.setPen(Qt::NoPen);
        p.setBrush(fill);
        p.drawPath(area);
    }

    // 折线
    p.setPen(QPen(line_color_, 2));
    p.setBrush(Qt::NoBrush);
    p.drawPolyline(poly);

    // 数据点：小圆
    if (show_dots_) {
        p.setPen(Qt::NoPen);
        p.setBrush(line_color_);
        const qreal r = kDotRadius; // 固定半径，布局极小时不放大不模糊
        for (const QPointF& pt : poly) {
            p.drawEllipse(pt, r, r);
        }
    }
}

} // namespace AwesomeQt
