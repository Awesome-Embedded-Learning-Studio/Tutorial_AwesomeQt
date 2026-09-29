/// @file    main.cpp
/// @brief   QFileDialog 自定义配置演示程序入口。
///
/// 对应教程：进阶层 03-QtWidgets/66-QFileDialog 进阶。

#include "file_dialog_demo.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    FileDialogDemo demo;
    demo.show();

    return app.exec();
}
