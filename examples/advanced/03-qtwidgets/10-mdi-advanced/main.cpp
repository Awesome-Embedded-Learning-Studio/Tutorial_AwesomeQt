/// @file    main.cpp
/// @brief   MDI 进阶演示程序入口。
///
/// 启动 MdiManager 主窗口，展示 QMdiArea 子窗口管理、级联/平铺排列、
/// 窗口菜单动态同步以及子窗口关闭前的保存确认。
///
/// 对应教程：进阶层 03-QtWidgets/10-MDI 进阶。

#include "mdi_manager.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    MdiManager manager;
    manager.show();

    return app.exec();
}
