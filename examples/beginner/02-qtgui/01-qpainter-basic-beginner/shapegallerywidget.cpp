#include "shapegallerywidget.h"

#include <QBrush>
#include <QColor>
#include <QFont>
#include <QLinearGradient>
#include <QPainter>
#include <QPen>
#include <QPolygon>

#include <cmath>

ShapeGalleryWidget::ShapeGalleryWidget(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("QPainter 基本图形展示");
    resize(600, 500);
}

void ShapeGalleryWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 开启抗锯齿，线条更平滑

    // ------------------------------------------------------------------
    // 1. 直线 —— 画一条红色的对角线
    // ------------------------------------------------------------------
    QPen redPen(Qt::red, 3);
    painter.setPen(redPen);
    painter.drawLine(20, 20, 180, 100);

    // ------------------------------------------------------------------
    // 2. 矩形 —— 蓝色边框 + 浅蓝填充
    // ------------------------------------------------------------------
    QPen bluePen(Qt::blue, 2);
    QBrush lightBlueBrush(QColor(173, 216, 230));  // 浅蓝色
    painter.setPen(bluePen);
    painter.setBrush(lightBlueBrush);
    painter.drawRect(200, 20, 150, 80);

    // ------------------------------------------------------------------
    // 3. 椭圆 —— 绿色边框 + 半透明绿色填充
    // ------------------------------------------------------------------
    QPen greenPen(Qt::darkGreen, 2);
    QBrush transGreenBrush(QColor(0, 200, 0, 120));  // RGBA 带透明度
    painter.setPen(greenPen);
    painter.setBrush(transGreenBrush);
    painter.drawEllipse(400, 20, 160, 80);

    // ------------------------------------------------------------------
    // 4. 圆角矩形 —— 橙色边框 + 渐变填充
    // ------------------------------------------------------------------
    QPen orangePen(QColor(255, 140, 0), 2);
    QLinearGradient gradient(20, 150, 20, 250);
    gradient.setColorAt(0.0, QColor(255, 200, 100));
    gradient.setColorAt(1.0, QColor(255, 140, 0));
    painter.setPen(orangePen);
    painter.setBrush(QBrush(gradient));
    painter.drawRoundedRect(20, 150, 200, 100, 15, 15);

    // ------------------------------------------------------------------
    // 5. 多边形（五角星）—— 紫色
    // ------------------------------------------------------------------
    QPen purplePen(Qt::darkMagenta, 2);
    painter.setPen(purplePen);
    painter.setBrush(QBrush(QColor(200, 100, 255, 150)));

    // 计算五角星的顶点坐标
    QPolygon star;
    qreal cx = 370, cy = 200, outerR = 60, innerR = 25;
    for (int i = 0; i < 5; ++i) {
        qreal outerAngle = M_PI / 2 + i * 2 * M_PI / 5;
        qreal innerAngle = outerAngle + M_PI / 5;
        star << QPoint(static_cast<int>(cx + outerR * std::cos(outerAngle)),
                       static_cast<int>(cy - outerR * std::sin(outerAngle)));
        star << QPoint(static_cast<int>(cx + innerR * std::cos(innerAngle)),
                       static_cast<int>(cy - innerR * std::sin(innerAngle)));
    }
    painter.drawPolygon(star);

    // ------------------------------------------------------------------
    // 6. 虚线 —— 用 Qt::DashLine 画一条虚线
    // ------------------------------------------------------------------
    QPen dashPen(Qt::gray, 2, Qt::DashLine);
    painter.setPen(dashPen);
    painter.setBrush(Qt::NoBrush);  // 不填充
    painter.drawLine(20, 300, 580, 300);

    // ------------------------------------------------------------------
    // 7. 文字 —— 用 drawText 的 QRect + 对齐标志版本（推荐）
    // ------------------------------------------------------------------
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 14, QFont::Bold));
    // 使用 QRect 版本指定绘制区域 + 对齐标志，不用担心基线问题
    painter.drawText(QRect(20, 320, 560, 40), Qt::AlignCenter,
                     "QPainter 基本图形展示 - by AwesomeQt");

    // ------------------------------------------------------------------
    // 8. fillRect —— 快速填充一个矩形区域（不需要设置画笔）
    // ------------------------------------------------------------------
    painter.fillRect(20, 380, 560, 80, QColor(240, 248, 255));  // 爱丽丝蓝背景
    painter.setPen(QPen(Qt::darkGray, 1));
    painter.drawRect(20, 380, 560, 80);

    // 在填充区域里画小圆点阵列
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(QColorConstants::Svg::cornflowerblue));
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 20; ++col) {
            painter.drawEllipse(35 + col * 28, 395 + row * 25, 12, 12);
        }
    }
}
