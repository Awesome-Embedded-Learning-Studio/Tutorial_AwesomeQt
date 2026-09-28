#include "widget.h"

#include "fontdemowidget.h"
#include "metricslayoutwidget.h"
#include "richtextcardwidget.h"

#include <QVBoxLayout>

// ============================================================================
// 主窗口：垂直堆叠三个文本渲染演示控件，一次窗口展示全部演示内容
// ============================================================================
Widget::Widget(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("QFont 与文本渲染基础");
    resize(680, 1150);

    // 按三个控件各自的推荐高度 450:350:380 ≈ 5:4:4 分配拉伸比例
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(new FontDemoWidget, 5);       // 上：QFont 属性 + drawText 用法
    layout->addWidget(new MetricsLayoutWidget, 4);  // 中：QFontMetrics 精确布局
    layout->addWidget(new RichTextCardWidget, 4);   // 下：QTextDocument 富文本卡片
}
