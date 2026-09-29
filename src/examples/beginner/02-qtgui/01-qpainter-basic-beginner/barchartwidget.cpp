#include "barchartwidget.h"

#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QLinearGradient>
#include <QPainter>
#include <QPen>
#include <QStringList>

#include <algorithm>

BarChartWidget::BarChartWidget(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("简易柱状图");
    resize(600, 400);
    setData({85, 60, 95, 40, 75, 88, 55});
}

void BarChartWidget::setData(const QList<int> &data)
{
    m_data = data;
    m_labels.clear();
    for (int i = 0; i < data.size(); ++i) {
        m_labels << QString("%1").arg(QChar('A' + i));
    }
    update();  // 触发重绘
}

void BarChartWidget::paintEvent(QPaintEvent *)
{
    if (m_data.isEmpty()) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const int marginTop = 40;
    const int marginBottom = 50;
    const int marginLeft = 50;
    const int marginRight = 30;

    int chartWidth = width() - marginLeft - marginRight;
    int chartHeight = height() - marginTop - marginBottom;

    // 找到数据最大值，用于计算比例
    int maxVal = *std::max_element(m_data.begin(), m_data.end());
    if (maxVal <= 0) maxVal = 1;  // 防止除零

    int barCount = m_data.size();
    int gap = 15;  // 柱子间距
    int barWidth = (chartWidth - gap * (barCount + 1)) / barCount;

    // 画标题
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 16, QFont::Bold));
    painter.drawText(QRect(marginLeft, 5, chartWidth, 30),
                     Qt::AlignCenter, "数据柱状图");

    // 画底部基线
    painter.setPen(QPen(Qt::gray, 1));
    painter.drawLine(marginLeft, height() - marginBottom,
                     width() - marginRight, height() - marginBottom);

    // 画纵轴参考线（0%, 25%, 50%, 75%, 100%）
    painter.setPen(QPen(QColor(220, 220, 220), 1, Qt::DotLine));
    QFontMetrics fm(painter.font());
    painter.setFont(QFont("Arial", 9));
    for (int i = 0; i <= 4; ++i) {
        int y = marginTop + chartHeight * i / 4;
        painter.drawLine(marginLeft, y, width() - marginRight, y);
        int val = maxVal * (4 - i) / 4;
        painter.setPen(Qt::gray);
        painter.drawText(QRect(5, y - 10, marginLeft - 10, 20),
                         Qt::AlignRight | Qt::AlignVCenter,
                         QString::number(val));
        painter.setPen(QPen(QColor(220, 220, 220), 1, Qt::DotLine));
    }

    // 画柱子
    QStringList barColors = {
        "#4A90D9", "#67B7DC", "#F4A460",
        "#E74C3C", "#2ECC71", "#9B59B6", "#F39C12"
    };

    for (int i = 0; i < barCount; ++i) {
        int barHeight = static_cast<int>(
            static_cast<double>(m_data[i]) / maxVal * chartHeight);
        int x = marginLeft + gap + i * (barWidth + gap);
        int y = marginTop + chartHeight - barHeight;

        // 渐变填充
        QLinearGradient barGradient(x, y, x, y + barHeight);
        QColor baseColor(barColors[i % barColors.size()]);
        barGradient.setColorAt(0.0, baseColor.lighter(120));
        barGradient.setColorAt(1.0, baseColor);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(barGradient));
        painter.drawRoundedRect(x, y, barWidth, barHeight, 4, 4);

        // 柱子顶部显示数值
        painter.setPen(Qt::black);
        painter.setFont(QFont("Arial", 10, QFont::Bold));
        painter.drawText(QRect(x, y - 25, barWidth, 22),
                         Qt::AlignCenter, QString::number(m_data[i]));

        // 底部标签
        painter.setFont(QFont("Arial", 11));
        painter.drawText(QRect(x, height() - marginBottom + 5, barWidth, 20),
                         Qt::AlignCenter, m_labels.value(i, ""));
    }
}
