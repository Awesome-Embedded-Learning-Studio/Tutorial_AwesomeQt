/// @file    main.cpp
/// @brief   QTabBar 进阶演示程序入口。
///
/// 启动 DragReorderTabBar 窗口，展示标签拖拽排序、setTabButton 自定义按钮
/// 以及 selectionBehaviorOnRemove 选择策略。
///
/// 对应教程：进阶层 03-QtWidgets/40-QTabBar 进阶。

#include "drag_reorder_tab_bar.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    DragReorderTabBar widget;
    widget.show();

    return app.exec();
}
