/// @file    main.cpp
/// @brief   QStatusBar 多区域状态显示演示程序入口。
///
/// 对应教程：进阶层 03-QtWidgets/58-QStatusBar 进阶。

#include "status_bar_demo.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    StatusBarDemo window;
    window.show();

    return app.exec();
}
