/// @file    api_server.h
/// @brief   REST API server built on QHttpServer with route registration.
///
/// Demonstrates HTTP routing, JSON responses, Bearer token authentication,
/// and CORS middleware — all within a single self-contained example.

#pragma once

#include <QHttpServerResponse>
#include <QObject>
#include <memory>

class QHttpServer;

/// @brief A lightweight REST API server wrapping QHttpServer.
///
/// Registers several demo routes (status, time, protected resource, echo)
/// and applies CORS headers to every outgoing response so the API can be
/// tested from a browser-based client without extra configuration.
class ApiServer : public QObject
{
    Q_OBJECT

public:
    /// @brief Constructs the server; does NOT start listening.
    /// @param[in] parent  Optional parent for Qt object-tree ownership.
    explicit ApiServer(QObject* parent = nullptr);

    /// @brief Destructor — stops the server if still running.
    ~ApiServer() override;

    // Non-copyable, non-movable
    ApiServer(const ApiServer&)            = delete;
    ApiServer& operator=(const ApiServer&) = delete;
    ApiServer(ApiServer&&)                 = delete;
    ApiServer& operator=(ApiServer&&)      = delete;

    /// @brief Starts listening on the given TCP port.
    /// @param[in] port  The port number to bind (default 8080).
    /// @return true if the server bound successfully, false on error.
    bool start(quint16 port = 8080);

    /// @brief Stops listening and releases the HTTP server resources.
    void stop();

signals:
    /// @brief Emitted when the server has started listening.
    /// @param[out] port  The actual port the server is bound to.
    void serverStarted(quint16 port);

private:
    /// @brief Registers all REST API routes on the internal QHttpServer.
    /// @note Called once during start(). Separating registration from
    ///       construction keeps the ctor cheap and testable.
    void registerRoutes();

    /// @brief Creates a JSON response with CORS headers applied.
    /// @param[in] body    The raw response body (usually JSON).
    /// @param[in] status  The HTTP status code (default 200).
    /// @return A fully-formed QHttpServerResponse ready to be returned
    ///         from a route handler.
    static QHttpServerResponse jsonResponse(
        const QByteArray& body,
        QHttpServerResponse::StatusCode status =
            QHttpServerResponse::StatusCode::Ok);

    std::unique_ptr<QHttpServer> m_server;  ///< Owns the HTTP server instance.
    bool m_running;                          ///< True while the server is listening.
};
