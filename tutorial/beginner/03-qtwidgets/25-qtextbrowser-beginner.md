# 现代Qt开发教程（新手篇）3.25——QTextBrowser：只读富文本浏览器

## 1. 前言 / 当 QTextEdit 变成只读，它还能做什么

我们刚刚在前两篇文章中分别拆解了 QTextEdit 和 QPlainTextEdit。QTextEdit 是完整的富文本编辑器，QPlainTextEdit 是纯文本性能优先的轻量替代。现在问题来了：如果你只需要"显示"富文本内容而不需要编辑呢？比如你的应用需要一个帮助文档查看器、一个"关于"页面、一个内嵌的 HTML 文档浏览器——这些场景下你不需要光标编辑、不需要格式化按钮、不需要修改状态追踪，你只需要一个能渲染 HTML 并让用户浏览的控件。如果你直接把 QTextEdit 设为 `setReadOnly(true)`，它确实能用，但你很快会发现缺少一些浏览器级别的能力——比如超链接点击导航、历史记录的前进后退、从文件加载内容。

Qt 对这个需求提供了一个专门的答案：QTextBrowser。QTextBrowser 继承自 QTextEdit，在 QTextEdit 的全部渲染能力之上增加了"浏览器"语义——超链接导航、历史记录栈、本地文件加载、只读保护。它本质上就是一个轻量级的嵌入式文档浏览器，适合在你的应用中显示帮助文档、 changelog、内嵌 HTML 页面等。我们今天来把 QTextBrowser 的四个核心能力讲清楚：HTML 和 Markdown 文档的显示、setSource 加载本地 HTML 文件、anchorClicked 信号处理链接点击、以及历史导航的前进后退机制。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QTextBrowser 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。QTextBrowser 继承自 QTextEdit，所以它拥有 QTextEdit 的全部 API——setHtml()、toHtml()、textCursor()、document() 等方法都可以直接调用。但 QTextBrowser 有两个关键差异：第一，它默认是只读的（readOnly 属性默认为 true）；第二，它增加了导航相关的方法和信号（backward()、forward()、home()、anchorClicked 等）。QTextBrowser 支持的 HTML 标签集合与 QTextEdit 完全一致——HTML 4 子集，不支持 JavaScript、CSS3、HTML5。

## 3. 核心概念讲解

### 3.1 显示 HTML / Markdown 格式文档

QTextBrowser 继承了 QTextEdit 的全部富文本渲染能力，所以你可以用完全相同的方式来设置内容。`setHtml(const QString &)` 设置 HTML 格式内容，`setMarkdown(const QString &)` 设置 Markdown 格式内容，`setPlainText(const QString &)` 设置纯文本内容。

```cpp
auto *browser = new QTextBrowser();

// 方式 1: 直接设置 HTML 内容
browser->setHtml(
    "<h1>应用帮助</h1>"
    "<p>这是一个 <b>富文本</b> 帮助文档查看器。</p>"
    "<p>你可以在这里显示 <i>格式化</i> 的文档内容。</p>"
    "<ul>"
    "  <li><a href='#intro'>简介</a></li>"
    "  <li><a href='#usage'>使用方法</a></li>"
    "  <li><a href='#faq'>常见问题</a></li>"
    "</ul>"
    "<h2 id='intro'>简介</h2>"
    "<p>本应用用于演示 QTextBrowser 的文档浏览能力。</p>"
    "<h2 id='usage'>使用方法</h2>"
    "<p>点击上方链接可以跳转到对应章节。</p>"
    "<h2 id='faq'>常见问题</h2>"
    "<p>暂无常见问题。</p>");

// 方式 2: 设置 Markdown 内容
browser->setMarkdown(
    "# 应用帮助\n"
    "\n"
    "这是一个 **富文本** 帮助文档查看器。\n"
    "\n"
    "## 简介\n"
    "\n"
    "本应用用于演示 QTextBrowser 的文档浏览能力。\n"
    "\n"
    "## 使用方法\n"
    "\n"
    "- 点击链接跳转\n"
    "- 使用导航按钮前进/后退\n");
```

QTextBrowser 默认的 `readOnly` 属性为 true——用户无法直接编辑内容。但如果你硬要，可以通过 `setReadOnly(false)` 把它变成可编辑的。不过这样做就失去了使用 QTextBrowser 而不是 QTextEdit 的意义——因为 QTextBrowser 的导航功能（历史栈、链接跳转）在可编辑模式下仍然可用，但用户体验会很怪异，一般没人这么干。

