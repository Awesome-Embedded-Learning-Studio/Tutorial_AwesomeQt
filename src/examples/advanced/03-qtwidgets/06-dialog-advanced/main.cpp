/// @file    main.cpp
/// @brief   对话框进阶演示程序入口。
///
/// 启动 ModalStrategyDemo 主窗口，附带独立工具窗口，
/// 展示 WindowModal 与 ApplicationModal 的模态范围差异，
/// 以及 ValidatedDialog 在 accept() 中执行输入验证的正确模式。
///
/// 对应教程：进阶层 03-QtWidgets/06-对话框进阶。

#include "modal_strategy_demo.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    ModalStrategyDemo demo;
    demo.show();

    return app.exec();
}
