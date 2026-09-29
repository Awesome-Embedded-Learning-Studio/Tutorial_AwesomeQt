/// @file    main.cpp
/// @brief   QLineEdit 进阶演示程序入口。
///
/// 启动 CustomValidatorDemo 窗口，展示自定义 QValidator、
/// 输入掩码 setInputMask 以及 QCompleter 自动补全。
///
/// 对应教程：进阶层 03-QtWidgets/22-QLineEdit 进阶。

#include "custom_validator_demo.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    CustomValidatorDemo widget;
    widget.show();

    return app.exec();
}
