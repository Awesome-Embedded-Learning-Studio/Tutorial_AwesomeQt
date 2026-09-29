// QtGui 入门示例 02: 坐标系与 QTransform 变换基础
// 演示：translate/rotate/scale 基础变换、save/restore 状态管理、模拟时钟

#include <QApplication>

#include "transformdemowidget.h"
#include "analogclockwidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 变换演示窗口
    TransformDemoWidget demo;
    demo.show();

    // 模拟时钟窗口
    AnalogClockWidget clock;
    clock.show();

    return app.exec();
}
