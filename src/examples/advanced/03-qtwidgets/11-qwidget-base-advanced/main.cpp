/// @file    main.cpp
/// @brief   无边框半透明窗口演示程序入口。
///
/// 启动 FramelessWidget 窗口，展示 WA_TranslucentBackground 半透明背景、
/// FramelessWindowHint 无边框以及手动鼠标拖动。
///
/// 对应教程：进阶层 03-QtWidgets/11-QWidget 基类进阶。

#include "frameless_widget.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    FramelessWidget widget;
    widget.show();

    return app.exec();
}
