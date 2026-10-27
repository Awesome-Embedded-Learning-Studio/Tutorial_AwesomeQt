/// @file    main.cpp
/// @brief   QListWidget 拖放排序与自定义 ItemWidget 示例程序入口。
///
/// 演示 QListWidget 的 InternalMove 拖放模式、setItemWidget 自定义项控件、
/// 以及 movement 开关与布局批处理开关的行为（Free/Snap 差异仅 IconMode 可见）。
///
/// 对应教程：进阶层 03-QtWidgets/46-qlistwidget-advanced。

#include "widget.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    Widget widget;
    widget.show();

    return app.exec();
}
