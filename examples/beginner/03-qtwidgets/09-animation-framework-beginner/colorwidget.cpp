// QtWidgets 入门示例 09: 属性动画框架基础
// ColorWidget 实现

#include "colorwidget.h"

#include <QPainter>
#include <QFont>

ColorWidget::ColorWidget(const QColor &color, const QString &text,
                         QWidget *parent)
    : QWidget(parent)
    , m_targetColor(color)
    , m_bgColor(Qt::white)
    , m_text(text)
{
    setFixedSize(160, 100);
    setStyleSheet("border-radius: 8px;");
}

QColor ColorWidget::backgroundColor() const { return m_bgColor; }

void ColorWidget::setBackgroundColor(const QColor &color)
{
    if (m_bgColor != color) {
        m_bgColor = color;
        update();
        emit backgroundColorChanged(color);
    }
}

void ColorWidget::reset()
{
    m_bgColor = Qt::white;
    update();
}

void ColorWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制圆角矩形背景
    painter.setBrush(m_bgColor);
    painter.setPen(QPen(m_bgColor.darker(120), 2));
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 8, 8);

    // 绘制文字标签
    painter.setPen(m_bgColor.lightness() > 128 ? Qt::black : Qt::white);
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    painter.drawText(rect(), Qt::AlignCenter, m_text);
}
