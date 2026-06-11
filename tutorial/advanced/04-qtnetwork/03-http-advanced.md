---
title: "4.3 HTTP 进阶：请求队列、拦截器、Cookie"
description: "入门篇我们把 QNetworkAccessManager 的基本 GET/POST 流程跑通了——构造请求、发出去、读响应、处理错误。一个请求一个回调，简单清晰。但工程项目里的 HTTP 通信远不止这么简单。"
---

# 现代Qt开发教程（进阶篇）4.3——HTTP 进阶：请求队列、拦截器、Cookie

## 1. 前言 / 从「发个请求」到「管理一票请求」

入门篇我们把 QNetworkAccessManager 的基本 GET/POST 流程跑通了——构造请求、发出去、读响应、处理错误。一个请求一个回调，简单清晰。但工程项目里的 HTTP 通信远不止这么简单。

比如，你的应用需要同时发起多个请求——下载 10 张图片、调用 5 个 API。你当然可以一口气全部 `get()` 出去，但 QNetworkAccessManager 内部对同一主机的并发连接数有上限（Qt 6 默认是 6 个 HTTP/1.1 连接，HTTP/2 则多路复用不受此限制）。如果你的请求量特别大，理解这个调度机制就很重要。

再比如鉴权。几乎每个 REST API 都需要 Bearer Token 或者自定义的签名头。如果你在每个请求构造时都手动 `setRawHeader("Authorization", ...)`，代码很快就变得又臭又长。你需要一个「请求拦截器」——在请求发出前自动注入公共 header。

还有 Cookie 和会话管理。很多 Web 服务用 Cookie 维持登录状态。QNetworkAccessManager 内置了 `QNetworkCookieJar` 来自动管理 Cookie，但默认实现只在内存中存活——程序退出 Cookie 就没了。如果你需要持久化 Cookie（比如「记住登录」功能），就需要自定义 CookieJar。

这篇我们就一起来把这三个工程场景拆干净：并发请求限流与优先级管理、全局请求拦截器模式、Cookie 持久化与断点续传。

## 2. 环境说明

本文档基于 Qt 6.2+ 编写，使用 C++17 标准和 CMake 3.26+ 构建系统。默认你已经掌握入门篇中 QNetworkAccessManager 的基本用法，包括 `get()`、`post()`、`finished` 信号、`readAll()` 读取响应。本篇依赖 Qt6::Network 模块。部分示例涉及 HTTPS 请求，Qt 6 默认使用系统的 SSL 库（Linux 上是 OpenSSL，Windows 上是 Schannel）。

## 3. 核心概念讲解

### 3.1 并发请求限流——理解 QNetworkAccessManager 的调度策略

QNetworkAccessManager 在内部维护了一个请求队列和连接池。当你调用 `get()`、`post()` 等方法时，请求并不会立刻被发送——它先进入队列，然后由内部的调度器决定何时发出。

对于 HTTP/1.1 连接，Qt 对每个主机名维护最多 6 个并发 TCP 连接（这是 HTTP/1.1 规范的建议值）。如果你对同一主机发起了 10 个 GET 请求，前 6 个会立即发送，后 4 个在队列里排队。当某个请求完成后，队列中的下一个请求自动发出。这一切对你来说是透明的——你只管调 `get()`，Qt 帮你排队。

对于 HTTP/2 连接（Qt 6 默认协商），情况不同。HTTP/2 在单个 TCP 连接上多路复用多个流（stream），理论上可以同时承载大量请求。Qt 6 会在 TLS 握手时通过 ALPN 协商 HTTP/2，如果服务器支持，后续请求都会走 HTTP/2 的多路复用。这意味着 HTTP/2 环境下并发连接数不再是瓶颈。

但在实际工程中，你可能需要更精细的控制。比如，你的应用需要同时访问多个不同的 API 服务，每个服务的限流策略不同。或者你需要区分「高优先级请求」（用户触发的操作）和「低优先级请求」（后台预加载）。QNetworkAccessManager 本身不提供优先级控制，但我们可以在外层实现一个请求调度器。

