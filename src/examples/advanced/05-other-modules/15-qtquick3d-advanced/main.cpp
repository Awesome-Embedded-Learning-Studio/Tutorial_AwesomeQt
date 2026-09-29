/// @file    main.cpp
/// @brief   Application entry point for Qt Quick 3D PBR material demo.
///
/// Sets up the QML engine and loads the main 3D scene. SceneConfig is
/// registered as a QML singleton so the QML layer can bind PBR properties
/// directly from C++.

#include "scene_config.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QtCore/QUrl>

using namespace Qt::StringLiterals;

int main(int argc, char* argv[])
{
    // High-DPI scaling must be set before QGuiApplication is constructed
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QGuiApplication app(argc, argv);

    // SceneConfig is declared with QML_ELEMENT, so the qt6_add_qml_module
    // call in CMakeLists.txt handles QML type registration automatically.
    // No manual qmlRegisterType needed here.

    QQmlApplicationEngine engine;

    // Qt 6 recommended way to load the main QML file from the QML module
    const QUrl url(u"qrc:/qt/qml/Quick3DAdvanced/Main.qml"_s);

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject* obj, const QUrl& objUrl) {
            // Exit with error if the root QML object failed to load
            if (!obj && url == objUrl) {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
