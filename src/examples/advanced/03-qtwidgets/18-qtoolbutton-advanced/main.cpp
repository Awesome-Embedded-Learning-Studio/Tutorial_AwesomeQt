/// @file    main.cpp
/// @brief   QToolButton 进阶演示程序入口。
///
/// 启动 ArrowToolDemo 窗口，展示 ArrowType 方向箭头、三种 PopupMode
/// 时序差异以及 QToolBar 集成行为。
///
/// 对应教程：进阶层 03-QtWidgets/18-QToolButton 进阶。

#include "arrow_tool_demo.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    ArrowToolDemo window;
    window.show();

    return app.exec();
}
