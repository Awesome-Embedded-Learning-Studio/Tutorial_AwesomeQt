/// @file    custom_resource_browser.cpp
/// @brief   CustomResourceBrowser 类实现——自定义协议资源加载与历史导航。
///
/// 对应教程：进阶层 03-QtWidgets/25-QTextBrowser 进阶。

#include "custom_resource_browser.h"

#include <QColor>
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QTextDocument>
#include <QVBoxLayout>

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

CustomResourceBrowser::CustomResourceBrowser(QWidget* parent)
    : QTextBrowser(parent)
    , m_btnBack(nullptr)
    , m_btnForward(nullptr)
    , m_btnHome(nullptr)
    , m_historyLabel(nullptr)
{
    initMockDatabase();

    // 禁用默认导航——完全手动控制链接行为
    setOpenLinks(false);
    setOpenExternalLinks(false);

    // 连接 anchorClicked 进行自定义链接处理
    connect(this, &QTextBrowser::anchorClicked,
            this, &CustomResourceBrowser::handleAnchorClicked);

    // sourceChanged 信号用于更新导航状态
    connect(this, &QTextBrowser::sourceChanged, this, [this]() {
        updateNavigationState();
    });

    // 加载首页内容
    setHtml(buildPageOne());
}

// ─────────────────────────────────────────────────────────────────────────────
// 导航工具栏创建
// ─────────────────────────────────────────────────────────────────────────────

QWidget* CustomResourceBrowser::createNavigationToolBar()
{
    auto* bar = new QWidget;
    auto* layout = new QHBoxLayout(bar);
    layout->setContentsMargins(4, 4, 4, 4);

    m_btnBack = new QPushButton(QStringLiteral("<< 后退"));
    m_btnForward = new QPushButton(QStringLiteral("前进 >>"));
    m_btnHome = new QPushButton(QStringLiteral("首页"));
    m_historyLabel = new QLabel;

    layout->addWidget(m_btnBack);
    layout->addWidget(m_btnForward);
    layout->addWidget(m_btnHome);
    layout->addStretch(1);
    layout->addWidget(m_historyLabel);

    // 安全导航：先检查历史栈数量再调用 backward/forward
    connect(m_btnBack, &QPushButton::clicked, this, &CustomResourceBrowser::safeBackward);
    connect(m_btnForward, &QPushButton::clicked, this, &CustomResourceBrowser::safeForward);
    connect(m_btnHome, &QPushButton::clicked, this, &CustomResourceBrowser::goHome);

    // 初始状态：首页没有后退/前进历史
    updateNavigationState();

    return bar;
}

// ─────────────────────────────────────────────────────────────────────────────
// 模拟图片数据库
// ─────────────────────────────────────────────────────────────────────────────

void CustomResourceBrowser::initMockDatabase()
{
    const int kImageSize = 120;

    // 图片 1：蓝色，标注 "Product A"
    QImage img1(kImageSize, kImageSize, QImage::Format_RGB32);
    img1.fill(QColor(100, 149, 237));
    {
        QPainter p(&img1);
        p.setPen(Qt::white);
        p.drawText(img1.rect(), Qt::AlignCenter, QStringLiteral("Product A"));
    }
    m_imageDatabase[QStringLiteral("product_a")] = img1;

    // 图片 2：绿色，标注 "Product B"
    QImage img2(kImageSize, kImageSize, QImage::Format_RGB32);
    img2.fill(QColor(102, 187, 106));
    {
        QPainter p(&img2);
        p.setPen(Qt::white);
        p.drawText(img2.rect(), Qt::AlignCenter, QStringLiteral("Product B"));
    }
    m_imageDatabase[QStringLiteral("product_b")] = img2;

    // 图片 3：橙色，标注 "Product C"
    QImage img3(kImageSize, kImageSize, QImage::Format_RGB32);
    img3.fill(QColor(255, 167, 38));
    {
        QPainter p(&img3);
        p.setPen(Qt::white);
        p.drawText(img3.rect(), Qt::AlignCenter, QStringLiteral("Product C"));
    }
    m_imageDatabase[QStringLiteral("product_c")] = img3;
}

// ─────────────────────────────────────────────────────────────────────────────
// HTML 页面构建
// ─────────────────────────────────────────────────────────────────────────────

QString CustomResourceBrowser::buildPageOne() const
{
    return QStringLiteral(
        "<html><body>"
        "<h2>Page 1: 自定义协议资源加载演示</h2>"
        "<p>以下图片通过 <code>myapp://</code> 协议加载，由覆写的 loadResource() "
        "从模拟数据库中获取：</p>"
        "<table cellpadding='8'><tr>"
        "<td align='center'><img src='myapp://product_a'/><br/>Product A</td>"
        "<td align='center'><img src='myapp://product_b'/><br/>Product B</td>"
        "<td align='center'><img src='myapp://product_c'/><br/>Product C</td>"
        "</tr></table>"
        "<hr/>"
        "<p><a href='page2'>前往第二页 (内部导航)</a></p>"
        "<p><a href='https://doc.qt.io/qt-6/qtextbrowser.html'>"
        "Qt 文档 - QTextBrowser (外部链接)</a></p>"
        "<hr/>"
        "<p><i>提示：内部链接由 anchorClicked 手动处理，"
        "外部链接通过 QDesktopServices::openUrl 打开。</i></p>"
        "</body></html>");
}

