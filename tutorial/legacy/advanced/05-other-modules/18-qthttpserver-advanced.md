---
title: "5.18 HttpServer 进阶：中间件链、静态文件服务、身份验证"
description: "入门篇我们用 QHttpServer 写了基本的路由：GET 返回 JSON，POST 处理表单。但真正的后端服务远不止这些——需要中间件模式统一处理鉴权和日志、需要托管静态文件、需要 JWT 身份验证、需要处理 CORS。这篇把这些工程化能力逐个拆开。"
---

# 现代Qt开发教程（进阶篇）5.18——HttpServer 进阶：中间件链、静态文件服务、身份验证

## 1. 前言

入门篇我们把 `QHttpServer` 的路由机制跑通了——注册几个 handler，GET 返回 JSON，POST 处理请求体。但如果你真的打算用 `QHttpServer` 做后端服务（比如给嵌入式设备提供 REST API、给桌面应用提供本地调试接口），马上就会遇到一堆工程化问题。

所有接口都需要鉴权，总不能每个 handler 里都写一遍 token 校验吧？前端页面和 API 放在同一个服务里，静态文件怎么托管？前端跑在不同端口需要跨域，CORS 头怎么统一注入？这些问题的答案是中间件模式和请求拦截。

`QHttpServer` 的设计比较轻量，它不像 Express.js 或 Django 那样有完整的中间件框架。但它提供了 `afterRequest` 钩子和灵活的路由匹配，足以让我们构建出中间件链的效果。这篇我们就用 Qt 的工具来搭一套「够用」的中间件体系。

## 2. 环境说明

本文档基于 Qt 6.5+ 编写，需要 Qt6::HttpServer 模块。CMake 中通过 `find_package(Qt6 REQUIRED COMPONENTS HttpServer)` 引入。`QHttpServer` 底层依赖 `QTcpServer`，不需要额外的第三方库。本篇同时用到 `QJsonDocument`、`QJwt`（如果 Qt 版本支持）或手动 JWT 解析。

## 3. 核心概念讲解

### 3.1 路由匹配与 afterRequest 中间件模式

`QHttpServer` 的路由注册使用 `route()` 方法，支持路径模板和 HTTP 方法匹配。我们先回顾一下基本路由，然后看怎么用 `afterRequest` 实现中间件。

```cpp
/// @brief 注册基本 API 路由。
/// @param[in] server HTTP 服务器实例。
void register_api_routes(QHttpServer* server)
{
    // GET /api/users — 获取用户列表
    server->route("/api/users", QHttpServerRequest::Method::Get,
        [](const QHttpServerRequest& request) {
            QJsonArray kUsers = fetch_user_list();
            return QHttpServerResponse(kUsers, QHttpServerResponse::StatusCode::Ok);
        });

    // POST /api/users — 创建用户
    server->route("/api/users", QHttpServerRequest::Method::Post,
        [](const QHttpServerRequest& request) {
            QJsonObject kBody = QJsonDocument::fromJson(request.body()).object();
            QString kName = kBody["name"].toString();
            // ... 创建用户逻辑
            return QHttpServerResponse(QHttpServerResponse::StatusCode::Created);
        });

    // 带路径参数的路由：GET /api/users/<id>
    server->route("/api/users/<arg>", QHttpServerRequest::Method::Get,
        [](const QString& user_id, const QHttpServerRequest& request) {
            QJsonObject kUser = fetch_user_by_id(user_id);
            if (kUser.isEmpty()) {
                return QHttpServerResponse(QHttpServerResponse::StatusCode::NotFound);
            }
            return QHttpServerResponse(kUser, QHttpServerResponse::StatusCode::Ok);
        });
}
```

`<arg>` 是路径参数占位符，Qt 会自动提取并作为回调参数传入。这很方便，但只支持单个路径段，不支持通配符或正则匹配。

接下来看 `afterRequest`——这是实现中间件的关键。`afterRequest` 在每个请求处理完成后被调用，可以修改响应对象。

