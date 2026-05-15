/// @file    paint_widget.cpp
/// @brief   PaintWidget 类实现——双缓冲、合成模式与抗锯齿演示。
///
/// 对应教程：进阶层 02-QtGui/01-QPainter 进阶。

#include "paint_widget.h"

#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>
#include <QResizeEvent>

PaintWidget::PaintWidget(QWidget* parent)
    : QWidget(parent)
{
    resize(800, 600);
    setWindowTitle(QStringLiteral("QPainter Advanced Demo"));
}

void PaintWidget::paintEvent(QPaintEvent* /*event*/)
{
    if (m_buffer.isNull()) {
        return;
    }

    // 双缓冲：直接将离屏画布 blit 到屏幕，无闪烁
    QPainter screenPainter(this);
    screenPainter.drawPixmap(0, 0, m_buffer);
}

void PaintWidget::resizeEvent(QResizeEvent* event)
{
    // 窗口大小变化时重建画布，避免拉伸模糊
    m_buffer = QPixmap(event->size());
    m_buffer.fill(Qt::white);
    renderToBuffer();
    update();
}

void PaintWidget::renderToBuffer()
{
    if (m_buffer.isNull()) {
        return;
    }

    QPainter painter(&m_buffer);
    painter.setRenderHint(QPainter::Antialiasing);

    drawAntialiasDemo(painter);
    drawCompositionDemo(painter);
    drawGradientDemo(painter);
}

void PaintWidget::drawAntialiasDemo(QPainter& painter)
{
    painter.save();

    // 左上角标题
    painter.setPen(Qt::black);
    painter.setFont(QFont(QStringLiteral("Sans"), 12, QFont::Bold));
    painter.drawText(20, 30, QStringLiteral("Antialiasing Comparison"));

    // 无抗锯齿圆形
    int y = 50;
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(QPen(Qt::darkBlue, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(30, y, 80, 80);
    painter.setPen(Qt::gray);
    painter.drawText(30, y + 100, QStringLiteral("No Antialiasing"));

    // 有抗锯齿圆形
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(Qt::darkRed, 2));
    painter.drawEllipse(140, y, 80, 80);
    painter.setPen(Qt::gray);
    painter.drawText(140, y + 100, QStringLiteral("With Antialiasing"));

    painter.restore();
}

void PaintWidget::drawCompositionDemo(QPainter& painter)
{
    painter.save();

    int baseX = 300;
    int baseY = 50;

    painter.setFont(QFont(QStringLiteral("Sans"), 12, QFont::Bold));
    painter.setPen(Qt::black);
    painter.drawText(baseX, 30, QStringLiteral("Composition Modes"));

    // SourceOver（默认合成模式）
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.setBrush(QColor(255, 0, 0, 180));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(baseX, baseY, 60, 60);
    painter.setBrush(QColor(0, 0, 255, 180));
    painter.drawEllipse(baseX + 30, baseY + 20, 60, 60);
    painter.setPen(Qt::gray);
    painter.drawText(baseX, baseY + 90, QStringLiteral("SourceOver"));

    // Multiply 合成模式
    int col2X = baseX + 130;
    painter.setCompositionMode(QPainter::CompositionMode_Multiply);
    painter.setBrush(QColor(255, 0, 0, 180));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(col2X, baseY, 60, 60);
    painter.setBrush(QColor(0, 0, 255, 180));
    painter.drawEllipse(col2X + 30, baseY + 20, 60, 60);
    painter.setPen(Qt::gray);
    painter.drawText(col2X, baseY + 90, QStringLiteral("Multiply"));

    // Screen 合成模式
    int col3X = col2X + 130;
    painter.setCompositionMode(QPainter::CompositionMode_Screen);
    painter.setBrush(QColor(255, 0, 0, 180));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(col3X, baseY, 60, 60);
    painter.setBrush(QColor(0, 0, 255, 180));
    painter.drawEllipse(col3X + 30, baseY + 20, 60, 60);
    painter.setPen(Qt::gray);
    painter.drawText(col3X, baseY + 90, QStringLiteral("Screen"));

    // 恢复默认合成模式
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.restore();
}

void PaintWidget::drawGradientDemo(QPainter& painter)
{
    painter.save();

    int baseY = 210;

    painter.setFont(QFont(QStringLiteral("Sans"), 12, QFont::Bold));
    painter.setPen(Qt::black);
    painter.drawText(20, baseY, QStringLiteral("Gradient Fills"));

    // 线性渐变
    QLinearGradient linearGrad(20, baseY + 20, 220, baseY + 120);
    linearGrad.setColorAt(0.0, Qt::blue);
    linearGrad.setColorAt(0.5, Qt::cyan);
    linearGrad.setColorAt(1.0, Qt::green);
    painter.setBrush(linearGrad);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(20, baseY + 20, 200, 100, 10, 10);
    painter.setPen(Qt::gray);
    painter.drawText(20, baseY + 140, QStringLiteral("Linear Gradient"));

    // 径向渐变
    QRadialGradient radialGrad(370, baseY + 70, 80);
    radialGrad.setColorAt(0.0, Qt::yellow);
    radialGrad.setColorAt(0.5, Qt::red);
    radialGrad.setColorAt(1.0, Qt::darkRed);
    painter.setBrush(radialGrad);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(270, baseY + 20, 200, 100);
    painter.setPen(Qt::gray);
    painter.drawText(320, baseY + 140, QStringLiteral("Radial Gradient"));

    // QPainterPath + 渐变组合
    int pathX = 500;
    QPainterPath path;
    path.moveTo(pathX + 50, baseY + 20);
    path.cubicTo(pathX + 100, baseY + 10, pathX + 150, baseY + 60,
                 pathX + 100, baseY + 120);
    path.cubicTo(pathX + 50, baseY + 80, pathX + 20, baseY + 50,
                 pathX + 50, baseY + 20);

    QLinearGradient pathGrad(pathX, baseY + 20, pathX + 150, baseY + 120);
    pathGrad.setColorAt(0.0, QColor(255, 100, 100));
    pathGrad.setColorAt(1.0, QColor(100, 100, 255));
    painter.setBrush(pathGrad);
    painter.setPen(QPen(Qt::darkGray, 1));
    painter.drawPath(path);
    painter.setPen(Qt::gray);
    painter.drawText(pathX + 30, baseY + 140, QStringLiteral("Path + Gradient"));

    painter.restore();
}