QTextBrowser 相比 QTextEdit 的一个重要增强是"超链接可点击"。在 QTextEdit 中，即使你设置了包含 `<a href="...">` 标签的 HTML，链接默认也是不可点击的——它只是把文字渲染成蓝色带下划线的样式，点击不会有任何反应（除非你在 QTextEdit 上设置了 `anchorClick` 相关的信号处理）。而 QTextBrowser 默认就会响应链接点击——它会尝试导航到链接指向的目标。这个"导航"行为的细节我们后面会详细讲。

### 3.2 setSource() 加载本地 HTML 文件

除了用 `setHtml()` 在代码中硬编码内容之外，QTextBrowser 还提供了 `setSource(const QUrl &)` 方法来直接加载外部文件。这是 QTextBrowser 的一个核心能力——它可以像浏览器一样加载 URL 指向的资源。

```cpp
auto *browser = new QTextBrowser();

// 加载本地 HTML 文件
browser->setSource(QUrl::fromLocalFile("/path/to/help/index.html"));

// 加载 Qt 资源系统中的文件
browser->setSource(QUrl("qrc:/docs/about.html"));
```

`setSource()` 的行为值得仔细说说。当你传入一个本地文件的 URL 时，QTextBrowser 会读取文件内容，根据文件扩展名或内容来决定如何渲染。如果文件是 `.html` 或 `.htm`，它会用 HTML 模式解析；如果是 `.md`，它会用 Markdown 模式解析；如果是 `.txt`，它会按纯文本显示。

加载完成后，QTextBrowser 会把当前 URL 记录到历史栈中（我们后面讲导航的时候会用到这个栈）。如果文件中有相对路径的链接（比如 `<a href="page2.html">`），点击这个链接时 QTextBrowser 会基于当前 URL 的路径来解析相对路径——和真正的浏览器行为一致。这意味着你可以创建一个多页面的本地文档系统，用 QTextBrowser 来浏览，链接导航完全自动处理。

```cpp
// 假设你有如下文件结构:
// /docs/
//   index.html      ← 主页
//   guide.html      ← 使用指南
//   faq.html        ← 常见问题

auto *browser = new QTextBrowser();
browser->setSearchPaths({"/docs"});  // 设置搜索路径
browser->setSource(QUrl::fromLocalFile("/docs/index.html"));

// index.html 中的 <a href="guide.html"> 链接会被自动解析为
// /docs/guide.html，点击后自动导航到该页面
```

`setSearchPaths(const QStringList &)` 方法设置了 QTextBrowser 在解析相对路径链接时的搜索目录列表。当你调用 `setSource()` 加载一个文件后，文件所在目录会自动被加入搜索路径。但如果你在其他地方（比如通过 `setHtml()` 设置的内容）引用了相对路径的文件，就需要手动设置搜索路径。

有一个很容易踩的坑：`setSource()` 在加载失败时不会抛出异常或弹窗。如果文件不存在或路径错误，QTextBrowser 会变成空白页面。你可以通过 `source()` 方法获取当前加载的 URL，通过 `sourceChanged(const QUrl &)` 信号监听 URL 变化，在槽函数中检查文件是否存在来处理加载失败的情况。

另外一点：`setSource()` 是异步的吗？不是。QTextBrowser 的 `setSource()` 是同步操作——它会在调用时立即读取文件内容并渲染。所以不要在主线程上加载特别大的文件，否则界面会卡住。不过在桌面应用的典型场景中（帮助文档、关于页面），HTML 文件通常不会太大，同步加载完全可以接受。

### 3.3 anchorClicked 信号处理链接点击

QTextBrowser 提供了一个 `anchorClicked(const QUrl &)` 信号，在用户点击 HTML 中的超链接时触发。这个信号的参数是链接指向的 QUrl 对象。

在默认情况下，QTextBrowser 对链接点击的处理是"导航到链接指向的页面"——如果链接指向的是本地 HTML 文件，QTextBrowser 会加载并显示该文件；如果链接指向的是同一个文档内的锚点（比如 `#intro`），QTextBrowser 会滚动到对应的锚点位置。这个默认行为覆盖了大部分文档浏览器的需求，你不需要写任何代码。

但有些场景下你可能需要拦截链接点击，执行自定义操作。比如链接指向的是一个外部网站——QTextBrowser 无法渲染外部网页（它不是真正的浏览器），你希望用系统默认浏览器打开外部链接。或者链接携带了自定义协议（比如 `action://open-settings`），你希望根据协议内容执行特定的操作。这时候你需要在 `anchorClicked` 信号的槽函数中进行拦截处理。

```cpp
auto *browser = new QTextBrowser();

// 拦截链接点击: 外部链接用系统浏览器打开
connect(browser, &QTextBrowser::anchorClicked,
        this, [](const QUrl &url) {
    if (url.scheme() == "http" || url.scheme() == "https") {
        // 用系统默认浏览器打开外部链接
        QDesktopServices::openUrl(url);
    }
    // 内部锚点链接和本地文件链接交给 QTextBrowser 自己处理
});
```

