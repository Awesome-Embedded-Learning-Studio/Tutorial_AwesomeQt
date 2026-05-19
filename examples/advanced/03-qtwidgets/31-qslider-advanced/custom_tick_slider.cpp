/// @file    custom_tick_slider.cpp
/// @brief   CustomTickSlider 类实现——点击跳转与自定义刻度标注。
///
/// 对应教程：进阶层 03-QtWidgets/31-QSlider 进阶。

#include "custom_tick_slider.h"

#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionSlider>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

CustomTickSlider::CustomTickSlider(Qt::Orientation orientation, QWidget* parent)
    : QSlider(orientation, parent)
{
    // 默认的刻度标注：五等分
    m_tickRatios = {0.0, 0.25, 0.5, 0.75, 1.0};

    // 开启系统自带刻度线，让自定义标注和系统刻度对齐
    setTickPosition(QSlider::TicksBelow);
    setTickInterval(25);
}

// ─────────────────────────────────────────────────────────────────────────────
// 公有方法
// ─────────────────────────────────────────────────────────────────────────────

void CustomTickSlider::setTickLabels(const QVector<double>& ratios)
{
    m_tickRatios = ratios;
    update();    // 触发重绘以显示新的刻度标注
}

// ─────────────────────────────────────────────────────────────────────────────
// 鼠标事件重写——直接映射点击位置到滑块值
// ─────────────────────────────────────────────────────────────────────────────

void CustomTickSlider::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        // 将点击位置直接映射为值，绕过默认的"移动手柄到点击位置"逻辑
        setValue(posToValue(event->position().toPoint()));
        // 事件已处理，不再传递给基类（避免默认行为覆盖我们的映射）
        event->accept();
    } else {
        QSlider::mousePressEvent(event);
    }
}

void CustomTickSlider::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton) {
        // 拖拽过程中同样使用直接映射，保持和点击一致的行为
        setValue(posToValue(event->position().toPoint()));
        event->accept();
    } else {
        QSlider::mouseMoveEvent(event);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 绘制事件——在 groove 下方添加百分比数值标注
// ─────────────────────────────────────────────────────────────────────────────

void CustomTickSlider::paintEvent(QPaintEvent* event)
{
    // 先让基类完成默认绘制（groove、handle、tick lines）
    QSlider::paintEvent(event);

    if (m_tickRatios.isEmpty()) {
        return;
    }

    QPainter painter(this);
    painter.setPen(palette().text().color());

    QRect gr = grooveRect();
    if (!gr.isValid()) {
        return;
    }

    // 文字高度偏移：画在 groove 下方，留出系统刻度线的空间
    // QSlider::TicksBelow 的刻度线大约在 groove 底部下方 10px，文字再往下 8px
    const int kTextOffsetY = 28;

    for (double ratio : m_tickRatios) {
        // 将比例值换算到 groove 上的像素位置
        int x = gr.left() + static_cast<int>(ratio * gr.width());

        // 将比例换算为百分比文字
        int percent = static_cast<int>(ratio * 100);
        QString text = QString::number(percent) + QStringLiteral("%");

        // 获取文字尺寸，居中对齐
        QFontMetrics fm = painter.fontMetrics();
        int textWidth = fm.horizontalAdvance(text);
        painter.drawText(x - textWidth / 2, gr.bottom() + kTextOffsetY, text);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 私有工具方法
// ─────────────────────────────────────────────────────────────────────────────

int CustomTickSlider::posToValue(const QPoint& pos) const
{
    QRect gr = grooveRect();
    if (!gr.isValid()) {
        return value();
    }

    double ratio;
    if (orientation() == Qt::Horizontal) {
        ratio = static_cast<double>(pos.x() - gr.left()) / gr.width();
    } else {
        // 垂直方向：上方对应最大值，下方对应最小值
        ratio = 1.0 - static_cast<double>(pos.y() - gr.top()) / gr.height();
    }

    // Clamp 到 [0.0, 1.0]，防止点击超出 groove 范围时产生无效值
    ratio = qBound(0.0, ratio, 1.0);

    return minimum() + static_cast<int>(ratio * (maximum() - minimum()));
}

QRect CustomTickSlider::grooveRect() const
{
    QStyleOptionSlider opt;
    initStyleOption(&opt);

    // 通过 QStyle 获取 groove 的真实矩形——这考虑了 margin、padding、notch 区域等偏移
    // 比直接用 rect() 准确得多，尤其是在 TicksBelow 开启时
    return style()->subControlRect(
        QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
}
