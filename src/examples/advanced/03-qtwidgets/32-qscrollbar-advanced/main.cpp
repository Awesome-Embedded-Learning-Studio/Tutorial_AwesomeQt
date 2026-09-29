/// @file    main.cpp
/// @brief   QScrollBar 进阶演示程序入口。
///
/// 启动 SliderSizeDemo 窗口，交互式展示 QScrollBar 手柄大小计算公式
/// 及其与 range / pageStep / singleStep 之间的联动关系。
///
/// 对应教程：进阶层 03-QtWidgets/32-QScrollBar 进阶。

#include "slider_size_demo.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    SliderSizeDemo widget;
    widget.show();

    return app.exec();
}
