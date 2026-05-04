# 现代Qt开发教程（新手篇）5.18--QtHttpServer 嵌入式 HTTP 服务器

## 1. 前言：Qt 应用里跑个 HTTP 服务，真的好用

说到 HTTP 服务器，大多数人第一反应是 Nginx、Apache、Express、Spring Boot 这些重量级方案。但如果你只是想在 Qt 应用内部提供一个 REST API 接口——比如嵌入式设备的配置页面、桌面应用的状态查询端口、局域网内的文件共享服务——拉一个 Nginx 进程或者引入一个第三方 HTTP 库就显得很重了。你需要的不是"一个完整的 Web 服务器"，而是"在 Qt 应用里注册几个路由、处理几个 GET/POST 请求、返回 JSON 数据"这种轻量级能力。

QtHttpServer 就是干这个的。它是 Qt 6 提供的嵌入式 HTTP 服务器模块，不需要任何外部依赖，直接在 Qt 应用内部启动一个监听端口的 HTTP 服务。你可以用 QHttpServer::route() 注册路由处理函数，处理 GET/POST 请求，读取请求体和请求头，返回 JSON 或 HTML 响应。整个过程用纯 Qt C++ 写，和 QCoreApplication 的事件循环无缝集成，不需要额外的线程管理。

这篇我们要做的是用 QHttpServer 搭建一个嵌入式 REST API 服务，注册 GET 和 POST 路由，处理 JSON 请求体并返回 JSON 响应，演示 QtHttpServer 作为轻量级 HTTP 服务端的核心用法。

## 2. 环境说明

本篇基于 Qt 6.9.1，需要引入 HttpServer 模块。由于示例同时会输出调试信息，还需要 Core 模块。CMake 配置如下：

```cmake
find_package(Qt6 REQUIRED COMPONENTS HttpServer)
```

QtHttpServer 在 Qt 6.4 引入，属于 Qt 附加模块（Qt Add-On）。它在 Qt Installer 中默认随标准安装包提供。底层实现基于 QTcpServer，处理 HTTP 协议的解析和响应——支持的 HTTP 特性覆盖了 HTTP/1.1 的大部分常用功能：GET/POST/PUT/DELETE 请求方法、请求头解析、请求体读取、JSON 响应、Content-Type 设置。它不支持 HTTPS（没有内置 TLS 终端）、不支持 HTTP/2、不支持 WebSocket——如果需要这些功能，应该用 QWebSocketServer 或者在外层套 Nginx 反向代理。

工具链：MSVC 2019+、GCC 11+、Clang 14+，C++17 标准，CMake 3.26+。QtHttpServer 纯 CPU 运行，无特殊硬件要求。

## 3. 核心概念讲解

### 3.1 QHttpServer::route()--路由注册

QHttpServer 的核心 API 是 route() 方法，用于注册路由处理函数。它的第一个参数是 URL 路径模式，第二个参数是 HTTP 方法（可选，默认匹配所有方法），第三个参数是处理函数（lambda 或者函数指针）。处理函数接收一个 const QHttpServerRequest& 引用作为参数，返回 QHttpServerResponse 对象作为响应。

```cpp
QHttpServer server;

// 最简单的路由——处理 GET /hello，返回纯文本
server.route(QStringLiteral("/hello"), QHttpServerRequest::Method::Get,
    []() {
        return QHttpServerResponse(QStringLiteral("Hello from QtHttpServer!"));
    });

// 带路径参数的路由——处理 GET /user/<name>
server.route(QStringLiteral("/user/<arg>"), QHttpServerRequest::Method::Get,
    [](const QString &name) {
        return QHttpServerResponse(
            QStringLiteral("Hello, %1!").arg(name));
    });
```

路径模式中的 `<arg>` 是通配符占位符——它会匹配 URL 路径中对应位置的一段，并作为参数传入处理函数。你可以用多个 `<arg>` 匹配多段路径，比如 `/api/<arg>/<arg>` 会匹配 `/api/users/123`，处理函数接收两个 QString 参数。

route() 的重载版本允许处理函数接收 QHttpServerRequest 引用作为额外参数——当你需要读取请求头、请求体、查询参数时就要用到这个版本。

QHttpServer 内部按照路由注册的顺序进行匹配——先注册的路由优先匹配。如果一个请求匹配了多个路由，只有第一个匹配的处理函数会被调用。所以注册顺序很重要——通用路由放在后面，精确路由放在前面。

