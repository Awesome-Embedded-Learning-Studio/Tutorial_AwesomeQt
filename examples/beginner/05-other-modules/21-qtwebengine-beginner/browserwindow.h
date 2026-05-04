/**
 * QtWebEngine 内嵌浏览器示例
 *
 * 本示例演示 QtWebEngine 模块的核心功能：
 * - QWebEngineView 加载 URL 与本地 HTML
 * - QWebEnginePage::runJavaScript() 注入执行 JS
 * - QWebEngineProfile 管理 Cookie / 缓存
 * - 页面加载进度与状态监控
 *
 * 启动后弹出一个带地址栏的浏览器窗口：
 *   - 输入 URL 按回车加载页面
 *   - 点击"提取信息"按钮 runJavaScript 提取页面数据
 *   - 点击"加载本地页面"加载内嵌 HTML
 *   - 底部显示 Cookie 变化计数
 */

#include <QMainWindow>

#include <QNetworkCookie>

class QProgressBar;
class QLabel;
class QLineEdit;
class QWebEngineView;

/// @brief 内嵌的本地演示 HTML 页面
static constexpr const char* kLocalHtml = R"html(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>本地演示页面</title>
<style>
body {
    font-family: 'Segoe UI', Arial, sans-serif;
    max-width: 600px;
    margin: 40px auto;
    padding: 0 20px;
    background: #f0f2f5;
    color: #333;
}
h1 { color: #2c3e50; }
.card {
    background: white;
    border-radius: 8px;
    padding: 20px;
    margin-bottom: 16px;
    box-shadow: 0 2px 4px rgba(0,0,0,0.1);
}
button {
    padding: 10px 20px;
    border: none;
    background: #007bff;
    color: white;
    border-radius: 4px;
    cursor: pointer;
    font-size: 14px;
}
button:hover { background: #0056b3; }
#output {
    margin-top: 16px;
    padding: 12px;
    background: #1e1e1e;
    color: #d4d4d4;
    border-radius: 4px;
    font-family: monospace;
    white-space: pre-wrap;
    min-height: 60px;
}
</style>
</head>
<body>
<h1>QtWebEngine 本地演示页面</h1>
<div class="card">
    <p>这是一个通过 setHtml() 加载的本地页面。</p>
    <p>页面中的 JavaScript 代码运行在 Chromium 渲染引擎中，
       可以通过 C++ 侧的 runJavaScript() 进行交互。</p>
    <button onclick="runDemo()">执行 JS 演示</button>
    <div id="output">点击按钮查看 JS 执行结果...</div>
</div>
<div class="card">
    <p>当前页面包含:</p>
    <ul>
        <li id="link-count">0 个链接</li>
        <li id="img-count">0 张图片</li>
        <li id="script-count">0 个脚本</li>
    </ul>
</div>
<script>
document.getElementById('link-count').textContent =
    document.querySelectorAll('a').length + ' 个链接';
document.getElementById('img-count').textContent =
    document.querySelectorAll('img').length + ' 张图片';
document.getElementById('script-count').textContent =
    document.querySelectorAll('script').length + ' 个脚本';

function runDemo() {
    var output = document.getElementById('output');
    var info = {
        userAgent: navigator.userAgent,
        platform: navigator.platform,
        language: navigator.language,
        cookieEnabled: navigator.cookieEnabled,
        timestamp: new Date().toISOString(),
        screen: screen.width + 'x' + screen.height
    };
    output.textContent = JSON.stringify(info, null, 2);
}
</script>
</body>
</html>
)html";

/// @brief 简单的内嵌浏览器窗口
/// 演示 QWebEngineView 加载 URL、runJavaScript 注入 JS、
/// QWebEngineProfile 管理 Cookie/缓存
class BrowserWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit BrowserWindow(QWidget *parent = nullptr);

public slots:
    /// @brief 加载地址栏中的 URL
    void loadUrl();

    /// @brief 加载内嵌的本地演示页面
    void loadLocalPage();

    /// @brief 通过 runJavaScript 提取当前页面信息
    void extractPageInfo();

    /// @brief Cookie 添加回调
    void on_cookie_added(const QNetworkCookie &cookie);

private:
    QWebEngineView* m_view;
    QLineEdit* m_urlEdit;
    QProgressBar* m_progressBar;
    QLabel* m_statusLabel;
    QLabel* m_cookieCountLabel;
    int m_cookieCount = 0;
};
