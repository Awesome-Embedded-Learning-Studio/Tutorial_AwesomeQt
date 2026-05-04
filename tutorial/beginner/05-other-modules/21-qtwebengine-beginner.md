# 现代Qt开发教程（新手篇）5.21--QtWebEngine 内嵌 Chromium 浏览器

## 1. 前言：在 Qt 窗口里塞一个完整的浏览器引擎

上一篇我们用 QWebEngineView 作为 QWebChannel 的前端容器，但当时没有深入讲 WebEngine 本身。这篇我们专门来拆解 QtWebEngine 这个模块——它在 Qt 应用里内嵌了一个完整的 Chromium 渲染引擎，让你可以在 Qt 窗口中加载任意 URL、渲染完整的 HTML/CSS/JS 页面、执行 JavaScript 并获取返回值、管理 Cookie 和缓存。

为什么要在 Qt 应用里嵌一个浏览器？最直接的理由是：你团队里有 Web 前端开发经验的人，他们用 HTML/CSS/JS 构建界面的效率远高于用 QWidgets 手写布局。或者你的应用需要展示复杂的富文本内容、SVG 图形、实时图表——这些东西用 Web 技术实现起来比 Qt 原生控件灵活得多。又或者你需要嵌入第三方 Web 页面（比如地图、在线文档编辑器），那 WebEngine 几乎是唯一的选择。

当然天下没有免费的午餐。QtWebEngine 内嵌的 Chromium 引擎在运行时会占用相当可观的内存——一个简单的 QWebEngineView 窗口启动后，Chromium 的多进程架构会额外拉起 GPU 进程、渲染进程、网络进程等，内存占用通常在 100-200MB 起步。如果你的应用对内存敏感（嵌入式设备、低配工控机），或者你只需要显示简单的 HTML 内容，QTextBrowser 或者轻量级的第三方库可能是更合适的选择。但如果你的需求涉及复杂的 Web 页面渲染和 JS 交互，QtWebEngine 就是 Qt 生态中最靠谱的方案。

这篇我们要做的是用 QWebEngineView 加载 URL 和本地 HTML，用 runJavaScript 注入执行 JS 代码，用 QWebEngineProfile 管理 Cookie 和缓存，最后讨论一下 WebEngine 的资源占用和使用场景权衡。

## 2. 环境说明

本篇基于 Qt 6.9.1，需要引入 WebEngineWidgets 模块。CMake 配置如下：

```cmake
find_package(Qt6 REQUIRED COMPONENTS WebEngineWidgets)
```

QtWebEngine 从 Qt 5.4 开始提供，在 Qt 6 中经历了重大架构更新。它基于 Chromium 项目，在 Qt 6.9.1 中对应的 Chromium 版本大约是 122.x。WebEngineWidgets 模块提供的是基于 QWidget 的集成方式（QWebEngineView），适合传统的桌面 Qt Widgets 应用。如果你的应用基于 Qt Quick，应该使用 WebEngine 模块（QML 类型 WebEngineView）。

WebEngine 的编译和部署有几个特殊要求。首先它不支持 MSVC 的静态链接——你只能动态链接。其次 Chromium 的多进程架构意味着你的应用运行时会同时存在多个进程，这在某些受控环境（沙箱、容器）中可能需要额外配置。再次，GPU 加速依赖系统的图形驱动——在某些老旧显卡或虚拟机环境中可能需要设置 `--disable-gpu` 环境变量来使用软件渲染。

工具链：MSVC 2019+、GCC 11+、Clang 14+，C++17 标准，CMake 3.26+。建议使用 Qt官方预编译安装包，从源码编译 WebEngine 需要下载 Chromium 源码，编译时间以小时计，普通开发者完全没必要折腾。

## 3. 核心概念讲解

### 3.1 QWebEngineView 加载 URL 与本地 HTML

QWebEngineView 是 WebEngine 模块的核心视图类——你可以把它理解为一个嵌入在 Qt 窗口中的浏览器标签页。它继承自 QWidget，可以像任何其他 Qt 控件一样嵌入到布局中。最基本的用法是调用 load() 加载一个 URL，或者调用 setHtml() 加载一段 HTML 字符串。