```cpp
/// @brief 注册 afterRequest 中间件，统一注入 CORS 头。
/// @param[in] server HTTP 服务器实例。
void setup_cors_middleware(QHttpServer* server)
{
    server->afterRequest(
        [](QHttpServerResponse&& response) -> QHttpServerResponse {
            // 统一注入 CORS 允许头
            response.addHeader("Access-Control-Allow-Origin", "*");
            response.addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            response.addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
            response.addHeader("Access-Control-Max-Age", "86400");
            return std::move(response);
        });
}
```

`afterRequest` 接收即将发送的响应，你可以修改它的 header、状态码甚至替换整个响应体。这里我们用它来注入 CORS 头——所有经过这个服务器的响应都会自动带上跨域允许头，不用在每个 handler 里重复写。

`afterRequest` 可以注册多个，它们按注册顺序依次执行。这让我们可以把不同关注点拆成独立的中间件：一个处理 CORS，一个记录日志，一个注入安全头。

```cpp
/// @brief 注册日志中间件。
void setup_logging_middleware(QHttpServer* server)
{
    server->afterRequest(
        [](QHttpServerResponse&& response) -> QHttpServerResponse {
            // 这里拿不到 request 对象...
            // afterRequest 只接收 response，如果需要日志得用其他方式
            return std::move(response);
        });
}
```

等一下，这里有个问题——`afterRequest` 的回调只接收 `QHttpServerResponse`，拿不到原始请求。这意味着你不能在 `afterRequest` 中根据请求路径做条件处理。这是 `QHttpServer` 当前设计的一个局限。如果需要「请求前拦截」（比如鉴权），需要用另一种方式——在路由 handler 的前面加一层检查函数。

### 3.2 静态文件服务

`QHttpServer` 没有内置的静态文件服务功能（不像 Express.js 的 `express.static()`），但我们可以自己实现。核心是读取请求路径，映射到文件系统路径，然后返回文件内容。

```cpp
/// @brief 注册静态文件服务路由。
/// @param[in] server HTTP 服务器实例。
/// @param[in] doc_root 静态文件根目录。
void serve_static_files(QHttpServer* server, const QString& doc_root)
{
    // 匹配所有 GET 请求（作为 fallback 路由，放在其他路由之后注册）
    server->route("/", QHttpServerRequest::Method::Get,
        [doc_root](const QHttpServerRequest& request) -> QHttpServerResponse {
            QString kPath = request.url().path();

            // 根路径映射到 index.html
            if (kPath == "/") {
                kPath = "/index.html";
            }

            // 安全检查：防止路径遍历攻击
            QString kFilePath = doc_root + kPath;
            QFileInfo kFileInfo(kFilePath);
            if (!kFileInfo.canonicalFilePath().startsWith(QDir(doc_root).canonicalPath())) {
                return QHttpServerResponse(QHttpServerResponse::StatusCode::Forbidden);
            }

            QFile kFile(kFilePath);
            if (!kFile.open(QIODevice::ReadOnly)) {
                return QHttpServerResponse(QHttpServerResponse::StatusCode::NotFound);
            }

            QByteArray kData = kFile.readAll();
            QString kMimeType = guess_mime_type(kFilePath);

            QHttpServerResponse response(kData, QHttpServerResponse::StatusCode::Ok);
            response.addHeader("Content-Type", kMimeType.toUtf8());
            return response;
        });
}

/// @brief 根据文件扩展名猜测 MIME 类型。
/// @param[in] path 文件路径。
/// @return MIME 类型字符串。
QString guess_mime_type(const QString& path)
{
    if (path.endsWith(".html")) return "text/html; charset=utf-8";
    if (path.endsWith(".css"))  return "text/css; charset=utf-8";
    if (path.endsWith(".js"))   return "application/javascript; charset=utf-8";
    if (path.endsWith(".json")) return "application/json; charset=utf-8";
    if (path.endsWith(".png"))  return "image/png";
    if (path.endsWith(".jpg") || path.endsWith(".jpeg")) return "image/jpeg";
    if (path.endsWith(".svg"))  return "image/svg+xml";
    if (path.endsWith(".ico"))  return "image/x-icon";
    return "application/octet-stream";
}
```

这段代码中有两个关键点。第一个是路径遍历攻击防护——用户可能请求 `/../../../etc/passwd`，如果直接拼接路径就会泄露系统文件。我们用 `QFileInfo::canonicalFilePath()` 获取规范化后的绝对路径，然后检查它是否在 `doc_root` 下面。第二个是 MIME 类型猜测——浏览器根据 `Content-Type` 头来决定怎么处理响应内容。

