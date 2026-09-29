/// @file    custom_scheme.h
/// @brief   Custom URL scheme handler for the "demo://" protocol.
///
/// Demonstrates how to register a custom URL scheme with QWebEngineProfile
/// and serve local HTML content through QWebEngineUrlSchemeHandler.

#pragma once

#include <QByteArray>
#include <QWebEngineUrlRequestJob>
#include <QWebEngineUrlSchemeHandler>

/// @brief Handles requests for the custom "demo://" URL scheme.
///
/// Routes demo://hello to a greeting page and demo://info to a page
/// that displays request metadata. All other demo:// URLs return a
/// "not found" page.
class CustomSchemeHandler : public QWebEngineUrlSchemeHandler
{
    Q_OBJECT

public:
    /// @brief Constructs the handler.
    /// @param[in] parent  Parent QObject for ownership via Qt object tree.
    explicit CustomSchemeHandler(QObject* parent = nullptr);

    /// @brief Called by QtWebEngine whenever a demo:// URL is requested.
    /// @param[in] request  The URL request job; call reply() to provide content.
    /// @note Ownership of @p request is NOT transferred — QtWebEngine deletes it.
    void requestStarted(QWebEngineUrlRequestJob* request) override;

private:
    /// @brief Generates the HTML page for demo://hello.
    /// @return UTF-8 encoded HTML bytes.
    static QByteArray helloPageHtml();

    /// @brief Generates the HTML page for demo://info, showing request metadata.
    /// @param[in] url  The full URL that was requested.
    /// @return UTF-8 encoded HTML bytes.
    static QByteArray infoPageHtml(const QUrl& url);

    /// @brief Generates a simple 404-style page for unknown demo:// paths.
    /// @param[in] url  The URL that could not be routed.
    /// @return UTF-8 encoded HTML bytes.
    static QByteArray notFoundPageHtml(const QUrl& url);
};