```cpp
#include <QWebEngineView>
#include <QUrl>

QWebEngineView* view = new QWebEngineView(parent);

// 加载远程 URL
view->load(QUrl(QStringLiteral("https://www.qt.io")));

// 加载本地 HTML 文件
view->load(QUrl::fromLocalFile(QStringLiteral("/path/to/page.html")));

// 直接加载 HTML 字符串
view->setHtml(QStringLiteral("<html><body><h1>Hello WebEngine</h1></body></html>"));
```

load() 和 setHtml() 的区别值得说清楚。load(QUrl) 会发起一个完整的页面加载流程——如果 URL 是远程地址，WebEngine 会通过 HTTP 请求获取内容；如果是本地文件（file:// 协议），直接从磁盘读取。setHtml(QString) 是直接把 HTML 字符串塞进渲染引擎，不经过网络请求，适合动态生成的 HTML 内容。setHtml 还有一个可选的第二个参数 baseUrl，用来指定页面的基础 URL——这会影响相对路径的解析（比如 HTML 中的 `<img src="logo.png">` 相对于哪个目录查找）。

页面加载过程有几个重要的信号：loadStarted() 在开始加载时触发，loadProgress(int) 在加载过程中定期报告进度（0-100），loadFinished(bool) 在加载完成时触发，bool 参数表示是否加载成功。连接 loadFinished 信号可以让你在页面就绪后执行后续操作：

```cpp
QObject::connect(view, &QWebEngineView::loadFinished,
    [](bool ok) {
        if (ok) {
            qDebug() << "页面加载完成";
        } else {
            qDebug() << "页面加载失败";
        }
    });
```

这里有个实际开发中很容易踩的坑：loadFinished(false) 不一定意味着页面完全不可用——有时候主框架加载成功了但某个子资源（图片、CSS、JS）加载失败，也会触发 loadFinished(false)。如果你需要精确控制加载完成后的操作，最好结合 loadProgress(100) 一起使用。

### 3.2 QWebEnginePage::runJavaScript 注入执行 JS

QWebEngineView 的核心交互能力之一是 page()->runJavaScript()——它让你从 C++ 侧向当前加载的页面注入并执行任意 JavaScript 代码，执行结果通过回调函数返回给 C++。这个能力是实现 C++/JS 双向交互的基础之一（另一种方式是上一篇讲的 QWebChannel）。

```cpp
// 执行简单的 JS 表达式，获取返回值
view->page()->runJavaScript(
    QStringLiteral("document.title"),
    [](const QVariant &result) {
        qDebug() << "页面标题:" << result.toString();
    });

// 执行复杂的 JS 代码块
view->page()->runJavaScript(
    QStringLiteral(R"(
        var elements = document.querySelectorAll('a');
        var urls = [];
        for (var i = 0; i < elements.length; i++) {
            urls.push(elements[i].href);
        }
        JSON.stringify(urls);
    )"),
    [](const QVariant &result) {
        qDebug() << "页面中所有链接:" << result.toString();
    });
```

runJavaScript 有几个关键特性要了解。首先，JS 代码在页面的 JavaScript 上下文中执行——它可以访问页面的 DOM、全局变量、已加载的库函数等。这意味着你可以在 C++ 侧注入代码来操作页面元素、提取页面数据、调用页面中的 JS 函数。

其次，执行是异步的——runJavaScript 本身不返回值，而是通过第二个参数（一个 lambda 回调）在 JS 执行完成后把结果传回 C++。回调的参数是 QVariant 类型，它会根据 JS 返回值的类型自动映射：JS 的 string 映射为 QString，number 映射为 double 或 int，boolean 映射为 bool，object/array 映射为 JSON 字符串。

再次，runJavaScript 只能在页面加载完成后调用——如果页面还没加载就注入 JS，代码不会执行。实际开发中应该在 loadFinished(true) 的槽函数中执行 JS 注入。

第三个参数（可选）是一个 int 值，指定 JS 执行的世界ID（world ID）。WebEngine 内部用不同的"世界"来隔离 JS 执行环境——默认是 0（主世界，即页面自身的 JS 环境），你可以用 1-255 的值指定一个隔离的世界，这样注入的 JS 代码不会和页面的全局变量冲突。这在开发浏览器扩展或者注入用户脚本时特别有用。

