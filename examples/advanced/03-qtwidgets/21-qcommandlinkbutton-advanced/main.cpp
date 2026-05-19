/// @file    main.cpp
/// @brief   QCommandLinkButton 进阶演示程序入口。
///
/// 启动 PlatformStyleDemo 窗口，展示 QCommandLinkButton 的平台样式差异、
/// QSS 美化方案以及动态 description 切换对 sizeHint 的影响。
///
/// 对应教程：进阶层 03-QtWidgets/21-QCommandLinkButton 进阶。

#include "platform_style_demo.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    PlatformStyleDemo widget;
    widget.show();

    return app.exec();
}
