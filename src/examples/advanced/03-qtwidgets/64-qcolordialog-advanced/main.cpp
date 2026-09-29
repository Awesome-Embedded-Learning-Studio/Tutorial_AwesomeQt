/// @file    main.cpp
/// @brief   QColorDialog 集成自定义颜色选择器面板演示程序入口。
///
/// 对应教程：进阶层 03-QtWidgets/64-QColorDialog 进阶。

#include "color_dialog_demo.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    ColorDialogDemo demo;
    demo.show();

    return app.exec();
}