### 3.3 QWebEngineProfile 管理 Cookie 和缓存

QWebEngineProfile 是 WebEngine 的"浏览器配置文件"——你可以把它理解为一个类似 Chrome 用户配置（Chrome Profile）的概念。每个 Profile 拥有独立的 Cookie 存储、HTTP 缓存、访问历史、权限设置、代理配置等。通过 Profile 你可以控制 WebEngine 的网络行为，比如持久化 Cookie、清除缓存、设置 HTTP 头等。

```cpp
#include <QWebEngineProfile>
#include <QWebEngineCookieStore>
#include <QWebEngineSettings>

// 获取默认 Profile
QWebEngineProfile* defaultProfile = QWebEngineProfile::defaultProfile();

// 设置持久化存储（Cookie、缓存等保存到磁盘）
defaultProfile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);
defaultProfile->setPersistentCookiesPolicy(
    QWebEngineProfile::ForcePersistentCookies);

// 设置自定义存储路径
defaultProfile->setCachePath(QStringLiteral("/path/to/cache"));
defaultProfile->setPersistentStoragePath(QStringLiteral("/path/to/storage"));

// 访问 Cookie 存储
QWebEngineCookieStore* cookieStore = defaultProfile->cookieStore();
QObject::connect(cookieStore, &QWebEngineCookieStore::cookieAdded,
    [](const QNetworkCookie &cookie) {
        qDebug() << "Cookie 添加:" << cookie.name() << "=" << cookie.value()
                 << "domain:" << cookie.domain();
    });

// 清除所有浏览数据
defaultProfile->clearAllVisitedLinks();
defaultProfile->clearHttpCache();
```

默认情况下，WebEngine 使用一个名为 "default" 的 Profile 实例。如果你不主动修改配置，这个 Profile 的 Cookie 和缓存在应用退出后会丢失（类似于浏览器的无痕模式）。通过 setPersistentCookiesPolicy(ForcePersistentCookies) 可以让 Cookie 持久化到磁盘，下次启动应用时自动恢复。setHttpCacheType(DiskHttpCache) 让 HTTP 缓存也保存到磁盘，避免每次启动都重新下载所有资源。

除了默认 Profile，你还可以创建自定义 Profile 来隔离不同的浏览上下文：

```cpp
// 创建独立的 Profile（类似 Chrome 的多账户）
QWebEngineProfile* customProfile = new QWebEngineProfile(
    QStringLiteral("MyProfile"), this);
customProfile->setHttpUserAgent(
    QStringLiteral("MyApp/1.0 (Custom Bot)"));

// 基于 Profile 创建 Page
QWebEnginePage* page = new QWebEnginePage(customProfile);
view->setPage(page);
```

这种方式在需要多账户隔离的场景下很有用——比如你的应用需要同时登录两个不同的 Web 服务，如果不隔离 Profile，两个服务的 Cookie 会互相干扰。创建了独立的 Profile 后，通过 setPage() 把自定义的 QWebEnginePage（关联到自定义 Profile）设置到 QWebEngineView 上，后续的所有网络请求都会使用这个 Profile 的配置。

### 3.4 资源占用与场景权衡

QtWebEngine 的最大代价是资源占用。由于内嵌了完整的 Chromium 引擎，你的应用启动后会多出好几个进程：主进程（你的应用）、GPU 进程（Chromium 的 GPU 加速）、渲染进程（每个页面一个，沙箱隔离）、网络进程（处理 HTTP 请求）、实用程序进程（处理音频、视频解码等）。一个只加载了一个空白页面的 QWebEngineView 应用，内存占用通常在 120-180MB 左右（取决于平台和 Chromium 版本）。每多加载一个复杂页面，渲染进程的内存还会继续增长。

这个资源开销决定了 WebEngine 不适合所有场景。如果你的应用只是显示一段格式化的 HTML 文本（比如帮助文档、富文本邮件预览），QTextBrowser 就够了，它基于 Qt 的富文本引擎，内存占用只有几 MB。如果你需要显示 Markdown 或者简单的 SVG，Qt 的 SVG 模块和第三方 Markdown 渲染库也是更轻量的选择。

