/// @file    main.cpp
/// @brief   QML 语法进阶示例的 C++ 入口文件。
///
/// 演示 QML 绑定陷阱、required property、readonly property
/// 以及 Component.onCompleted 的用法。C++ 侧仅负责启动
/// QML 引擎并加载 main.qml，所有知识点在 QML 中呈现。

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QString>

/// @brief  应用程序入口，初始化 QML 运行环境并加载主界面。
/// @param[in] argc 命令行参数个数。
/// @param[in] argv 命令行参数数组。
/// @return 应用程序退出码。
int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    // QML 引擎负责加载和执行 QML 文档
    QQmlApplicationEngine engine;

    // 使用 qt6_add_qml_module 注册的 URI 自动加载 main.qml
    const QUrl url(QStringLiteral("qrc:/qt/qml/QmlSyntaxAdvanced/main.qml"));

    // 确保引擎完成加载后再继续，避免窗口未创建就进入事件循环
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject* obj, const QUrl& objUrl) {
            if (!obj && url == objUrl)
            {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
