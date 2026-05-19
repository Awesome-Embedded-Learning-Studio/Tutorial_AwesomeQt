/// @file    main.cpp
/// @brief   QProgressBar 进阶演示程序入口。
///
/// 启动 BusyBarDemo 窗口，展示 busy 模式动画驱动和
/// 子类化 paintEvent 自定义文本覆盖。
///
/// 对应教程：进阶层 03-QtWidgets/35-QProgressBar 进阶。

#include "busy_bar_demo.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    BusyBarDemo widget;
    widget.show();

    return app.exec();
}
