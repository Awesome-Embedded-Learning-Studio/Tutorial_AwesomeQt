---
title: "3.25 QTextBrowser 进阶"
description: "入门篇我们知道了 QTextBrowser 是 QTextEdit 的只读版本，能加载 HTML、处理链接点击、做历史导航。但到了工程实践里，光靠 setSource + anchorClicked 很快就会撞墙——自定义协议的资源加载不会触发 loadResource、空历史栈上调 backward 直接崩溃、setOpenExternalLinks 一开 anchorClicked 就被拦截。"
---

# 现代Qt开发教程（进阶篇）3.25——QTextBrowser 进阶

## 1. 前言 / 为什么入门篇那点知识撑不住真实项目

入门篇我们把 QTextBrowser 当一个"能点链接的 QTextEdit"来用——setSource 加载 HTML，anchorClicked 拦截外部链接，backward/forward 做导航。说实话，做一个简单的"关于"页面或者几十页的离线帮助文档，这些知识确实够用了。但如果你尝试用 QTextBrowser 做一个稍微正式的文档系统——比如支持自定义资源协议（从数据库或网络加载图片）、支持自定义链接处理逻辑（点击某类链接弹对话框而不是导航）、需要精确控制历史栈——你会发现入门篇的那些 API 突然不好使了。loadResource 对自定义 scheme 没反应，backward() 在空历史栈上直接让程序崩溃，setOpenExternalLinks 设成 true 之后 anchorClicked 信号就不发了。这三个坑每一个都够调试半天的。我们今天把这三块进阶内容拆透：loadResource 资源加载覆写的完整机制、历史栈的内部结构和安全操作方式、以及 anchorClicked 与 setOpenExternalLinks 之间的微妙关系。

## 2. 环境说明

本篇基于 Qt 6.5+ 版本，CMake 3.26+，C++17 标准。QTextBrowser 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。涉及的网络相关类（QNetworkAccessManager 等）属于 QtNetwork 模块，如果示例中用到网络加载需要额外链接 Qt6::Network。

## 3. 核心概念讲解

### 3.1 loadResource 资源加载覆写

QTextBrowser 在渲染文档时需要加载各种资源——HTML 中引用的图片、CSS 样式表、甚至是 iframe 内嵌的子文档。默认情况下，它只能加载本地文件和 Qt 资源系统（qrc）中的资源。但实际项目中你可能需要从网络、数据库或者自定义协议加载资源。这时候就需要覆写 `loadResource(int type, const QUrl &name)` 方法。

loadResource 的 type 参数标识资源类型，对应 QTextDocument::ResourceType 枚举：QTextDocument::HtmlResource、QTextDocument::ImageResource、QTextDocument::StyleSheetResource、QTextDocument::MarkdownResource 等。name 参数就是资源的 URL。你需要根据 type 和 name 返回对应的 QVariant——图片返回 QImage 或 QPixmap，HTML 返回 QString，CSS 返回 QString。

```cpp
class CustomTextBrowser : public QTextBrowser
{
protected:
    QVariant loadResource(int type, const QUrl &name) override
    {
        if (name.scheme() == "dbimg") {
            // 从数据库加载图片: dbimg://product/12345
            auto image = loadFromDatabase(name.host(), name.path());
            if (!image.isNull()) {
                return QVariant(image);
            }
        }
        // 回退到默认实现处理本地文件和 qrc
        return QTextBrowser::loadResource(type, name);
    }
};
```

这里有一个关键细节需要注意：loadResource 只对 QTextBrowser 能够识别的 URL scheme 生效。具体来说，当 QTextBrowser 遇到一个 `<img src="dbimg://product/12345">` 标签时，它会尝试调用 loadResource 来获取图片数据。但如果你用的是 `setSource()` 加载 HTML 文件，而文件中引用了自定义 scheme 的资源，QTextBrowser 有时会在调用 loadResource 之前就因为无法解析 scheme 而跳过加载。解决方案是确保 loadResource 的覆写逻辑足够健壮，同时不要在 setSource 的文件路径本身使用自定义 scheme——只在文档内部引用资源时使用。

另一个实战场景是延迟加载网络图片。QTextBrowser 的 loadResource 是同步调用——它必须立即返回结果。但网络请求是异步的。这时候的通用做法是在 loadResource 中先返回一个占位图片，同时发起异步网络请求，请求完成后再通过 QTextDocument 的 `addResource()` 方法把真实图片注入文档，然后触发文档刷新。

