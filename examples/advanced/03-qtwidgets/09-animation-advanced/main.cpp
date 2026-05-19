/// @file    main.cpp
/// @brief   QStateMachine 动画演示程序入口。
///
/// 启动 StateMachineDemo 窗口，展示 QStateMachine 多状态驱动、
/// QParallelAnimationGroup 并行动画和 QPropertyAnimation 属性过渡。
///
/// 对应教程：进阶层 03-QtWidgets/09-动画进阶。

#include "state_machine_demo.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    StateMachineDemo demo;
    demo.show();

    return app.exec();
}
