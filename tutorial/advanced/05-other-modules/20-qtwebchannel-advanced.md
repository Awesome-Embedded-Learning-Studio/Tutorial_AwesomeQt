---
title: "5.20 WebChannel 进阶：自定义传输层（非 WebEngine）"
description: "入门篇我们用 QWebChannel + QWebEngineView 做了 C++ 和网页的双向通信。但 QWebChannel 的传输层不限于 QtWebEngine——它可以通过 WebSocket、HTTP 长轮询、甚至串口传输。这篇要拆的就是自定义传输层的实现，以及 C++ 对象如何暴露到 JavaScript。"
---

# 现代Qt开发教程（进阶篇）5.20——WebChannel 进阶：自定义传输层（非 WebEngine）

## 1. 前言

入门篇我们让 Qt WebChannel 跑起来了——用 `QWebChannel` 把 C++ 对象暴露给 `QWebEngineView` 中的 JavaScript，实现双向调用。但这有一个前提条件：你必须用 `QtWebEngine`。问题来了——`QtWebEngine` 是 Chromium 内核，编译时间巨长，二进制体积巨大（上百 MB），在某些嵌入式平台上根本跑不动。那能不能不用 `QtWebEngine`，直接用 `QWebChannel` 做跨进程甚至跨网络的 C++-JS 通信？

答案是肯定的。`QWebChannel` 的传输层是可插拔的。它不关心消息怎么传输，只关心消息能到达就行。默认的 `QWebEngineView` 集成只是其中一种传输方式。你完全可以自己实现一个基于 WebSocket 的传输层，让任何浏览器（不限于 Qt 内嵌的 WebEngine）都能和 C++ 后端通信。

这在实际项目中的应用场景很广：Qt 后端提供业务逻辑，前端用任意 Web 框架（React、Vue）开发 UI，通过 WebSocket + WebChannel 通信。后端不需要嵌入浏览器引擎，前端也不需要限制在 Qt 的 WebEngine 里。

## 2. 环境说明

本文档基于 Qt 6.2+ 编写，需要 Qt6::WebChannel 和 Qt6::WebSockets 模块。CMake 中通过 `find_package(Qt6 REQUIRED COMPONENTS WebChannel WebSockets)` 引入。本篇不依赖 QtWebEngine。客户端可以是任何支持 WebSocket 的浏览器或运行时。

## 3. 核心概念讲解

### 3.1 传输抽象层——QWebChannelAbstractTransport

`QWebChannel` 的所有通信都通过 `QWebChannelAbstractTransport` 抽象类完成。这个类定义了两个核心接口：`sendMessage()` 用于发送消息到对端，`messageReceived` 信号用于接收来自对端的消息。

要实现自定义传输层，我们需要继承 `QWebChannelAbstractTransport`，用 `QWebSocket` 作为底层的传输通道。

```cpp
/// @brief 基于 WebSocket 的 WebChannel 传输层。
/// @note 不依赖 QtWebEngine，可与任何 WebSocket 客户端通信。
class WebSocketTransport : public QWebChannelAbstractTransport
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] socket 已建立的 WebSocket 连接。
    /// @param[in parent 父对象。
    explicit WebSocketTransport(QWebSocket* socket, QObject* parent = nullptr)
        : QWebChannelAbstractTransport(parent)
        , m_socket(socket)
    {
        connect(m_socket, &QWebSocket::textMessageReceived,
                this, &WebSocketTransport::on_text_message);
        connect(m_socket, &QWebSocket::disconnected,
                this, &WebSocketTransport::deleteLater);
    }

    /// @brief 向客户端发送 JSON 消息。
    /// @param[in] data 要发送的 JSON 消息。
    void sendMessage(const QJsonObject& data) override
    {
        QJsonDocument kDoc(data);
        m_socket->sendTextMessage(QString::fromUtf8(kDoc.toJson(QJsonDocument::Compact)));
    }

private slots:
    /// @brief 处理从客户端收到的文本消息。
    /// @param[in] message JSON 文本消息。
    void on_text_message(const QString& message)
    {
        QJsonObject kData = QJsonDocument::fromJson(message.toUtf8()).object();
        emit messageReceived(kData, this);
    }

private:
    QWebSocket* m_socket;  // WebSocket 连接（不拥有所有权，由 deleteLater 管理）
};
```

