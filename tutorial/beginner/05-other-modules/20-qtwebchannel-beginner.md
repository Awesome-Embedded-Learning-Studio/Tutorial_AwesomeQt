# 现代Qt开发教程（新手篇）5.20--QtWebChannel Web 与 Qt 双向互通

## 1. 前言：让浏览器里的 JavaScript 直接调用你的 C++ 对象

上一篇我们用 QWebSocketServer 搭了个聊天室，客户端用 JavaScript 通过 WebSocket 协议和服务端通信。那套方案没问题，但本质上你还是在手动处理消息格式——服务端收到一条 JSON 字符串，解析它，分发到对应的处理函数，再把结果序列化回 JSON 发回去。这个模式写多了你会发现，大量代码都花在"消息路由和序列化"这个胶水层上，真正有价值的业务逻辑反而被淹没了。

QtWebChannel 要解决的就是这个问题。它提供了一种机制，让你把一个 C++ QObject 直接"暴露"给 JavaScript 环境——JavaScript 侧可以直接调用这个 QObject 的属性和方法，可以连接它的信号，就像操作一个本地 JS 对象一样。底层传输可以是 WebSocket，也可以是 QWebEngineView 的内部通信通道，但你的代码完全不需要关心这些细节。

这种能力在什么场景下特别有用？如果你在 Qt 应用里内嵌了一个 QWebEngineView 作为前端界面（比如用 HTML/CSS/JS 写了一个漂亮的仪表盘或者配置面板），你需要让 JS 前端和 C++ 后端双向交互——前端读取设备状态、调用 C++ 方法执行操作、实时接收后端的状态变化通知。QtWebChannel 就是连接这两端的桥梁。

这篇我们要做的是用 QWebChannel 把一个 C++ 后端对象暴露给 JavaScript，实现 JS 调用 Qt 方法和 Qt 发信号到 JS 的双向通信，并和 QWebEngineView 集成构建一个完整的前后端交互示例。

## 2. 环境说明

本篇基于 Qt 6.9.1，需要同时引入 WebChannel 模块和 WebEngineWidgets 模块。WebChannel 负责通信桥接，WebEngineWidgets 提供 QWebEngineView 作为前端容器。CMake 配置如下：

```cmake
find_package(Qt6 REQUIRED COMPONENTS WebChannel WebEngineWidgets)
```

QtWebChannel 从 Qt 5.4 开始提供。它的核心思路是：C++ 侧创建一个 QWebChannel 实例，注册若干 QObject 到 channel 上；JavaScript 侧加载 qwebchannel.js 库，创建一个 QWebChannel 实例连接到传输层，然后就可以访问注册的 C++ 对象了。所有属性读写、方法调用、信号连接都通过这个传输层自动完成，开发者完全不需要手动处理消息格式。

传输层有两种选择。第一种是 QWebSocket——适用于浏览器独立页面连接 Qt 后端的场景，前后端运行在不同的进程中。第二种是 QWebEngineView 的内部传输——适用于 Qt 应用内嵌 WebEngine 作为 UI 的场景，JS 运行在 WebEngine 的 Chromium 渲染进程中，和 C++ 主进程通过 QtWebChannel 的内部管道通信。本篇示例使用第二种方式，因为它是桌面 Qt 应用中最常见的集成模式。

工具链：MSVC 2019+、GCC 11+、Clang 14+，C++17 标准，CMake 3.26+。WebEngineWidgets 模块内嵌了 Chromium 渲染引擎，二进制体积较大（约 200-300MB），编译时间也较长，建议使用预编译的 Qt 安装包而不是从源码编译。WebEngine 目前不支持静态链接。

## 3. 核心概念讲解

### 3.1 QWebChannel 注册 C++ 对象

QWebChannel 的工作原理是在 C++ 侧把 QObject 注册到一个"通道"上，每个注册的对象有一个唯一的标识符字符串。JavaScript 侧连接到这个通道后，通过标识符就可以获取到对应的对象代理——代理上暴露了该 QObject 的所有 Q_PROPERTY 属性、Q_INVOKABLE 方法以及所有信号。

我们先定义一个可以被暴露的后端对象。它需要继承 QObject，使用 Q_PROPERTY 声明可从 JS 访问的属性，使用 Q_INVOKABLE 或 public slots 声明可从 JS 调用的方法：