```cpp
struct ScheduledRequest
{
    int priority;            // 数字越小优先级越高
    QNetworkRequest request;
    QByteArray verb;         // "GET", "POST", etc.
    QByteArray body;
    std::function<void(QNetworkReply*)> callback;
};

class RequestScheduler : public QObject
{
    Q_OBJECT
public:
    RequestScheduler(QNetworkAccessManager *nam, int maxConcurrent,
                     QObject *parent = nullptr)
        : QObject(parent), nam_(nam), maxConcurrent_(maxConcurrent)
    {}

    void enqueue(const ScheduledRequest &req)
    {
        queue_.append(req);
        // 按 priority 排序，小的在前
        std::sort(queue_.begin(), queue_.end(),
                  [](const ScheduledRequest &a, const ScheduledRequest &b) {
                      return a.priority < b.priority;
                  });
        processQueue();
    }

private:
    void processQueue()
    {
        while (activeCount_ < maxConcurrent_ && !queue_.isEmpty()) {
            ScheduledRequest req = queue_.takeFirst();
            dispatch(req);
        }
    }

    void dispatch(const ScheduledRequest &req)
    {
        activeCount_++;
        QNetworkReply *reply = nullptr;

        if (req.verb == "GET") {
            reply = nam_->get(req.request);
        } else if (req.verb == "POST") {
            reply = nam_->post(req.request, req.body);
        } else if (req.verb == "PUT") {
            reply = nam_->put(req.request, req.body);
        }

        if (!reply) {
            activeCount_--;
            return;
        }

        connect(reply, &QNetworkReply::finished, this, [this, reply, cb = req.callback]() {
            activeCount_--;
            cb(reply);
            reply->deleteLater();
            processQueue();  // 完成一个就尝试发下一个
        });
    }

    QNetworkAccessManager *nam_;
    int maxConcurrent_;
    int activeCount_ = 0;
    QList<ScheduledRequest> queue_;
};
```

这个调度器的核心是一个优先级队列和一个 `activeCount_` 计数器。每次请求完成后调用 `processQueue()`，从队列头取下一个请求发出。优先级通过 `std::sort` 保证——每次入队后重新排序，保证高优先级的请求总是先被取出。这当然不是最高效的优先级队列实现（大顶堆更优），但对于几十到几百个请求的场景，`QList` + `std::sort` 的性能完全够用。

### 3.2 全局请求拦截器——自动注入公共 Header

Qt 的 QNetworkAccessManager 提供了一个非常方便的虚拟函数 `createRequest()`，所有通过 `get()`、`post()` 等方法发出的请求最终都会调用它。通过继承 QNetworkAccessManager 并重写这个函数，我们可以在请求发出前拦截并修改它——这就是「拦截器」模式。

最常见的拦截器场景是自动注入鉴权 header。你的应用登录后拿到了一个 access token，后续所有 API 请求都需要带上 `Authorization: Bearer <token>` 头。如果你在每个请求构造时都手动 setRawHeader，代码会变得又臭又长，而且很容易遗漏。

```cpp
class AuthAwareNam : public QNetworkAccessManager
{
    Q_OBJECT
public:
    explicit AuthAwareNam(QObject *parent = nullptr)
        : QNetworkAccessManager(parent)
    {}

    void setAccessToken(const QByteArray &token)
    {
        token_ = token;
    }

protected:
    QNetworkReply *createRequest(Operation op,
                                  const QNetworkRequest &request,
                                  QIODevice *outgoingData) override
    {
        QNetworkRequest mutableRequest(request);

        // 自动注入 Authorization header
        if (!token_.isEmpty()) {
            mutableRequest.setRawHeader("Authorization",
                                        "Bearer " + token_);
        }

        // 自动注入 User-Agent
        mutableRequest.setRawHeader("User-Agent",
                                    "AwesomeQtApp/1.0");

        // 调用父类方法实际发送请求
        return QNetworkAccessManager::createRequest(
            op, mutableRequest, outgoingData);
    }

private:
    QByteArray token_;
};
```

