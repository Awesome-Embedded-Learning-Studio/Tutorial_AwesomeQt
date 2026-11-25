---
title: "5.21 WebEngine 进阶：自定义 URL Scheme、安全策略"
description: "入门篇我们用 QWebEngineView 加载了网页，跑了 JavaScript，做了 C++ 和 JS 的双向调用。进阶篇要拆的是更底层的控制能力：自定义 URL Scheme 让你拦截所有请求、安全策略控制页面能做什么不能做什么、Profile 管理数据持久化、请求拦截器修改网络请求。"
---

# 现代Qt开发教程（进阶篇）5.21——WebEngine 进阶：自定义 URL Scheme、安全策略

## 1. 前言

入门篇我们让 `QWebEngineView` 跑起来了——加载 URL、执行 JavaScript、用 `QWebChannel` 做双向通信。但如果你想把 `QWebEngineView` 当作应用的 UI 引擎来用（不只是显示网页），很快就会遇到一堆问题。

你想从应用内部资源加载页面，不通过网络——需要自定义 URL Scheme。你不想让页面里嵌入的第三方脚本乱发网络请求——需要安全策略。你想给每个用户独立的浏览数据（Cookie、LocalStorage）——需要 Profile 管理。你需要在请求发出之前注入认证头或修改参数——需要请求拦截器。

这些都是 QtWebEngine 提供的底层控制能力。它们在 Chromium 的 Content API 层面实现，给了你对浏览器引擎的精细控制。

## 2. 环境说明

本文档基于 Qt 6.2+ 编写，需要 Qt6::WebEngineWidgets 和 Qt6::WebEngineCore 模块。CMake 中通过 `find_package(Qt6 REQUIRED COMPONENTS WebEngineWidgets WebEngineCore)` 引入。QtWebEngine 底层是 Chromium，编译需要较长时间，且在 WSL2 上可能因 EGL 支持问题无法运行——建议在 Windows 或 macOS 上开发和测试。

## 3. 核心概念讲解

### 3.1 自定义 URL Scheme——QWebEngineUrlSchemeHandler

`QWebEngineUrlSchemeHandler` 允许你注册自定义的 URL 协议。比如你想在页面中用 `myscheme://data/config.json` 这样的地址来访问应用内部的资源，而不是通过 HTTP 从网络加载。

自定义 URL Scheme 在嵌入式应用中特别有用：你的 HTML/CSS/JS 文件打包在 Qt 资源系统（qrc）或本地文件中，页面内用 `myscheme://` 引用它们，而不是部署一个本地 HTTP 服务器。

实现步骤分两步：先注册 Scheme，然后实现 Handler。

```cpp
/// @brief 自定义 URL Scheme 处理器。
/// @note 处理 myapp:// 协议的请求，从 Qt 资源系统返回文件内容。
class MyAppSchemeHandler : public QWebEngineUrlSchemeHandler
{
    Q_OBJECT

public:
    /// @brief 处理 myapp:// 协议的请求。
    /// @param[in] request 请求对象。
    void requestStarted(QWebEngineUrlRequestJob* request) override
    {
        QUrl kUrl = request->requestUrl();
        QString kPath = kUrl.path();

        // 从 Qt 资源系统读取文件
        QString kResourcePath = ":/web" + kPath;
        QFile kFile(kResourcePath);
        if (!kFile.open(QIODevice::ReadOnly)) {
            request->fail(QWebEngineUrlRequestJob::UrlNotFound);
            return;
        }

        QByteArray kData = kFile.readAll();
        QString kMimeType = guess_mime_from_path(kPath);

        // 构造响应
        QMimeType kMime = QMimeDatabase().mimeTypeForFile(kPath);
        auto* kReply = new QBuffer(this);
        kReply->setData(kData);
        kReply->open(QIODevice::ReadOnly);

        // 设置响应头
        QMap<QByteArray, QByteArray> kHeaders;
        kHeaders["Content-Type"] = kMimeType.toUtf8();
        kHeaders["Cache-Control"] = "no-cache";

        request->reply(kHeaders, kReply);
    }

private:
    /// @brief 根据文件扩展名返回 MIME 类型。
    static QString guess_mime_from_path(const QString& path)
    {
        if (path.endsWith(".html")) return "text/html";
        if (path.endsWith(".css"))  return "text/css";
        if (path.endsWith(".js"))   return "application/javascript";
        if (path.endsWith(".png"))  return "image/png";
        if (path.endsWith(".svg"))  return "image/svg+xml";
        return "application/octet-stream";
    }
};
```

