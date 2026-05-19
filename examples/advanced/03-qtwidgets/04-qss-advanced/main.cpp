/// @file    main.cpp
/// @brief   QSS 进阶演示程序入口。
///
/// 启动 ThemeSwitcher 窗口，展示动态主题切换、选择器特异性对比
/// 以及样式级联机制。
///
/// 对应教程：进阶层 03-QtWidgets/04-QSS 进阶。

#include "theme_switcher.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    ThemeSwitcher widget;
    widget.show();

    return app.exec();
}
