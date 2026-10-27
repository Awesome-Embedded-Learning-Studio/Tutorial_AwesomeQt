/// @file    main.cpp
/// @brief   程序入口：演示优先级队列、并发限制、拦截器注入请求头、Cookie 持久化。
///
/// @details 对应教程：进阶层 04-QtNetwork/03-HTTP 高级用法。
///          入队 5 个 HTTP GET 请求（含不同优先级），使用 maxConcurrent=2 限制并发，
///          通过拦截器自动注入 User-Agent 和 Authorization 请求头，
///          并将 Cookie 存储到临时文件以演示持久化。

#include "cookie_persistence.h"
#include "request_manager.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QNetworkCookie>
#include <QTimer>

#include <cstdio>

/// @brief 截断过长的响应体，用于终端输出预览。
/// @param[in] body     原始响应体。
/// @param[in] maxBytes 最大显示字节数。
/// @return 截断后的字符串（附加省略号标记）。
static QString snippet(const QByteArray& body, int maxBytes = 200)
{
    if (body.size() <= maxBytes)
    {
        return QString::fromUtf8(body);
    }
    return QString::fromUtf8(body.left(maxBytes)) + QStringLiteral("...");
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    // -- Cookie 持久化：使用临时路径，演示跨会话 Cookie 保存 ----------
    // @note 实际项目中应使用 QStandardPaths 标准路径，此处为演示简洁使用临时文件。
    const QString cookiePath =
        QStringLiteral("/tmp/03_http_advanced_cookies.json");
    auto* cookieJar = new PersistentCookieJar(cookiePath, &app);

    // 演示：手动向 Jar 中添加一个 Cookie，验证序列化
    {
        QNetworkCookie demoCookie;
        demoCookie.setName("session_id");
        demoCookie.setValue("demo_token_abc123");
        demoCookie.setDomain(".httpbin.org");
        demoCookie.setPath("/");
        demoCookie.setExpirationDate(
            QDateTime::currentDateTime().addDays(1));
        // @note setAllCookiesAndSave 内部调用 protected 的 setAllCookies + saveToDisk
        QList<QNetworkCookie> all = cookieJar->getAllCookies();
        all.append(demoCookie);
        cookieJar->setAllCookiesAndSave(all);
        std::printf("[Cookie] Manually injected session_id for .httpbin.org\n");
    }

    // -- 请求管理器：最大 2 并发 ------------------------------------
    auto* manager = new RequestManager(/*maxConcurrent=*/2, &app);

    // -- 拦截器：自动注入 User-Agent 和 Authorization 请求头 ---------
    manager->setInterceptor([](QNetworkRequest& request) {
        request.setHeader(QNetworkRequest::UserAgentHeader,
                          QVariant(QStringLiteral("AwesomeQtTutorial/1.0")));
        request.setRawHeader("Authorization",
                             "Bearer demo-token-for-tutorial");
        std::printf("  [Interceptor] Injected headers for: %s\n",
                    qPrintable(request.url().toString()));
    });

    // -- 完成计数，用于自动退出 ------------------------------------
    int finishedCount = 0;
    constexpr int kTotalRequests = 5;

    // -- 连接信号 --------------------------------------------------
    QObject::connect(manager, &RequestManager::requestFinished,
                     [&](const QUrl& url, int httpStatus,
                         const QByteArray& body) {
                         ++finishedCount;
                         std::printf("\n[%d/%d] Finished: %s\n"
                                     "  Status: %d\n"
                                     "  Body snippet: %s\n",
                                     finishedCount, kTotalRequests,
                                     qPrintable(url.toString()),
                                     httpStatus,
                                     qPrintable(snippet(body)));
                     });

    QObject::connect(manager, &RequestManager::allFinished, [&]() {
        std::printf("\n=== All %d requests completed ===\n", kTotalRequests);

        // 验证 Cookie 持久化：打印保存的 Cookie
        const QList<QNetworkCookie> saved = cookieJar->getAllCookies();
        std::printf("\n[Cookie] Persisted cookies (%zu):\n",
                    static_cast<size_t>(saved.size()));
        for (const QNetworkCookie& c : saved)
        {
            std::printf("  %s=%s (domain=%s)\n",
                        c.name().constData(),
                        c.value().constData(),
                        qPrintable(c.domain()));
        }

        // 延迟退出，确保所有输出已刷新
        QTimer::singleShot(100, &app, &QCoreApplication::quit);
    });

    // -- 入队 5 个请求（展示不同优先级） -----------------------------
    std::printf("Enqueuing %d requests with maxConcurrent=2...\n\n",
                kTotalRequests);

    // kLow 优先级（最后出队）
    manager->enqueue(QUrl("https://httpbin.org/get?seq=1"),
                     RequestManager::Priority::kLow);
    manager->enqueue(QUrl("https://httpbin.org/get?seq=2"),
                     RequestManager::Priority::kLow);

    // kNormal 优先级
    manager->enqueue(QUrl("https://httpbin.org/get?seq=3"),
                     RequestManager::Priority::kNormal);

    // kHigh 优先级（最先出队）
    manager->enqueue(QUrl("https://httpbin.org/get?seq=4"),
                     RequestManager::Priority::kHigh);
    manager->enqueue(QUrl("https://httpbin.org/get?seq=5"),
                     RequestManager::Priority::kHigh);

    std::printf("Queue populated: pending=%d, active=%d\n",
                manager->pendingCount(), manager->activeCount());
    std::printf("Expected dispatch order: High(seq4/5) -> Normal(seq3)"
                " -> Low(seq1/2)\n");

    return app.exec();
}
