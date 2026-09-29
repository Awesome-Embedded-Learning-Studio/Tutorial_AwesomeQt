#include <QGuiApplication>

#include <Qt3DExtras/Qt3DWindow>

#include "sceneutils.h"

// ========================================
// 主函数
// ========================================

int main(int argc, char *argv[])
{
    // Qt 3D 的 Qt3DWindow 是 QWindow，需要用 QGuiApplication
    QGuiApplication app(argc, argv);

    qDebug() << "Qt 3D 基础场景搭建示例";
    qDebug() << "本示例演示 Qt3DWindow + ECS 架构 + 基础几何体 + 光照材质";

    // 创建 Qt3D 3D 渲染窗口
    auto *view = new Qt3DExtras::Qt3DWindow();
    view->setTitle("Qt 3D 基础场景 - 球体 / 立方体 / 地面");
    view->resize(900, 650);

    // 构建 3D 场景
    setupScene(view);

    view->show();

    return app.exec();
}