不过这里有一个微妙的行为需要理解。`anchorClicked` 信号在 QTextBrowser 执行默认导航之前触发。也就是说，信号触发后 QTextBrowser 仍然会尝试导航到链接指向的 URL。如果你希望完全阻止默认导航行为，需要在信号处理中设置一个标志位，然后重写 `QTextBrowser::loadResource` 或使用事件过滤器来阻止后续的加载操作。说实话这个设计有点让人头疼——Qt 没有提供一个"取消导航"的直接机制。

一个更实用的做法是在 `anchorClicked` 的槽函数中，根据 URL 的类型决定是否手动调用 `setSource()` 来覆盖导航目标：

```cpp
connect(browser, &QTextBrowser::anchorClicked,
        this, [this, browser](const QUrl &url) {
    if (url.scheme() == "action") {
        // 自定义协议: 执行操作而不是导航
        if (url.host() == "open-settings") {
            openSettingsDialog();
        } else if (url.host() == "check-update") {
            checkForUpdates();
        }
        // 注意: QTextBrowser 仍会尝试导航到 action://...
        // 但因为这个 URL 不指向任何文件，结果就是页面变成空白
        // 可以在操作完成后重新设置原来的内容来避免空白
    }
});
```

与 `anchorClicked` 相关的还有两个信号：`backwardAvailable(bool)` 和 `forwardAvailable(bool)`。它们在历史栈的状态发生变化时触发——参数为 true 表示有可用的后退/前进目标，false 表示没有。这两个信号通常用来控制导航按钮的启用/禁用状态。

### 3.4 历史导航：backward() / forward() / home()

QTextBrowser 内部维护了一个"历史栈"，记录了用户通过链接点击或 `setSource()` 访问过的 URL 序列。这个历史栈和浏览器的"后退/前进"按钮是同一个概念——QTextBrowser 提供了三个方法来操作这个栈。

`backward()` 导航到历史栈中的上一个 URL。如果当前是栈顶，调用 backward 后会回到上一个访问的页面。`forward()` 导航到历史栈中的下一个 URL。如果当前没有"前进"目标（因为当前已经在栈的最新位置），forward 不会做任何事。`home()` 导航到历史栈中的第一个 URL——也就是用户在这个 QTextBrowser 中访问的第一个页面。

```cpp
// 导航按钮
auto *backBtn = new QPushButton("后退");
auto *forwardBtn = new QPushButton("前进");
auto *homeBtn = new QPushButton("首页");

connect(backBtn, &QPushButton::clicked, browser, &QTextBrowser::backward);
connect(forwardBtn, &QPushButton::clicked, browser, &QTextBrowser::forward);
connect(homeBtn, &QPushButton::clicked, browser, &QTextBrowser::home);

// 根据历史栈状态控制按钮启用/禁用
connect(browser, &QTextBrowser::backwardAvailable,
        backBtn, &QPushButton::setEnabled);
connect(browser, &QTextBrowser::forwardAvailable,
        forwardBtn, &QPushButton::setEnabled);
```

这个历史栈的运作方式和真正的浏览器非常类似。假设用户依次访问了 A -> B -> C 三个页面，此时历史栈为 [A, B, C]，当前在 C。点击 backward 后回到 B，栈仍然是 [A, B, C]，但当前位置变成了 B。再点击 forward 回到 C。如果从 B 点击 backward 回到 A，再点击一个新链接 D，那么 C 会从栈中被移除，栈变成 [A, B, D]——这和浏览器的行为完全一致。

`backwardAvailable(bool)` 和 `forwardAvailable(bool)` 信号在栈状态变化时触发。在上面的代码中，我们把这两个信号直接连接到按钮的 `setEnabled` 槽——当没有可后退的目标时后退按钮变灰，没有可前进的目标时前进按钮变灰。这是一个非常干净的实现模式，不需要你手动维护栈状态。

还有一个 `sourceChanged(const QUrl &)` 信号，在当前显示的文档 URL 发生变化时触发（不管是通过 `setSource()` 加载还是通过 `backward()`/`forward()` 导航）。你可以用这个信号来更新地址栏显示或窗口标题。

```cpp
connect(browser, &QTextBrowser::sourceChanged,
        this, [this](const QUrl &url) {
    setWindowTitle(QString("文档浏览器 — %1").arg(url.fileName()));
});
```

`historyTitle(int)` 和 `historyUrl(int)` 方法可以让你查看历史栈中指定位置的标题和 URL。`backwardHistoryCount()` 和 `forwardHistoryCount()` 分别返回后退方向和前进方向上有多少个历史条目。这些 API 在你需要构建一个完整的历史记录面板时很有用。

