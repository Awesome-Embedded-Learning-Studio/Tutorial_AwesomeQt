/// @file    main.cpp
/// @brief   QFontDialog 过滤字体并预览效果演示程序入口。
///
/// 对应教程：进阶层 03-QtWidgets/65-QFontDialog 进阶。

#include "font_dialog_demo.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    FontDialogDemo demo;
    demo.show();

    return app.exec();
}
