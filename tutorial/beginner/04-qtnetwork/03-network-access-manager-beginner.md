# 现代Qt开发教程（新手篇）4.3——HTTP 通信

## 1. 前言：为什么需要 QNetworkAccessManager

到目前为止我们讲了 TCP 和 UDP，它们是传输层的协议，属于网络编程的底层操作。但在实际项目中，你大概率不需要直接操作 Socket——你需要的是更高层的协议，比如 HTTP。获取天气数据、调用 REST API、下载文件、上传图片，这些全部走 HTTP。

如果你用过 libcurl，你一定知道它的 C API 写起来有多痛苦——手动拼接 header、手动处理 chunked encoding、手动管理 cookie、手动释放一堆资源。Qt 给我们提供了 QNetworkAccessManager，一个基于信号槽的异步 HTTP 客户端。你不需要关心底层是 HTTP/1.1 还是 HTTP/2（Qt 6 默认支持 HTTP/2 协商），不需要手动管理连接池，不需要解析 chunked 响应——Qt 在内部帮你处理了这一切。

QNetworkAccessManager 的设计哲学是「一个应用一个实例」。它内部维护了一个连接池和 cookie 存储，自动管理 keep-alive 连接和请求调度。你只需要 `new` 一个 QNetworkAccessManager，然后用它的 `get()`、`post()`、`put()`、`deleteResource()` 方法发起请求，通过信号槽接收响应。

这一篇我们把 HTTP GET/POST 请求、请求头设置、异步回调处理、下载进度追踪这些最常见的操作全部过一遍。

## 2. 环境说明

本教程基于 Qt 6.9.1，适用于 Windows 10/11（MSVC 2019+ 或 MinGW 11.2+）、Linux（GCC 11+）、WSL2 + WSLg（GUI 支持）。所有示例代码均使用 C++17 标准和 CMake 3.26+ 构建系统。本篇同样需要链接 Qt6::Network 模块。示例程序使用控制台输出，但 QNetworkAccessManager 的用法在 GUI 程序中完全一致。

## 3. 核心概念讲解

### 3.1 QNetworkAccessManager 的架构

在写代码之前，我们先搞清楚 Qt 网络访问的三个核心类之间的关系，不然后面你会对谁负责什么感到困惑。

QNetworkAccessManager 是网络访问的入口和管理器。它负责调度请求、管理连接池、处理 cookie 和缓存。你通常只需要创建一个实例（可以设为单例或者作为 QApplication 的成员），然后通过它发起所有的 HTTP 请求。

QNetworkRequest 封装了一个 HTTP 请求的所有信息：URL、请求头、SSL 配置等。它是一个值类型（value type），可以被自由拷贝。你每次发请求之前先构造一个 QNetworkRequest，设置好 URL 和 header，然后把它传给 QNetworkAccessManager。

QNetworkReply 是异步响应的句柄。当你调用 `manager->get(request)` 之后，返回的不是响应数据，而是一个 QNetworkReply 指针。这个对象会在后台异步接收数据，数据就绪后通过信号通知你。你需要在响应处理完毕后手动 `deleteLater()` 来释放它。

这三者的关系可以简化为：你构造 QNetworkRequest 描述「我要什么」，交给 QNetworkAccessManager 去发请求，它返回一个 QNetworkReply 让你「等结果」。

### 3.2 GET 请求：发起和接收

GET 是最简单的 HTTP 请求方式，用于从服务器获取资源。我们先来看一个最基本的 GET 请求：

```cpp
QNetworkAccessManager *manager = new QNetworkAccessManager(this);

QNetworkRequest request(QUrl("https://httpbin.org/get"));

QNetworkReply *reply = manager->get(request);

// 异步等待响应完成
connect(reply, &QNetworkReply::finished, [=]() {
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        qDebug() << "Response:" << data;
    } else {
        qDebug() << "Error:" << reply->errorString();
    }
    reply->deleteLater();  // 必须手动释放
});
```

这段代码做了四件事：创建 manager、构造请求、发起 GET 请求、连接 `finished` 信号处理响应。

`manager->get(request)` 是非阻塞的，调用后立刻返回一个 QNetworkReply 指针。此时响应数据还没到，但你可以通过这个指针来连接信号槽。当 HTTP 响应的全部数据都收到后（包括 body），`finished` 信号会被触发。

在 `finished` 的槽函数里，我们先检查 `reply->error()` 判断请求是否成功。注意，HTTP 的 4xx 和 5xx 状态码不算网络错误——Qt 仍然会把完整的响应数据交给你，`error()` 返回的是 `NoError`。如果你想检查 HTTP 状态码，需要用 `reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()`。

