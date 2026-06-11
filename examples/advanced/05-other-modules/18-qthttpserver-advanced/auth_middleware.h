/// @file    auth_middleware.h
/// @brief   JWT-like authentication middleware for the HTTP server demo.
///
/// Demonstrates a simple Bearer token validation pattern and CORS header
/// injection for REST API endpoints.

#pragma once

#include <QHttpHeaders>
#include <QString>

class QHttpServerRequest;

/// @brief Holds parsed user identity extracted from an Authorization header.
struct UserInfo
{
    QString username;   ///< Display name decoded from the token.
    QString role;       ///< Role string (e.g. "admin", "guest").
    bool tokenValid;    ///< True only when the token passes validation.
};

/// @brief Stateless authentication middleware for QHttpServer routes.
///
/// This class is intentionally lightweight: it checks a hard-coded demo
/// token so the example stays self-contained. In production code you would
/// replace checkToken() with real JWT verification against a secret key.
class AuthMiddleware
{
public:
    /// @brief Validates the Authorization header on an incoming request.
    /// @param[in] request  The incoming HTTP request to inspect.
    /// @return A UserInfo struct; tokenValid is true only on success.
    /// @note The demo accepts exactly "Bearer demo-token-123" as a valid
    ///       credential. Any other value (or missing header) is rejected.
    static UserInfo checkToken(const QHttpServerRequest& request);

    /// @brief Builds a QHttpHeaders populated with standard CORS entries.
    /// @return A QHttpHeaders object ready to be set on any response.
    /// @note Static so it can be called from any route handler without
    ///       an AuthMiddleware instance.
    static QHttpHeaders corsHeaders();
};
