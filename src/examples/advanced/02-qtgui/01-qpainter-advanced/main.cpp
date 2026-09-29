/// @file    main.cpp
/// @brief   QPainter 进阶演示程序入口。
///
/// 启动双缓冲绘制演示窗口，展示合成模式、抗锯齿和渐变填充。
///
/// 对应教程：进阶层 02-QtGui/01-QPainter 进阶。

#include "paint_widget.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    PaintWidget widget;
    widget.show();

    return app.exec();
}