WebEngine 真正发挥价值的场景是：需要运行复杂的 JavaScript 应用（在线地图、图表库、富文本编辑器）、需要完整的 CSS3 渲染能力（动画、Flex/Grid 布局、CSS 变量）、需要嵌入第三方 Web 页面、需要和 Web 生态的前端框架（React/Vue/Angular）集成。在这些场景下，WebEngine 的资源开销换来的是 Web 生态的巨大灵活性和开发效率——用前端技术栈构建复杂 UI 比用 QWidgets 手写布局高效得多。

另外一个需要注意的点：WebEngine 的首次页面加载时间通常比较长（冷启动时 Chromium 需要初始化各种子进程），后续页面加载会快很多。如果你的应用对启动速度敏感，可以考虑在应用启动后立即创建一个隐藏的 QWebEngineView 预热 Chromium 引擎，等真正需要显示页面时再 load() 目标 URL。

## 4. 综合示例：内嵌浏览器面板

我们把前面的内容整合成一个完整的示例。创建一个基于 QWebEngineView 的内嵌浏览器面板——支持加载 URL 和本地 HTML，通过 runJavaScript 提取页面信息，配置 Profile 持久化 Cookie 和缓存，并在界面上展示页面加载状态。

CMake 配置：

```cmake
find_package(Qt6 REQUIRED COMPONENTS WebEngineWidgets)

qt_add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::WebEngineWidgets)
```

main.cpp 的完整代码：

