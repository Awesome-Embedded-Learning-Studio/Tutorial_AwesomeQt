/// @file    main.cpp
/// @brief   QTransform 坐标变换演示程序入口。
///
/// 启动坐标变换演示窗口，展示 translate / rotate / scale 三种变换
/// 及其对应的 3x3 仿射矩阵数值。
///
/// 对应教程：进阶层 02-QtGui/02-坐标变换进阶。

#include "transform_widget.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    TransformWidget widget;
    widget.show();

    return app.exec();
}
