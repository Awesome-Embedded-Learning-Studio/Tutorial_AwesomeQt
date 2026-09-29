/// @file    main.cpp
/// @brief   QMainWindow 多显示器适配与全屏切换演示程序入口。
///
/// 对应教程：进阶层 03-QtWidgets/55-QMainWindow 进阶。

#include "main_window.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
