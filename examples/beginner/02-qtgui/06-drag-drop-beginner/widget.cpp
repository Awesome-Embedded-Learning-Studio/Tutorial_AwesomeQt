// QtGui 入门示例 06: 拖放系统基础 —— 主窗口容器实现
// 组装拖放源与拖放目标两个演示控件

#include "widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "dragsourcewidget.h"
#include "droptargetwidget.h"

// ============================================================================
// 构造函数：创建窗口展示拖放交互
// ============================================================================
Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Qt 拖放系统演示");
    resize(600, 400);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // 顶部说明
    auto *infoLabel = new QLabel(
        "从左侧拖拽文本到右侧，或从文件管理器拖入文件到右侧区域");
    infoLabel->setStyleSheet("color: #666; font-size: 11px; padding: 4px;");
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);

    // 水平排列两个 Widget
    auto *hLayout = new QHBoxLayout;
    hLayout->setSpacing(20);

    auto *source = new DragSourceWidget;
    auto *target = new DropTargetWidget;

    hLayout->addWidget(source, 1);
    hLayout->addWidget(target, 1);

    mainLayout->addLayout(hLayout, 1);
}
