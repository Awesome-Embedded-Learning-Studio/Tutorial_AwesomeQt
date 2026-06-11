/// @file    web_profile_manager.cpp
/// @brief   Implementation of WebProfileManager — configures profile, cookies, and scheme.

#include "web_profile_manager.h"

#include "custom_scheme.h"

#include <QWebEngineProfile>

namespace
{
    /// User-agent string that identifies this demo application.
    const QString kUserAgent = QStringLiteral(
        "QtWebEngineDemo/1.0 (Advanced Tutorial; Custom Scheme Example)");
} // namespace

WebProfileManager::WebProfileManager(QObject* parent)
    : QObject(parent)
    , m_schemeHandler(new CustomSchemeHandler(this))  // Owned by this via Qt tree
{
}

void WebProfileManager::setupProfile(QWebEngineProfile* profile)
{
    if (!profile) {
        return;
    }

    // Use off-the-record mode so no persistent cache/cookies are written
    profile->setHttpUserAgent(kUserAgent);

    // Do not persist cookies to disk (off-the-record profile).
    // For fine-grained cookie filtering, use QWebEngineCookieStore::setCookieFilter().
    profile->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);

    // Persist neither HTTP cache nor persistent data to disk
    profile->setHttpCacheType(QWebEngineProfile::NoCache);

    // Install our custom demo:// scheme handler on this profile.
    // The handler's lifetime is tied to this manager, which outlives the profile.
    profile->installUrlSchemeHandler("demo", m_schemeHandler);
}

CustomSchemeHandler* WebProfileManager::schemeHandler() const
{
    return m_schemeHandler;
}