`createRequest()` 的三个参数分别是操作类型（GET/POST/PUT 等）、原始请求对象、以及 POST/PUT 的请求体数据流。我们在调用父类的 `createRequest()` 之前，先把 request 复制一份，在副本上添加公共 header，然后把修改后的副本传给父类。这样不影响外层构造的原始 request 对象。

这个模式可以扩展为更复杂的拦截链——比如自动添加请求签名、自动重试失败的请求、记录请求日志等。核心思路都是一样的：在 `createRequest()` 里拦截，修改，然后传递给父类。

有一点需要注意：`createRequest()` 是在主线程（或 QNetworkAccessManager 所在的线程）调用的。如果你在这里做了耗时的操作（比如计算签名时涉及加密运算），会阻塞后续请求的调度。把耗时操作放到请求构造阶段（在 `enqueue` 之前），`createRequest()` 里只做轻量操作。

### 3.3 Cookie 管理——从内存到磁盘

QNetworkAccessManager 内置了一个 `QNetworkCookieJar`，它会自动处理 HTTP 响应中的 `Set-Cookie` 头，在后续请求中自动附加对应的 `Cookie` 头。这是默认行为——你不需要做任何配置，Cookie 就能自动在请求之间传递。

但默认的 `QNetworkCookieJar` 有一个致命缺陷：它只在内存中存储 Cookie。程序退出后所有 Cookie 丢失，下次启动时用户需要重新登录。对于桌面应用来说这很难接受。

解决方案是继承 `QNetworkCookieJar`，重写 `cookiesForUrl()` 和 `setCookiesFromReply()`（在 Qt 6 中实际上是重写 `virtual QList<QNetworkCookie> cookiesForUrl(const QUrl &url) const`），在内部用 QSettings 把 Cookie 序列化到磁盘。

```cpp
class PersistentCookieJar : public QNetworkCookieJar
{
    Q_OBJECT
public:
    explicit PersistentCookieJar(const QString &filePath,
                                  QObject *parent = nullptr)
        : QNetworkCookieJar(parent), filePath_(filePath)
    {
        loadFromDisk();
    }

    ~PersistentCookieJar()
    {
        saveToDisk();
    }

    QList<QNetworkCookie> cookiesForUrl(const QUrl &url) const override
    {
        // 先移除过期的 Cookie
        const_cast<PersistentCookieJar*>(this)->removeExpired();

        return QNetworkCookieJar::cookiesForUrl(url);
    }

    bool setCookiesFromReply(
        const QList<QNetworkCookie> &cookieList,
        const QUrl &url) override
    {
        bool result = QNetworkCookieJar::setCookiesFromReply(cookieList, url);
        if (result) {
            saveToDisk();
        }
        return result;
    }

private:
    void saveToDisk()
    {
        QList<QNetworkCookie> allCookies = allCookies();
        QSettings settings(filePath_, QSettings::IniFormat);
        settings.beginWriteArray("cookies");
        for (int i = 0; i < allCookies.size(); ++i) {
            settings.setArrayIndex(i);
            settings.setValue("data", allCookies[i].toRawForm());
        }
        settings.endArray();
    }

    void loadFromDisk()
    {
        QSettings settings(filePath_, QSettings::IniFormat);
        int size = settings.beginReadArray("cookies");
        QList<QNetworkCookie> cookies;
        for (int i = 0; i < size; ++i) {
            settings.setArrayIndex(i);
            QByteArray raw = settings.value("data").toByteArray();
            QList<QNetworkCookie> parsed =
                QNetworkCookie::parseCookies(raw);
            cookies.append(parsed);
        }
        settings.endArray();
        setAllCookies(cookies);
    }

    void removeExpired()
    {
        QList<QNetworkCookie> all = allCookies();
        QDateTime now = QDateTime::currentDateTime();
        all.erase(
            std::remove_if(all.begin(), all.end(),
                [&now](const QNetworkCookie &c) {
                    return c.expirationDate().isValid() &&
                           c.expirationDate() < now;
                }),
            all.end());
        setAllCookies(all);
    }

    QString filePath_;
};
```

