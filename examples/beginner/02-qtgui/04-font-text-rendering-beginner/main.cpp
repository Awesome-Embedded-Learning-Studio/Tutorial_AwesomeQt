// QtGui 入门示例 04: QFont 与文本渲染基础
// 演示：QFont 属性设置、drawText 多种用法、QFontMetrics 布局计算、QTextDocument 富文本渲染

#include <QApplication>

#include "widget.h"

// ============================================================================
// 主函数：创建主窗口，聚合展示字体属性、度量布局与富文本三种渲染效果
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Widget widget;
    widget.show();

    return app.exec();
}
