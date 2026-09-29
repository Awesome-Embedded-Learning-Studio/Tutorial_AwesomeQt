/// @file    main.cpp
/// @brief   Qt Quick Controls 进阶示例的 C++ 入口文件。
///
/// 演示 QQuickStyle 在 C++ 侧设置 Material 风格、
/// QQmlApplicationEngine 加载 QML 主界面。
/// 所有 Controls 演示逻辑在 main.qml 中呈现。

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>                                              // 运行时风格选择

using namespace Qt::StringLiterals;                                 // 字符串字面量 _s

/// @brief  应用程序入口，设置 Material 风格并启动 QML 引擎。
/// @param[in] argc 命令行参数个数。
/// @param[in] argv 命令行参数数组。
/// @return 应用程序退出码。
int main(int argc, char* argv[])
{
    // 必须在 QGuiApplication 构造之后、引擎加载之前调用 setStyle
    // 否则 Controls 无法正确应用目标风格
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle("Material");

    QQmlApplicationEngine engine;

    // 使用 qt6_add_qml_module 注册的 URI 自动加载 main.qml
    const QUrl url(u"qrc:/qt/qml/QuickControlsAdvanced/main.qml"_s);

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
