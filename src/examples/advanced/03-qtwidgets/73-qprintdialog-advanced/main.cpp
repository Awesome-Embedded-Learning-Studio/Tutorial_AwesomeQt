/// @file    main.cpp
/// @brief   程序入口，构建并展示 PrintDialogMainWindow。
///
/// 对应教程：进阶层 03-QtWidgets 打印对话框高级用法。

#include "widget.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    PrintDialogMainWindow window;
    window.show();

    return app.exec();
}
