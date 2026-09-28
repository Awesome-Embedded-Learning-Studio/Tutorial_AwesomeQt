// QtGui 入门示例 06: 拖放系统基础 —— 主窗口容器
// 组装拖放源（DragSourceWidget）与拖放目标（DropTargetWidget）两个演示控件

#pragma once

#include <QWidget>

class DragSourceWidget;
class DropTargetWidget;

// ============================================================================
// 主窗口：顶部说明标签 + 水平排列的拖放源/拖放目标
// ============================================================================
class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
};
