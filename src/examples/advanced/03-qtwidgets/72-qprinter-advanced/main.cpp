/// @file    main.cpp
/// @brief   程序入口，构建并展示 PrintMainWindow。
///
/// 对应教程：进阶层 03-QtWidgets 打印高级用法。

#include "widget.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    PrintMainWindow window;
    window.show();

    return app.exec();
}
