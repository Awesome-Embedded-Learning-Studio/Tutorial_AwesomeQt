#include "HelpBrowser.h"

// ============================================================================
// 帮助文档内容 (内嵌 HTML)
// ============================================================================

/// @brief 生成目录页 HTML
static QString get_index_html()
{
    return
        "<h1>MiniHelp 帮助文档</h1>"
        "<p>欢迎使用 MiniHelp 文档浏览器。本页面展示了 QTextBrowser"
        " 的文档浏览和导航能力。</p>"
        "<h2>目录</h2>"
        "<ul>"
        "  <li><a href='intro.html'>1. 简介</a></li>"
        "  <li><a href='guide.html'>2. 使用指南</a></li>"
        "  <li><a href='faq.html'>3. 常见问题</a></li>"
        "  <li><a href='about.html'>4. 关于此文档</a></li>"
        "</ul>"
        "<hr>"
        "<p style='color: #888; font-size: 11px;'>"
        "提示：点击上方链接可以导航到对应页面，"
        "使用工具栏的导航按钮可以后退/前进/返回首页。</p>";
}

/// @brief 生成简介页 HTML
static QString get_intro_html()
{
    return
        "<h1>1. 简介</h1>"
        "<p>QTextBrowser 是 QTextEdit 的只读版本，"
        "在富文本渲染能力之上增加了<b>浏览器语义</b>——"
        "超链接可点击、可导航。</p>"
        "<h2>核心能力</h2>"
        "<p>QTextBrowser 继承了 QTextEdit 的全部富文本渲染能力，"
        "包括 HTML 标签支持、段落格式、列表、表格等。"
        "在此基础上，它增加了以下浏览器级功能：</p>"
        "<ul>"
        "  <li><b>超链接导航</b>: 点击 &lt;a href&gt; 自动跳转</li>"
        "  <li><b>历史栈</b>: 支持后退/前进导航</li>"
        "  <li><b>本地文件加载</b>: setSource() 加载 HTML 文件</li>"
        "  <li><b>搜索路径</b>: 自动解析相对路径链接</li>"
        "</ul>"
        "<p><a href='index.html'>返回目录</a></p>";
}

/// @brief 生成使用指南页 HTML
static QString get_guide_html()
{
    return
        "<h1>2. 使用指南</h1>"
        "<h2>加载内容</h2>"
        "<p>QTextBrowser 提供了多种方式加载内容：</p>"
        "<ul>"
        "  <li><code>setHtml()</code> — 直接设置 HTML 字符串</li>"
        "  <li><code>setSource()</code> — 加载本地 HTML 文件</li>"
        "  <li><code>setMarkdown()</code> — 设置 Markdown 内容</li>"
        "</ul>"
        "<h2>导航操作</h2>"
        "<p>QTextBrowser 内部维护了一个<b>历史栈</b>，"
        "记录用户访问过的页面序列。你可以通过以下方法操作历史栈：</p>"
        "<ul>"
        "  <li><code>backward()</code> — 后退到上一页</li>"
        "  <li><code>forward()</code> — 前进到下一页</li>"
        "  <li><code>home()</code> — 返回首页（历史栈第一个条目）</li>"
        "</ul>"
        "<p><a href='index.html'>返回目录</a></p>";
}

/// @brief 生成常见问题页 HTML
static QString get_faq_html()
{
    return
        "<h1>3. 常见问题</h1>"
        "<h2>Q: QTextBrowser 能打开外部网站吗？</h2>"
        "<p>A: 不能。QTextBrowser 渲染的是 HTML 4 子集，"
        "不是完整的浏览器。如果需要显示外部网页，"
        "应该使用 Qt WebEngine 模块。</p>"
        "<h2>Q: anchorClicked 信号能取消导航吗？</h2>"
        "<p>A: 不能直接取消。QTextBrowser 在发出 anchorClicked "
        "信号后仍然会执行默认导航。如果需要拦截特定链接，"
        "需要在槽函数中做额外处理。</p>"
        "<h2>Q: setSource 加载失败会怎样？</h2>"
        "<p>A: 页面会变成空白，不会报错。建议在 sourceChanged "
        "信号中检查 URL 是否有效。</p>"
        "<p><a href='index.html'>返回目录</a></p>";
}

/// @brief 生成关于页 HTML
static QString get_about_html()
{
    return
        "<h1>4. 关于此文档</h1>"
        "<p>本帮助文档系统使用 QTextBrowser 构建，"
        "所有页面以 HTML 文件的形式存储在临时目录中，"
        "通过 <code>setSource()</code> 加载。</p>"
        "<p>导航按钮的状态（启用/禁用）由 QTextBrowser 的"
        "<code>backwardAvailable</code> 和 "
        "<code>forwardAvailable</code> 信号自动驱动，"
        "无需手动维护历史栈。</p>"
        "<p>外部链接（如 "
        "<a href='https://doc.qt.io/qt-6/qtextbrowser.html'>"
        "Qt 官方文档</a>）通过 <code>QDesktopServices::openUrl()</code>"
        " 在系统默认浏览器中打开。</p>"
        "<p><a href='index.html'>返回目录</a></p>";
}

// ============================================================================
// HelpBrowser: 迷你帮助文档浏览器
// ============================================================================

