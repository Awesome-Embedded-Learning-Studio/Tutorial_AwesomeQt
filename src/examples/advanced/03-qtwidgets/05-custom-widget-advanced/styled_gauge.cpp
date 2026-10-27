/// @file    styled_gauge.cpp
/// @brief   StyledGauge 类实现——QStylePainter 绘制环形仪表盘 + heightForWidth 约束。
///
/// 对应教程：进阶层 03-QtWidgets/05-自定义控件进阶。

#include "styled_gauge.h"

#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>
#include <QStylePainter>

#include <QtMath>

// ─────────────────────────────────────────────────────────────────────────────
// 常量定义
// ─────────────────────────────────────────────────────────────────────────────

/// 默认建议尺寸（正方形边长）
static constexpr int kDefaultSize = 120;

/// 最小建议尺寸（正方形边长）
static constexpr int kMinSize = 60;

/// 环形弧线的起始角度（12 点钟方向，Qt 角度制：0 = 3 点钟，90 = 12 点钟）
static constexpr int kStartAngle = 90 * 16;

/// 完整圆弧对应的 1/16 度数
static constexpr int kFullSpan = 360 * 16;

/// 环形线条的默认宽度（像素），小于此值视觉上难以分辨
static constexpr int kPenWidth = 8;

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

StyledGauge::StyledGauge(QWidget* parent)
    : QWidget(parent)
    , m_progress(0)
{
    // 设置 heightForWidth 标志：必须先取出 QSizePolicy，修改后再设回去
    // sizePolicy() 返回的是拷贝，链式调用 setHeightForWidth 不会生效
    QSizePolicy sp(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sp.setHeightForWidth(true);
    setSizePolicy(sp);
}

// ─────────────────────────────────────────────────────────────────────────────
// 属性访问器
// ─────────────────────────────────────────────────────────────────────────────

int StyledGauge::progress() const
{
    return m_progress;
}

void StyledGauge::setProgress(int value)
{
    const int clamped = qBound(0, value, 100);
    if (clamped == m_progress) {
        return;
    }
    m_progress = clamped;
    Q_EMIT progressChanged(m_progress);
    update();    // 触发重绘
}

// ─────────────────────────────────────────────────────────────────────────────
// 尺寸约束
// ─────────────────────────────────────────────────────────────────────────────

QSize StyledGauge::sizeHint() const
{
    return QSize(kDefaultSize, kDefaultSize);
}

QSize StyledGauge::minimumSizeHint() const
{
    return QSize(kMinSize, kMinSize);
}

int StyledGauge::heightForWidth(int w) const
{
    // 1:1 宽高比——布局系统分配多宽，高度就请求多高
    return w;
}

// ─────────────────────────────────────────────────────────────────────────────
// 绘制
// ─────────────────────────────────────────────────────────────────────────────

void StyledGauge::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    // 使用 QStylePainter 代替 QPainter——内部持有当前平台的 QStyle 指针
    QStylePainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 准备 QStyleOption，initFrom() 自动提取 enabled/palette/fontMetrics/state
    QStyleOption opt;
    opt.initFrom(this);

    const int side = qMin(width(), height());
    // 扣除线条宽度，让弧线不被控件边缘截断
    const int penW = qMin(kPenWidth, side / 6);
    const QRectF arcRect(
        (width() - side) / 2.0 + penW,
        (height() - side) / 2.0 + penW,
        side - 2.0 * penW,
        side - 2.0 * penW);

    // ── 背景弧线 ──
    // 从系统 palette 取中灰色作为背景轨道，避免硬编码颜色
    painter.setPen(QPen(opt.palette.mid().color(), penW, Qt::SolidLine, Qt::RoundCap));
    painter.drawArc(arcRect, kStartAngle, -kFullSpan);

    // ── 前景弧线（进度） ──
    if (m_progress > 0) {
        // 从系统 palette 取高亮色作为进度色，在深色/浅色主题下自动适配
        const QColor progressColor = opt.palette.highlight().color();
        painter.setPen(QPen(progressColor, penW, Qt::SolidLine, Qt::RoundCap));

        // 弧线跨度按比例缩放，负值表示顺时针
        const int spanAngle = -qFloor(m_progress / 100.0 * kFullSpan);
        painter.drawArc(arcRect, kStartAngle, spanAngle);
    }

    // ── 中央文本 ──
    // 使用 QStylePainter::drawItemText 而非 QPainter::drawText
    // drawItemText 会自动使用系统 palette 和对齐方式，风格感知
    painter.drawItemText(
        rect(),
        Qt::AlignCenter,
        opt.palette,
        opt.state & QStyle::State_Enabled,
        QStringLiteral("%1%").arg(m_progress),
        QPalette::Text);
}
