/// @file    main.cpp
/// @brief   程序入口，构建并展示 QDialog 异步对话框演示窗口。
///
/// 对应教程：进阶层 03-QtWidgets/60-QDialog 异步对话框与结果回调。

#include "main_window.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