使用方式非常简单——在创建 QNetworkAccessManager 后设置自定义的 CookieJar：

```cpp
auto *nam = new QNetworkAccessManager(this);
auto *jar = new PersistentCookieJar("cookies.ini", this);
nam->setCookieJar(jar);
```

这里有几个细节值得说一下。`QNetworkCookie::toRawForm()` 把 Cookie 序列化成 `name=value; expires=...; path=...` 格式的字符串，`QNetworkCookie::parseCookies()` 反向解析。这是 Qt 内置的序列化格式，不需要我们自己处理格式细节。

过期 Cookie 的清理放在了 `cookiesForUrl()` 里——每次有请求需要查 Cookie 时顺便清理。也可以放在定时器里定期清理，效果一样。关键是别让过期的 Cookie 一直占着内存和磁盘空间。

### 3.4 断点续传——Range 头与本地进度记录

断点续传的原理很简单：HTTP 的 `Range` 头告诉服务器「我只要这个文件的从第 N 个字节开始的部分」。如果服务器支持范围请求（响应头中有 `Accept-Ranges: bytes`），它会返回 206 Partial Content 状态码和请求的字节范围。

```cpp
void resumeDownload(QNetworkAccessManager *nam, const QUrl &url,
                     const QString &localPath)
{
    QFile *file = new QFile(localPath);
    qint64 existingSize = 0;

    if (file->exists()) {
        if (file->open(QIODevice::Append)) {
            existingSize = file->size();
        }
    } else {
        file->open(QIODevice::WriteOnly);
    }

    QNetworkRequest request(url);
    if (existingSize > 0) {
        // 从已有大小位置开始续传
        QByteArray rangeHeader = "bytes=" +
            QByteArray::number(existingSize) + "-";
        request.setRawHeader("Range", rangeHeader);
        qDebug() << "Resuming from byte" << existingSize;
    }

    QNetworkReply *reply = nam->get(request);

    connect(reply, &QNetworkReply::readyRead, [=]() {
        file->write(reply->readAll());
    });

    connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "Download complete.";
        } else if (reply->error() ==
                   QNetworkReply::OperationCanceledError) {
            qDebug() << "Download paused, can resume later.";
        } else {
            qDebug() << "Download error:" << reply->errorString();
        }
        file->close();
        file->deleteLater();
        reply->deleteLater();
    });
}
```

`Range: bytes=1000-` 表示从第 1000 字节开始读到文件末尾。服务器响应 206 时，响应头会包含 `Content-Range: bytes 1000-9999/10000`，告诉你这次返回的范围和文件总大小。如果服务器不支持范围请求，会返回 200 和完整文件——你的 `readyRead` 处理代码需要能应对这种情况（检测到 200 而不是 206 时，应该清空本地文件从头写）。

断点续传的关键在于本地进度记录。上面的代码用 `QFile::Append` 模式打开文件，新数据追加到已有数据后面。但这里有一个隐患：如果上一次下载中断时恰好写了一半的数据（文件系统缓存还没刷盘），续传时就会有一段脏数据。更稳健的做法是把下载进度（已下载字节数）单独保存到一个 QSettings 里，续传时先校验本地文件大小与记录是否一致，不一致就从头下载。

现在有一道调试题。你的断点续传在下载大文件时总是从头发起，Range 头明明设了但服务器返回 200 而不是 206。可能的原因是什么？

最可能的原因是服务器压根不支持范围请求——它没有在响应头中返回 `Accept-Ranges: bytes`。不是所有服务器都支持 Range，特别是一些 CDN 或对象存储服务在没有特定配置的情况下会忽略 Range 头。另一个可能的原因是你请求的是 HTTPS 但 Range 头被中间代理（比如公司内网的 HTTP 代理）剥离了。你可以在 `finished` 回调里检查 `reply->attribute(QNetworkRequest::HttpStatusCodeAttribute)` 来确认实际收到的状态码。