```cpp
QVariant loadResource(int type, const QUrl &name) override
{
    if (type == QTextDocument::ImageResource && name.scheme() == "https") {
        // 先返回占位图
        auto placeholder = QPixmap(kPlaceholderSize, kPlaceholderSize);
        placeholder.fill(Qt::lightGray);

        // 异步加载真实图片
        auto *reply = m_network_mgr->get(QNetworkRequest(name));
        connect(reply, &QNetworkReply::finished, this, [this, name, reply]() {
            QImage img;
            img.loadFromData(reply->readAll());
            reply->deleteLater();
            if (!img.isNull()) {
                document()->addResource(QTextDocument::ImageResource, name, QVariant(img));
                // 触发重绘
                viewport()->update();
            }
        });
        return QVariant(placeholder);
    }
    return QTextBrowser::loadResource(type, name);
}
```

这种"占位 + 异步回填"的模式在需要展示网络图片的文档浏览器中非常实用，但要注意内存管理——QNetworkReply 必须在完成后 deleteLater，否则会泄漏。

### 3.2 历史栈导航的内部机制与安全操作

入门篇我们用了 backward()、forward()、home() 三个方法和 backwardAvailable、forwardAvailable 两个信号来构建导航。但入门篇没有讲的是：这些方法在边界条件下的行为非常危险。

QTextBrowser 内部维护了一个 QVector<Entry> 形式的历史栈，每个 Entry 记录了 URL 和对应的文档标题。当前浏览位置用一个索引值来标识。backwardAvailable 和 forwardAvailable 信号在索引变化时触发。这个设计本身没问题，但问题在于：如果你在历史栈为空时（比如刚创建 QTextBrowser，还没调过 setSource）直接调用 backward()，在 Qt 6 的某些版本中会导致越界访问，程序直接崩溃。这不是理论上的危险，是我在实际项目中踩过的雷——一个"帮助"按钮创建了一个 QTextBrowser，用户习惯性地按了 Alt+Left（后退快捷键），程序当场去世。

安全操作历史栈的方式是在调用 backward/forward 之前先检查栈状态。QTextBrowser 提供了 `backwardHistoryCount()` 和 `forwardHistoryCount()` 两个方法，分别返回向后和向前的历史条目数量。只有 count > 0 时才调用对应的方法：

```cpp
void safeBackward(QTextBrowser *browser)
{
    if (browser->backwardHistoryCount() > 0) {
        browser->backward();
    }
}

void safeForward(QTextBrowser *browser)
{
    if (browser->forwardHistoryCount() > 0) {
        browser->forward();
    }
}
```

如果你给 QTextBrowser 配了键盘快捷键（比如 Alt+Left 后退、Alt+Right 前进），一定要在快捷键的槽函数中做这个检查，而不是直接 connect 到 backward/forward。backwardAvailable/forwardAvailable 信号可以安全地用来控制按钮的 enabled 状态（因为 Qt 内部做了保护），但直接调用 backward/forward 方法没有保护。

另外还有一个容易忽略的行为：当你在历史栈中间位置（不是栈顶）调用 setSource 加载一个新页面时，当前位置之后的所有历史条目会被清除，新页面被追加到当前位置后面。这和浏览器的行为一致——你在 B 页面上打开了新页面 D，C 就从历史中消失了。如果你需要保留完整的历史记录（比如做文档阅读器的阅读历史），就需要在 setSource 之前手动保存当前历史，或者自己维护一套独立的历史数据结构。

`historyTitle(int)` 和 `historyUrl(int)` 方法接受一个相对索引参数——负数表示后退方向的历史（-1 是上一个页面，-2 是上上个页面），正数表示前进方向（1 是下一个页面）。historyTitle 返回的是文档的 `<title>` 标签内容或者 URL 的文件名。

### 3.3 anchorClicked 与 setOpenExternalLinks 的关系

入门篇我们用 anchorClicked 信号拦截链接点击。但这里有一个非常容易踩坑的地方：`setOpenExternalLinks(bool)` 属性。这个属性的语义是"当用户点击一个外部链接（http/https 协议）时，自动用系统默认浏览器打开"。听起来很方便，但它的副作用很少被提及：当你把 setOpenExternalLinks 设为 true 之后，QTextBrowser 会自动拦截所有 http/https 链接的点击事件，直接调用 QDesktopServices::openUrl，而不会发出 anchorClicked 信号。

这意味着你失去了对 http/https 链接的完全控制权。如果你想在打开外部链接之前做一些处理（比如记录用户点击了哪些外部链接、检查 URL 是否安全、或者根据内容决定是用内嵌浏览器还是外部浏览器打开），就不能用 setOpenExternalLinks(true)。

正确的做法是保持 setOpenExternalLinks 为 false（默认值），自己在 anchorClicked 的槽函数中处理所有链接类型：

```cpp
browser->setOpenExternalLinks(false);
browser->setOpenLinks(false);  // 同时禁用默认的内部导航

connect(browser, &QTextBrowser::anchorClicked,
        this, [this](const QUrl &url) {
    if (url.scheme() == "http" || url.scheme() == "https") {
        QDesktopServices::openUrl(url);
    } else if (url.scheme() == "action") {
        handleCustomAction(url);
    } else if (url.isRelative() || url.scheme() == "file") {
        // 内部导航：手动调用 setSource
        browser->setSource(url);
    }
});
```