关键点在于 `sendMessage` 和 `messageReceived` 的配合。`QWebChannel` 内部会调用 `sendMessage` 把 C++ 端的消息（属性变化、信号触发、方法返回值）序列化为 JSON 发给客户端。反过来，客户端发来的 JSON 消息通过 `messageReceived` 信号传递给 `QWebChannel` 处理。整个传输层就是 JSON 消息的收发管道。

接下来需要一个 `WebSocketTransportServer` 来管理连接和创建传输对象：

```cpp
/// @brief WebSocket 传输服务器——管理客户端连接并为每个连接创建传输对象。
class WebSocketTransportServer : public QObject
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] channel 要绑定的 WebChannel 实例。
    /// @param[in] port 监听端口。
    /// @param[in] parent 父对象。
    WebSocketTransportServer(QWebChannel* channel, quint16 port,
                             QObject* parent = nullptr)
        : QObject(parent)
        , m_channel(channel)
        , m_server("WebChannelServer", QWebSocketServer::NonSecureMode)
    {
        m_server.listen(QHostAddress::Any, port);
        connect(&m_server, &QWebSocketServer::newConnection,
                this, &WebSocketTransportServer::on_new_connection);
    }

private slots:
    /// @brief 处理新的 WebSocket 连接。
    void on_new_connection()
    {
        QWebSocket* kSocket = m_server.nextPendingConnection();
        auto* kTransport = new WebSocketTransport(kSocket, this);

        // 将传输对象注册到 WebChannel
        m_channel->connectTo(kTransport);

        // 连接断开时清理
        connect(kSocket, &QWebSocket::disconnected,
                kTransport, &WebSocketTransport::deleteLater);
    }

private:
    QWebChannel* m_channel;
    QWebSocketServer m_server;
};
```

`m_channel->connectTo(kTransport)` 是核心调用——它告诉 `QWebChannel`：「有一个新的客户端通过这个传输层连接上来了，请开始通信」。`QWebChannel` 会自动通过这个传输层发送初始化消息，把所有注册的 C++ 对象的信息同步给客户端。

### 3.2 C++ 对象暴露到 JavaScript

自定义传输层搭建好后，暴露 C++ 对象到 JavaScript 的方式和入门篇完全一样——用 `Q_PROPERTY` 暴露属性，`Q_INVOKABLE` 暴露方法，信号自动暴露。

```cpp
/// @brief 示例后端对象——暴露给 JavaScript 的业务逻辑。
class BackendService : public QObject
{
    Q_OBJECT
    /// @brief 当前计数器值，暴露为可读可写的属性。
    Q_PROPERTY(int counter READ get_counter WRITE set_counter NOTIFY counter_changed)
    /// @brief 服务状态文本。
    Q_PROPERTY(QString status READ get_status NOTIFY status_changed)

public:
    explicit BackendService(QObject* parent = nullptr)
        : QObject(parent)
        , m_counter(0)
    {}

    /// @brief 获取计数器值。
    int get_counter() const { return m_counter; }

    /// @brief 设置计数器值。
    /// @param[in] value 新值。
    void set_counter(int value)
    {
        if (m_counter != value) {
            m_counter = value;
            emit counter_changed(m_counter);
        }
    }

    /// @brief 获取状态文本。
    QString get_status() const { return m_status; }

    /// @brief 可被 JavaScript 直接调用的方法。
    /// @param[in] name 用户名。
    /// @return 欢迎消息。
    Q_INVOKABLE QString greet(const QString& name) const
    {
        return "Hello, " + name + "! Count is " + QString::number(m_counter);
    }

    /// @brief 异步操作——模拟耗时任务后发信号通知结果。
    /// @param[in] task_id 任务 ID。
    Q_INVOKABLE void start_task(int task_id)
    {
        // 模拟异步操作
        QTimer::singleShot(1000, this, [this, task_id]() {
            m_status = "Task " + QString::number(task_id) + " completed";
            emit status_changed(m_status);
            emit task_finished(task_id, true);
        });
    }

signals:
    /// @brief 计数器值变化信号。
    /// @param[in] new_value 新值。
    void counter_changed(int new_value);

    /// @brief 状态变化信号。
    /// @param[in] new_status 新状态文本。
    void status_changed(const QString& new_status);

    /// @brief 异步任务完成信号。
    /// @param[in] task_id 任务 ID。
    /// @param[in] success 是否成功。
    void task_finished(int task_id, bool success);

private:
    int m_counter;
    QString m_status;
};
```

