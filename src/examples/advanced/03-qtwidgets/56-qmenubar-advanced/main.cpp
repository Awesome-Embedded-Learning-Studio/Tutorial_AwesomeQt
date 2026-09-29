/// @file    main.cpp
/// @brief   QMenuBar 动态菜单与最近文件演示程序入口。
///
/// 对应教程：进阶层 03-QtWidgets/56-QMenuBar 进阶。

#include "menu_bar_demo.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    MenuBarDemo window;
    window.show();

    return app.exec();
}
