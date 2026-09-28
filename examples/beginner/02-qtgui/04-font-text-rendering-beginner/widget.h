#pragma once

#include <QWidget>

// 示例主窗口 —— 聚合本示例的三个文本渲染演示控件：
// 上部是 QFont 属性与 drawText 用法（FontDemoWidget），
// 中部是 QFontMetrics 精确布局（MetricsLayoutWidget），
// 下部是 QTextDocument 富文本卡片（RichTextCardWidget）。
class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
};