```cpp
#include <QObject>

class BackendObject : public QObject
{
    Q_OBJECT

    // Q_PROPERTY: JS 可以读写这个属性，也可以监听它的变化信号
    Q_PROPERTY(QString serverStatus READ serverStatus NOTIFY serverStatusChanged)
    Q_PROPERTY(int requestCount READ requestCount NOTIFY requestCountChanged)

public:
    explicit BackendObject(QObject *parent = nullptr)
        : QObject(parent), m_requestCount(0)
    {
        m_serverStatus = QStringLiteral("Running");
    }

    QString serverStatus() const { return m_serverStatus; }
    int requestCount() const { return m_requestCount; }

    // Q_INVOKABLE: JS 可以直接调用这个方法
    Q_INVOKABLE QString greet(const QString &name)
    {
        m_requestCount++;
        emit requestCountChanged(m_requestCount);
        return QStringLiteral("Hello, %1! Request #%2 processed.")
            .arg(name)
            .arg(m_requestCount);
    }

    // public slots 同样可以被 JS 调用
public slots:
    void triggerAlert(const QString &message)
    {
        m_requestCount++;
        emit requestCountChanged(m_requestCount);
        emit alertRequested(message);
    }

signals:
    // 这些信号可以在 JS 侧连接回调函数
    void serverStatusChanged(const QString &status);
    void requestCountChanged(int count);
    void alertRequested(const QString &message);

private:
    QString m_serverStatus;
    int m_requestCount;
};
```

Q_PROPERTY 宏的第三个参数 NOTIFY 很重要——它指定一个信号，当属性值变化时 QWebChannel 会自动通知 JS 侧更新。如果你省略了 NOTIFY，JS 侧就只能在首次获取值时读到属性，后续的属性变化不会被推送。Q_INVOKABLE 标记的方法可以被 JS 当成普通函数调用，返回值会通过 Promise 传递回 JS。public slots 和 Q_INVOKABLE 在"可被外部调用"这一点上效果相同，选择哪种取决于你的代码风格偏好。

注册到 QWebChannel 的方式如下：

```cpp
#include <QWebChannel>

BackendObject backend;
QWebChannel channel;
channel.registerObject(QStringLiteral("backend"), &backend);
```

registerObject 的第一个参数是标识符字符串，JS 侧通过这个名字访问对象。第二个参数是 QObject 指针。注意 backend 对象的生命周期必须覆盖整个 channel 的使用周期——如果 backend 在 channel 还在使用时被销毁，会导致未定义行为。

### 3.2 qwebchannel.js 前端侧配置

JavaScript 侧需要加载 Qt 提供的 qwebchannel.js 库。这个库随 Qt 安装一起发布，通常在 Qt 安装目录的 qtwebchannel 目录下（比如 `QT_INSTALL_PREFIX/resources/qtwebchannel/qwebchannel.js`）。你需要把这个文件复制到你的项目资源中，或者在 HTML 中引用它。

如果使用 QWebEngineView 作为前端容器，你可以通过 QWebChannel 的内部传输直接通信——不需要 WebSocket，不需要额外的传输层配置。JS 侧的初始化代码非常简洁：

```html
<script src="qwebchannel.js"></script>
<script>
// new QWebChannel 的第一个参数是传输对象
// 在 QWebEngineView 内部使用 qt.webChannelTransport
new QWebChannel(qt.webChannelTransport, function(channel) {
    // 通过注册时的标识符获取 C++ 对象
    var backend = channel.objects.backend;

    // 读取属性
    console.log("Server status:", backend.serverStatus);
    console.log("Request count:", backend.requestCount);

    // 调用方法
    backend.greet("Qt Developer", function(result) {
        console.log("C++ returned:", result);
    });

    // 连接信号
    backend.alertRequested.connect(function(message) {
        console.log("Alert from C++:", message);
        alert(message);
    });

    // 监听属性变化（通过 NOTIFY 信号）
    backend.requestCountChanged.connect(function(count) {
        console.log("Request count updated:", count);
    });
});
```

这里有几个细节要搞清楚。QWebChannel 构造函数的第一个参数 qt.webChannelTransport 是 QWebEngineView 自动注入的全局对象——只有当 JS 运行在 QWebEngineView 内部时这个对象才存在。如果你是在独立浏览器中通过 WebSocket 连接，需要传入 WebSocket 对象作为传输层。

方法调用是异步的——backend.greet() 的返回值通过回调函数获取（第二个参数），而不是直接返回。这是因为 C++ 和 JS 之间隔着一个传输层（即使是 QWebEngineView 的内部管道也是进程间通信），所有跨进程调用天然是异步的。

