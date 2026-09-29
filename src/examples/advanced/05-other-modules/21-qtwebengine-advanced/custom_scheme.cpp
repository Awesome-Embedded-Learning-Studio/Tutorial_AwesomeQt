/// @file    custom_scheme.cpp
/// @brief   Implementation of the custom "demo://" URL scheme handler.

#include "custom_scheme.h"

#include <QBuffer>
#include <QUrl>

CustomSchemeHandler::CustomSchemeHandler(QObject* parent)
    : QWebEngineUrlSchemeHandler(parent)
{
}

void CustomSchemeHandler::requestStarted(QWebEngineUrlRequestJob* request)
{
    if (!request) {
        return;
    }

    const QUrl url = request->requestUrl();
    const QString path = url.path().toLower();

    QByteArray html;

    // Route based on the path component of the demo:// URL
    if (path == QStringLiteral("/hello") || path == QStringLiteral("hello")) {
        html = helloPageHtml();
    } else if (path == QStringLiteral("/info") || path == QStringLiteral("info")) {
        html = infoPageHtml(url);
    } else {
        html = notFoundPageHtml(url);
    }

    // QBuffer takes ownership of the QByteArray data; request takes ownership
    // of the buffer via reply() and will delete it when done.
    auto* buffer = new QBuffer;
    buffer->setData(html);
    buffer->open(QIODevice::ReadOnly);

    request->reply("text/html;charset=UTF-8", buffer);
}

QByteArray CustomSchemeHandler::helloPageHtml()
{
    const QByteArray html = R"(<!DOCTYPE html>
<html>
<head><meta charset="UTF-8"><title>Demo - Hello</title></head>
<body style="font-family:sans-serif; padding:2em;">
    <h1>Hello from Custom Scheme</h1>
    <p>This page was served by a <code>QWebEngineUrlSchemeHandler</code>.</p>
    <p>The URL scheme <strong>demo://</strong> is fully custom &mdash;
       no network request was made.</p>
    <hr>
    <p><a href="demo://info">Go to Info page</a></p>
</body>
</html>)";
    return html;
}

QByteArray CustomSchemeHandler::infoPageHtml(const QUrl& url)
{
    // Build a page that reflects the request URL back to the user
    const QString urlStr = url.toString();
    const QString html =
        QStringLiteral("<!DOCTYPE html>"
            "<html>"
            "<head><meta charset=\"UTF-8\"><title>Demo - Info</title></head>"
            "<body style=\"font-family:sans-serif; padding:2em;\">"
            "<h1>Page Info</h1>"
            "<table border=\"1\" cellpadding=\"6\" cellspacing=\"0\">"
            "<tr><td><strong>Scheme</strong></td><td>")
        + url.scheme()
        + QStringLiteral("</td></tr>"
            "<tr><td><strong>Host</strong></td><td>")
        + url.host()
        + QStringLiteral("</td></tr>"
            "<tr><td><strong>Path</strong></td><td>")
        + url.path()
        + QStringLiteral("</td></tr>"
            "<tr><td><strong>Full URL</strong></td><td>")
        + urlStr
        + QStringLiteral("</td></tr>"
            "</table>"
            "<hr>"
            "<p><a href=\"demo://hello\">Back to Hello page</a></p>"
            "</body></html>");
    return html.toUtf8();
}

QByteArray CustomSchemeHandler::notFoundPageHtml(const QUrl& url)
{
    const QString urlStr = url.toString();
    const QString html =
        QStringLiteral("<!DOCTYPE html>"
            "<html>"
            "<head><meta charset=\"UTF-8\"><title>Demo - Not Found</title></head>"
            "<body style=\"font-family:sans-serif; padding:2em;\">"
            "<h1>Page Not Found</h1>"
            "<p>No handler for URL: <code>")
        + urlStr
        + QStringLiteral("</code></p>"
            "<p>Try one of these:</p>"
            "<ul>"
            "<li><a href=\"demo://hello\">demo://hello</a></li>"
            "<li><a href=\"demo://info\">demo://info</a></li>"
            "</ul>"
            "</body></html>");
    return html.toUtf8();
}
