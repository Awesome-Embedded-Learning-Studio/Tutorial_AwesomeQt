/// @file    main.cpp
/// @brief   QSizePolicy 进阶演示程序入口。
///
/// 启动 SizePolicyDemo 窗口，展示六种 sizePolicy 策略对比、动态策略切换
/// 以及 stretch 因子对空间分配的影响。
///
/// 对应教程：进阶层 03-QtWidgets/01-布局系统进阶。

#include "size_policy_demo.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    SizePolicyDemo widget;
    widget.show();

    return app.exec();
}
