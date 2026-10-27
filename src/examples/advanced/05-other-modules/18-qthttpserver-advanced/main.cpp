/// @file    main.cpp
/// @brief   Entry point — starts the API server and runs a self-test demo.
///
/// Creates an ApiServer, starts it on port 8080, then issues a few HTTP
/// requests to itself using QNetworkAccessManager to demonstrate every
/// route. The application exits automatically after the demo completes.

#include "api_server.h"
#include "auth_middleware.h"

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>

#include <cstdio>
#include <memory>

// ── Constants ────────────────────────────────────────────────────────────────
static constexpr quint16 kServerPort    = 8080;
static const QString kBaseUrl           = QStringLiteral("http://localhost:%1")
                                              .arg(kServerPort);
static const QByteArray kAuthHeader     = QByteArrayLiteral("Authorization");
static const QByteArray kValidToken     = QByteArrayLiteral("Bearer demo-token-123");
static const QByteArray kInvalidToken   = QByteArrayLiteral("Bearer wrong-token");

/// @brief Prints a separator line to stdout for readability.
static void printSeparator()
{
    std::printf("\n────────────────────────────────────────"
                "────────────────────────\n");
}

/// @brief Pretty-prints a JSON byte array to stdout.
/// @param[in] label   A descriptive label (e.g. "GET /api/status").
/// @param[in] data    Raw JSON bytes from the response body.
static void printJson(const char* label, const QByteArray& data)
{
    std::printf("\n[%s]\n", label);
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        // Not JSON — print raw text
        std::printf("  %s\n", data.constData());
        return;
    }
    std::printf("  %s\n",
                doc.toJson(QJsonDocument::Indented).constData());
}

// ─────────────────────────────────────────────────────────────────────────────
int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("18-qthttpserver-advanced"));

    // ── 1. Create and start the server ───────────────────────────────────
    ApiServer server;

    // Print available endpoints as soon as the server starts
    QObject::connect(&server, &ApiServer::serverStarted,
                     [](quint16 port) {
        std::printf("API server listening on port %u\n", port);
        printSeparator();
        std::printf("Available endpoints:\n");
        std::printf("  GET  http://localhost:%u/api/status\n",    port);
        std::printf("  GET  http://localhost:%u/api/time\n",      port);
        std::printf("  GET  http://localhost:%u/api/protected\n", port);
        std::printf("  POST http://localhost:%u/api/data\n",      port);
        std::printf("  GET  http://localhost:%u/\n",              port);
        printSeparator();
    });

    if (!server.start(kServerPort)) {
        std::fprintf(stderr, "ERROR: failed to start server on port %u\n",
                     kServerPort);
        return 1;
    }

    // ── 2. Self-test: fire requests after a short delay ──────────────────
    // Using a single-shot timer so the event loop processes the server's
    // bind() before we begin connecting to it.
    QTimer::singleShot(200, [&app]() {
        auto mgr = new QNetworkAccessManager(&app);
        // Heap-allocate the counter so it survives until all reply lambdas finish.
        auto pending = std::make_shared<int>(0);

        // ── GET /api/status ──────────────────────────────────────────────
        ++*pending;
        QNetworkRequest statusReq(
            QUrl(kBaseUrl + QStringLiteral("/api/status")));
        QNetworkReply* statusReply = mgr->get(statusReq);
        QObject::connect(statusReply, &QNetworkReply::finished,
                         statusReply, [statusReply, pending]() {
            printJson("GET /api/status  (expect 200)",
                      statusReply->readAll());
            statusReply->deleteLater();
            if (--*pending == 0) {
                QCoreApplication::quit();
            }
        });

        // ── GET /api/protected with valid token ─────────────────────────
        ++*pending;
        QNetworkRequest authReq(
            QUrl(kBaseUrl + QStringLiteral("/api/protected")));
        authReq.setRawHeader(kAuthHeader, kValidToken);
        QNetworkReply* authReply = mgr->get(authReq);
        QObject::connect(authReply, &QNetworkReply::finished,
                         authReply, [authReply, pending]() {
            printJson("GET /api/protected  (valid token -> 200)",
                      authReply->readAll());
            authReply->deleteLater();
            if (--*pending == 0) {
                QCoreApplication::quit();
            }
        });

        // ── GET /api/protected with invalid token ───────────────────────
        ++*pending;
        QNetworkRequest badReq(
            QUrl(kBaseUrl + QStringLiteral("/api/protected")));
        badReq.setRawHeader(kAuthHeader, kInvalidToken);
        QNetworkReply* badReply = mgr->get(badReq);
        QObject::connect(badReply, &QNetworkReply::finished,
                         badReply, [badReply, pending]() {
            printJson("GET /api/protected  (bad token -> 401)",
                      badReply->readAll());
            badReply->deleteLater();
            if (--*pending == 0) {
                QCoreApplication::quit();
            }
        });

        // ── POST /api/data ──────────────────────────────────────────────
        ++*pending;
        QJsonObject payload;
        payload[QStringLiteral("name")]  = QStringLiteral("Qt");
        payload[QStringLiteral("value")] = 42;

        QNetworkRequest postReq(
            QUrl(kBaseUrl + QStringLiteral("/api/data")));
        postReq.setHeader(QNetworkRequest::ContentTypeHeader,
                          QByteArrayLiteral("application/json"));
        QNetworkReply* postReply = mgr->post(
            postReq,
            QJsonDocument(payload).toJson(QJsonDocument::Compact));
        QObject::connect(postReply, &QNetworkReply::finished,
                         postReply, [postReply, pending]() {
            printJson("POST /api/data  (echo JSON)",
                      postReply->readAll());
            postReply->deleteLater();
            if (--*pending == 0) {
                QCoreApplication::quit();
            }
        });

        // ── GET /api/time ───────────────────────────────────────────────
        ++*pending;
        QNetworkRequest timeReq(
            QUrl(kBaseUrl + QStringLiteral("/api/time")));
        QNetworkReply* timeReply = mgr->get(timeReq);
        QObject::connect(timeReply, &QNetworkReply::finished,
                         timeReply, [timeReply, pending]() {
            printJson("GET /api/time", timeReply->readAll());
            timeReply->deleteLater();
            if (--*pending == 0) {
                QCoreApplication::quit();
            }
        });

        // Safety timeout: if any reply hangs, exit after 10 seconds
        QTimer::singleShot(10000, [&app]() {
            std::fprintf(stderr, "\nTimeout - forcing exit.\n");
            app.exit(1);
        });
    });

    return app.exec();
}