信号的连接使用 .connect() 方法，传入一个回调函数。当 C++ 侧 emit 这个信号时，JS 侧的回调会被触发。这是从 C++ 向 JS 推送数据的主要方式——你不需要 JS 轮询属性值，只需要在 C++ 侧 emit 一个信号，JS 侧自动收到通知。

### 3.3 与 QWebEngineView 集成

把 QWebChannel 和 QWebEngineView 组合起来就形成了一个完整的前后端交互架构。C++ 侧创建 QWebEngineView 加载 HTML 页面，创建 QWebChannel 注册后端对象，然后把 channel 设置到 QWebEnginePage 上。HTML 页面中的 JS 通过 qt.webChannelTransport 连接到 channel，访问 C++ 对象。

```cpp
#include <QApplication>
#include <QWebChannel>
#include <QWebEngineView>

// 在 QApplication 的基础上
QWebEngineView view;
QWebChannel channel;

// 注册 C++ 对象到 channel
BackendObject backend;
channel.registerObject(QStringLiteral("backend"), &backend);

// 把 channel 关联到 view 的 page
view.webEnginePage()->setWebChannel(&channel);

// 加载本地 HTML 文件
view.load(QUrl(QStringLiteral("qrc:/index.html")));
// 或者加载一个 HTML 字符串
// view.setHtml(htmlString);

view.resize(800, 600);
view.show();
```

setWebChannel() 是关键的一步——它把 QWebChannel 和 QWebEnginePage 绑定起来，使得页面中的 JS 可以通过 qt.webChannelTransport 访问 channel。如果你忘了调用这一步，JS 侧 new QWebChannel(qt.webChannelTransport, ...) 会直接报错，因为 qt.webChannelTransport 这个全局对象是由 setWebChannel 注入的。

HTML 页面通常放在 Qt 资源文件（.qrc）中，通过 qrc:/ 协议加载。这样做的好处是 HTML/CSS/JS 和 C++ 代码打包在一起，部署时不需要额外携带前端文件。缺点是每次修改 HTML 都需要重新编译资源文件——开发阶段可以用 view.load(QUrl::fromLocalFile(...)) 加载本地文件来加快迭代速度。

## 4. 综合示例：Qt 后端 + Web 前端交互面板

我们把前面的内容整合成一个完整的示例。C++ 后端注册一个 BackendObject，提供系统信息查询和时间格式化方法；QWebEngineView 加载一个内嵌的 HTML 页面，页面上的按钮调用 C++ 方法，显示区域实时更新属性值和信号推送。

CMake 配置：

```cmake
find_package(Qt6 REQUIRED COMPONENTS WebChannel WebEngineWidgets)

qt_add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::WebChannel Qt6::WebEngineWidgets)
```

main.cpp 的完整代码：

