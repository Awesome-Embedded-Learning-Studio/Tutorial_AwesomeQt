/// @file    main.cpp
/// @brief   程序入口，构建并展示 QDockWidget 布局持久化演示窗口。
///
/// 对应教程：进阶层 03-QtWidgets/59-QDockWidget 布局持久化。

#include "main_window.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 设置组织名和应用名，与 QSettings 构造参数保持一致
    QCoreApplication::setOrganizationName("AwesomeQt");
    QCoreApplication::setApplicationName("DockWidgetDemo");

    MainWindow window;
    window.show();

    return app.exec();
}
