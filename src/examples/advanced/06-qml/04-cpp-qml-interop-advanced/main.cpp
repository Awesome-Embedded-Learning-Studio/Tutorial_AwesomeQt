/// @file    main.cpp
/// @brief   程序入口，创建 QML 引擎并加载主界面。
///
/// 演示如何将 C++ 类型（QML_ELEMENT）注册到 QML 运行时环境。
/// 对应教程：进阶层 06-QML/04-C++ 与 QML 互操作。

#include "app_controller.h"
#include "task_model.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    // 设置应用元信息，QML 引擎据此确定 QML 模块的导入路径
    app.setApplicationName(QStringLiteral("CppQmlInteropAdvanced"));
    app.setOrganizationName(QStringLiteral("AwesomeQt"));

    QQmlApplicationEngine engine;

    // 创建模型实例并设为 QML 上下文属性，供 QML 直接按 id 访问
    TaskModel taskModel;
    engine.rootContext()->setContextProperty(
        QStringLiteral("taskModel"), &taskModel);

    // 加载主 QML 文件，QML 引擎自动解析 QML_ELEMENT 注册的类型
    const QUrl mainQml(QStringLiteral("qrc:/qt/qml/CppQmlInteropAdvanced/main.qml"));
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated,
        &app, [mainQml](QObject* obj, const QUrl& objUrl) {
            // 如果 QML 加载失败，退出应用以避免空窗口
            if (!obj && mainQml == objUrl)
            {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

    engine.load(mainQml);

    return app.exec();
}