注册 Scheme 需要在 `QApplication` 构造之前完成（这是 Chromium 的要求，Scheme 注册是全局的、一次性的）：

```cpp
/// @brief 在 main() 中 QApplication 构造之前调用。
void register_custom_scheme()
{
    QWebEngineUrlScheme kScheme("myapp");
    kScheme.setFlags(QWebEngineUrlScheme::SecureScheme |
                     QWebEngineUrlScheme::LocalScheme |
                     QWebEngineUrlScheme::LocalAccessAllowed);
    QWebEngineUrlScheme::registerScheme(kScheme);
}

int main(int argc, char* argv[])
{
    register_custom_scheme();  // 必须在 QApplication 之前
    QApplication app(argc, argv);

    QWebEngineView view;
    auto* profile = view.page()->profile();
    auto* handler = new MyAppSchemeHandler(profile);
    profile->installUrlSchemeHandler("myapp", handler);

    view.load(QUrl("myapp:///index.html"));
    view.show();
    return app.exec();
}
```

Scheme 的 Flag 控制了它的行为：`SecureScheme` 表示这是一个安全协议（类似 HTTPS，允许访问 HTTPS-only 的 API），`LocalScheme` 表示本地协议，`LocalAccessAllowed` 允许本地页面访问。

### 3.2 Content Security Policy 配置

Content Security Policy（CSP）是浏览器层面的安全策略，控制页面能加载哪些资源。在 QtWebEngine 中，你可以通过自定义 HTTP 响应头来注入 CSP。

```cpp
/// @brief 通过 QWebEngineUrlRequestInterceptor 注入安全头。
class SecurityHeaderInterceptor : public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT

public:
    /// @brief 拦截请求并注入安全头。
    /// @param[in,out] info 请求信息。
    void interceptRequest(QWebEngineUrlRequestInfo& info) override
    {
        // 注入 CSP 头（仅对自定义 Scheme 的响应有效）
        info.setExtraHeader("Content-Security-Policy",
            "default-src 'self' myapp:; "
            "script-src 'self' myapp: 'unsafe-inline'; "
            "style-src 'self' myapp: 'unsafe-inline'; "
            "img-src 'self' myapp: data:; "
            "connect-src 'self' ws://localhost:12345; "
            "frame-ancestors 'none';");
    }
};
```

这段 CSP 策略的含义：`default-src 'self' myapp:` 默认只允许加载同源和 `myapp:` 协议的资源；`script-src` 允许内联脚本（因为我们的 HTML 可能有内联 JS）；`connect-src` 允许连接本地 WebSocket 端口（用于 WebChannel 通信）；`frame-ancestors 'none'` 禁止被嵌入 iframe。

不过要注意，CSP 头是通过 HTTP 响应头传递的。如果你用自定义 URL Scheme 加载页面，需要在 Scheme Handler 的 `requestStarted` 中设置 CSP 响应头，而不是在请求拦截器中。请求拦截器修改的是发出的请求头，不是收到的响应头。

### 3.3 Profile 管理——持久化与 off-the-record

`QWebEngineProfile` 管理浏览器的持久化数据：Cookie、LocalStorage、缓存、下载记录等。每个 `QWebEnginePage` 都关联一个 Profile，Profile 决定了数据是持久化到磁盘还是只存在内存中。

