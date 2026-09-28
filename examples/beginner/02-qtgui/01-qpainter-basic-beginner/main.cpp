// QtGui 入门示例 01: QPainter 绘图基础
// 演示：paintEvent 基本用法、QPen/QBrush/QColor 设置、基本图形绘制、简易柱状图

#include <QApplication>

#include "widget.h"

// ============================================================================
// 主函数：创建主窗口，聚合展示基本图形与柱状图两种绘图效果
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Widget widget;
    widget.show();

    return app.exec();
}
