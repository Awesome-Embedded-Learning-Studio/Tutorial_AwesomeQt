#include "browserwindow.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QNetworkCookie>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWebEngineCookieStore>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineView>

BrowserWindow::BrowserWindow(QWidget *parent)
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

            // 加载完成后自动提取页面信息
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
    auto* localBtn = new QPushButton(
        QStringLiteral("加载本地页面"), this);

    urlBarLayout->addWidget(m_urlEdit);
    urlBarLayout->addWidget(goBtn);
    urlBarLayout->addWidget(jsBtn);
    urlBarLayout->addWidget(localBtn);

    mainLayout->addLayout(urlBarLayout);

    // 进度条（扁平化显示）
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setMaximumHeight(6);
    m_progressBar->setTextVisible(false);
    mainLayout->addWidget(m_progressBar);

    // 状态标签
    m_statusLabel = new QLabel(
        QStringLiteral("就绪"), this);
    mainLayout->addWidget(m_statusLabel);

    // WebEngineView（主区域）
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

void BrowserWindow::loadUrl()
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

void BrowserWindow::loadLocalPage()
{
    QString html = QString::fromUtf8(kLocalHtml);
    m_view->setHtml(html);
    m_statusLabel->setText(
        QStringLiteral("已加载本地演示页面"));
}

void BrowserWindow::extractPageInfo()
{
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
                    QStringLiteral("页面信息: %1")
                        .arg(result.toString().left(200)));
                qDebug() << "提取到的页面信息:" << result.toString();
            }
        });
}

void BrowserWindow::on_cookie_added(const QNetworkCookie &cookie)
{
    m_cookieCount++;
    m_cookieCountLabel->setText(
        QStringLiteral("Cookie 数量: %1").arg(m_cookieCount));
    qDebug() << "Cookie:" << cookie.name() << "=" << cookie.value()
             << "domain:" << cookie.domain();
}