### 3.2 GET/POST 请求处理与 JSON 响应

实际项目中最常见的场景是处理 JSON 请求和返回 JSON 响应。GET 请求通常用来查询数据，POST 请求用来提交数据。QHttpServerRequest 提供了 body() 方法获取请求体的原始字节数据，headers() 方法获取所有请求头的键值对。

```cpp
// GET /api/status —— 返回 JSON 格式的状态信息
server.route(QStringLiteral("/api/status"), QHttpServerRequest::Method::Get,
    []() {
        QJsonObject status;
        status[QStringLiteral("app")] = QStringLiteral("QtHttpServer Demo");
        status[QStringLiteral("version")] = QStringLiteral("1.0.0");
        status[QStringLiteral("uptime")] = QDateTime::currentDateTime().toString(Qt::ISODate);

        return QHttpServerResponse(status);  // 自动设置 Content-Type: application/json
    });

// POST /api/echo —— 接收 JSON，原样返回
server.route(QStringLiteral("/api/echo"), QHttpServerRequest::Method::Post,
    [](const QHttpServerRequest &req) {
        // 解析请求体为 JSON
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(req.body(), &err);

        if (err.error != QJsonParseError::NoError) {
            QJsonObject errObj;
            errObj[QStringLiteral("error")] = QStringLiteral("Invalid JSON");
            errObj[QStringLiteral("detail")] = err.errorString();
            return QHttpServerResponse(errObj, QHttpServerResponse::StatusCode::BadRequest);
        }

        // 构建响应：回显收到的 JSON，附加处理时间戳
        QJsonObject response = doc.object();
        response[QStringLiteral("echoed_at")] = QDateTime::currentDateTime().toString(Qt::ISODate);

        return QHttpServerResponse(response);
    });
```

QHttpServerResponse 的构造函数有多个重载版本。传入 QJsonObject 或 QJsonArray 时，它会自动序列化为 JSON 字符串并设置 Content-Type 为 application/json。传入 QString 时，Content-Type 默认是 text/plain。传入 QByteArray 时，你可以手动指定 Content-Type。

QHttpServerResponse::StatusCode 枚举覆盖了常用的 HTTP 状态码——Ok（200）、Created（201）、BadRequest（400）、NotFound（404）、InternalServerError（500）等。默认状态码是 200 Ok。

### 3.3 QHttpServerRequest--读取请求体与请求头

QHttpServerRequest 封装了客户端发来的 HTTP 请求的全部信息。除了前面用到的 body() 获取请求体之外，还有几个常用的方法：

query() 返回 QUrlQuery 对象，用于解析 URL 中的查询参数。比如请求 `/api/search?keyword=qt&page=2`，query().queryItemValue("keyword") 返回 "qt"，query().queryItemValue("page") 返回 "2"。

headers() 返回的是一个 QHttpHeaders 对象（Qt 6.5+ 引入的新类型），你可以通过它读取任意请求头的值。常见的用途包括读取 Content-Type 判断请求体格式、读取 Authorization 做简单的身份验证、读取 User-Agent 记录客户端信息。

url() 返回完整的请求 URL，method() 返回请求方法（Get/Post/Put/Delete 等），remoteAddress() 返回客户端的 IP 地址。

```cpp
// GET /api/search?keyword=xxx&page=N —— 查询参数解析
server.route(QStringLiteral("/api/search"), QHttpServerRequest::Method::Get,
    [](const QHttpServerRequest &req) {
        QString keyword = req.query().queryItemValue(QStringLiteral("keyword"));
        QString pageStr = req.query().queryItemValue(QStringLiteral("page"));

        int page = pageStr.isEmpty() ? 1 : pageStr.toInt();

        QJsonObject result;
        result[QStringLiteral("keyword")] = keyword;
        result[QStringLiteral("page")] = page;
        result[QStringLiteral("results")] = QJsonArray();

        return QHttpServerResponse(result);
    });
```

这里有一点需要注意：QHttpServerRequest 的 body() 返回的是 QByteArray——原始字节数据。对于 JSON 请求体，你需要自己用 QJsonDocument::fromJson() 解析。QtHttpServer 不会自动帮你做请求体的反序列化，这和 Express.js 的 body-parser 中间件、Spring Boot 的 @RequestBody 注解不同。好处是你可以完全控制解析过程和错误处理，坏处是多写几行代码。

### 3.4 嵌入式 REST API 的典型应用场景

