/// @file    main.cpp
/// @brief   程序入口，创建 MigrationDemo 并执行所有迁移演示后自动退出。
///
/// 对应教程：进阶层 05-其他模块 / 25-Qt5Compat 模块。
/// 本程序是纯控制台应用（QCoreApplication），无需 GUI 环境。

#include "migration_demo.h"

#include <QCoreApplication>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    // 创建演示对象，app 作为父对象管理其生命周期
    MigrationDemo demo(&app);

    // 执行所有迁移演示，输出对比信息到控制台
    demo.runAllDemos();

    // 演示完毕后自动退出，无需进入事件循环
    return 0;
}
