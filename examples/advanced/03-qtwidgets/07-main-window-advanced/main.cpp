/// @file    main.cpp
/// @brief   QMainWindow 布局持久化演示程序入口。
///
/// 创建 PersistentMainWindow 并在 show() 之后调用 restoreLayout，
/// 确保 restoreState 能正确还原 Dock 的停靠位置。
///
/// 对应教程：进阶层 03-QtWidgets/07-主窗口进阶。

#include "persistent_main_window.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 设置组织名和应用名，供 QSettings 默认构造使用
    QCoreApplication::setOrganizationName(QStringLiteral("AwesomeQt"));
    QCoreApplication::setApplicationName(QStringLiteral("MainWindowDemo"));

    PersistentMainWindow window;
    // show() 必须在 restoreLayout 之前，否则 restoreState 无法正确还原 Dock 停靠位置
    window.show();
    window.restoreLayout();

    return app.exec();
}
