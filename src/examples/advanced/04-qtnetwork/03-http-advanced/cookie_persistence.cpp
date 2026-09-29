/// @file    cookie_persistence.cpp
/// @brief   PersistentCookieJar 类的实现：Cookie 持久化到 JSON 文件。
///
/// @details 对应教程：进阶层 04-QtNetwork/03-HTTP 高级用法。

#include "cookie_persistence.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QStandardPaths>

// --------------------------------------------------------------------------
// 构造 / 析构
// --------------------------------------------------------------------------

PersistentCookieJar::PersistentCookieJar(const QString& filePath, QObject* parent)
    : QNetworkCookieJar(parent)
    , m_filePath(filePath)
{
    // @note 若未指定路径，则使用 QStandardPaths 在标准缓存目录下生成文件，
    //       避免硬编码路径，保证跨平台可移植性。
    if (m_filePath.isEmpty())
    {
        const QString cacheDir =
            QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
        QDir().mkpath(cacheDir);  // 确保目录存在
        m_filePath = cacheDir + QStringLiteral("/cookies.json");
    }

    loadFromDisk();
}

PersistentCookieJar::~PersistentCookieJar()
{
    saveToDisk();
}

// --------------------------------------------------------------------------
// 重写方法
// --------------------------------------------------------------------------

QList<QNetworkCookie> PersistentCookieJar::cookiesForUrl(const QUrl& url) const
{
    // 直接委托给基类，基类内部已完成域名、路径、HttpOnly 等匹配
    return QNetworkCookieJar::cookiesForUrl(url);
}

bool PersistentCookieJar::setCookiesFromUrl(
    const QList<QNetworkCookie>& cookieList, const QUrl& url)
{
    // 基类方法负责实际的 Cookie 存储和过期处理
    const bool ok = QNetworkCookieJar::setCookiesFromUrl(cookieList, url);
    if (ok)
    {
        saveToDisk();
    }
    return ok;
}

// --------------------------------------------------------------------------
// 公有方法
// --------------------------------------------------------------------------

QList<QNetworkCookie> PersistentCookieJar::getAllCookies() const
{
    return allCookies();
}

void PersistentCookieJar::setAllCookiesAndSave(const QList<QNetworkCookie>& cookies)
{
    setAllCookies(cookies);
    saveToDisk();
}

void PersistentCookieJar::saveToDisk()
{
    const QByteArray data = serializeCookies(allCookies());

    // @note 使用 QSaveFile 替代 QFile 可避免写入中断导致文件损坏，
    //       但此处为教学简洁使用 QFile，生产环境建议 QSaveFile。
    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qWarning("Failed to open cookie file for writing: %s",
                 qPrintable(m_filePath));
        return;
    }

    file.write(data);
    file.close();
}

// --------------------------------------------------------------------------
// 私有方法
// --------------------------------------------------------------------------

void PersistentCookieJar::loadFromDisk()
{
    QFile file(m_filePath);
    if (!file.exists())
    {
        return;  // 首次运行，无历史 Cookie
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning("Failed to open cookie file for reading: %s",
                 qPrintable(m_filePath));
        return;
    }

    const QByteArray data = file.readAll();
    file.close();

    const QList<QNetworkCookie> cookies = deserializeCookies(data);
    setAllCookies(cookies);
}

QByteArray PersistentCookieJar::serializeCookies(
    const QList<QNetworkCookie>& cookies)
{
    QJsonArray arr;
    for (const QNetworkCookie& cookie : cookies)
    {
        QJsonObject obj;
        obj[QStringLiteral("name")] = QString::fromUtf8(cookie.name());
        obj[QStringLiteral("domain")] = cookie.domain();
        obj[QStringLiteral("path")] = cookie.path();
        obj[QStringLiteral("value")] = QString::fromUtf8(cookie.value());
        obj[QStringLiteral("secure")] = cookie.isSecure();
        obj[QStringLiteral("httponly")] = cookie.isHttpOnly();

        if (cookie.expirationDate().isValid())
        {
            obj[QStringLiteral("expires")] =
                cookie.expirationDate().toString(Qt::ISODate);
        }

        arr.append(obj);
    }

    QJsonDocument doc(arr);
    return doc.toJson(QJsonDocument::Indented);
}

QList<QNetworkCookie> PersistentCookieJar::deserializeCookies(const QByteArray& data)
{
    QList<QNetworkCookie> cookies;

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        qWarning("Cookie JSON parse error: %s", qPrintable(parseError.errorString()));
        return cookies;
    }

    if (!doc.isArray())
    {
        qWarning("Cookie file root is not a JSON array");
        return cookies;
    }

    const QJsonArray arr = doc.array();
    for (const QJsonValue& val : arr)
    {
        if (!val.isObject())
        {
            continue;
        }

        const QJsonObject obj = val.toObject();
        QNetworkCookie cookie;

        cookie.setName(obj[QStringLiteral("name")].toString().toUtf8());
        cookie.setDomain(obj[QStringLiteral("domain")].toString());
        cookie.setPath(obj[QStringLiteral("path")].toString());
        cookie.setValue(obj[QStringLiteral("value")].toString().toUtf8());
        cookie.setSecure(obj[QStringLiteral("secure")].toBool());
        cookie.setHttpOnly(obj[QStringLiteral("httponly")].toBool());

        const QString expiresStr = obj[QStringLiteral("expires")].toString();
        if (!expiresStr.isEmpty())
        {
            cookie.setExpirationDate(QDateTime::fromString(expiresStr, Qt::ISODate));
        }

        // @note 跳过已过期的 Cookie，避免加载后立即失效
        if (cookie.expirationDate().isValid()
            && cookie.expirationDate() < QDateTime::currentDateTime())
        {
            continue;
        }

        cookies.append(cookie);
    }

    return cookies;
}
