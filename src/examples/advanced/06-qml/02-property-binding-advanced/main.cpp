/// @file    main.cpp
/// @brief   QML application entry point for Property Binding Advanced example.
///
/// @details Demonstrates Binding element with when condition, Qt.binding() for
///          imperative binding restoration, and property alias usage in QML.
///          Corresponds to tutorial: advanced layer 06-QML/02-property-binding.

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <QDebug>
#include <QStringLiteral>

auto main(int argc, char* argv[]) -> int
{
    using namespace Qt::StringLiterals;        // Brings operator""_s into scope

    QGuiApplication app(argc, argv);

    // Set application metadata for QML engine identification
    QGuiApplication::setApplicationName("PropertyBindingAdvanced");
    QGuiApplication::setApplicationVersion("1.0");

    QQmlApplicationEngine engine;

    // Load the main QML file from the registered QML module
    const QUrl url(u"qrc:/qt/qml/PropertyBindingAdvanced/main.qml"_s);

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject* obj, const QUrl& objUrl)
        {
            // @note If QML fails to load, exit immediately to surface the error
            if (!obj && url == objUrl)
            {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection
    );

    engine.load(url);

    return app.exec();
}
