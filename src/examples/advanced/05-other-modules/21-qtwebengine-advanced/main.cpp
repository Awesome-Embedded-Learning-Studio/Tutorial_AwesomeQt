/// @file    main.cpp
/// @brief   Application entry point — sets up a QMainWindow with QWebEngineView.
///
/// Creates an off-the-record QWebEngineProfile, registers the custom demo://
/// scheme via WebProfileManager, and loads demo://hello as the start page.
///
/// @note    QtWebEngine requires a working display server (X11/Wayland).
///          Compilation will succeed in headless/WSL2 environments, but
///          rendering will fail without a display. That is expected.

#include "custom_scheme.h"
#include "web_profile_manager.h"

#include <QApplication>
#include <QMainWindow>
#include <QStatusBar>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineUrlScheme>
#include <QWebEngineView>

namespace
{
    /// Initial URL to load after startup.
    const QString kStartUrl = QStringLiteral("demo://hello");

    /// Default window dimensions.
    const int kWindowWidth  = 800;
    const int kWindowHeight = 600;
} // namespace

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("QtWebEngine Advanced Demo"));

    // Register the "demo" scheme at application startup.
    // This must happen BEFORE any QWebEngineProfile is created, so that
    // Chromium's internal URL parser knows about the custom scheme.
    {
        QWebEngineUrlScheme demoScheme("demo");
        demoScheme.setFlags(QWebEngineUrlScheme::SecureScheme
                            | QWebEngineUrlScheme::LocalScheme
                            | QWebEngineUrlScheme::LocalAccessAllowed);
        QWebEngineUrlScheme::registerScheme(demoScheme);
    }

    // The default constructor creates an off-the-record profile —
    // no data is persisted to disk between sessions.
    QWebEngineProfile profile;

    // Apply our custom settings and install the demo:// handler
    WebProfileManager manager;
    manager.setupProfile(&profile);

    // Create the page with our custom profile
    QWebEnginePage page(&profile);

    // Create the main window
    QMainWindow mainWindow;
    mainWindow.setWindowTitle(QStringLiteral("QtWebEngine Advanced — Custom Scheme Demo"));
    mainWindow.resize(kWindowWidth, kWindowHeight);

    // Embed a QWebEngineView as the central widget
    auto* view = new QWebEngineView(&mainWindow);
    mainWindow.setCentralWidget(view);
    view->setPage(&page);

    // Show the current URL in the status bar whenever it changes
    QObject::connect(view, &QWebEngineView::urlChanged, &mainWindow,
                     [&mainWindow](const QUrl& url) {
                         mainWindow.statusBar()->showMessage(url.toString());
                     });

    // Also show the initial URL immediately
    mainWindow.statusBar()->showMessage(kStartUrl);

    mainWindow.show();

    // Load the start page — served by our CustomSchemeHandler
    view->load(QUrl(kStartUrl));

    return app.exec();
}