最后，`reply->deleteLater()` 是必须的。QNetworkReply 继承自 QObject，但 QNetworkAccessManager 不会自动帮你删除它。如果你忘了这行，每次请求都会泄漏一个 QNetworkReply 对象。

### 3.3 POST 请求：发送数据

POST 用于向服务器提交数据，常见于表单提交、API 调用、文件上传等场景。POST 请求的关键在于构造请求体（body）和设置正确的 Content-Type。

```cpp
QNetworkAccessManager *manager = new QNetworkAccessManager(this);

QNetworkRequest request(QUrl("https://httpbin.org/post"));

// 设置 Content-Type 为 JSON
request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

// 构造 JSON 请求体
QByteArray jsonData = R"({"name": "Qt", "version": "6.9.1"})";

QNetworkReply *reply = manager->post(request, jsonData);

connect(reply, &QNetworkReply::finished, [=]() {
    if (reply->error() == QNetworkReply::NoError) {
        qDebug() << "POST response:" << reply->readAll();
    } else {
        qDebug() << "Error:" << reply->errorString();
    }
    reply->deleteLater();
});
```

`setHeader()` 用来设置 HTTP 请求头。这里我们设置了 `ContentTypeHeader`，告诉服务器请求体是 JSON 格式。如果你发送的是表单数据，Content-Type 应该设为 `application/x-www-form-urlencoded`；如果是文件上传，应该用 `multipart/form-data`。

`manager->post()` 的第二个参数就是请求体，可以是 QByteArray 或者 QIODevice。如果你要上传一个文件，可以直接传一个 QFile 指针，Qt 会从文件中流式读取数据上传，不会一次性把整个文件加载到内存。

除了 GET 和 POST，QNetworkAccessManager 还支持 PUT（`manager->put()`）、DELETE（`manager->deleteResource()`）、HEAD（`manager->head()`）和自定义方法（`manager->sendCustomRequest()`）。它们的用法大同小异，都是构造 QNetworkRequest 然后调用对应的方法。

### 3.4 setHeader 设置请求头

HTTP 请求头在 API 调用中扮演着非常关键的角色——认证令牌、内容类型、自定义标识，全部通过 header 传递。QNetworkRequest 提供了两种设置 header 的方式。

第一种是用 Qt 预定义的 header 枚举：

```cpp
request.setHeader(QNetworkRequest::ContentTypeHeader,
                  "application/json");
request.setHeader(QNetworkRequest::ContentLengthHeader,
                  QByteArray::number(jsonData.size()));
request.setHeader(QNetworkRequest::UserAgentHeader,
                  "QtNetworkTutorial/1.0");
```

第二种是用原始的 header 名称设置任意自定义 header：

```cpp
request.setRawHeader("X-Custom-Header", "my-value");
request.setRawHeader("Authorization",
                     "Bearer your-token-here");
```

`setRawHeader()` 接受的参数是 QByteArray 而不是 QString，所以如果你有字符串需要转换，记得用 `QString::toUtf8()`。

这里有一个经常被忽略的细节：`setHeader()` 和 `setRawHeader()` 可以混合使用。但如果你对同一个 header 同时用了 `setHeader()` 和 `setRawHeader()`，后调用的那个会覆盖前面的。另外，`Content-LengthHeader` 在大多数情况下不需要你手动设置——QNetworkAccessManager 会根据请求体的大小自动计算。只有在使用 QIODevice 作为请求体且无法预知大小的时候，你才可能需要手动设置。

### 3.5 QNetworkReply::finished 异步回调

`finished` 信号是处理 HTTP 响应的核心。但我们还需要了解它的一些细节行为。

首先，`finished` 信号只会触发一次——当整个 HTTP 响应（包括 header 和 body）都接收完毕后。如果你想在数据传输过程中就逐步读取（比如流式处理大响应），应该连接 `readyRead` 信号：

```cpp
connect(reply, &QNetworkReply::readyRead, [=]() {
    QByteArray chunk = reply->readAll();
    // 逐步处理数据，而不是等到最后一次性读取
    qDebug() << "Received chunk of" << chunk.size() << "bytes";
});
```

其次，`finished` 触发后，即使有 `readyRead` 没读完的数据，你仍然可以在 `finished` 的槽函数里通过 `readAll()` 读取剩余数据。`finished` 只是通知你「数据收完了」，并不清空缓冲区。

