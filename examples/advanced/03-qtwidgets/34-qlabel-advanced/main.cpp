/// @file    main.cpp
/// @brief   QLabel 进阶演示程序入口。
///
/// 启动 LabelModeDemo 窗口，展示 setTextFormat 三种模式、setBuddy 快捷键
/// 以及 QFontMetrics::elidedText 文本截断。
///
/// 对应教程：进阶层 03-QtWidgets/34-QLabel 进阶。

#include "label_mode_demo.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    LabelModeDemo widget;
    widget.show();

    return app.exec();
}