```cpp
#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QNetworkCookie>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWebEngineCookieStore>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineView>

/// @brief 简单的内嵌浏览器窗口
/// 演示 QWebEngineView 加载 URL、runJavaScript 注入 JS、
/// QWebEngineProfile 管理 Cookie/缓存
class BrowserWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit BrowserWindow(QWidget *parent = nullptr)
        : QMainWindow(parent)
    {
        setWindowTitle(QStringLiteral("QtWebEngine 内嵌浏览器"));
        resize(1024, 768);

        // === 配置 Profile：持久化 Cookie 和缓存 ===
        QWebEngineProfile* profile = QWebEngineProfile::defaultProfile();
        profile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);
        profile->setPersistentCookiesPolicy(
            QWebEngineProfile::ForcePersistentCookies);

        // 监听 Cookie 变化
        QObject::connect(profile->cookieStore(),
            &QWebEngineCookieStore::cookieAdded,
            this, &BrowserWindow::on_cookie_added);

        // === 创建 WebEngineView ===
        m_view = new QWebEngineView(this);

        // 创建关联 Profile 的 Page
        QWebEnginePage* page = new QWebEnginePage(profile, this);
        m_view->setPage(page);

        // 页面加载进度
        QObject::connect(m_view, &QWebEngineView::loadProgress,
            this, [this](int progress) {
                m_progressBar->setValue(progress);
            });

        // 页面加载完成
        QObject::connect(m_view, &QWebEngineView::loadFinished,
            this, [this](bool ok) {
                m_statusLabel->setText(
                    ok ? QStringLiteral("加载完成")
                       : QStringLiteral("加载失败"));
                m_progressBar->setValue(ok ? 100 : 0);

                // 加载完成后提取页面标题
                if (ok) {
                    extractPageInfo();
                }
            });

        // === 构建界面 ===
        auto* centralWidget = new QWidget(this);
        auto* mainLayout = new QVBoxLayout(centralWidget);

        // 地址栏
        auto* urlBarLayout = new QHBoxLayout();
        m_urlEdit = new QLineEdit(this);
        m_urlEdit->setPlaceholderText(
            QStringLiteral("输入 URL 并按回车..."));
        m_urlEdit->setText(QStringLiteral("https://www.qt.io"));

        auto* goBtn = new QPushButton(QStringLiteral("加载"), this);
        auto* jsBtn = new QPushButton(QStringLiteral("提取信息"), this);
        auto* localBtn = new QPushButton(QStringLiteral("加载本地页面"), this);

        urlBarLayout->addWidget(m_urlEdit);
        urlBarLayout->addWidget(goBtn);
        urlBarLayout->addWidget(jsBtn);
        urlBarLayout->addWidget(localBtn);

        mainLayout->addLayout(urlBarLayout);

        // 进度条和状态
        m_progressBar = new QProgressBar(this);
        m_progressBar->setRange(0, 100);
        m_progressBar->setValue(0);
        m_progressBar->setMaximumHeight(6);
        m_progressBar->setTextVisible(false);
        mainLayout->addWidget(m_progressBar);

        m_statusLabel = new QLabel(
            QStringLiteral("就绪"), this);
        mainLayout->addWidget(m_statusLabel);

        // WebEngineView
        mainLayout->addWidget(m_view, 1);

        // Cookie 计数
        m_cookieCountLabel = new QLabel(
            QStringLiteral("Cookie 数量: 0"), this);
        mainLayout->addWidget(m_cookieCountLabel);

        setCentralWidget(centralWidget);

        // === 信号连接 ===
        QObject::connect(m_urlEdit, &QLineEdit::returnPressed,
            this, &BrowserWindow::loadUrl);
        QObject::connect(goBtn, &QPushButton::clicked,
            this, &BrowserWindow::loadUrl);
        QObject::connect(jsBtn, &QPushButton::clicked,
            this, &BrowserWindow::extractPageInfo);
        QObject::connect(localBtn, &QPushButton::clicked,
            this, &BrowserWindow::loadLocalPage);
    }

private slots:
    void loadUrl()
    {
        QString urlText = m_urlEdit->text().trimmed();
        if (urlText.isEmpty()) return;

        // 自动补全协议前缀
        if (!urlText.startsWith(QStringLiteral("http://"))
            && !urlText.startsWith(QStringLiteral("https://"))) {
            urlText.prepend(QStringLiteral("https://"));
            m_urlEdit->setText(urlText);
        }

        QUrl url(urlText);
        if (url.isValid()) {
            m_statusLabel->setText(
                QStringLiteral("正在加载: %1").arg(urlText));
            m_progressBar->setValue(0);
            m_view->load(url);
        } else {
            m_statusLabel->setText(
                QStringLiteral("无效 URL: %1").arg(urlText));
        }
    }

    void loadLocalPage()
    {
        // 加载内嵌的本地演示页面
        QString html = QString::fromUtf8(kLocalHtml);
        m_view->setHtml(html);
        m_statusLabel->setText(
            QStringLiteral("已加载本地演示页面"));
    }

    void extractPageInfo()
    {
        // 使用 runJavaScript 提取页面信息
        m_view->page()->runJavaScript(
            QStringLiteral(R"(
                (function() {
                    return JSON.stringify({
                        title: document.title,
                        url: window.location.href,
                        links: document.querySelectorAll('a').length,
                        images: document.querySelectorAll('img').length,
                        scripts: document.querySelectorAll('script').length,
                        charset: document.characterSet,
                        viewport: window.innerWidth + 'x' + window.innerHeight
                    });
                })()
            )"),
            [this](const QVariant &result) {
                if (result.isValid()) {
                    m_statusLabel->setText(
                        QStringLiteral("页面信息: %1").arg(result.toString().left(200)));
                    qDebug() << "提取到的页面信息:" << result.toString();
                }
            });
    }

    void on_cookie_added(const QNetworkCookie &cookie)
    {
        m_cookieCount++;
        m_cookieCountLabel->setText(
            QStringLiteral("Cookie 数量: %1").arg(m_cookieCount));
        qDebug() << "Cookie:" << cookie.name() << "=" << cookie.value()
                 << "domain:" << cookie.domain();
    }

private:
    static constexpr const char* kLocalHtml = R"(
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
// 统计页面元素数量（演示 DOM 操作）
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
)";

    QWebEngineView* m_view;
    QLineEdit* m_urlEdit;
    QProgressBar* m_progressBar;
    QLabel* m_statusLabel;
    QLabel* m_cookieCountLabel;
    int m_cookieCount = 0;
};

#include "main.moc"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qDebug() << "QtWebEngine 内嵌浏览器示例";
    qDebug() << "本示例演示 QWebEngineView + runJavaScript + Profile 管理";

    BrowserWindow window;
    window.show();

    // 默认加载 Qt 官网
    window.loadUrl();

    return app.exec();
}
```

