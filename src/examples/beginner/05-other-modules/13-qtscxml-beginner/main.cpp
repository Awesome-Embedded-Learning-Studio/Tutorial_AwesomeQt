#include <QApplication>

#include "trafficlightwindow.h"

// ========================================
// 主函数
// ========================================

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qDebug() << "Qt SCXML 交通灯模拟器示例";
    qDebug() << "本示例演示从 .scxml 文件加载状态机并驱动 UI";

    TrafficLightWindow window;
    window.show();

    return app.exec();
}
