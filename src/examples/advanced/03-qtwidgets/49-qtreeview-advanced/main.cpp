/// @file    main.cpp
/// @brief   QTreeView 自定义展开图标与整行选中示例程序入口。
///
/// 演示自定义 QStyledItemDelegate 绘制三角形展开/折叠图标，
/// 以及实现整行选中高亮效果。
///
/// 对应教程：进阶层 03-QtWidgets/49-qtreeview-advanced。

#include "widget.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    Widget widget;
    widget.show();

    return app.exec();
}
