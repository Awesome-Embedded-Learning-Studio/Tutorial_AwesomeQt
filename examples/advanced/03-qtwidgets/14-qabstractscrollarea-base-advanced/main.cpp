/// @file    main.cpp
/// @brief   QAbstractScrollArea 进阶演示程序入口。
///
/// 启动 SyncScrollWidget 窗口，展示双面板同步滚动、
/// blockSignals 防循环、同步/独立模式切换。
///
/// 对应教程：进阶层 03-QtWidgets/14-QAbstractScrollArea 基类进阶。

#include "sync_scroll_widget.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    SyncScrollWidget widget;
    widget.show();

    return app.exec();
}
