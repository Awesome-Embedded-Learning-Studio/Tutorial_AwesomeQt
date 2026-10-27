/// @file    main.cpp
/// @brief   Qt3D 高级示例程序入口。
///
/// 对应教程：进阶层 05-其他模块/14-Qt3D。
/// 创建 Qt3DWindow，使用 SceneBuilder 构建带材质和变换层次的 3D 场景。

// Qt 头文件
#include <QGuiApplication>

#include <Qt3DCore/QEntity>

#include <Qt3DExtras/Qt3DWindow>

// 项目内头文件
#include "scene_builder.h"

auto main(int argc, char* argv[]) -> int
{
    QGuiApplication app(argc, argv);

    // Qt3DWindow 是 Qt3D 的专用窗口，内含 FrameGraph 和渲染循环
    Qt3DExtras::Qt3DWindow window;

    // 创建场景根实体，所有 3D 对象最终挂载到该节点
    // @note rootEntity 的生命周期由 Qt3DWindow 管理，无需手动释放
    auto* rootEntity = new Qt3DCore::QEntity();

    SceneBuilder builder;
    builder.setupCamera(&window, rootEntity);
    builder.buildScene(rootEntity);

    window.setRootEntity(rootEntity);
    window.setTitle("Qt3D Advanced - Material & Transform Hierarchy");
    window.resize(800, 600);
    window.show();

    return app.exec();
}