最后一个细节：`home()` 导航到的"首页"是什么？它是历史栈中的第一个条目——也就是通过 `setSource()` 加载的第一个 URL。如果你在创建 QTextBrowser 后没有调用过 `setSource()`，而是用 `setHtml()` 设置了内容，那么 home() 不会导航到任何地方（因为历史栈为空）。所以如果你计划使用 home() 功能，请确保通过 `setSource()` 来设置初始内容。

## 4. 踩坑预防

第一个坑是 `setSource()` 加载失败时不会报错。文件不存在或路径错误只会导致空白页面。建议在 `sourceChanged` 信号中检查 URL 是否有效，或者在调用 `setSource()` 之前先判断文件是否存在。

第二个坑是 `anchorClicked` 信号不能取消默认导航。QTextBrowser 在发出 `anchorClicked` 信号后仍然会执行导航。如果你需要拦截某些链接不让 QTextBrowser 导航，需要在槽函数中做额外的处理（比如重新设置内容覆盖导航结果）。

第三个坑是外部链接无法在 QTextBrowser 中打开。QTextBrowser 不是真正的浏览器，它无法加载和渲染互联网上的网页。如果你的文档中有外部链接，需要通过 `anchorClicked` 信号拦截并用 `QDesktopServices::openUrl()` 在系统浏览器中打开。

第四个坑是用 `setHtml()` 设置内容后 `home()` 无效。`home()` 导航到的是历史栈的第一个条目，而 `setHtml()` 不会向历史栈中添加条目。如果需要 home() 功能，请使用 `setSource()` 加载初始内容。

第五个坑是 QTextBrowser 渲染的是 HTML 4 子集，不是完整的 HTML5。它不支持 `<video>`、`<audio>`、`<canvas>`、CSS3 动画、JavaScript 等现代 Web 特性。如果你的帮助文档需要复杂的交互和布局，考虑嵌入 Qt WebEngine 而不是用 QTextBrowser。

## 5. 练习项目

我们来做一个综合练习：创建一个"迷你帮助文档浏览器"窗口，展示 QTextBrowser 的文档浏览和导航能力。窗口顶部有一排导航按钮——"后退"、"前进"、"首页"三个按钮，根据历史栈的状态自动启用或禁用。旁边还有一个标签显示当前文档标题（从 sourceChanged 信号获取 URL 文件名）。窗口中央是 QTextBrowser 作为文档显示区域。我们不用外部文件，而是通过 `setHtml()` 在代码中构建一个多页面的文档内容——第一页是一个目录页，包含到各章节的锚点链接。窗口中用三个按钮来切换不同"页面"的内容（模拟 `setSource()` 的效果，但由于我们不用外部文件，所以用 `setHtml()` 来模拟页面切换，同时手动维护历史栈的效果）。

几个提示：导航按钮通过 `backwardAvailable` 和 `forwardAvailable` 信号控制启用状态；页面切换按钮调用 `setHtml()` 更新内容；由于我们用 `setHtml()` 而不是 `setSource()`，所以需要自己维护一个历史记录列表来模拟后退/前进功能，或者更好的做法是把帮助内容写成实际的 HTML 文件放在构建目录中，通过 `setSource()` 加载，这样 QTextBrowser 的历史导航就是完全自动的。

## 6. 官方文档参考链接

[Qt 文档 · QTextBrowser](https://doc.qt.io/qt-6/qtextbrowser.html) -- 只读富文本浏览器

[Qt 文档 · QTextEdit](https://doc.qt.io/qt-6/qtextedit.html) -- 富文本编辑器（QTextBrowser 的父类）

[Qt 文档 · QUrl](https://doc.qt.io/qt-6/qurl.html) -- URL 处理

[Qt 文档 · QDesktopServices](https://doc.qt.io/qt-6/qdesktopservices.html) -- 系统服务（打开外部链接）

---

到这里，QTextBrowser 的四个核心维度我们就全部讲完了。它是 QTextEdit 的只读版本，在富文本渲染能力之上增加了"浏览器"语义——超链接可点击、可导航。`setSource()` 可以直接加载本地 HTML 文件并自动处理相对路径链接。`anchorClicked` 信号让你可以拦截链接点击执行自定义操作，但要注意它不能取消默认导航。`backward()` / `forward()` / `home()` 提供了完整的历史导航能力，配合 `backwardAvailable` 和 `forwardAvailable` 信号可以轻松构建导航工具栏。如果你的应用需要一个轻量级的嵌入式文档浏览器，QTextBrowser 是最直接的选择；如果需要完整的 Web 渲染能力，请转向 Qt WebEngine。
