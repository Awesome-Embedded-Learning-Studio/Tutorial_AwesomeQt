/**
 * @file cpu_history_view.cpp
 * @brief CpuHistoryView 实现——背景网格 + 折线 + 当前值标注
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "cpu_history_view.h"

#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <algorithm>

CpuHistoryView::CpuHistoryView(int capacity, QWidget* parent)
    : QWidget(parent), capacity_(capacity > 4 ? capacity : 4) {
    setMinimumSize(220, 90);
    setAutoFillBackground(true);
    // 深色底，与亮色折线对比强
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(24, 24, 28));
    setPalette(pal);
}

void CpuHistoryView::push(int percent) {
    if (static_cast<int>(samples_.size()) >= capacity_) {
        samples_.erase(samples_.begin()); // 满了丢最旧，滑动窗口
    }
    samples_.push_back(percent);
    update();
}

QSize CpuHistoryView::sizeHint() const {
    return {capacity_ * 4 + 4, 120};
}

void CpuHistoryView::paintEvent(QPaintEvent* event) {
    (void)event;
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const QRect r = rect().adjusted(0, 0, -1, -1);
    const int w = r.width();
    const int h = r.height();

    // 1. 水平网格线（25/50/75/100% 参考线，浅灰）
    p.setPen(QPen(QColor(60, 60, 70), 1, Qt::DotLine));
    for (int pct = 25; pct <= 100; pct += 25) {
        const int y = r.top() + h - h * pct / 100;
        p.drawLine(r.left(), y, r.right(), y);
    }

    // 2. 折线 + 半透明填充：按「连续有效段」分组，每段单独画折线和填充多边形——
    //    缺口处（-1 无效点）折线断开、填充也断开（不越过缺口），二者语义一致（review 抓的渲染
    //    bug）。
    if (samples_.size() >= 2) {
        const double dx = static_cast<double>(w) / (samples_.size() - 1);
        auto x_at = [&](size_t i) { return r.left() + static_cast<int>(i * dx); };
        auto y_at = [&](int v) { return r.top() + h - h * v / 100; };

        size_t i = 0;
        while (i < samples_.size()) {
            // 跳过无效点（缺口）
            while (i < samples_.size() && (samples_[i] < 0 || samples_[i] > 100)) {
                ++i;
            }
            if (i >= samples_.size()) {
                break;
            }
            // 收集本段连续有效点 [seg_start, seg_end)
            const size_t seg_start = i;
            while (i < samples_.size() && samples_[i] >= 0 && samples_[i] <= 100) {
                ++i;
            }
            const size_t seg_end = i;
            if (seg_end - seg_start < 2) {
                continue; // 单点画不了折线
            }
            // 本段折线
            QPainterPath seg;
            seg.moveTo(x_at(seg_start), y_at(samples_[seg_start]));
            for (size_t j = seg_start + 1; j < seg_end; ++j) {
                seg.lineTo(x_at(j), y_at(samples_[j]));
            }
            // 本段填充多边形（下到段首闭合）——只在段内，不越过缺口
            QPainterPath fill = seg;
            fill.lineTo(x_at(seg_end - 1), r.bottom());
            fill.lineTo(x_at(seg_start), r.bottom());
            fill.closeSubpath();
            p.fillPath(fill, QColor(80, 180, 240, 70));
            p.setPen(QPen(QColor(120, 200, 240), 2));
            p.setBrush(Qt::NoBrush);
            p.drawPath(seg);
        }
    }

    // 3. 边框
    p.setPen(QPen(QColor(80, 80, 90), 1));
    p.setBrush(Qt::NoBrush);
    p.drawRect(rect().adjusted(0, 0, -1, -1));
}