QtHttpServer 最大的价值在于"嵌入式"——它不是一个独立的 Web 服务器进程，而是运行在你的 Qt 应用内部的一个组件。这意味着 HTTP 请求处理函数可以直接访问你应用内的任何 C++ 对象：读取设备传感器数据返回给前端页面、接收远程配置命令修改应用参数、提供局域网内的设备状态查询接口。

典型的应用场景包括：嵌入式设备（树莓派、工业网关）上运行的 Qt 程序通过 HTTP 暴露设备状态和控制接口；桌面应用提供一个本地 REST API 让其他程序查询其状态；局域网内的 Qt 服务程序之间通过 HTTP 通信（比 Qt Remote Objects 更简单但功能也更基础）。

性能方面，QtHttpServer 是单线程的——所有请求处理在主线程的事件循环中完成。对于低并发的嵌入式场景（十几个客户端同时连接）完全够用。如果需要高并发，可以在处理函数中把耗时操作分发到 QThreadPool 或者自定义的工作线程中，处理完成后通过信号槽把结果送回主线程构造响应。

## 4. 综合示例：嵌入式 REST API 服务

把前面的内容整合起来，我们写一个基于 QHttpServer 的 REST API 服务。它提供以下几个接口：GET /api/status 返回服务状态信息，GET /api/items 返回内存中的数据列表，POST /api/items 添加新数据项（接收 JSON），GET /api/items/search?keyword=xxx 按关键词搜索，以及一个兜底的 404 处理。服务启动后监听 8080 端口，在控制台打印访问日志。

这个示例是纯控制台程序（QCoreApplication），不需要 GUI。

CMake 配置：

```cmake
find_package(Qt6 REQUIRED COMPONENTS HttpServer)

qt_add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::HttpServer)
```

main.cpp 的完整代码：