静态文件服务在处理大文件时应该用流式响应而不是一次性读取整个文件到内存。不过 `QHttpServer` 的响应模型是一次性返回 `QByteArray`，不支持分块传输。对于大文件场景（视频、大 PDF），可能需要直接用 `QTcpServer` 手动实现 HTTP 响应，或者用 nginx 做前端代理来托管静态文件。

### 3.3 JWT/Bearer Token 验证中间件

因为 `afterRequest` 拿不到请求对象，鉴权中间件不能用 `afterRequest` 实现。我们需要用「包装函数」模式——把每个需要鉴权的 handler 包装在一个检查 token 的函数里。

```cpp
/// @brief 验证 JWT Token 的有效性。
/// @param[in] token JWT Token 字符串。
/// @return 解析成功返回 true，并将 payload 存入 out_payload。
bool verify_jwt_token(const QString& token, QJsonObject& out_payload)
{
    // 简化版 JWT 验证——生产环境请用成熟的 JWT 库
    QStringList kParts = token.split('.');
    if (kParts.size() != 3) {
        return false;
    }

    // 验证签名（这里省略 HMAC-SHA256 签名验证的实现）
    // ...

    // 解析 payload
    QByteArray kPayload = QByteArray::fromBase64(
        kParts[1].toUtf8(), QByteArray::Base64UrlEncoding);
    out_payload = QJsonDocument::fromJson(kPayload).object();

    // 检查过期时间
    qint64 kExp = out_payload["exp"].toInteger();
    if (QDateTime::currentSecsSinceEpoch() > kExp) {
        return false;
    }

    return true;
}

/// @brief 鉴权包装器——检查请求中的 Bearer Token。
/// @param[in] request HTTP 请求。
/// @param[in] handler 实际业务处理函数。
/// @return HTTP 响应。
using ApiHandler = std::function<QHttpServerResponse(const QHttpServerRequest&)>;

QHttpServerResponse with_auth(const QHttpServerRequest& request, ApiHandler handler)
{
    QByteArray kAuthHeader = request.value("Authorization");
    if (!kAuthHeader.startsWith("Bearer ")) {
        return QHttpServerResponse(
            QJsonDocument::fromJson(R"({"error":"missing token"})").object(),
            QHttpServerResponse::StatusCode::Unauthorized);
    }

    QString kToken = QString::fromUtf8(kAuthHeader.mid(7));
    QJsonObject kPayload;
    if (!verify_jwt_token(kToken, kPayload)) {
        return QHttpServerResponse(
            QJsonDocument::fromJson(R"({"error":"invalid token"})").object(),
            QHttpServerResponse::StatusCode::Unauthorized);
    }

    // Token 有效，执行实际 handler
    return handler(request);
}
```

使用的时候，把需要鉴权的路由 handler 包装在 `with_auth` 中：

```cpp
server->route("/api/protected", QHttpServerRequest::Method::Get,
    [](const QHttpServerRequest& request) {
        return with_auth(request, [](const QHttpServerRequest& req) {
            QJsonObject kData;
            kData["message"] = "This is protected data";
            return QHttpServerResponse(kData, QHttpServerResponse::StatusCode::Ok);
        });
    });
```

这种模式虽然不如 Express.js 的中间件链优雅，但功能等价。你可以把 `with_auth` 看作一个高阶函数，它在调用实际 handler 之前拦截检查 token。如果你想提取 payload 中的用户信息传给 handler，可以把 `kPayload` 作为参数传递。

现在有一个思考题：如果某个路由既需要鉴权又需要 CORS 头，它们的执行顺序应该是什么？

答案是 CORS 中间件应该在鉴权之前执行（通过 `afterRequest` 注入 CORS 头，`afterRequest` 在响应返回前执行）。同时 OPTIONS 预检请求不需要鉴权（浏览器自动发送的，不带 token），所以需要单独注册一个 OPTIONS handler 返回 200。

### 3.4 CORS 预检请求处理

跨域请求中，浏览器会先发一个 OPTIONS 预检请求，询问服务器是否允许跨域。如果服务器不正确响应 OPTIONS 请求，前端的所有跨域 API 调用都会失败。

