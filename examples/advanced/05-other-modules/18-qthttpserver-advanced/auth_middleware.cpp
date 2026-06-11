/// @file    auth_middleware.cpp
/// @brief   Implementation of authentication middleware utilities.

#include "auth_middleware.h"

#include <QByteArray>
#include <QHttpHeaders>
#include <QHttpServerRequest>

// ── Hard-coded demo credentials ──────────────────────────────────────────────
// In a real application these would come from environment variables, a vault
// service, or a JWT secret key. Keeping them here makes the example self-
// contained and easy to run without external configuration.
static const QByteArray kDemoToken   = QByteArrayLiteral("Bearer demo-token-123");
static const QString kDemoUser       = QStringLiteral("demo_user");
static const QString kDemoRole       = QStringLiteral("admin");

// ─────────────────────────────────────────────────────────────────────────────
UserInfo AuthMiddleware::checkToken(const QHttpServerRequest& request)
{
    UserInfo info;
    info.username   = QString();
    info.role       = QString();
    info.tokenValid = false;

    // QHttpServerRequest::headers() returns a const QHttpHeaders&.
    // We look up the Authorization header and compare it to the demo token.
    const QHttpHeaders& headers = request.headers();
    QByteArrayView authValue = headers.value(
        QHttpHeaders::WellKnownHeader::Authorization);

    if (!authValue.isNull()) {
        // QByteArrayView → QByteArray for comparison
        if (authValue.toByteArray() == kDemoToken) {
            info.username   = kDemoUser;
            info.role       = kDemoRole;
            info.tokenValid = true;
        }
    }

    return info;
}

// ─────────────────────────────────────────────────────────────────────────────
QHttpHeaders AuthMiddleware::corsHeaders()
{
    // These headers allow cross-origin requests from browsers.
    // In production you would restrict Access-Control-Allow-Origin to
    // specific trusted domains rather than using the wildcard "*".
    QHttpHeaders h;
    h.append(QHttpHeaders::WellKnownHeader::AccessControlAllowOrigin,
             QByteArrayLiteral("*"));
    h.append(QHttpHeaders::WellKnownHeader::AccessControlAllowMethods,
             QByteArrayLiteral("GET, POST, OPTIONS"));
    h.append(QHttpHeaders::WellKnownHeader::AccessControlAllowHeaders,
             QByteArrayLiteral("Content-Type, Authorization"));
    h.append(QHttpHeaders::WellKnownHeader::AccessControlMaxAge,
             QByteArrayLiteral("86400"));
    return h;
}
