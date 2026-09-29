// QtGui 入门示例 04: QFont 与文本渲染基础
// 演示：QFont 属性设置、drawText 多种用法、QFontMetrics 布局计算、QTextDocument 富文本渲染

#include <QApplication>

#include "fontdemowidget.h"
#include "metricslayoutwidget.h"
#include "richtextcardwidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 演示 1: QFont 属性 + drawText 各种用法
    FontDemoWidget fontDemo;
    fontDemo.show();

    // 演示 2: QFontMetrics 精确布局
    MetricsLayoutWidget layoutDemo;
    layoutDemo.show();

    // 演示 3: QTextDocument 富文本卡片
    RichTextCardWidget cardDemo;
    cardDemo.show();

    return app.exec();
}