还有一个很重要的事情：在 `finished` 的槽函数里你应该检查所有可能的错误情况，不仅仅是网络错误。完整的错误检查应该包括网络层错误、HTTP 状态码检查、以及业务层的响应内容校验。

```cpp
connect(reply, &QNetworkReply::finished, [=]() {
    // 1. 检查网络层错误
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Network error:" << reply->errorString();
        reply->deleteLater();
        return;
    }

    // 2. 检查 HTTP 状态码
    int statusCode = reply->attribute(
        QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode != 200) {
        qDebug() << "HTTP error:" << statusCode;
        QByteArray reason = reply->attribute(
            QNetworkRequest::HttpReasonPhraseAttribute).toByteArray();
        qDebug() << "Reason:" << reason;
        reply->deleteLater();
        return;
    }

    // 3. 读取响应数据
    QByteArray data = reply->readAll();
    // 4. 业务层校验...
    qDebug() << "Success:" << data.size() << "bytes received";

    reply->deleteLater();
});
```

### 3.6 downloadProgress 信号追踪下载进度

如果你下载的是一个大文件（几十 MB 甚至几个 GB），用户需要看到进度条，否则会以为程序卡死了。QNetworkReply 提供了 `downloadProgress(qint64 bytesReceived, qint64 bytesTotal)` 信号，实时汇报下载进度。

```cpp
connect(reply, &QNetworkReply::downloadProgress,
        [](qint64 bytesReceived, qint64 bytesTotal) {
    if (bytesTotal > 0) {
        double progress = static_cast<double>(bytesReceived)
                          / bytesTotal * 100.0;
        qDebug() << QString("Download progress: %1% (%2 / %3 bytes)")
                    .arg(progress, 0, 'f', 1)
                    .arg(bytesReceived)
                    .arg(bytesTotal);
    } else {
        // 服务器没有返回 Content-Length，无法计算百分比
        qDebug() << "Downloaded" << bytesReceived << "bytes (unknown total)";
    }
});
```

`bytesTotal` 的值来自 HTTP 响应头的 `Content-Length`。如果服务器没有返回这个 header（比如使用了 chunked transfer encoding），`bytesTotal` 会是 -1 或者 0。这时候你只能显示已下载的字节数，无法计算百分比。

另外，`downloadProgress` 信号的触发频率取决于网络速度和 Qt 的内部缓冲策略。如果下载速度很快，这个信号可能每秒触发多次。如果你在 GUI 线程的槽函数里更新进度条，要注意不要在每次触发时都做太重的 UI 操作——可以考虑用定时器节流，比如每 200ms 更新一次界面。

如果是上传文件，对应的信号是 `uploadProgress(qint64 bytesSent, qint64 bytesTotal)`，用法完全一样。

## 4. 踩坑预防清单

关于 QNetworkAccessManager 的生命周期问题：它是一个比较重的对象，内部维护了连接池和线程，所以不要频繁创建和销毁。最佳实践是在整个应用生命周期内只维护一个实例。如果它在栈上创建（比如作为 main 函数的局部变量），确保它的事件循环在它析构之前已经处理完所有 pending 的 reply。

关于 SSL 错误：如果你访问 HTTPS 地址遇到 SSL 相关的错误，首先检查你的 Qt 是否编译了 SSL 支持。Qt 默认使用系统的 SSL 库（Windows 上是 Schannel，Linux 上是 OpenSSL）。如果你的 Qt 没有编译 SSL 支持，所有 HTTPS 请求都会失败。可以用 `QSslSocket::supportsSsl()` 检查。在开发阶段，你可以通过连接 `sslErrors` 信号并调用 `reply->ignoreSslErrors()` 来跳过 SSL 验证，但千万不要在生产环境这么做。

关于重定向：HTTP 301/302 重定向在 Qt 中默认是自动跟随的。你可以通过 `reply->attribute(QNetworkRequest::RedirectionTargetAttribute)` 获取重定向目标 URL。如果你不想自动跟随重定向，可以在 QNetworkRequest 上设置 `QNetworkRequest::RedirectPolicyAttribute` 来控制行为。

关于超时：QNetworkAccessManager 没有内置的请求超时机制。如果一个请求迟迟没有响应，它永远不会触发 `finished` 信号。在生产代码中，你应该用 QTimer 自己实现超时逻辑：启动请求的同时启动一个定时器，定时器超时后调用 `reply->abort()` 来强制结束请求。

