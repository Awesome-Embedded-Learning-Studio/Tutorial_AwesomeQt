/// @file    api_server.cpp
/// @brief   Implementation of the REST API server with middleware.

#include "api_server.h"
#include "auth_middleware.h"

#include <QByteArray>
#include <QDateTime>
#include <QHttpHeaders>
#include <QHttpServer>
#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpServer>
#include <QTimeZone>

// ─────────────────────────────────────────────────────────────────────────────
ApiServer::ApiServer(QObject* parent)
    : QObject(parent)
    , m_server(std::make_unique<QHttpServer>())
    , m_running(false)
{
}

// ─────────────────────────────────────────────────────────────────────────────
ApiServer::~ApiServer()
{
    stop();
}

// ─────────────────────────────────────────────────────────────────────────────
void ApiServer::registerRoutes()
{
    // Pre-build the CORS headers so every response can share the same set.
    QHttpHeaders cors = AuthMiddleware::corsHeaders();

    // ── Root: serve a minimal HTML landing page ──────────────────────────
    // Returning HTML at "/" lets developers open the server directly in a
    // browser to confirm it is alive before hitting API endpoints.
    m_server->route(
        "/", QHttpServerRequest::Method::Get,
        [cors](const QHttpServerRequest&) -> QHttpServerResponse {
            static const QByteArray html =
                "<!DOCTYPE html>\n"
                "<html><head><title>Qt HTTP Server Demo</title></head>\n"
                "<body>\n"
                "  <h1>Qt HTTP Server &mdash; Advanced Demo</h1>\n"
                "  <ul>\n"
                "    <li><code>GET /api/status</code> &mdash; server status</li>\n"
                "    <li><code>GET /api/time</code> &mdash; current server time</li>\n"
                "    <li><code>GET /api/protected</code> &mdash; requires Bearer token</li>\n"
                "    <li><code>POST /api/data</code> &mdash; echo JSON body</li>\n"
                "  </ul>\n"
                "</body></html>";

            QHttpServerResponse response(
                QByteArrayLiteral("text/html"), html);
            response.setHeaders(QHttpHeaders(cors));
            return response;
        });

    // ── GET /api/status ──────────────────────────────────────────────────
    m_server->route(
        "/api/status", QHttpServerRequest::Method::Get,
        [cors](const QHttpServerRequest&) -> QHttpServerResponse {
            QJsonObject obj;
            obj[QStringLiteral("status")]  = QStringLiteral("ok");
            obj[QStringLiteral("version")] = QStringLiteral("1.0.0");
            obj[QStringLiteral("uptime")]  = QDateTime::currentDateTime().toString(
                Qt::ISODate);

            return jsonResponse(
                QJsonDocument(obj).toJson(QJsonDocument::Compact));
        });

    // ── GET /api/time ────────────────────────────────────────────────────
    m_server->route(
        "/api/time", QHttpServerRequest::Method::Get,
        [cors](const QHttpServerRequest&) -> QHttpServerResponse {
            QJsonObject obj;
            obj[QStringLiteral("time")] = QDateTime::currentDateTime().toString(
                Qt::ISODate);
            obj[QStringLiteral("tz")]   = QString::fromUtf8(
                QDateTime::currentDateTime().timeZone().id());

            return jsonResponse(
                QJsonDocument(obj).toJson(QJsonDocument::Compact));
        });

    // ── GET /api/protected (requires auth) ───────────────────────────────
    // This route demonstrates middleware-style token checking. The handler
    // inspects the Authorization header and returns 401 when the token is
    // missing or invalid.
    m_server->route(
        "/api/protected", QHttpServerRequest::Method::Get,
        [](const QHttpServerRequest& request) -> QHttpServerResponse {
            UserInfo user = AuthMiddleware::checkToken(request);

            if (!user.tokenValid) {
                QJsonObject err;
                err[QStringLiteral("error")]  = QStringLiteral("unauthorized");
                err[QStringLiteral("message")] =
                    QStringLiteral("Valid Bearer token required");

                return jsonResponse(
                    QJsonDocument(err).toJson(QJsonDocument::Compact),
                    QHttpServerResponse::StatusCode::Unauthorized);
            }

            QJsonObject obj;
            obj[QStringLiteral("message")]  = QStringLiteral("access granted");
            obj[QStringLiteral("username")] = user.username;
            obj[QStringLiteral("role")]     = user.role;

            return jsonResponse(
                QJsonDocument(obj).toJson(QJsonDocument::Compact));
        });

    // ── POST /api/data (echo back JSON body) ─────────────────────────────
    m_server->route(
        "/api/data", QHttpServerRequest::Method::Post,
        [](const QHttpServerRequest& request) -> QHttpServerResponse {
            // Parse the incoming JSON body; if invalid, return 400.
            QJsonParseError parseError;
            const QJsonDocument doc =
                QJsonDocument::fromJson(request.body(), &parseError);

            if (parseError.error != QJsonParseError::NoError) {
                QJsonObject err;
                err[QStringLiteral("error")]  = QStringLiteral("invalid_json");
                err[QStringLiteral("message")] = parseError.errorString();

                return jsonResponse(
                    QJsonDocument(err).toJson(QJsonDocument::Compact),
                    QHttpServerResponse::StatusCode::BadRequest);
            }

            QJsonObject echo;
            echo[QStringLiteral("echo")]   = doc.object();
            echo[QStringLiteral("length")] = request.body().size();

            return jsonResponse(
                QJsonDocument(echo).toJson(QJsonDocument::Compact));
        });

    // ── OPTIONS preflight (CORS) ─────────────────────────────────────────
    // Browsers send an OPTIONS request before cross-origin POST/PUT/DELETE.
    // Responding with 204 + CORS headers allows the actual request through.
    m_server->route(
        "/api/", QHttpServerRequest::Method::Options,
        [](const QHttpServerRequest&) -> QHttpServerResponse {
            QHttpServerResponse response(
                QHttpServerResponse::StatusCode::NoContent);
            response.setHeaders(AuthMiddleware::corsHeaders());
            return response;
        });
}

// ─────────────────────────────────────────────────────────────────────────────
bool ApiServer::start(quint16 port)
{
    if (m_running) {
        return true;
    }

    registerRoutes();

    // QHttpServer::bind() takes ownership of the QTcpServer.
    auto tcpServer = std::make_unique<QTcpServer>();
    if (!tcpServer->listen(QHostAddress::Any, port)) {
        return false;
    }

    m_server->bind(tcpServer.release());
    m_running = true;

    emit serverStarted(port);
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
void ApiServer::stop()
{
    if (!m_running) {
        return;
    }

    // Re-creating QHttpServer ensures all route handlers are cleared and
    // old connections are dropped. Cheaper than manually unbinding.
    m_server   = std::make_unique<QHttpServer>();
    m_running  = false;
}

// ─────────────────────────────────────────────────────────────────────────────
QHttpServerResponse ApiServer::jsonResponse(
    const QByteArray& body,
    QHttpServerResponse::StatusCode status)
{
    QHttpServerResponse response(
        QByteArrayLiteral("application/json"), body, status);
    response.setHeaders(AuthMiddleware::corsHeaders());
    return response;
}