```cpp
#include <QCoreApplication>
#include <QDateTime>
#include <QHttpServer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QTcpServer>

#include <QDebug>

/// @brief 内存中的数据存储（简单演示用）
/// 生产环境应该用数据库
static QJsonArray kItems = {};

/// @brief 生成带时间戳的 JSON 响应
static QHttpServerResponse makeJsonResponse(const QJsonObject &data,
                                            QHttpServerResponse::StatusCode code
                                            = QHttpServerResponse::StatusCode::Ok)
{
    return QHttpServerResponse(data, code);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "QtHttpServer 嵌入式 REST API 服务示例";
    qDebug() << "本示例演示 QHttpServer::route() + GET/POST + JSON";

    // 初始化一些演示数据
    {
        QJsonObject item1;
        item1[QStringLiteral("id")] = 1;
        item1[QStringLiteral("name")] = QStringLiteral("QtQuick3D");
        item1[QStringLiteral("category")] = QStringLiteral("3D");
        kItems.append(item1);

        QJsonObject item2;
        item2[QStringLiteral("id")] = 2;
        item2[QStringLiteral("name")] = QStringLiteral("QtPdf");
        item2[QStringLiteral("category")] = QStringLiteral("Document");
        kItems.append(item2);

        QJsonObject item3;
        item3[QStringLiteral("id")] = 3;
        item3[QStringLiteral("name")] = QStringLiteral("QtHttpServer");
        item3[QStringLiteral("category")] = QStringLiteral("Network");
        kItems.append(item3);
    }

    QHttpServer server;

    // GET /api/status —— 服务状态
    server.route(QStringLiteral("/api/status"), QHttpServerRequest::Method::Get,
        [](const QHttpServerRequest &req) {
            qDebug() << "[GET /api/status]"
                     << "from:" << req.remoteAddress().toString();

            QJsonObject status;
            status[QStringLiteral("service")] = QStringLiteral("Qt REST API Demo");
            status[QStringLiteral("version")] = QStringLiteral("1.0.0");
            status[QStringLiteral("status")] = QStringLiteral("running");
            status[QStringLiteral("item_count")] = kItems.size();
            status[QStringLiteral("timestamp")]
                = QDateTime::currentDateTime().toString(Qt::ISODate);

            return makeJsonResponse(status);
        });

    // GET /api/items —— 获取全部数据
    server.route(QStringLiteral("/api/items"), QHttpServerRequest::Method::Get,
        [](const QHttpServerRequest &req) {
            qDebug() << "[GET /api/items]"
                     << "from:" << req.remoteAddress().toString();

            QJsonObject result;
            result[QStringLiteral("items")] = kItems;
            result[QStringLiteral("total")] = kItems.size();

            return makeJsonResponse(result);
        });

    // POST /api/items —— 添加新数据项
    server.route(QStringLiteral("/api/items"), QHttpServerRequest::Method::Post,
        [](const QHttpServerRequest &req) {
            qDebug() << "[POST /api/items]"
                     << "from:" << req.remoteAddress().toString()
                     << "body size:" << req.body().size();

            // 解析请求体 JSON
            QJsonParseError parseErr;
            QJsonDocument doc = QJsonDocument::fromJson(req.body(), &parseErr);

            if (parseErr.error != QJsonParseError::NoError) {
                qDebug() << "  JSON 解析失败:" << parseErr.errorString();
                QJsonObject errObj;
                errObj[QStringLiteral("error")] = QStringLiteral("Invalid JSON");
                errObj[QStringLiteral("detail")] = parseErr.errorString();
                return makeJsonResponse(errObj,
                    QHttpServerResponse::StatusCode::BadRequest);
            }

            QJsonObject newItem = doc.object();

            // 校验必需字段
            if (!newItem.contains(QStringLiteral("name"))
                || !newItem.contains(QStringLiteral("category"))) {
                QJsonObject errObj;
                errObj[QStringLiteral("error")]
                    = QStringLiteral("Missing required fields: name, category");
                return makeJsonResponse(errObj,
                    QHttpServerResponse::StatusCode::BadRequest);
            }

            // 分配 ID 并存储
            int newId = kItems.size() + 1;
            newItem[QStringLiteral("id")] = newId;
            kItems.append(newItem);

            qDebug() << "  新增数据项 id:" << newId
                     << "name:" << newItem[QStringLiteral("name")].toString();

            QJsonObject response;
            response[QStringLiteral("message")] = QStringLiteral("Item created");
            response[QStringLiteral("item")] = newItem;

            return makeJsonResponse(response,
                QHttpServerResponse::StatusCode::Created);
        });

    // GET /api/items/search?keyword=xxx —— 按关键词搜索
    server.route(QStringLiteral("/api/items/search"),
        QHttpServerRequest::Method::Get,
        [](const QHttpServerRequest &req) {
            QString keyword = req.query().queryItemValue(
                QStringLiteral("keyword"));

            qDebug() << "[GET /api/items/search]"
                     << "keyword:" << keyword
                     << "from:" << req.remoteAddress().toString();

            QJsonArray results;
            for (const QJsonValue &val : kItems) {
                QJsonObject item = val.toObject();
                QString name = item[QStringLiteral("name")].toString();
                QString cat = item[QStringLiteral("category")].toString();

                // 在 name 和 category 中搜索关键词（不区分大小写）
                if (name.contains(keyword, Qt::CaseInsensitive)
                    || cat.contains(keyword, Qt::CaseInsensitive)) {
                    results.append(item);
                }
            }

            QJsonObject response;
            response[QStringLiteral("keyword")] = keyword;
            response[QStringLiteral("results")] = results;
            response[QStringLiteral("count")] = results.size();

            return makeJsonResponse(response);
        });

    // 兜底路由——未匹配的路径返回 404
    server.afterRequest(
        [](QHttpServerResponse &&resp) {
            // 全局响应后处理（可以加 CORS 头等）
            resp.addHeader(QStringLiteral("X-Powered-By"),
                           QStringLiteral("QtHttpServer"));
            return std::move(resp);
        });

    // 绑定 TCP 服务器并监听端口
    auto tcpServer = std::make_unique<QTcpServer>();
    if (!tcpServer->listen(QHostAddress::Any, 8080)) {
        qCritical() << "无法监听端口 8080:" << tcpServer->serverError();
        return -1;
    }

    server.bind(tcpServer.get());

    qDebug() << "HTTP 服务器已启动";
    qDebug() << "监听地址:" << tcpServer->serverAddress().toString();
    qDebug() << "监听端口:" << tcpServer->serverPort();
    qDebug() << "";
    qDebug() << "可用路由:";
    qDebug() << "  GET  /api/status              -- 服务状态";
    qDebug() << "  GET  /api/items               -- 获取全部数据";
    qDebug() << "  POST /api/items               -- 添加新数据项";
    qDebug() << "  GET  /api/items/search?keyword=xxx -- 按关键词搜索";
    qDebug() << "";
    qDebug() << "测试命令:";
    qDebug() << "  curl http://localhost:8080/api/status";
    qDebug() << "  curl http://localhost:8080/api/items";
    qDebug() << "  curl -X POST http://localhost:8080/api/items"
                " -H 'Content-Type: application/json'"
                " -d '{\"name\":\"QtNetwork\",\"category\":\"Network\"}'";
    qDebug() << "  curl 'http://localhost:8080/api/items/search?keyword=Qt'";

    return app.exec();
}
```

