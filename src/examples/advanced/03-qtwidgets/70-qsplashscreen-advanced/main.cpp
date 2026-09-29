/// @file    main.cpp
/// @brief   程序入口，先显示 QSplashScreen 再展示主窗口。
///
/// 对应教程：进阶层 03-QtWidgets/70-QSplashScreen 进阶。

#include "splash_demo.h"

#include <QApplication>
#include <QTimer>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 在事件循环启动前先创建主窗口（但不显示）
    SplashDemo mainWindow;
    mainWindow.setWindowTitle(QObject::tr("QSplashScreen - Advanced"));
    mainWindow.resize(500, 300);

    // @note 应用启动时自动展示一次启动画面。
    // 通过 QTimer::singleShot(0, ...) 延迟到事件循环启动后执行，
    // 确保 QSplashScreen 的事件处理正常。
    QTimer::singleShot(0, &mainWindow, [&mainWindow]() {
        mainWindow.restartSplash();
    });

    mainWindow.show();

    return app.exec();
}
