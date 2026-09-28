#include "widget.h"

#include "barchartwidget.h"
#include "shapegallerywidget.h"

#include <QVBoxLayout>

// ============================================================================
// 主窗口：垂直堆叠两个自定义绘图控件，一次窗口展示全部演示内容
// ============================================================================
Widget::Widget(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("QPainter 绘图基础");
    resize(600, 900);

    // 按两个控件各自的推荐高度 5:4（500:400）分配拉伸比例
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(new ShapeGalleryWidget, 5);  // 上：基本图形展示
    layout->addWidget(new BarChartWidget, 4);      // 下：简易柱状图
}
