/// @file    main.cpp
/// @brief   碰撞检测演示程序入口。
///
/// 启动 CollisionScene 视图，展示自定义 QGraphicsItem 的 shape() 精确碰撞
/// 与 collidingItems() 实时检测。
///
/// 对应教程：进阶层 03-QtWidgets/08-图形视图进阶。

#include "collision_scene.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    CollisionScene view;
    view.show();

    return app.exec();
}
