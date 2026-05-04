/**
 * QtWebChannel 交互面板示例
 *
 * 本示例演示 QtWebChannel 模块的核心功能：
 * - QWebChannel 将 C++ 对象暴露给 JavaScript
 * - qwebchannel.js 前端侧配置
 * - 双向通信：JS 调用 Qt 方法 / Qt 发信号到 JS
 * - 与 QWebEngineView 集成
 *
 * 启动后弹出一个 WebEngineView 窗口，显示交互面板：
 *   - 获取时间：JS 调用 C++ getCurrentTime() 方法
 *   - 获取主机名：JS 调用 C++ getHostName() 方法
 *   - 系统信息：JS 调用 C++ getSystemInfo() 方法
 *   - 推送通知：JS 触发 C++ emit signal 回传 JS
 */

#include "backendobject.h"

#include <QApplication>
#include <QDebug>
#include <QWebChannel>
#include <QWebEngineView>

// 内嵌 HTML 页面（简化部署，不依赖外部文件）
// 生产环境建议使用 .qrc 资源文件管理 HTML/CSS/JS
static const char* kHtmlPage = R"html(
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

<!-- qwebchannel.js 由 Qt 内部自动注册到 qrc 资源 -->
<script src="qrc:///qtwebchannel/qwebchannel.js"></script>
<script>
var backend = null;

// 初始化 QWebChannel，连接 C++ 后端对象
new QWebChannel(qt.webChannelTransport, function(channel) {
    backend = channel.objects.backend;

    // 读取 C++ 对象的初始属性值
    document.getElementById('platform').textContent = backend.platformInfo;
    document.getElementById('calls').textContent = backend.callCount;

    // 监听 callCount 属性变化（通过 NOTIFY 信号自动推送）
    backend.callCountChanged.connect(function(count) {
        document.getElementById('calls').textContent = count;
    });

    // 监听 C++ 推送的通知（自定义信号）
    backend.notificationPushed.connect(function(message) {
        appendLog('[通知] ' + message);
    });

    appendLog('QWebChannel 已连接');
    appendLog('平台: ' + backend.platformInfo);
});

// JS 调用 C++ Q_INVOKABLE 方法（异步，结果通过回调返回）
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

// 触发 C++ 侧 emit signal，信号通过 WebChannel 回传到 JS
function callNotify() {
    if (!backend) return;
    backend.sendNotification(
        '来自按钮的测试通知 @ ' + new Date().toLocaleTimeString());
}

function appendLog(text) {
    var log = document.getElementById('log');
    var time = new Date().toLocaleTimeString();
    log.textContent += '[' + time + '] ' + text + '\n';
    log.scrollTop = log.scrollHeight;
}
</script>
</body>
</html>
)html";

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qDebug() << "QtWebChannel 交互面板示例";
    qDebug() << "本示例演示 QWebChannel + QWebEngineView 双向通信";

    // 创建后端对象（生命周期覆盖整个 app）
    BackendObject backend;

    // 创建 QWebChannel 并注册后端对象
    QWebChannel channel;
    channel.registerObject(QStringLiteral("backend"), &backend);

    // 创建 QWebEngineView 并关联 WebChannel
    QWebEngineView view;
    view.page()->setWebChannel(&channel);

    // 加载内嵌 HTML 页面
    view.setHtml(QString::fromUtf8(kHtmlPage));
    view.resize(800, 600);
    view.setWindowTitle(QStringLiteral("QtWebChannel Demo"));
    view.show();

    qDebug() << "窗口已打开";
    qDebug() << "点击页面上的按钮测试 JS 调用 C++ 方法";

    return app.exec();
}