运行程序后会弹出一个带地址栏的浏览器窗口。默认加载 Qt 官网，地址栏可以输入任意 URL 按回车跳转。点击"提取信息"按钮，C++ 通过 runJavaScript 注入 JS 代码，提取当前页面的标题、URL、链接数、图片数等信息，显示在状态栏和调试输出中。点击"加载本地页面"按钮加载一个内嵌的演示 HTML 页面，演示 setHtml() 的用法。底部显示当前会话中收集到的 Cookie 数量。

几个实现细节需要说明。BrowserWindow 类包含 Q_OBJECT 宏且定义在 .cpp 文件中，所以末尾需要 `#include "main.moc"` 来包含 MOC 生成的元对象代码。QWebEngineProfile::defaultProfile() 返回的是全局默认 Profile，所有没有显式指定 Profile 的 QWebEnginePage 都共享这个实例。示例中通过 setHttpCacheType 和 setPersistentCookiesPolicy 配置了持久化存储——应用重启后 Cookie 和缓存仍然有效。Cookie 监听通过 QWebEngineCookieStore::cookieAdded 信号实现，每次页面设置新 Cookie 时都会触发，让你可以追踪 Cookie 的变化。

## 5. 练习项目

练习项目：简易网页内容提取工具。

在示例基础上扩展为一个网页内容提取工具：用户输入 URL 后加载页面，加载完成后自动用 runJavaScript 提取页面中的所有文本段落（`<p>` 标签内容）、所有图片 URL（`<img src>` 属性值）、所有外部链接（`<a href>` 属性值）；提取到的内容以结构化的方式显示在侧边栏面板中（使用 QSplitter 分割 WebEngineView 和信息面板）；提供"清除缓存"按钮，调用 QWebEngineProfile::clearHttpCache() 清除所有缓存；提供"导出"按钮，将提取到的内容保存为 JSON 文件。

完成标准是这样的：加载一个新闻网站页面后，侧边栏正确显示该页面的所有段落文本（去除 HTML 标签）、图片 URL 列表和外部链接列表；图片 URL 数量和页面中实际的 `<img>` 标签数量一致；点击"清除缓存"后重新加载同一页面，可以从网络请求日志（通过 QWebEngineUrlRequestInterceptor）确认资源被重新下载而非使用缓存。

几个实现提示：文本段落提取用 `document.querySelectorAll('p')` 遍历所有 p 标签，取 textContent 属性。图片 URL 提取用 `document.querySelectorAll('img')` 取 src 属性，注意有些 src 可能是相对路径，需要用 `new URL(src, document.baseURI).href` 转成绝对路径。QWebEngineUrlRequestInterceptor 是 WebEngine 的请求拦截器接口，你可以继承它并实现 interceptRequest 方法来监控所有网络请求。

## 6. 官方文档参考

[Qt 文档 · QWebEngineView](https://doc.qt.io/qt-6/qwebengineview.html) -- WebEngine 视图类，在 Qt 窗口中嵌入 Web 内容

[Qt 文档 · QWebEnginePage](https://doc.qt.io/qt-6/qwebenginepage.html) -- WebEngine 页面类，提供 runJavaScript 和页面控制 API

[Qt 文档 · QWebEngineProfile](https://doc.qt.io/qt-6/qwebengineprofile.html) -- 浏览器配置文件管理，Cookie/缓存/权限

[Qt 文档 · QtWebEngine 模块](https://doc.qt.io/qt-6/qtwebengine-index.html) -- WebEngine 模块总览与架构说明

[Qt 文档 · QWebEngineCookieStore](https://doc.qt.io/qt-6/qwebenginecookiestore.html) -- Cookie 存储管理

---

到这里就大功告成了。QtWebEngine 把完整的 Chromium 渲染引擎装进了 Qt 应用——QWebEngineView 加载 URL 和 HTML、runJavaScript 从 C++ 注入 JS 并获取返回值、QWebEngineProfile 管理 Cookie 缓存和浏览器配置。资源占用确实不小（百 MB 级内存起步），但对于需要复杂 Web 页面渲染、前端框架集成、第三方网页嵌入的场景，WebEngine 提供的能力远超 QTextBrowser 或者其他轻量级方案。选不选它，取决于你的需求是否真的需要一套完整的浏览器引擎。
