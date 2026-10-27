/// @file    main.cpp
/// @brief   QMainWindow 布局持久化演示程序入口。
///
/// 创建 PersistentMainWindow 并在 show() 之后调用 restoreLayout。
/// 构造期恢复同样受支持：窗口不可见时，恢复结果由布局引擎保留，
/// show() 时应用。
///
/// 对应教程：进阶层 03-QtWidgets/07-主窗口进阶。

#include "persistent_main_window.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // 设置组织名和应用名，供 QSettings 默认构造使用
    QCoreApplication::setOrganizationName(QStringLiteral("AwesomeQt"));
    QCoreApplication::setApplicationName(QStringLiteral("MainWindowDemo"));

    PersistentMainWindow window;
    // restoreLayout 在 show() 前后调用均可：窗口不可见时布局引擎保留恢复结果，show() 时应用
    window.show();
    window.restoreLayout();

    return app.exec();
}
