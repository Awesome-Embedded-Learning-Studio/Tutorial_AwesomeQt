/// @file    main.cpp
/// @brief   QImage 像素操作演示程序入口。
///
/// 启动图像处理演示窗口，展示 setPixelColor、scanLine 和
/// convertToFormat 三种像素操作方式。
///
/// 对应教程：进阶层 02-QtGui/03-图像处理进阶。

#include "pixel_widget.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    PixelWidget widget;
    widget.show();

    return app.exec();
}