QString CustomResourceBrowser::buildPageTwo() const
{
    return QStringLiteral(
        "<html><body>"
        "<h2>Page 2: 历史栈导航演示</h2>"
        "<p>你已经通过内部导航来到了第二页。点击工具栏的「后退」按钮可以回到第一页。</p>"
        "<p>历史栈信息显示在底部，包含 backwardHistoryCount 和 forwardHistoryCount。</p>"
        "<h3>导航说明</h3>"
        "<ul>"
        "<li><b>后退按钮</b>：仅在 backwardHistoryCount &gt; 0 时启用</li>"
        "<li><b>前进按钮</b>：仅在 forwardHistoryCount &gt; 0 时启用</li>"
        "<li><b>首页按钮</b>：加载初始页面</li>"
        "</ul>"
        "<hr/>"
        "<p><a href='page1'>返回第一页</a></p>"
        "<p><a href='https://www.qt.io'>Qt 官网 (外部链接)</a></p>"
        "</body></html>");
}

// ─────────────────────────────────────────────────────────────────────────────
// loadResource 覆写
// ─────────────────────────────────────────────────────────────────────────────

QVariant CustomResourceBrowser::loadResource(int type, const QUrl& name)
{
    // 只处理图片资源且 scheme 为 "myapp" 的情况
    if (type == QTextDocument::ImageResource && name.scheme() == QStringLiteral("myapp")) {
        // 从 URL 的 host 部分提取图片键名：myapp://product_a -> "product_a"
        const QString key = name.host();
        if (m_imageDatabase.contains(key)) {
            return QVariant(m_imageDatabase.value(key));
        }
        // 未找到则返回一张灰色占位图
        QImage placeholder(120, 120, QImage::Format_RGB32);
        placeholder.fill(Qt::lightGray);
        {
            QPainter p(&placeholder);
            p.setPen(Qt::red);
            p.drawText(placeholder.rect(), Qt::AlignCenter,
                       QStringLiteral("Not Found\n%1").arg(key));
        }
        return QVariant(placeholder);
    }

    // 回退到基类实现处理本地文件和 qrc 资源
    return QTextBrowser::loadResource(type, name);
}

// ─────────────────────────────────────────────────────────────────────────────
// 安全导航操作
// ─────────────────────────────────────────────────────────────────────────────

void CustomResourceBrowser::safeBackward()
{
    // 空历史栈上调用 backward() 可能导致崩溃，必须先检查
    if (backwardHistoryCount() > 0) {
        backward();
    }
}

void CustomResourceBrowser::safeForward()
{
    if (forwardHistoryCount() > 0) {
        forward();
    }
}

void CustomResourceBrowser::goHome()
{
    setHtml(buildPageOne());
}

// ─────────────────────────────────────────────────────────────────────────────
// 链接点击处理
// ─────────────────────────────────────────────────────────────────────────────

void CustomResourceBrowser::handleAnchorClicked(const QUrl& url)
{
    // 内部页面导航：相对路径代表内部页面切换
    if (url.isRelative()) {
        const QString path = url.path();
        if (path == QStringLiteral("page1")) {
            setHtml(buildPageOne());
        } else if (path == QStringLiteral("page2")) {
            setHtml(buildPageTwo());
        }
        return;
    }

    // 外部 http/https 链接用系统默认浏览器打开
    if (url.scheme() == QStringLiteral("http")
        || url.scheme() == QStringLiteral("https")) {
        QDesktopServices::openUrl(url);
        return;
    }

    // 自定义 myapp:// 协议的非图片链接（展示完整 scheme 处理）
    if (url.scheme() == QStringLiteral("myapp")) {
        setHtml(QStringLiteral(
            "<html><body>"
            "<h2>自定义协议链接</h2>"
            "<p>你点击了一个 myapp:// 协议的链接：</p>"
            "<pre>%1</pre>"
            "<p><a href='page1'>返回第一页</a></p>"
            "</body></html>").arg(url.toString()));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// 导航状态更新
// ─────────────────────────────────────────────────────────────────────────────

void CustomResourceBrowser::updateNavigationState()
{
    // 根据历史栈数量安全地启用/禁用导航按钮
    if (m_btnBack) {
        m_btnBack->setEnabled(backwardHistoryCount() > 0);
    }
    if (m_btnForward) {
        m_btnForward->setEnabled(forwardHistoryCount() > 0);
    }
    if (m_historyLabel) {
        m_historyLabel->setText(
            QStringLiteral("后退栈: %1 | 前进栈: %2")
                .arg(backwardHistoryCount())
                .arg(forwardHistoryCount()));
    }
}