注意这里还用了 `setOpenLinks(false)`。setOpenLinks 控制 QTextBrowser 是否自动处理链接点击的导航行为——设为 false 后，QTextBrowser 不会在用户点击链接时自动调用 setSource，所有导航逻辑完全由你的 anchorClicked 槽函数控制。这是一个更干净的设计：你明确知道每种类别的链接会怎么处理，不会有隐含的默认行为跳出来捣乱。

现在有一道思考题。如果你的 QTextBrowser 同时设置了 setOpenExternalLinks(true) 和 setOpenLinks(false)，用户点击一个 http 链接时会发生什么？答案是：setOpenLinks(false) 会阻止所有链接导航，包括外部链接的自动打开。也就是说 setOpenLinks(false) 的优先级高于 setOpenExternalLinks(true)。两个属性的组合效果是：没有任何自动导航行为。所以如果你要手动控制导航，只设 setOpenLinks(false) 就够了，不需要同时操作 setOpenExternalLinks。

## 4. 踩坑预防

第一个坑是 backward() 在空历史栈上可能崩溃。QTextBrowser 内部的 QVector<Entry> 在空状态下访问会产生越界。后果是程序直接 segfault，没有异常可以捕获。解决方案是在调用 backward/forward/home 之前，始终通过 backwardHistoryCount/forwardHistoryCount 检查栈状态。如果你通过快捷键触发导航，槽函数中必须做这个检查。

第二个坑是 loadResource 对自定义 scheme 的资源不总是被调用。当 setSource 加载的 HTML 文件中引用了自定义 scheme（比如 dbimg://）的图片时，某些情况下 QTextBrowser 会在调用 loadResource 之前就判定 URL 无效而跳过加载。解决方案是在 HTML 中使用相对路径配合 setSearchPaths，或者对自定义 scheme 的资源使用完整的 loadResource 覆写并在其中处理所有可能的路径格式。

第三个坑是 setOpenExternalLinks(true) 会阻止 http/https 链接的 anchorClicked 信号发射。如果你依赖 anchorClicked 来记录或拦截外部链接，开启这个属性后你的逻辑就不会被执行了。后果是外部链接"静默"地被系统浏览器打开，你没有任何机会做中间处理。解决方案是不使用 setOpenExternalLinks，自己在 anchorClicked 槽函数中处理所有链接类型。

## 5. 练习项目

练习项目：增强型文档浏览器。我们要实现一个支持自定义图片资源协议的 QTextBrowser，核心功能是从一个模拟的数据库（用 QMap 代替）加载图片并显示在文档中。

具体要求是：继承 QTextBrowser 创建 CustomTextBrowser，覆写 loadResource 处理 "imgdb://" 协议的图片请求；准备若干张测试图片存入 QMap<QString, QImage> 作为模拟数据库；构建一个 HTML 文档，其中用 `<img src="imgdb://image1">` 引用这些图片；实现安全的历史导航（后退、前进按钮在空栈时禁用）；所有外部链接通过 anchorClicked 在系统浏览器中打开，内部链接由 QTextBrowser 处理。完成标准是文档中的自定义协议图片能正确显示，历史导航不会崩溃，外部链接不会在 QTextBrowser 内部打开。

提示几个关键点：loadResource 的 type 参数为 QTextDocument::ImageResource 时表示图片资源；文档中的 imgdb:// 链接会被传入 loadResource 的 name 参数；setOpenLinks(false) 配合手动 anchorClicked 处理是最可控的方案。

## 6. 官方文档参考链接

[Qt 文档 · QTextBrowser](https://doc.qt.io/qt-6/qtextbrowser.html) -- 只读富文本浏览器，包含 loadResource、历史栈、 setOpenLinks 等进阶 API

[Qt 文档 · QTextDocument](https://doc.qt.io/qt-6/qtextdocument.html) -- 底层文档模型，ResourceType 枚举定义了 loadResource 的 type 参数

[Qt 文档 · QTextDocument::ResourceType](https://doc.qt.io/qt-6/qtextdocument.html#ResourceType-enum) -- 资源类型枚举值

[Qt 文档 · QDesktopServices](https://doc.qt.io/qt-6/qdesktopservices.html) -- 系统服务，openUrl 用于打开外部链接

---

到这里，QTextBrowser 的进阶内容我们就过了一遍。loadResource 覆写让我们可以自由控制文档中的资源来源——从数据库、网络、或者任何自定义协议加载图片和样式。历史栈的安全操作原则是"先检查后调用"，backwardHistoryCount/forwardHistoryCount 是保护程序不崩溃的关键。anchorClicked 和 setOpenExternalLinks 之间的关系搞清楚了，以后做链接拦截就不会踩坑了。下一篇我们来看 QKeySequenceEdit 的进阶用法。
