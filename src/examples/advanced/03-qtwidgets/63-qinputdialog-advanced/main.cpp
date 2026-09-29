/// @file    main.cpp
/// @brief   程序入口，构建并展示 InputDialogDemo 窗口。
///
/// 对应教程：进阶层 03-QtWidgets/63-QInputDialog 自定义验证器与输入范围。

#include "input_dialog_demo.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    InputDialogDemo demo;
    demo.show();

    return app.exec();
}
