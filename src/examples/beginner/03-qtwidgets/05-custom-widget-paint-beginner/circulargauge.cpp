#include "circulargauge.h"

#include <QFont>
#include <QPainter>
#include <QPen>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QDebug>

// ============================================================================
// CircularGauge: 自定义圆形仪表盘控件
// ============================================================================
CircularGauge::CircularGauge(const QString &title, QWidget *parent)
    : QWidget(parent), m_title(title), m_value(0.0)
{
    // 设置 sizePolicy 让控件可以在水平方向拉伸，垂直方向固定
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

QSize CircularGauge::sizeHint() const { return QSize(250, 170); }

QSize CircularGauge::minimumSizeHint() const { return QSize(120, 90); }

double CircularGauge::value() const { return m_value; }

void CircularGauge::setValue(double newValue)
{
    newValue = qBound(0.0, newValue, 100.0);
    if (qFuzzyCompare(m_value, newValue)) {
        return;
    }

    // 使用 QPropertyAnimation 做平滑过渡
    auto *animation = new QPropertyAnimation(this, "value");
    animation->setDuration(500);
    animation->setStartValue(m_value);
    animation->setEndValue(newValue);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void CircularGauge::setAnimatedValue(double newValue)
{
    newValue = qBound(0.0, newValue, 100.0);
    if (qFuzzyCompare(m_value, newValue)) {
        return;
    }
    m_value = newValue;
    emit valueChanged(newValue);
    update();  // 异步请求重绘，不立即执行 paintEvent
}

void CircularGauge::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // ---- 计算几何参数（全部基于控件实际尺寸动态计算）----
    int w = width();
    int h = height();
    int side = qMin(w, h);

    // 仪表盘圆弧的半径，留出边距
    double radius = side / 2.0 - 15;
    double penWidth = qMax(radius * 0.12, 6.0);

    QPointF center(w / 2.0, h * 0.55);

    // 仪表盘弧线的包围矩形
    QRectF arcRect(center.x() - radius, center.y() - radius,
                   radius * 2, radius * 2);

    // 半圆弧：从 180° 到 0°（9 点钟 → 3 点钟方向）
    // Qt 角度单位是 1/16 度
    int startAngle = 180 * 16;    // 9 点钟方向
    int spanAngle = -180 * 16;    // 逆时针扫过 180 度

    // ---- 画背景半圆弧（灰色）----
    painter.setPen(QPen(QColor("#E8E8E8"), penWidth, Qt::SolidLine, Qt::RoundCap));
    painter.drawArc(arcRect, startAngle, spanAngle);

    // ---- 画进度弧（根据值计算颜色和扫过角度）----
    double ratio = m_value / 100.0;
    int valueSpan = static_cast<int>(-180.0 * ratio * 16);

    if (valueSpan != 0) {
        QColor arcColor = getArcColor(ratio);
        painter.setPen(QPen(arcColor, penWidth, Qt::SolidLine, Qt::RoundCap));
        painter.drawArc(arcRect, startAngle, valueSpan);
    }

    // ---- 画中心数值 ----
    int fontSize = qMax(static_cast<int>(radius * 0.35), 12);
    QFont valueFont("Arial", fontSize, QFont::Bold);
    painter.setFont(valueFont);
    painter.setPen(QColor("#2C3E50"));
    QRectF textRect(center.x() - radius, center.y() - radius * 0.4,
                    radius * 2, radius * 0.8);
    painter.drawText(textRect, Qt::AlignCenter,
                     QString::number(static_cast<int>(m_value)));

    // ---- 画标题（控件顶部）----
    QFont titleFont("Arial", qMax(fontSize / 2, 10));
    painter.setFont(titleFont);
    painter.setPen(QColor("#7F8C8D"));
    QRectF titleRect(0, 5, w, 20);
    painter.drawText(titleRect, Qt::AlignCenter, m_title);

    // ---- 画底部刻度标签（0 和 100）----
    QFont scaleFont("Arial", qMax(fontSize / 3, 8));
    painter.setFont(scaleFont);
    painter.setPen(QColor("#95A5A6"));

    // 左侧 "0"
    QPointF leftPoint(center.x() - radius - penWidth / 2 - 2,
                      center.y() + penWidth);
    painter.drawText(QRectF(leftPoint.x() - 15, leftPoint.y(),
                            30, 16), Qt::AlignCenter, "0");

    // 右侧 "100"
    QPointF rightPoint(center.x() + radius + penWidth / 2 - 28,
                       center.y() + penWidth);
    painter.drawText(QRectF(rightPoint.x(), rightPoint.y(),
                            30, 16), Qt::AlignCenter, "100");
}

QColor CircularGauge::getArcColor(double ratio) const
{
    if (ratio < 0.5) {
        // 绿色 → 黄色
        int r = static_cast<int>(255 * (ratio * 2));
        return QColor(r, 200, 50);
    } else {
        // 黄色 → 红色
        int g = static_cast<int>(200 * (1.0 - (ratio - 0.5) * 2));
        return QColor(255, g, 50);
    }
}
