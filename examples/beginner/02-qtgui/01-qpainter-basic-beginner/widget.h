#pragma once

#include <QWidget>

// 示例主窗口 —— 聚合本示例的两个绘图演示控件：
// 上半部分是基本图形展示（ShapeGalleryWidget），
// 下半部分是简易柱状图（BarChartWidget）。
class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
};