```cpp
/// @brief 处理 CORS 预检请求。
/// @param[in] server HTTP 服务器实例。
void handle_cors_preflight(QHttpServer* server)
{
    server->route("/<arg>", QHttpServerRequest::Method::Options,
        [](const QHttpServerRequest& request) {
            QHttpServerResponse response(QHttpServerResponse::StatusCode::NoContent);
            response.addHeader("Access-Control-Allow-Origin", "*");
            response.addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            response.addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
            response.addHeader("Access-Control-Max-Age", "86400");
            return response;
        });
}
```

这个 OPTIONS handler 需要在其他路由之前注册，确保所有路径的 OPTIONS 请求都能被捕获。注意路径用 `"/<arg>"` 来匹配所有子路径。

## 4. 踩坑预防

第一个坑是路径遍历攻击。前面代码里已经演示了防御方法，但这个问题值得再强调一遍。如果你用 `QHttpServer` 托管静态文件，永远不要直接把请求路径拼接到文件系统路径上。`QFile(docRoot + "/../../etc/passwd")` 这种路径在 Linux 上会成功打开文件。必须用 `QFileInfo::canonicalFilePath()` 做规范化后检查路径前缀。后果是攻击者可以读取服务器上的任意文件，包括配置文件、密钥文件等。

第二个坑是 JWT 验证不检查签名。很多教程示例只解析 payload 不验证签名，这在生产环境中等于没有鉴权——任何人都可以伪造 token。JWT 的签名验证需要用 HMAC-SHA256（或 RS256）对 header + payload 部分重新计算签名，然后和 token 中的签名部分比对。建议使用成熟的 JWT 库（如 `QJwt` 或第三方库），不要自己手写签名验证。

第三个坑是 `QHttpServer` 的路由匹配是按注册顺序的，先匹配到的先执行。如果你把静态文件的通配路由放在 API 路由前面注册，所有 API 请求都会被静态文件处理吞掉。解决方案是先注册具体路径（API 路由），最后注册通配路由（静态文件 fallback）。

## 5. 练习项目

练习项目是一个带身份验证的 REST API 服务器。我们要构建一个用户管理 API，包含登录、获取用户列表、创建用户三个接口。

登录接口 `POST /api/login` 接收用户名和密码，验证成功后返回 JWT Token。获取用户列表 `GET /api/users` 需要 Bearer Token 验证，返回用户列表 JSON。创建用户 `POST /api/users` 同样需要 Token 验证。所有接口都要正确处理 CORS 预检请求。额外要求是添加一个日志中间件，在每个请求处理完成后打印请求方法、路径、状态码和耗时。

完成标准是登录成功返回有效 Token、Token 过期后请求被拒绝、CORS 预检正确响应、日志正确记录。不需要连接真实数据库，用内存中的 `QMap` 存储用户数据即可。

提示几个关键点：密码不要明文存储，用 `QCryptographicHash::Sha256` 做哈希；JWT 的过期时间建议设为 1 小时；日志中间件可以用 `QElapsedTimer` 计算耗时。

## 6. 官方文档参考链接

[Qt 文档 · QHttpServer](https://doc.qt.io/qt-6/qhttpserver.html) -- HTTP 服务器核心类

[Qt 文档 · QHttpServerResponse](https://doc.qt.io/qt-6/qhttpserverresponse.html) -- HTTP 响应构建

[Qt 文档 · QHttpServerRequest](https://doc.qt.io/qt-6/qhttpserverrequest.html) -- HTTP 请求对象

[Qt 文档 · QCryptographicHash](https://doc.qt.io/qt-6/qcryptographichash.html) -- 哈希计算（密码哈希）

---

到这里 `QHttpServer` 的工程化用法就拆完了。虽然它不如专业 Web 框架那么完善，但对于嵌入式设备的 REST API、桌面应用的本地调试接口来说，已经足够了。中间件模式、JWT 鉴权、CORS 处理——这三板斧拿捏住，大部分后端需求都能搞定。如果项目规模继续增长，建议迁移到更成熟的 HTTP 框架（比如 Crow、cpp-httplib）或者直接用 nginx 做反向代理。
