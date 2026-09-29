/// @file    main.cpp
/// @brief   QTabWidget 进阶演示程序入口。
///
/// 启动 ClosableTabWidget 窗口，展示可关闭标签页、tabBar() 细粒度定制
/// 以及 documentMode 跨平台风格。
///
/// 对应教程：进阶层 03-QtWidgets/39-QTabWidget 进阶。

#include "closable_tab_widget.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    ClosableTabWidget widget;
    widget.show();

    return app.exec();
}
