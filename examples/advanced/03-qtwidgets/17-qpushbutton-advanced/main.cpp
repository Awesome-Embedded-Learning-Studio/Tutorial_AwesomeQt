/// @file    main.cpp
/// @brief   QPushButton 进阶演示程序入口。
///
/// 启动 AutoDefaultDemo 对话框，展示 autoDefault 键盘拦截、
/// setMenu() 信号抑制和 flat 按钮焦点框行为。
///
/// 对应教程：进阶层 03-QtWidgets/17-QPushButton 进阶。

#include "auto_default_demo.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    AutoDefaultDemo dialog;
    dialog.show();

    return app.exec();
}
