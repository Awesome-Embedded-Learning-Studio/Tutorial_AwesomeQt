/// @file    main.cpp
/// @brief   QDial 进阶演示程序入口。
///
/// 启动 NonlinearDial 窗口，展示对数刻度映射、wrapping 模式切换、
/// 穿越点修正以及 notchTarget 与实际刻度的差异。
///
/// 对应教程：进阶层 03-QtWidgets/33-QDial 进阶。

#include "nonlinear_dial.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    NonlinearDial widget;
    widget.show();

    return app.exec();
}