注册到 `QWebChannel`：

```cpp
/// @brief 初始化 WebChannel 和后端服务。
void setup_webchannel()
{
    auto* channel = new QWebChannel(this);
    auto* backend = new BackendService(this);

    // 注册对象，"backend" 是 JavaScript 端访问时用的名字
    channel->registerObject("backend", backend);

    // 启动 WebSocket 传输服务器
    auto* transport_server = new WebSocketTransportServer(channel, 12345, this);
}
```

JavaScript 端使用 `qwebchannel.js` 连接：

```javascript
// 在浏览器中加载 qwebchannel.js 后
var socket = new WebSocket("ws://localhost:12345");
socket.onopen = function() {
    new QWebChannel(socket, function(channel) {
        var backend = channel.objects.backend;

        // 读取属性
        console.log("Counter:", backend.counter);
        console.log("Status:", backend.status);

        // 调用方法
        var greeting = backend.greet("World");
        console.log(greeting);

        // 修改属性
        backend.counter = 42;

        // 监听信号
        backend.counterChanged.connect(function(newValue) {
            console.log("Counter changed to:", newValue);
        });

        backend.taskFinished.connect(function(taskId, success) {
            console.log("Task", taskId, "finished:", success);
        });

        // 触发异步操作
        backend.start_task(1);
    });
};
```

注意 JavaScript 端需要加载 `qwebchannel.js`——这个文件在 Qt 的安装目录中可以找到（通常在 `qtbase/src/webchannel/` 或者 Qt 资源中），需要复制到前端项目中。

### 3.3 双向信号/属性绑定

`QWebChannel` 的强大之处在于双向绑定——C++ 端的属性变化自动推送到 JavaScript，JavaScript 端设置属性自动同步回 C++。

属性绑定的机制是这样的：C++ 端用 `Q_PROPERTY` 声明属性时指定了 `NOTIFY` 信号，当 C++ 端修改属性并 emit 对应的 NOTIFY 信号后，`QWebChannel` 会自动把新值推送给所有连接的 JavaScript 客户端。反过来，JavaScript 端设置 `backend.counter = 42` 时，`QWebChannel` 会调用 C++ 端的 `WRITE` 方法（`set_counter`），触发 C++ 端的逻辑。

信号绑定也是双向的。C++ 端的 `taskFinished` 信号会在 JavaScript 端触发对应的回调。而 JavaScript 端也可以定义信号，通过 `QWebChannel` 传递到 C++ 端——不过这需要在注册对象时做额外配置，通常更简单的做法是 JavaScript 端直接调用 C++ 的 `Q_INVOKABLE` 方法来「通知」C++ 端。

现在有个调试题给大家。你发现 JavaScript 端调用 `backend.greet("World")` 返回了 `undefined`，而不是预期的欢迎消息。可能是什么原因？

