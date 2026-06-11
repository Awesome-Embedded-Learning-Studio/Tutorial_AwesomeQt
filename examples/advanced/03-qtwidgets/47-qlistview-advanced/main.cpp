/// @file    main.cpp
/// @brief   QListView 大数据虚拟列表优化示例程序入口。
///
/// 演示自定义 QAbstractListModel 配合 QListView 的虚拟滚动机制，
/// 以及 canFetchMore/fetchMore 增量加载 100,000+ 条数据。
///
/// 对应教程：进阶层 03-QtWidgets/47-qlistview-advanced。

#include "widget.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    Widget widget;
    widget.show();

    return app.exec();
}