## 4. 踩坑预防

第一个坑是 QNetworkReply 的生命周期管理。QNetworkReply 是 `QNetworkAccessManager::get()` 的返回值，你必须在处理完毕后调用 `deleteLater()` 来释放它。但如果你在 `finished` 信号之前就 delete 了 reply（比如用户取消操作时直接 delete），Qt 内部的信号分发会访问已释放的对象，导致崩溃。正确做法是调用 `reply->abort()` 而不是直接 delete——abort 会触发 finished 信号（带 OperationCanceledError），你在 finished 的槽里再 deleteLater。

第二个坑是 QNetworkAccessManager 在错误的线程中使用。QNetworkAccessManager 必须在拥有事件循环的线程中创建和使用。如果你在工作线程里 new 了一个 QNetworkAccessManager，但没有在那个线程运行事件循环（`QThread::exec()`），finished 信号永远不会被投递，回调永远不会执行。解决方案是确保 QNetworkAccessManager 所在的线程有运行中的事件循环，或者用 `moveToThread()` 把它移到合适的工作线程。

第三个坑是 Cookie 的域匹配规则。浏览器的 Cookie 机制有严格的域匹配规则：`Set-Cookie: session=abc; Domain=example.com` 生成的 Cookie 只会在请求 `*.example.com` 时被发送，不会发给 `evil-example.com`。但 QNetworkCookieJar 的默认实现做了更宽松的匹配——某些边界情况下可能把 Cookie 发给了不该收到的域。如果你的应用涉及敏感的认证 Cookie，建议在 PersistentCookieJar 中增加域名校验逻辑。

## 5. 练习项目

练习项目：带登录状态的 API 客户端。我们要实现一个 HTTP 客户端，能够登录 REST API、自动维护会话状态、并发请求带限流控制。

具体要求：使用 AuthAwareNam 自动注入 Authorization 头，使用 PersistentCookieJar 持久化登录状态。支持 RequestScheduler 控制最大 3 个并发请求。登录接口调用后保存 token，后续请求自动带上。程序重启后检查 Cookie 有效性（调用一个需要认证的 API 测试），有效则恢复登录态，无效则重新登录。完成标准：登录后连续发起 10 个 API 请求全部成功、并发不超过 3 个、程序重启后无需重新登录即可访问 API。

提示几个关键点：可以用 httpbin.org 作为测试 API（`/post` 接受 POST、`/cookies/set` 设置 Cookie）。token 存在 QSettings 里配合 PersistentCookieJar 使用。RequestScheduler 的 `activeCount_` 在 abort/错误时也要递减，否则队列会卡死。

## 6. 官方文档参考链接

[Qt 文档 · QNetworkAccessManager](https://doc.qt.io/qt-6/qnetworkaccessmanager.html) -- HTTP 客户端核心类，包含请求调度、连接池、Cookie 管理等完整 API

[Qt 文档 · QNetworkReply](https://doc.qt.io/qt-6/qnetworkreply.html) -- HTTP 响应句柄，包含信号、错误处理和响应数据读取

[Qt 文档 · QNetworkRequest](https://doc.qt.io/qt-6/qnetworkrequest.html) -- HTTP 请求封装，包含 header 设置和 SSL 配置

[Qt 文档 · QNetworkCookieJar](https://doc.qt.io/qt-6/qnetworkcookiejar.html) -- Cookie 存储管理，支持子类化实现持久化

[Qt 文档 · QNetworkCookie](https://doc.qt.io/qt-6/qnetworkcookie.html) -- Cookie 数据结构，包含序列化和解析方法

*（链接已验证，2026-06-11 可访问）*

---

到这里就大功告成了。请求限流、拦截器模式、Cookie 持久化、断点续传——搞定了这些，你的 HTTP 客户端就从「能发请求」进化到了「会管理会话」。下一篇我们来看 WebSocket 的进阶用法，包括断线重连和心跳保活。
