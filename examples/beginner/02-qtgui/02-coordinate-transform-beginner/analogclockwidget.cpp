#include "analogclockwidget.h"

#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QTimer>
#include <QTime>
#include <QPolygonF>
#include <QPointF>

AnalogClockWidget::AnalogClockWidget(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("模拟时钟 —— 坐标变换实战");
    resize(400, 400);

    // 每秒刷新一次
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, qOverload<>(&QWidget::update));
    timer->start(1000);
}

void AnalogClockWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(250, 250, 245));

    // 把坐标原点移到窗口中央
    painter.translate(width() / 2.0, height() / 2.0);

    // 计算表盘半径（取宽高的较小值的一半，留点边距）
    int side = qMin(width(), height());
    double radius = side / 2.0 - 20;

    // ===== 画表盘 =====
    drawClockFace(&painter, radius);

    // ===== 画指针 =====
    QTime time = QTime::currentTime();

    // 时针：每小时 30 度，每分钟 0.5 度
    double hourAngle = (time.hour() % 12 + time.minute() / 60.0) * 30.0;
    drawHand(&painter, hourAngle, radius * 0.5, 6, Qt::black);

    // 分针：每分钟 6 度，每秒 0.1 度
    double minuteAngle = (time.minute() + time.second() / 60.0) * 6.0;
    drawHand(&painter, minuteAngle, radius * 0.7, 4, Qt::darkGray);

    // 秒针：每秒 6 度
    double secondAngle = time.second() * 6.0;
    drawHand(&painter, secondAngle, radius * 0.85, 2, Qt::red);

    // 画中心圆点
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(Qt::black));
    painter.drawEllipse(QPointF(0, 0), 5, 5);
}

void AnalogClockWidget::drawClockFace(QPainter *painter, double radius)
{
    painter->save();

    // 外框
    QPen outerPen(Qt::darkGray, 3);
    painter->setPen(outerPen);
    painter->setBrush(QBrush(QColor(255, 255, 250)));
    painter->drawEllipse(QPointF(0, 0), radius, radius);

    // 刻度标记：60 个小刻度，12 个大刻度
    for (int i = 0; i < 60; ++i) {
        painter->save();
        painter->rotate(i * 6.0);  // 每个刻度 6 度

        if (i % 5 == 0) {
            // 大刻度（每 5 分钟一个）
            painter->setPen(QPen(Qt::black, 3));
            painter->drawLine(0, static_cast<int>(-radius + 8),
                              0, static_cast<int>(-radius + 25));

            // 数字标签
            int hour = i / 5;
            if (hour == 0) hour = 12;
            painter->setFont(QFont("Arial", 12, QFont::Bold));
            QRectF textRect(-15, -radius + 28, 30, 20);
            painter->drawText(textRect, Qt::AlignCenter, QString::number(hour));
        } else {
            // 小刻度
            painter->setPen(QPen(Qt::gray, 1));
            painter->drawLine(0, static_cast<int>(-radius + 8),
                              0, static_cast<int>(-radius + 15));
        }

        painter->restore();
    }

    painter->restore();
}

void AnalogClockWidget::drawHand(QPainter *painter, double angle, double length, int width, const QColor &color)
{
    painter->save();

    // rotate 的 0 度是 3 点方向（水平右），时钟的 0 度是 12 点方向（垂直上）
    // 所以需要减 90 度来对齐
    painter->rotate(angle - 90);

    // 用多边形画一个尖头指针
    QPolygonF hand;
    hand << QPointF(0, -width / 2.0)
         << QPointF(length, 0)
         << QPointF(0, width / 2.0)
         << QPointF(-length * 0.15, 0);  // 尾部短一点

    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(color));
    painter->drawPolygon(hand);

    painter->restore();
}