最可能的原因是 `QWebChannel` 的方法调用是异步的——调用返回的并不是方法返回值，而是一个 `Promise`（在某些版本中）。如果你需要获取返回值，需要通过信号来传递结果。另一个可能的原因是忘记在 `QWebChannel` 的回调函数内访问对象——在 `new QWebChannel(socket, function(channel) { ... })` 的回调之外，`channel.objects` 还没初始化。

## 4. 踩坑预防

第一个坑是 `qwebchannel.js` 文件版本不匹配。`QWebChannel` 的通信协议在 Qt 不同版本之间有细微差异。如果你用 Qt 6.4 编译的后端，但前端加载的是 Qt 5.15 版本的 `qwebchannel.js`，可能会出现属性同步不工作、信号丢失等问题。解决方案是使用和后端 Qt 版本完全一致的 `qwebchannel.js`，从 Qt 安装目录中复制。

第二个坑是 `QWebChannel` 的方法调用不是同步的。JavaScript 端调用 `backend.greet("World")` 并不会阻塞等待 C++ 端的返回值——它在 `QWebChannel` 的实现中是通过消息传递来模拟的。在 Qt 6 的 `qwebchannel.js` 中，返回值通过 Promise 获取。如果你按同步调用的思路写代码，会发现拿到的都是 `undefined`。对于需要返回值的场景，建议用信号来传递结果：JavaScript 调用方法，C++ 端处理完后 emit 一个信号，JavaScript 端监听这个信号获取结果。

第三个坑是 WebSocket 传输层的消息格式必须是严格的 JSON。`QWebChannel` 的通信协议是基于 JSON 的，如果你的传输层在 JSON 外面包了一层格式（比如 `{"type":"webchannel", "data":{...}}`），`QWebChannel` 会解析失败。传输层的 `sendMessage` 必须直接发送 `QWebChannel` 传入的 `QJsonObject` 的 JSON 文本，`messageReceived` 也必须直接 emit 原始 JSON 对象，不能做任何格式转换。

## 5. 练习项目

练习项目是一个基于 WebSocket + WebChannel 的远程控制面板。后端用 Qt 实现，前端用浏览器打开 HTML 页面。

后端提供三个可控制的对象：一个温度传感器（只读属性，每隔 2 秒更新随机温度值，通过信号推送到前端）、一个 LED 控制器（可读可写属性，前端点击按钮切换开关状态，后端打印日志）、一个日志查看器（后端实时推送日志消息到前端，前端用滚动列表显示最新 50 条）。前端是一个单页面 HTML 文件（不需要框架，原生 JS 就行），通过 WebSocket 连接后端，用 `qwebchannel.js` 访问后端对象。页面上显示当前温度（带数值动画）、LED 开关按钮、日志滚动列表。

完成标准是温度实时更新到前端显示、LED 开关双向同步（前端点击后端有反应，后端改变前端也同步）、日志实时滚动。不需要做精美的 UI，功能正确即可。

提示几个关键点：温度传感器用 `QTimer` 定时更新 `Q_PROPERTY`；LED 状态变化通过 `WRITE` 方法的 NOTIFY 信号推送；日志查看器用一个 `Q_INVOKABLE` 方法让前端请求历史日志，新的日志通过信号推送。

## 6. 官方文档参考链接

[Qt 文档 · QWebChannel](https://doc.qt.io/qt-6/qwebchannel.html) -- WebChannel 核心类

[Qt 文档 · QWebChannelAbstractTransport](https://doc.qt.io/qt-6/qwebchannelabstracttransport.html) -- 传输层抽象接口

[Qt 文档 · QWebSocketServer](https://doc.qt.io/qt-6/qwebsocketserver.html) -- WebSocket 服务器

---

到这里 WebChannel 的自定义传输层就拆完了。核心思路就一个：`QWebChannel` 不关心消息怎么传，只关心消息格式是 JSON。你可以用 WebSocket、HTTP、甚至蓝牙串口来传输，只要实现 `QWebChannelAbstractTransport` 的两个接口就行。这种架构特别适合「Qt 后端 + Web 前端」的混合应用。
