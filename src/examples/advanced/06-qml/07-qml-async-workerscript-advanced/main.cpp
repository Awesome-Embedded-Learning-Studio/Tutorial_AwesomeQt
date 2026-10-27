/// @file    main.cpp
/// @brief   QML 异步编程示例程序入口。
///
/// 演示 WorkerScript 后台计算、Loader 异步加载、XMLHttpRequest 网络请求。
/// 对应教程：进阶层 06-QML/07-异步工作线程与网络请求。

#include <QGuiApplication>
#include <QQmlApplicationEngine>

auto main(int argc, char* argv[]) -> int
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    // 加载主 QML 文件，包含三个 Tab 页分别演示三种异步模式
    const QUrl url(QStringLiteral("qrc:/qt/qml/QmlAsyncAdvanced/main.qml"));
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject* obj, const QUrl& objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    engine.load(url);
    return app.exec();
}