```cpp
/// @brief 创建持久化 Profile（数据保存到磁盘）。
/// @param[in] profile_name Profile 名称。
/// @return Profile 对象指针。
QWebEngineProfile* create_persistent_profile(const QString& profile_name)
{
    auto* profile = new QWebEngineProfile(profile_name, this);

    // 数据存储路径——不同 Profile 的数据完全隔离
    profile->setPersistentStoragePath(
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
        + "/" + profile_name);

    // 持久化 Cookie 策略
    profile->setPersistentCookiesPolicy(
        QWebEngineProfile::ForcePersistentCookies);

    // HTTP 缓存大小（字节）
    profile->setHttpCacheMaximumSize(50 * 1024 * 1024);  // 50 MB

    return profile;
}

/// @brief 创建 off-the-record Profile（不保存数据到磁盘）。
/// @return Profile 对象指针。
QWebEngineProfile* create_incognito_profile()
{
    auto* profile = new QWebEngineProfile(this);
    // off-the-record 的 Profile 不调用 setPersistentStoragePath
    // 数据仅存在于内存中，Profile 销毁后数据消失
    profile->setPersistentCookiesPolicy(
        QWebEngineProfile::NoPersistentCookies);
    return profile;
}
```

持久化 Profile 适合主窗口——用户的登录状态、偏好设置在重启应用后保留。off-the-record Profile 适合隐私浏览或临时窗口——关闭后数据全部清除。一个应用可以有多个 Profile，每个 Profile 的数据完全隔离。比如你可以为「工作账号」和「个人账号」创建不同的 Profile，它们各自有独立的 Cookie 和 LocalStorage。

### 3.4 请求拦截——QWebEngineUrlRequestInterceptor

`QWebEngineUrlRequestInterceptor` 可以拦截所有从页面发出的网络请求，修改请求头或 URL，甚至直接阻塞请求。

```cpp
/// @brief 请求拦截器——注入认证头和阻塞特定请求。
class AuthInterceptor : public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT

public:
    /// @brief 拦截并修改请求。
    /// @param[in,out] info 请求信息。
    void interceptRequest(QWebEngineUrlRequestInfo& info) override
    {
        QUrl kUrl = info.requestUrl();

        // 对 API 请求注入认证头
        if (kUrl.host() == "api.example.com") {
            info.setExtraHeader("Authorization",
                                "Bearer " + m_auth_token.toUtf8());
        }

        // 阻塞广告和追踪脚本
        if (kUrl.host().endsWith("ads-service.com") ||
            kUrl.host().endsWith("tracker.io")) {
            info.block(true);
            return;
        }

        // 为所有请求添加自定义标识头
        info.setExtraHeader("X-App-Version", "2.0.1");
    }

    /// @brief 设置认证 Token。
    /// @param[in] token JWT Token。
    void set_auth_token(const QString& token) { m_auth_token = token; }

private:
    QString m_auth_token;  // 认证 Token
};
```

注册拦截器到 Profile：

```cpp
/// @brief 将拦截器注册到 Profile。
void setup_interceptor(QWebEngineProfile* profile)
{
    auto* interceptor = new AuthInterceptor(profile);
    profile->setUrlRequestInterceptor(interceptor);
}
```

`interceptRequest` 中可以对请求做三种操作：修改请求头（`setExtraHeader`）、修改 URL（`setRequestUrl`）、阻塞请求（`block(true)`）。拦截器在每个请求发出之前被调用，包括页面导航、资源加载、XHR/Fetch 请求、WebSocket 连接等。

现在有个思考题：如果在拦截器中修改了请求 URL（比如把 HTTP 重定向到 HTTPS），页面会察觉到这个修改吗？

答案是取决于情况。如果是资源加载请求（图片、脚本），页面通常不会察觉。如果是页面导航请求，地址栏的 URL 不会自动更新。拦截器是在网络层修改请求，不是在 UI 层。

## 4. 踩坑预防

