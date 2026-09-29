/// @file    main.cpp
/// @brief   程序入口，构建并展示 GroupHeaderWindow。
///
/// 对应教程：进阶层 03-QtWidgets/52-QHeaderView 双级表头。

#include "group_header_view.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    GroupHeaderWindow window;
    window.show();

    return app.exec();
}