现在我们来做一个小的代码填空练习，检验一下前面的内容。补全以下代码，实现一个带进度追踪的文件下载：

```cpp
QNetworkAccessManager *manager = new QNetworkAccessManager(this);
QNetworkRequest request(QUrl("https://example.com/file.zip"));

QNetworkReply *reply = manager->__________(request);

connect(reply, &QNetworkReply::_________________,
        [](qint64 received, qint64 total) {
    qDebug() << "Progress:" << received << "/" << total;
});

connect(reply, &QNetworkReply::__________, [=]() {
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        qDebug() << "Downloaded" << data.size() << "bytes";
    }
    reply->__________ __________();
});
```

提示：需要填入的分别是 `get`、`downloadProgress`、`finished`、`deleteLater`。

参考答案如下：

```cpp
QNetworkAccessManager *manager = new QNetworkAccessManager(this);
QNetworkRequest request(QUrl("https://example.com/file.zip"));

QNetworkReply *reply = manager->get(request);

connect(reply, &QNetworkReply::downloadProgress,
        [](qint64 received, qint64 total) {
    qDebug() << "Progress:" << received << "/" << total;
});

connect(reply, &QNetworkReply::finished, [=]() {
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        qDebug() << "Downloaded" << data.size() << "bytes";
    }
    reply->deleteLater();
});
```

## 5. 练习项目

我们来做一个简单的 HTTP 文件下载器，把这一节学的东西串起来。程序接受一个 URL 作为命令行参数，下载对应的文件并保存到本地，同时显示下载进度。

完成标准：支持命令行参数指定 URL 和输出文件名，下载过程中每秒显示进度百分比和速度（KB/s），支持断点续传（通过 Range 请求头），超时 30 秒自动取消。

提示几个方向：用 `QNetworkAccessManager::get()` 发起请求，连接 `downloadProgress` 信号计算速度和百分比，连接 `readyRead` 信号将数据逐步写入 QFile 而不是等到最后一次性写入（避免大文件占满内存）。断点续传需要检查本地已有文件的大小，然后在请求中设置 `Range` header：`request.setRawHeader("Range", "bytes=已下载大小-")`。

再看一个调试挑战：以下 HTTP 下载代码有什么问题？

```cpp
QNetworkAccessManager manager;
QNetworkRequest request(QUrl("https://example.com/bigfile.zip"));
QNetworkReply *reply = manager.get(request);

connect(reply, &QNetworkReply::finished, [=]() {
    QFile file("bigfile.zip");
    file.open(QIODevice::WriteOnly);
    file.write(reply->readAll());
    file.close();
    reply->deleteLater();
});
```

这里面有几个隐患。首先，`readAll()` 会把整个文件内容加载到内存，如果文件是 1GB，内存直接爆炸。正确的做法是连接 `readyRead` 信号，边收边写磁盘。其次，没有检查 `reply->error()`，如果请求失败（网络断开、404 等），你会写一个空文件或者错误数据到磁盘。另外，没有检查 HTTP 状态码，服务器返回 302 重定向或者 500 错误时也会当作正常数据处理。最后，QFile::open 的返回值没有检查，如果磁盘空间不足或者路径不存在，写入会静默失败。

## 6. 完整示例代码

本篇的完整示例代码在 `examples/beginner/04-qtnetwork/03-network-access-manager-beginner/` 目录下，包含一个控制台程序，演示了 HTTP GET/POST 请求、请求头设置和下载进度追踪。

## 7. 官方文档参考

- [Qt 文档 · QNetworkAccessManager 类](https://doc.qt.io/qt-6/qnetworkaccessmanager.html) -- 网络访问管理器的完整 API，包含请求调度和连接管理
- [Qt 文档 · QNetworkRequest 类](https://doc.qt.io/qt-6/qnetworkrequest.html) -- HTTP 请求的封装，包含 header 和属性设置
- [Qt 文档 · QNetworkReply 类](https://doc.qt.io/qt-6/qnetworkreply.html) -- 异步响应的句柄，包含所有信号和错误处理
- [Qt 文档 · HTTP 示例](https://doc.qt.io/qt-6/qtnetwork-http-example.html) -- Qt 官方的 HTTP 客户端示例

*（链接已验证，2026-04-23 可访问）*

---

到这里就大功告成了。QNetworkAccessManager 是 Qt 网络模块中最常用的类之一，掌握它的基本用法之后，绝大多数 HTTP 通信需求都能轻松应对。下一篇我们会聊 WebSocket，看看 Qt 如何处理双向实时通信。
