/// @file    transform_widget.cpp
/// @brief   QTransform 坐标变换演示控件的实现。
///
/// 实现四种绘制模式：原始参考、平移、旋转、缩放，每种模式在独立面板
/// 中展示。所有面板共享同一形状定义，仅变换不同，便于对比理解。
///
/// 对应教程：进阶层 02-QtGui/02-坐标变换进阶。

#include "transform_widget.h"

#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QTransform>

TransformWidget::TransformWidget(QWidget* parent) : QWidget(parent)
{
    setWindowTitle("QTransform Coordinate Transform Demo");
    setMinimumSize(800, 600);
    // 背景填充不透明，避免闪烁
    setAutoFillBackground(true);
}

void TransformWidget::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int w = width();
    const int h = height();

    // 2x2 网格布局：上排为原始+平移，下排为旋转+缩放
    const int halfW = w / 2;
    const int halfH = h / 2;

    // 四个面板区域，各留少量间距
    const QRectF topLeft(0, 0, halfW, halfH);
    const QRectF topRight(halfW, 0, halfW, halfH);
    const QRectF bottomLeft(0, halfH, halfW, halfH);
    const QRectF bottomRight(halfW, halfH, halfW, halfH);

    // 绘制整体背景
    painter.fillRect(rect(), Qt::white);

    // 绘制分隔线，将四个面板视觉分开
    QPen sepPen(QColor(180, 180, 180), 1);
    painter.setPen(sepPen);
    painter.drawLine(halfW, 0, halfW, h);
    painter.drawLine(0, halfH, w, halfH);

    // 每个面板独立绘制，save/restore 确保变换状态互不干扰
    painter.save();
    drawOriginal(painter, topLeft);
    painter.restore();

    painter.save();
    drawTranslate(painter, topRight);
    painter.restore();

    painter.save();
    drawRotate(painter, bottomLeft);
    painter.restore();

    painter.save();
    drawScale(painter, bottomRight);
    painter.restore();
}

void TransformWidget::drawGrid(QPainter& painter, const QRectF& area)
{
    QPen gridPen(QColor(220, 220, 220), 1, Qt::DotLine);
    painter.setPen(gridPen);

    // 以 area 中心为原点绘制网格
    const double cx = area.center().x();
    const double cy = area.center().y();

    // 竖线
    for (double x = cx; x <= area.right(); x += kGridSpacing)
    {
        painter.drawLine(static_cast<int>(x), static_cast<int>(area.top()),
                         static_cast<int>(x), static_cast<int>(area.bottom()));
    }
    for (double x = cx - kGridSpacing; x >= area.left(); x -= kGridSpacing)
    {
        painter.drawLine(static_cast<int>(x), static_cast<int>(area.top()),
                         static_cast<int>(x), static_cast<int>(area.bottom()));
    }

    // 横线
    for (double y = cy; y <= area.bottom(); y += kGridSpacing)
    {
        painter.drawLine(static_cast<int>(area.left()), static_cast<int>(y),
                         static_cast<int>(area.right()), static_cast<int>(y));
    }
    for (double y = cy - kGridSpacing; y >= area.top(); y -= kGridSpacing)
    {
        painter.drawLine(static_cast<int>(area.left()), static_cast<int>(y),
                         static_cast<int>(area.right()), static_cast<int>(y));
    }

    // 坐标轴用稍深的颜色标出
    QPen axisPen(QColor(160, 160, 160), 1);
    painter.setPen(axisPen);
    painter.drawLine(static_cast<int>(cx), static_cast<int>(area.top()),
                     static_cast<int>(cx), static_cast<int>(area.bottom()));
    painter.drawLine(static_cast<int>(area.left()), static_cast<int>(cy),
                     static_cast<int>(area.right()), static_cast<int>(cy));
}

void TransformWidget::drawShape(QPainter& painter, double size)
{
    const double half = size / 2.0;

    // 矩形轮廓（蓝色）
    QPen shapePen(QColor(52, 101, 164), 2);
    painter.setPen(shapePen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(QRectF(-half, -half, size, size));

    // 对角线（灰色虚线）
    QPen diagPen(QColor(136, 136, 136), 1, Qt::DashLine);
    painter.setPen(diagPen);
    painter.drawLine(QPointF(-half, -half), QPointF(half, half));
    painter.drawLine(QPointF(half, -half), QPointF(-half, half));

    // 中心圆点（红色）
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(204, 0, 0));
    painter.drawEllipse(QPointF(0, 0), 4.0, 4.0);
}

QStringList TransformWidget::formatMatrix(const QTransform& transform)
{
    QStringList lines;
    // QTransform 的 m11/m12/m13... 对应 3x3 仿射矩阵的各元素
    lines << QString("  [%1  %2  %3]")
                 .arg(transform.m11(), 7, 'f', 2)
                 .arg(transform.m12(), 7, 'f', 2)
                 .arg(transform.m13(), 7, 'f', 2);
    lines << QString("  [%1  %2  %3]")
                 .arg(transform.m21(), 7, 'f', 2)
                 .arg(transform.m22(), 7, 'f', 2)
                 .arg(transform.m23(), 7, 'f', 2);
    lines << QString("  [%1  %2  %3]")
                 .arg(transform.m31(), 7, 'f', 2)
                 .arg(transform.m32(), 7, 'f', 2)
                 .arg(transform.m33(), 7, 'f', 2);
    return lines;
}

