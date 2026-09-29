// QtGui 入门示例 01: QPainter 绘图基础
// 演示：paintEvent 基本用法、QPen/QBrush/QColor 设置、基本图形绘制、简易柱状图

#include <QApplication>

#include "barchartwidget.h"
#include "shapegallerywidget.h"

// ============================================================================
// 主函数：创建窗口展示两种绘图效果
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 展示基本图形
    ShapeGalleryWidget shapes;
    shapes.show();

    // 展示柱状图
    BarChartWidget chart;
    chart.show();

    return app.exec();
}
