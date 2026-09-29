/// @file    main.cpp
/// @brief   Entry point for the QToolBox custom styling demo.
///
/// Builds and displays the ToolboxDemo widget, which showcases QSS-styled
/// QToolBox title tabs, item icons, and dynamic add/remove operations.
///
/// 对应教程：进阶层 03-QtWidgets/43-QToolBox 自定义标题栏样式。

#include "toolbox_demo.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    ToolboxDemo demo;
    demo.setWindowTitle("QToolBox Custom Styling Demo");
    demo.resize(400, 500);
    demo.show();

    return app.exec();
}