void TransformWidget::drawMatrixText(QPainter& painter,
                                     const QTransform& transform,
                                     const QPointF& position)
{
    const QStringList lines = formatMatrix(transform);

    painter.setPen(QColor(85, 85, 85));
    QFont monoFont("monospace", 9);
    painter.setFont(monoFont);

    double y = position.y();
    for (const QString& line : lines)
    {
        painter.drawText(QPointF(position.x(), y), line);
        y += 16.0;
    }
}

void TransformWidget::drawPanelHeader(QPainter& painter, const QRectF& panelRect,
                                      const QString& title)
{
    painter.setPen(QColor(50, 50, 50));
    QFont titleFont("Sans", 11, QFont::Bold);
    painter.setFont(titleFont);
    painter.drawText(QPointF(panelRect.left() + 10, panelRect.top() + 22),
                     title);
}

void TransformWidget::drawOriginal(QPainter& painter, const QRectF& rect)
{
    drawPanelHeader(painter, rect, "Original (Identity)");
    drawGrid(painter, rect);

    // 移动到面板中心，绘制无变换的形状
    painter.translate(rect.center().x(), rect.center().y());
    drawShape(painter, kShapeSize);

    // 显示单位矩阵
    QTransform identity;
    drawMatrixText(painter, identity,
                   QPointF(rect.left() + 10, rect.bottom() - 65));
}

void TransformWidget::drawTranslate(QPainter& painter, const QRectF& rect)
{
    drawPanelHeader(painter, rect,
                    QString("Translate(%1, %2)").arg(kTranslateX).arg(kTranslateY));
    drawGrid(painter, rect);

    const double cx = rect.center().x();
    const double cy = rect.center().y();

    // 先画一个浅色的原始位置参考
    painter.save();
    painter.translate(cx, cy);
    QPen ghostPen(QColor(200, 200, 200), 1, Qt::DashLine);
    painter.setPen(ghostPen);
    painter.setBrush(Qt::NoBrush);
    const double half = kShapeSize / 2.0;
    painter.drawRect(QRectF(-half, -half, kShapeSize, kShapeSize));
    painter.restore();

    // 应用平移变换
    painter.translate(cx, cy);
    painter.translate(kTranslateX, kTranslateY);
    drawShape(painter, kShapeSize);

    // 获取当前变换矩阵并显示
    painter.resetTransform();
    QTransform t;
    t.translate(kTranslateX, kTranslateY);
    drawMatrixText(painter, t,
                   QPointF(rect.left() + 10, rect.bottom() - 65));
}

void TransformWidget::drawRotate(QPainter& painter, const QRectF& rect)
{
    drawPanelHeader(painter, rect,
                    QString("Rotate(%1 deg)").arg(kRotationAngle));
    drawGrid(painter, rect);

    const double cx = rect.center().x();
    const double cy = rect.center().y();

    // 画原始位置参考
    painter.save();
    painter.translate(cx, cy);
    QPen ghostPen(QColor(200, 200, 200), 1, Qt::DashLine);
    painter.setPen(ghostPen);
    painter.setBrush(Qt::NoBrush);
    const double half = kShapeSize / 2.0;
    painter.drawRect(QRectF(-half, -half, kShapeSize, kShapeSize));
    painter.restore();

    // 应用旋转变换（以面板中心为旋转中心）
    painter.translate(cx, cy);
    painter.rotate(kRotationAngle);
    drawShape(painter, kShapeSize);

    // 计算纯旋转矩阵
    painter.resetTransform();
    const double rad = qDegreesToRadians(static_cast<double>(kRotationAngle));
    const double cosA = std::cos(rad);
    const double sinA = std::sin(rad);
    QTransform r(cosA, sinA, -sinA, cosA, 0, 0);
    drawMatrixText(painter, r,
                   QPointF(rect.left() + 10, rect.bottom() - 65));
}

void TransformWidget::drawScale(QPainter& painter, const QRectF& rect)
{
    drawPanelHeader(
        painter, rect,
        QString("Scale(%1, %2)").arg(kScaleX).arg(kScaleY));
    drawGrid(painter, rect);

    const double cx = rect.center().x();
    const double cy = rect.center().y();

    // 画原始位置参考
    painter.save();
    painter.translate(cx, cy);
    QPen ghostPen(QColor(200, 200, 200), 1, Qt::DashLine);
    painter.setPen(ghostPen);
    painter.setBrush(Qt::NoBrush);
    const double half = kShapeSize / 2.0;
    painter.drawRect(QRectF(-half, -half, kShapeSize, kShapeSize));
    painter.restore();

    // 应用缩放变换
    painter.translate(cx, cy);
    painter.scale(kScaleX, kScaleY);
    drawShape(painter, kShapeSize);

    // 计算纯缩放矩阵
    painter.resetTransform();
    QTransform s(kScaleX, 0, 0, kScaleY, 0, 0);
    drawMatrixText(painter, s,
                   QPointF(rect.left() + 10, rect.bottom() - 65));
}