HelpBrowser::HelpBrowser(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("QTextBrowser 综合演示 — 迷你帮助文档浏览器");
    resize(680, 560);
    createHelpFiles();
    initUi();
}

HelpBrowser::~HelpBrowser()
{
    // 临时目录会在 QTemporaryDir 析构时自动清理
}

void HelpBrowser::createHelpFiles()
{
    m_tempDir = std::make_unique<QTemporaryDir>();
    if (!m_tempDir->isValid()) {
        return;
    }

    QString dirPath = m_tempDir->path();

    write_file(dirPath + "/index.html", get_index_html());
    write_file(dirPath + "/intro.html", get_intro_html());
    write_file(dirPath + "/guide.html", get_guide_html());
    write_file(dirPath + "/faq.html", get_faq_html());
    write_file(dirPath + "/about.html", get_about_html());
}

void HelpBrowser::write_file(const QString &path, const QString &content)
{
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << content;
        file.close();
    }
}

void HelpBrowser::initUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // ---- 标题 ----
    auto *titleLabel = new QLabel("迷你帮助文档浏览器");
    titleLabel->setFont(QFont("Arial", 15, QFont::Bold));
    mainLayout->addWidget(titleLabel);

    // ================================================================
    // 导航工具栏: 后退 / 前进 / 首页 + 当前页面标签
    // ================================================================
    auto *navGroup = new QGroupBox("导航");
    auto *navLayout = new QHBoxLayout(navGroup);
    navLayout->setSpacing(6);

    m_backBtn = new QPushButton("<< 后退");
    m_backBtn->setAutoDefault(false);
    m_backBtn->setEnabled(false);

    m_forwardBtn = new QPushButton("前进 >>");
    m_forwardBtn->setAutoDefault(false);
    m_forwardBtn->setEnabled(false);

    m_homeBtn = new QPushButton("首页");
    m_homeBtn->setAutoDefault(false);
    m_homeBtn->setEnabled(false);

    m_sourceLabel = new QLabel("当前: (无)");
    m_sourceLabel->setStyleSheet(
        "color: #666; font-size: 11px;");

    navLayout->addWidget(m_backBtn);
    navLayout->addWidget(m_forwardBtn);
    navLayout->addWidget(m_homeBtn);
    navLayout->addSpacing(20);
    navLayout->addWidget(m_sourceLabel, 1);

    mainLayout->addWidget(navGroup);

    // ================================================================
    // 文档浏览器: QTextBrowser
    // ================================================================
    m_browser = new QTextBrowser();
    m_browser->setOpenExternalLinks(false);

    mainLayout->addWidget(m_browser, 1);

    // ================================================================
    // 状态栏
    // ================================================================
    auto *statusLayout = new QHBoxLayout();

    auto *tipLabel = new QLabel(
        "提示: 点击页面中的链接进行导航，"
        "使用工具栏按钮后退/前进");
    tipLabel->setStyleSheet(
        "color: #888; font-size: 11px;");
    statusLayout->addWidget(tipLabel);
    statusLayout->addStretch();

    mainLayout->addLayout(statusLayout);

    // ---- 信号连接 ----

    // 导航按钮
    connect(m_backBtn, &QPushButton::clicked,
            m_browser, &QTextBrowser::backward);
    connect(m_forwardBtn, &QPushButton::clicked,
            m_browser, &QTextBrowser::forward);
    connect(m_homeBtn, &QPushButton::clicked,
            m_browser, &QTextBrowser::home);

    // 历史栈状态驱动按钮启用/禁用
    connect(m_browser, &QTextBrowser::backwardAvailable,
            m_backBtn, &QPushButton::setEnabled);
    connect(m_browser, &QTextBrowser::forwardAvailable,
            m_forwardBtn, &QPushButton::setEnabled);
    connect(m_browser, &QTextBrowser::backwardAvailable,
            m_homeBtn, [this](bool available) {
        m_homeBtn->setEnabled(available);
    });

    // URL 变化时更新状态标签
    connect(m_browser, &QTextBrowser::sourceChanged,
            this, [this](const QUrl &url) {
        QString fileName = url.fileName();
        if (fileName.isEmpty()) {
            m_sourceLabel->setText("当前: (无)");
        } else {
            m_sourceLabel->setText(
                QString("当前: %1").arg(fileName));
        }
    });

    // 拦截外部链接: 用系统浏览器打开
    connect(m_browser, &QTextBrowser::anchorClicked,
            this, &HelpBrowser::onAnchorClicked);

    // 加载首页
    if (m_tempDir && m_tempDir->isValid()) {
        QString indexPath = m_tempDir->path() + "/index.html";
        m_browser->setSource(QUrl::fromLocalFile(indexPath));
    }
}

void HelpBrowser::onAnchorClicked(const QUrl &url)
{
    if (url.scheme() == "http" || url.scheme() == "https") {
        // 外部链接: 阻止 QTextBrowser 导航，用系统浏览器打开
        QDesktopServices::openUrl(url);

        // 重新加载当前页面（覆盖 QTextBrowser 的默认导航）
        QTimer::singleShot(0, this, [this]() {
            QUrl current = m_browser->source();
            if (current.isValid()) {
                m_browser->setSource(current);
            }
        });
    }
    // 内部链接交给 QTextBrowser 自动处理
}
