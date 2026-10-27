/*
 *  Qt 6 入门教程 - 示例 6.7
 *  主题：QML Canvas 绘图与粒子系统
 *
 * 本示例为纯 QML 演示（Canvas + Particles），C++ 端仅加载引擎。
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/qt/qml/QmlCanvasParticlesDemo/Main.qml"_s);

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
