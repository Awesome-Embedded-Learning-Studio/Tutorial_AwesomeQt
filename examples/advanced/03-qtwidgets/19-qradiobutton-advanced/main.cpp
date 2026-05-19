/// @file    main.cpp
/// @brief   QRadioButton 进阶演示程序入口。
///
/// 启动 DynamicRadioGroup 窗口，展示 QButtonGroup 的 exclusive 边界情况、
/// 动态增删按钮管理以及 idClicked 与 buttonClicked 信号差异。
///
/// 对应教程：进阶层 03-QtWidgets/19-QRadioButton 进阶。

#include "dynamic_radio_group.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    DynamicRadioGroup widget;
    widget.show();

    return app.exec();
}
