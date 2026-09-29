/**
 * QtQuick3D QML 3D 场景基础示例
 *
 * 本示例的 3D 场景逻辑全部在 Main.qml 中实现，
 * C++ 侧只需要创建 QQmlApplicationEngine 加载 QML 文件。
 *
 * QtQuick3D 的核心优势：
 * - 声明式 QML 语法定义 3D 场景
 * - PrincipledMaterial PBR 物理材质
 * - 与 Qt Quick 2D 元素无缝混合
 * - 跨平台 RHI 渲染后端（OpenGL/Vulkan/D3D/Metal）
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <QDebug>

// ========================================
// 主函数
// ========================================

int main(int argc, char *argv[])
{
    // 设置高 DPI 缩放（Qt 6 默认启用，这里显式声明确保一致性）
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QGuiApplication app(argc, argv);

    qDebug() << "QtQuick3D QML 3D 场景基础示例";
    qDebug() << "本示例演示 View3D + Model + PrincipledMaterial + 2D 混合";

    // 创建 QML 应用引擎
    QQmlApplicationEngine engine;

    // 加载主 QML 文件
    // qt_add_qml_module 会把 Main.qml 注册到 qrc 资源中
    const QUrl url(QString::fromUtf8("qrc:/qt/qml/Quick3DScene/Main.qml"));

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            // QML 加载失败时退出程序
            if (!obj && url == objUrl) {
                qCritical() << "QML 文件加载失败:" << url;
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
