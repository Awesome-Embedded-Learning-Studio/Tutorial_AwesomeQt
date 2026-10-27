#include <QApplication>

#include "taskwindow.h"

// ========================================
// 主函数
// ========================================

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qDebug() << "Qt StateMachine 任务管理器示例";
    qDebug() << "本示例演示状态机驱动的 UI 状态管理";

    TaskWindow window;
    window.show();

    return app.exec();
}
