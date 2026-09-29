/// @file    main.cpp
/// @brief   Application entry point — loads the QML UI.
///
/// Creates a QGuiApplication and a QQmlApplicationEngine that loads
/// main.qml.  The QML scene instantiates ContactModel and
/// SortFilterModel directly via QML_ELEMENT registration.

#include "contact_model.h"
#include "sort_filter_model.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    // Register types for QML instantiation (QML_ELEMENT handles this
    // automatically via the build system, but we set the application
    // metadata for the QML engine).
    QGuiApplication::setApplicationName(QStringLiteral("QML Model/View Advanced"));
    QGuiApplication::setOrganizationName(QStringLiteral("AwesomeQt"));

    QQmlApplicationEngine engine;

    // Load the main QML file from the embedded QRC module
    const QUrl url(QStringLiteral("qrc:/QmlModelViewAdvanced/main.qml"));

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject* obj, const QUrl& objUrl)
        {
            // Exit with error if the QML file failed to load
            if (!obj && url == objUrl)
            {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
