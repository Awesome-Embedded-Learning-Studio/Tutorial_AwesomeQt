#pragma once

#include <QWidget>

// 演示 2: 模拟时钟 —— 练习项目的参考实现
class AnalogClockWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AnalogClockWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void drawClockFace(QPainter *painter, double radius);
    void drawHand(QPainter *painter, double angle, double length, int width, const QColor &color);
};
