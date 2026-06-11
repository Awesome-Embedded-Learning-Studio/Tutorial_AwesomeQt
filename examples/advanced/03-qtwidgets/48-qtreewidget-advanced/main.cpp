/// @file    main.cpp
/// @brief   QTreeWidget 延迟加载子节点示例程序入口。
///
/// 演示 QTreeWidget 的 itemExpanded 信号实现懒加载，
/// 通过 "Loading..." 占位子项指示可展开节点。
///
/// 对应教程：进阶层 03-QtWidgets/48-qtreewidget-advanced。

#include "widget.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    Widget widget;
    widget.show();

    return app.exec();
}