```cpp
#include <QApplication>
#include <QDebug>
#include <QDateTime>
#include <QHostInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QSysInfo>
#include <QWebChannel>
#include <QWebEngineView>

/// @brief 暴露给 JavaScript 的后端交互对象
/// 演示 Q_PROPERTY 属性暴露、Q_INVOKABLE 方法调用、信号推送
class BackendObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString platformInfo READ platformInfo CONSTANT)
    Q_PROPERTY(int callCount READ callCount NOTIFY callCountChanged)

public:
    explicit BackendObject(QObject *parent = nullptr)
        : QObject(parent), m_callCount(0)
    {
    }

    QString platformInfo() const
    {
        return QStringLiteral("%1 / %2 / %3")
            .arg(QSysInfo::prettyProductName())
            .arg(QSysInfo::currentCpuArchitecture())
            .arg(QSysInfo::kernelVersion());
    }

    int callCount() const { return m_callCount; }

    /// @brief 获取当前时间字符串
    Q_INVOKABLE QString getCurrentTime()
    {
        incrementCount();
        return QDateTime::currentDateTime().toString(Qt::ISODate);
    }

    /// @brief 获取主机名
    Q_INVOKABLE QString getHostName()
    {
        incrementCount();
        return QHostInfo::localHostName();
    }

    /// @brief 获取系统信息 JSON
    Q_INVOKABLE QString getSystemInfo()
    {
        incrementCount();
        QJsonObject info;
        info[QStringLiteral("hostname")] = QHostInfo::localHostName();
        info[QStringLiteral("platform")] = QSysInfo::prettyProductName();
        info[QStringLiteral("arch")] = QSysInfo::currentCpuArchitecture();
        info[QStringLiteral("kernel")] = QSysInfo::kernelVersion();
        info[QStringLiteral("machineId")] = QSysInfo::machineUniqueId();
        info[QStringLiteral("timestamp")]
            = QDateTime::currentDateTime().toString(Qt::ISODate);
        return QJsonDocument(info).toJson(QJsonDocument::Compact);
    }

    /// @brief 从 C++ 向 JS 推送消息
    Q_INVOKABLE void sendNotification(const QString &text)
    {
        incrementCount();
        emit notificationPushed(text);
    }

signals:
    void callCountChanged(int count);
    void notificationPushed(const QString &message);

private:
    void incrementCount()
    {
        m_callCount++;
        emit callCountChanged(m_callCount);
    }

    int m_callCount;
};

// 内嵌的 HTML 页面
static const char* kHtmlPage = R"(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>QtWebChannel Demo</title>
<style>
body {
    font-family: 'Segoe UI', Arial, sans-serif;
    max-width: 700px;
    margin: 30px auto;
    padding: 0 20px;
    background: #f8f9fa;
    color: #212529;
}
h1 { color: #2c3e50; font-size: 1.6em; }
.panel {
    background: white;
    border: 1px solid #dee2e6;
    border-radius: 8px;
    padding: 20px;
    margin-bottom: 16px;
}
button {
    padding: 8px 16px;
    margin: 4px;
    border: 1px solid #007bff;
    background: #007bff;
    color: white;
    border-radius: 4px;
    cursor: pointer;
    font-size: 14px;
}
button:hover { background: #0056b3; }
#log {
    background: #1e1e1e;
    color: #d4d4d4;
    padding: 16px;
    border-radius: 4px;
    font-family: 'Consolas', monospace;
    font-size: 13px;
    white-space: pre-wrap;
    max-height: 300px;
    overflow-y: auto;
}
.status { color: #6c757d; font-size: 0.9em; margin-top: 8px; }
</style>
</head>
<body>
<h1>QtWebChannel 交互面板</h1>

<div class="panel">
    <strong>平台信息:</strong>
    <span id="platform">加载中...</span>
    <div class="status">调用次数: <span id="calls">0</span></div>
</div>

<div class="panel">
    <button onclick="callGetTime()">获取时间</button>
    <button onclick="callGetHost()">获取主机名</button>
    <button onclick="callGetSystemInfo()">系统信息</button>
    <button onclick="callNotify()">推送通知</button>
</div>

<div class="panel">
    <div id="log"></div>
</div>

<!-- qwebchannel.js 需要在运行时可用 -->
<!-- 在 QWebEngineView 中通过 qt.webChannelTransport 自动注入 -->
<script src="qrc:///qtwebchannel/qwebchannel.js"></script>
<script>
var backend = null;

// 初始化 QWebChannel
new QWebChannel(qt.webChannelTransport, function(channel) {
    backend = channel.objects.backend;

    // 读取初始属性
    document.getElementById('platform').textContent = backend.platformInfo;
    document.getElementById('calls').textContent = backend.callCount;

    // 监听属性变化
    backend.callCountChanged.connect(function(count) {
        document.getElementById('calls').textContent = count;
    });

    // 监听 C++ 推送的通知
    backend.notificationPushed.connect(function(message) {
        appendLog('[通知] ' + message);
    });

    appendLog('QWebChannel 已连接');
    appendLog('平台: ' + backend.platformInfo);
});

function callGetTime() {
    if (!backend) return;
    backend.getCurrentTime(function(result) {
        appendLog('[时间] ' + result);
    });
}

function callGetHost() {
    if (!backend) return;
    backend.getHostName(function(result) {
        appendLog('[主机名] ' + result);
    });
}

function callGetSystemInfo() {
    if (!backend) return;
    backend.getSystemInfo(function(result) {
        appendLog('[系统信息] ' + result);
    });
}

function callNotify() {
    if (!backend) return;
    backend.sendNotification('来自按钮的测试通知 @ ' + new Date().toLocaleTimeString());
}

function appendLog(text) {
    var log = document.getElementById('log');
    var time = new Date().toLocaleTimeString();
    log.textContent += '[' + time + '] ' + text + '\\n';
    log.scrollTop = log.scrollHeight;
}
</script>
</body>
</html>
)";

#include "main.moc"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qDebug() << "QtWebChannel 交互面板示例";
    qDebug() << "本示例演示 QWebChannel + QWebEngineView 双向通信";

    BackendObject backend;

    QWebChannel channel;
    channel.registerObject(QStringLiteral("backend"), &backend);

    QWebEngineView view;
    view.page()->setWebChannel(&channel);
    view.setHtml(QString::fromUtf8(kHtmlPage));
    view.resize(800, 600);
    view.setWindowTitle(QStringLiteral("QtWebChannel Demo"));
    view.show();

    qDebug() << "窗口已打开";
    qDebug() << "点击页面上的按钮测试 JS 调用 C++ 方法";

    return app.exec();
}
```