第一个坑是自定义 URL Scheme 必须在 `QApplication` 构造之前注册。Chromium 的 Scheme 注册是进程级别的，一旦 Chromium 初始化完成就不能再注册新 Scheme 了。如果你在 `QApplication` 构造之后调用 `QWebEngineUrlScheme::registerScheme`，会直接失败（某些版本会 crash）。解决方案是把 Scheme 注册放在 `main()` 函数的最前面，在创建 `QApplication` 之前。

第二个坑是 `QWebEngineUrlRequestJob::reply()` 的第二个参数（`QIODevice*`）的生命周期管理。`reply()` 方法不会立即读取数据，它会在 Chromium 的 IO 线程上异步读取。如果你在 `requestStarted` 返回后立即销毁了 `QIODevice`，Chromium 会读取到无效数据。解决方案是将 `QIODevice` 的父对象设为 `request` 对象（或 Scheme Handler），这样当请求处理完成后 Qt 会自动销毁 `QIODevice`。上面代码中 `new QBuffer(this)` 把 Buffer 的父对象设为 Handler，可以确保它在请求期间有效。

第三个坑是 off-the-record Profile 不会被复用。每次 `new QWebEngineProfile()` 不传名称参数时，都会创建一个新的 off-the-record Profile。如果你为每个 `QWebEnginePage` 都创建了一个新的 off-the-record Profile，会导致大量的内存浪费。解决方案是复用同一个 Profile 对象——创建一次，所有需要的 Page 共享。

## 5. 练习项目

练习项目是一个使用自定义 URL Scheme 的离线 HTML 应用容器。我们要构建一个 Qt 应用，它从 Qt 资源系统中加载 HTML/CSS/JS 文件，通过自定义 URL Scheme `app://` 提供给 WebEngine。

应用中加载一个简单的 HTML 页面（打包在 qrc 中），页面使用 `app://` 协议引用 CSS 和 JS 文件。页面中有一个按钮，点击后通过 WebChannel 调用 C++ 方法读取系统信息并显示在页面上。实现请求拦截器，给所有 `app://` 请求注入 `X-App-Mode: offline` 头。使用 off-the-record Profile，确保关闭应用后不留下浏览数据。

完成标准是页面能通过 `app://` 协议正确加载所有资源（HTML、CSS、JS、图片）、WebChannel 双向通信正常工作、拦截器正确注入自定义头、关闭应用后无残留数据。

提示几个关键点：Scheme 注册放在 `main()` 最前面；资源文件用 Qt 资源系统（.qrc）打包；off-the-record Profile 只创建一次并复用。

## 6. 官方文档参考链接

[Qt 文档 · QWebEngineUrlSchemeHandler](https://doc.qt.io/qt-6/qwebengineurlschemehandler.html) -- 自定义 URL Scheme 处理

[Qt 文档 · QWebEngineUrlScheme](https://doc.qt.io/qt-6/qwebengineurlscheme.html) -- URL Scheme 注册

[Qt 文档 · QWebEngineProfile](https://doc.qt.io/qt-6/qwebengineprofile.html) -- Profile 管理与持久化

[Qt 文档 · QWebEngineUrlRequestInterceptor](https://doc.qt.io/qt-6/qwebengineurlrequestinterceptor.html) -- 请求拦截器

[Qt 文档 · QWebEngineUrlRequestJob](https://doc.qt.io/qt-6/qwebengineurlrequestjob.html) -- URL 请求任务（Scheme Handler 的响应对象）

---

到这里 QtWebEngine 的底层控制能力就拆完了。自定义 URL Scheme、CSP 安全策略、Profile 管理、请求拦截——这四个工具组合起来，你可以把 QtWebEngine 当作一个完全可控的浏览器引擎来用。如果你的应用需要更精细的安全控制（比如拦截 JavaScript 的 `eval`、限制 `file://` 访问），需要配合 Chromium 的命令行开关和 CSP 策略来实现。
