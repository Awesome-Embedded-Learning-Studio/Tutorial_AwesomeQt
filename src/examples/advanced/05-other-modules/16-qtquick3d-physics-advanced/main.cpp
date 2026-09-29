/// @file    main.cpp
/// @brief   Application entry point for Qt Quick 3D physics simulation demo.
///
/// Sets up the QML engine and loads the main 3D scene. PhysicsConfig is
/// registered as a QML type so the QML layer can bind gravity and force
/// properties directly from the C++ backend.

#include "physics_config.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>

int main(int argc, char* argv[])
{
    // High-DPI scaling must be set before QGuiApplication is constructed
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QGuiApplication app(argc, argv);

    // PhysicsConfig is declared with QML_ELEMENT, so the qt6_add_qml_module
    // call in CMakeLists.txt handles QML type registration automatically.
    // No manual qmlRegisterType needed here.

    QQmlApplicationEngine engine;

    // Qt 6 recommended way to load the main QML file from the QML module
    const QUrl url(QStringLiteral("qrc:/qt/qml/Quick3DPhysicsAdvanced/Main.qml"));

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