运行程序后，控制台会打印出所有可用路由和测试命令。然后在终端用 curl 测试各个接口：

`curl http://localhost:8080/api/status` 返回服务状态 JSON，包含服务名称、版本号、数据项数量和时间戳。

`curl http://localhost:8080/api/items` 返回内存中的全部数据项列表。

`curl -X POST http://localhost:8080/api/items -H 'Content-Type: application/json' -d '{"name":"QtNetwork","category":"Network"}'` 提交一条新数据，服务端自动分配 ID 并返回创建成功的响应，状态码 201。再次 GET /api/items 可以看到新增的数据项。

`curl 'http://localhost:8080/api/items/search?keyword=Qt'` 搜索 name 或 category 中包含 "Qt" 的数据项，返回匹配结果。

几个值得注意的实现细节。kItems 用了一个全局的 QJsonArray 做内存存储，纯粹是为了演示——生产环境一定要用数据库或者至少用持久化文件存储。afterRequest() 是一个全局响应后处理钩子，这里用它给所有响应加了一个 X-Powered-By 头，实际项目中可以在这里统一添加 CORS 头、日志记录、响应压缩等中间件逻辑。QTcpServer 的 listen() 调用可能会失败（端口被占用、权限不够），一定要检查返回值。

## 5. 练习项目

练习项目：带文件上传的 REST API 服务。

在基础 REST API 上增加文件上传和下载功能：POST /api/upload 接收 multipart/form-data 格式的文件上传（QHttpServerRequest::body() 包含整个 multipart 数据，需要手动解析或者借助 QHttpMultiPart），文件保存到本地目录；GET /api/files 返回已上传文件的列表（文件名、大小、上传时间）；GET /api/files/<name> 下载指定文件（返回文件内容，Content-Type 设为 application/octet-stream）。

完成标准是这样的：上传一个 PNG 图片后 GET /api/files 能在列表中看到这个文件（包含文件名、字节数大小、上传时间），GET /api/files/<filename> 能下载到原始文件（md5 校验一致）。服务端在保存文件时自动创建上传目录（如果不存在），文件名冲突时自动追加序号。

几个实现提示：QHttpServer 不提供 multipart 解析的便捷 API，你需要从 request.body() 中手动解析 boundary 和 part，或者简单处理——直接把整个 body 作为文件内容保存（适合单文件上传场景）。文件大小建议加个上限校验（比如 10MB），不然有人上传一个 1GB 的文件直接把内存吃满。下载文件时用 QHttpServerResponse(QByteArray, QHttpServerResponse::StatusCode::Ok) 返回文件内容，通过 addHeader("Content-Disposition", "attachment; filename=xxx") 触发浏览器下载。

## 6. 官方文档参考

[Qt 文档 · QtHttpServer 模块](https://doc.qt.io/qt-6/qthttpserver-index.html) -- QtHttpServer 模块总览

[Qt 文档 · QHttpServer](https://doc.qt.io/qt-6/qhttpserver.html) -- HTTP 服务器类

[Qt 文档 · QHttpServerResponse](https://doc.qt.io/qt-6/qhttpserverresponse.html) -- HTTP 响应构造

[Qt 文档 · QHttpServerRequest](https://doc.qt.io/qt-6/qhttpserverrequest.html) -- HTTP 请求对象

*(链接已验证，2026-04-23 可访问)*

---

到这里就大功告成了。QtHttpServer 把嵌入式 HTTP 服务的能力带到了 Qt 应用中——QHttpServer::route() 注册路由处理函数，QHttpServerRequest 读取请求体、请求头和查询参数，QHttpServerResponse 构造 JSON 或纯文本响应返回给客户端。整个 HTTP 服务运行在 Qt 事件循环中，不需要额外的线程管理或第三方库。对于嵌入式设备配置页面、局域网设备状态查询、应用内 REST API 这些场景，QtHttpServer 的轻量级方案比拉一个 Nginx 进程或者引入 Node.js/Spring Boot 不知简洁了多少倍。
