/// @file    main.cpp
/// @brief   QLCDNumber 进阶演示程序入口。
///
/// 启动 SegmentStyleDemo 窗口，展示 SegmentStyle 三种风格、digitCount 溢出行为
/// 以及 setMode 进制切换时内部存储值与显示值的分离机制。
///
/// 对应教程：进阶层 03-QtWidgets/36-QLCDNumber 进阶。

#include "segment_style_demo.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    SegmentStyleDemo widget;
    widget.show();

    return app.exec();
}
