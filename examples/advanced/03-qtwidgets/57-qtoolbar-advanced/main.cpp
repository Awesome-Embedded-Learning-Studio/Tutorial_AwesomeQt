/// @file    main.cpp
/// @brief   QToolBar 响应式工具栏演示程序入口。
///
/// 对应教程：进阶层 03-QtWidgets/57-QToolBar 进阶。

#include "toolbar_demo.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    ToolbarDemo window;
    window.show();

    return app.exec();
}
