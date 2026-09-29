/**
 * QtQuick3D Physics 物理模拟基础示例
 *
 * 本示例的物理场景逻辑全部在 Main.qml 中实现，
 * C++ 侧只需要创建 QQmlApplicationEngine 加载 QML 文件。
 *
 * QtQuick3D Physics 核心组件：
 * - PhysicsWorld：物理世界容器，配置重力等全局参数
 * - StaticRigidBody：静态刚体（地面、墙壁等不可移动物体）
 * - DynamicRigidBody：动态刚体（球体、箱子等受物理模拟驱动）
 * - BoxShape / SphereShape / CapsuleShape：碰撞体形状
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

    qDebug() << "QtQuick3D Physics 物理模拟基础示例";
    qDebug() << "本示例演示 PhysicsWorld + DynamicRigidBody + 碰撞体";

    // 创建 QML 应用引擎
    QQmlApplicationEngine engine;

    // 加载主 QML 文件
    // qt_add_qml_module 会把 Main.qml 注册到 qrc 资源中
    const QUrl url(QString::fromUtf8("qrc:/qt/qml/PhysicsScene/Main.qml"));

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