运行程序后会弹出一个 WebEngineView 窗口，显示一个简单的交互面板。页面顶部显示平台信息和调用计数，中间是四个功能按钮，底部是日志输出区域。点击"获取时间"按钮，JS 调用 C++ 的 getCurrentTime() 方法，返回值显示在日志中。点击"推送通知"按钮，JS 调用 sendNotification()，C++ 侧触发 notificationPushed 信号，JS 通过信号回调在日志中显示通知内容。

几个关键实现点要解释一下。HTML 中引用 qwebchannel.js 的路径是 `qrc:///qtwebchannel/qwebchannel.js`——这是 Qt 内部自动将 qwebchannel.js 注册到的资源路径，不需要你手动添加到 .qrc 文件中。`#include "main.moc"` 这行不能少——因为 BackendObject 定义在 .cpp 文件中且包含 Q_OBJECT 宏，MOC 需要为它生成元对象代码。如果 BackendObject 声明在头文件中就不需要这行。setHtml() 直接传入 HTML 字符串而不是加载文件——这避免了资源文件管理，适合小型演示页面。生产项目中建议把 HTML 放在 .qrc 资源文件中。

## 5. 练习项目

练习项目：实时系统监控面板。

在示例基础上扩展为一个更完整的前后端交互面板：C++ 后端使用 QTimer 每秒采集一次系统数据（可以用 QProcess 读取 /proc/stat 或调用系统命令获取 CPU 使用率、内存占用），通过 Q_PROPERTY 暴露实时数据；HTML 前端用纯 CSS/JS 绘制简单的进度条或折线图显示历史趋势；添加一个"执行命令"输入框，用户在页面上输入命令字符串，JS 调用 C++ 的 QProcess 执行命令并返回输出结果。

完成标准是这样的：页面打开后 CPU 和内存使用率每秒自动更新（通过 NOTIFY 信号推送），进度条平滑过渡；历史趋势显示最近 60 秒的数据，用 CSS 绘制的简单柱状图即可；在命令输入框输入 `uname -a` 并点击执行，页面显示出命令的完整输出；连续快速点击按钮时调用计数器准确递增，不出现计数跳跃或丢失。

几个实现提示：QProcess 可以用 execute() 静态方法执行简单命令并获取输出，如果需要异步执行就用 start() + waitForFinished()。CPU 使用率的计算需要两次采样，第一次读取总时间和空闲时间，一秒后再读一次，差值除以总时间差就是使用率。历史数据在 C++ 侧维护一个 QJsonArray 或者 QList，通过 Q_PROPERTY 暴露为 JSON 字符串给 JS 解析。

## 6. 官方文档参考

[Qt 文档 · QWebChannel](https://doc.qt.io/qt-6/qwebchannel.html) -- Web 通道类，注册 C++ 对象供 JavaScript 访问

[Qt 文档 · QtWebChannel 模块](https://doc.qt.io/qt-6/qtwebchannel-index.html) -- WebChannel 模块总览与集成指南

[Qt 文档 · QWebEngineView](https://doc.qt.io/qt-6/qwebengineview.html) -- WebEngine 视图类，用于嵌入 Web 内容

[Qt 文档 · QWebEnginePage::setWebChannel](https://doc.qt.io/qt-6/qwebenginepage.html#setWebChannel) -- 将 WebChannel 关联到 WebEngine 页面

---

到这里就大功告成了。QtWebChannel 在 C++ 和 JavaScript 之间搭了一座透明的桥梁——C++ 的 QObject 通过 registerObject() 暴露出去，JS 通过 channel.objects 拿到对象代理后就能直接读写属性、调用方法、连接信号。整个交互过程不需要你手动定义消息格式或者实现路由逻辑，QWebChannel 在传输层自动处理序列化和反序列化。对于 Qt 应用内嵌 Web UI 的场景，这套方案比手搓 WebSocket 协议不知道省了多少胶水代码。
