---
title: "3.25 QTextBrowser 进阶"
description: "入门篇把 setSource 加载 HTML、anchorClicked 拦链接与 backward 前进后退过了一遍，进阶篇从资源装载这一层讲起，覆写 loadResource 接管自定义协议与异步回填，摸清历史栈的两根栈结构与导航方法自带的保险，把链接点击的分流次序从头捋到尾，配套示例在 examples/advanced/03-qtwidgets/25-qtextbrowser-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.25——QTextBrowser 进阶

[3.24 QPlainTextEdit 进阶](./24-qplaintextedit-advanced.md) 收尾把话递到了第四站，[3.23 QTextEdit 进阶](./23-qtextedit-advanced.md) 还埋过一颗种子，setAnchor 只给文字挂锚点记号，anchorClicked 与 openExternalLinks 都长在 QTextBrowser 上，本篇就把这个交代兑现。QTextBrowser 是 QTextEdit 的只读子类，控件壳与文档树照旧，比 QTextEdit 多出来的是三样，锚点的点击响应、历史导航、资源装载。入门篇把 setSource 加载 HTML、anchorClicked 拦外部链接、backward 与 forward 前进后退过了一遍，做个“关于”页面或者几十页的离线帮助够用。需求一旦走进正式些的文档系统，要从数据库取图片、要精确控制历史栈、要给每类链接各自安排去处，那几样就不够了，咱们把这三层分别看过去。

咱们按这样的次序走，资源装载怎么收在 loadResource 一个覆写里，网络图片的异步回填怎么落地，接着是历史栈的结构与边界行为，链接点击的分流次序收尾。正文里的行号都带着出处，示例代码在 examples/advanced/03-qtwidgets/25-qtextbrowser-advanced/，Qt 内部实现指源码树里的 qtextbrowser.cpp 与 qtextdocument.cpp。

## 覆写 loadResource，资源来源就归您管

咱们从默认行为看起。QTextBrowser 渲染文档要用到文档外面的东西，`<img>` 引用的图片、样式表、内嵌子文档都算，基类的默认实现把 name 当文件名处理（qtextbrowser.cpp 第 1138-1154 行），非绝对路径会在 searchPaths 属性列出的目录里找，找不到再去当前 source 所在的目录，读回来的是 QByteArray，本地文件与 qrc 资源都从这里走，官方注释还写明这个默认实现连 type 参数都不看。要从数据库、网络或任何自定义协议取资源，覆写 loadResource(int type, const QUrl &name)，type 标资源类型，取值是 QTextDocument::ResourceType，name 是资源自己的 URL，返回的 QVariant 按类型给，对应关系源码注释里有一张表（第 1131-1136 行）：

| ResourceType 取值 | loadResource 返回的 QVariant |
| ---- | ---- |
| HtmlResource | QString 或 QByteArray |
| ImageResource | QImage、QPixmap 或 QByteArray |
| StyleSheetResource | QString 或 QByteArray |
| MarkdownResource | QString 或 QByteArray |

```cpp
// 子类 protected 段里覆写，没命中的请求回退基类
QVariant loadResource(int type, const QUrl &name) override
{
    if (type == QTextDocument::ImageResource
        && name.scheme() == QLatin1String("dbimg")) {
        // dbimg://product/12345，host 是 product，path 是 /12345
        QImage img = loadFromDatabase(name.host(), name.path());
        if (!img.isNull())
            return QVariant(img);
        return QVariant(grayPlaceholder());           // 找不到给一枚灰色占位
    }
    return QTextBrowser::loadResource(type, name);    // 本地与 qrc 照旧
}
```

流传的说法是，自定义 scheme 的资源有时会在 loadResource 之前被跳过，咱们对着源码把卡点找出来，scheme 白名单并不存在。文档本体走 setSource，装载入口无条件问一次 loadResource（第 291 行），拿回 QString 或 QByteArray 才交给 setHtml（第 344 行），`<img>` 这类文档内引用的请求从排版层转发进来（qtextedit.cpp 第 76-83 行的钩子把资源请求转给控件的 loadResource），两处都不认 scheme。真会把您拦住的是另外两处。一处是同 URL 短路，setSource 的装载条件是新旧 URL 去掉 fragment 之后不相等（第 289-290 行），同一个地址装第二遍不会重新装载，reload() 就为这事存在，它把 currentURL 暂清，回头再调一次 setSource（第 785-791 行）。另一处是 baseUrl 改写，文档装载时 baseUrl 被设成文档 URL 去掉文件名的部分（第 333-335 行），QTextDocument::resource 拿它把相对 src 解析成绝对 URL，出处是 qtextdocument.cpp 第 2152 行。所以自定义 scheme 放在文档内部的资源引用里没问题，文档本体的路径用本地文件，或者相对路径配 setSearchPaths（第 776-780 行），更省事。

## 网络图片异步回填，addResource 站在查找链最前面

loadResource 是同步调用，排版等在那一行上，必须立刻返回 QVariant，网络请求偏偏是异步的，两头对不上。咱们通行的写法是占位图顶在前面，真图到了再回填：

```cpp
QVariant loadResource(int type, const QUrl &name) override
{
    if (type == QTextDocument::ImageResource && name.scheme() == QLatin1String("https")) {
        QPixmap placeholder(kSize, kSize);              // 占位图顶住排版
        placeholder.fill(Qt::lightGray);
        auto *reply = m_networkMgr->get(QNetworkRequest(name));
        connect(reply, &QNetworkReply::finished, this, [this, name, reply] {
            QImage img;
            img.loadFromData(reply->readAll());
            reply->deleteLater();                       // 不删就漏
            if (img.isNull())
                return;
            document()->addResource(QTextDocument::ImageResource, name, img);
            viewport()->update();                       // 触发一次重绘
        });
        return placeholder;
    }
    return QTextBrowser::loadResource(type, name);
}
```

回填为什么生效，查找次序说了算。图片绘制时问的是 QTextDocument::resource。它头一环查 addResource 注入的 resources 表，出处是 qtextdocument.cpp 第 2153 行，跟着查内部缓存（第 2155 行），最后轮到 loadResource（第 2157 行）。addResource 存的永远排在最前头，真图一进表，之后每次绘制拿到的都是它。占位图的尺寸要跟真图对上，布局按装载那一刻拿到的图算尺寸，占住的那块不会因为您换了图自己改。QNetworkReply 用完必须 deleteLater，这里最容易漏，一次点击一张图，泄漏按点击数累积。

## 历史栈是两根栈，导航方法自带保险

历史在私有类里是两根 QStack<HistoryEntry>（qtextbrowser.cpp 第 87-88 行），stack 存浏览过的页面，forwardStack 存后退之后待前进的页面，每个条目记 URL，记文档类型，记 documentTitle，连横向纵向滚动位置也一起存（第 832-837 行）。backward、forward、home 在 Qt 6.9.1 里全带保险，backward 查 stack.size() <= 1 就返回（第 918 行），forward 查 forwardStack 为空（第 940 行），home 查记录的首页 URL 是否有效（第 960 行），官方注释把行为写明，没有上一页就什么都不做。流传很广的说法是空栈上调 backward() 直接崩溃，在这个版本站不住，咱们对着源码把这句改掉，直接调用不崩。

检查照样要写，理由在判定的讲究上。backward 的可达性取 stack.size() > 1，首页本身占一条，栈非空不代表能退，isBackwardAvailable 的实现就是这个式子（第 1165-1169 行），isForwardAvailable 看 forwardStack 是否为空，出处见第 1179-1183 行。backwardHistoryCount 与 forwardHistoryCount 各自报数，一个返回 stack.size() - 1（第 1263-1266 行），另一个返回 forwardStack.size()（第 1252-1255 行）。您要列历史面板的话，historyTitle 与 historyUrl 按相对索引取条目，负数往回数，0 是当前页，正数往前数，标题取条目里存的 documentTitle，historyTitle 出处见第 1240-1244 行，historyUrl 在第 1220-1224 行。

您给浏览器配 Alt+Left 快捷键时，槽里查过 backwardHistoryCount() 再调 backward()，按钮的 enabled 交给 backwardAvailable 与 forwardAvailable 两条信号，两条路都通着，按钮和状态就永远跟栈对得上。示例的导航栏是现成的样板，按钮连的是带检查的 safeBackward 与 safeForward，出处是 custom_resource_browser.cpp 第 77-78 行，updateNavigationState 按两个 count 摆按钮与标签（第 270-285 行），sourceChanged 一发（qtextbrowser.cpp 第 371 行，装载收尾处）就刷新一遍（第 39-41 行），backwardAvailable 与 forwardAvailable 到来时也各刷一遍（第 46-49 行），两个信号都在压栈之后才发（qtextbrowser.cpp 第 840 与 844 行），一次导航之内按钮和标签就能走到位，不用等下一趟。

## 中途 setSource，身后的历史就被清掉

历史栈的维护集中在 doSetSource 一处，出处见 qtextbrowser.cpp 第 814-851 行。装载前给当前页拍一张快照，装载后把栈顶换成快照本身（第 830 行），再把新页面压进 stack（第 838 行），然后处理 forwardStack，新 URL 恰好等于前进栈顶时只弹出前进栈的栈顶（第 842-844 行），其余情况整个清空（第 846-847 行）。这跟您熟悉的浏览器一致，停在 B 页点开 D，C 就从历史里消失了。要做阅读历史，想把完整轨迹留下来，就在 setSource 之前，用 historyUrl 与 historyTitle 把条目复制出来，放进自己维护的结构里。

有一个误判从示例里冒出来，得当场说清。示例的第二页切换用的是 setHtml，出处是 custom_resource_browser.cpp 第 239 与 243 行，QTextBrowser 没有重载 setHtml，QTextEdit::setHtml 也不是虚函数，改文档绕开了 doSetSource，历史栈纹丝不动，您跑起来会看到底部标签的后退栈恒为 -1，上一节那个 stack.size() - 1 在空栈上算出来的就是它，后退按钮一直是灰的。机制没错，是装载方式没走历史入口。想让历史动起来，装载得走 setSource，文档本体照样可以出自 loadResource，setSource 装的 HTML 本来就从那里来（第 291 行），给覆写加一个 HtmlResource 分支，把 myapp://page1 与 myapp://page2 这样的文档 URL 也从内存里给出去，历史、后退、前进就全通了。home() 回的地址也是装载时记下的，首次装载时 home URL 还无效，记下的就是它（第 326-327 行）。

## 链接点击的分流次序：openLinks 定全手动，openExternalLinks 只管绝对 URL

咱们把 activateAnchor 的次序摆开，它是链接点击的统一入口（qtextbrowser.cpp 第 199-238 行）。openLinks 为 false 时只发 anchorClicked，发完就返回（第 211-213 行）。openLinks 为 true 时接着轮到 openExternalLinks，条件是它开着、URL 不属于 file/qrc 这类本地 scheme 且不是相对 URL（第 219-228 行），命中就交 QDesktopServices::openUrl 然后返回，这个 return 在 anchorClicked 发射（第 232 行）之前，信号没机会出。都没命中才发 anchorClicked，槽里没动文档的话，Qt 会自动 setSource 导航，出处见第 237 行。

setOpenExternalLinks(true) 之所以让 anchorClicked 发不出来，根因就在这个 return 上，页面已经交给系统浏览器了，您想在打开前记录点击、检查 URL、按内容挑用哪个浏览器，全都轮不上。相对链接还有个冷门分支，当前文档挂在绝对 URL 上时，activateAnchor 拿到 href 就交给 resolveUrl（第 209 行），文档内的相对链接在第 185 行就拼成了绝对 URL，scheme 不是 file/qrc 的话，openExternalLinks 开着就命中第 225 行第一子句，照样送进系统浏览器，第 226 行的第二子句要的是解析之后仍是相对 URL 的链接，这个场景给不出来。openExternalLinks 关着的话，这类相对链接照发 anchorClicked，在浏览器内正常导航。本地 HTML 文档里的相对链接另算，openExternalLinks 开关两边都拦不着它。四种组合的效果咱们摆成表，看点击一个 http 链接会发生什么：

| openLinks | openExternalLinks | 点击 http 链接的结果 |
| ---- | ---- | ---- |
| true（默认） | false（默认） | 发 anchorClicked，随后自动 setSource 导航 |
| true | true | 直接交给系统浏览器，anchorClicked 不发 |
| false | false | 只发 anchorClicked，导航全归您的槽 |
| false | true | 仍然只发 anchorClicked，openExternalLinks 那道够不着 |

openLinks(false) 站在入口第一道，openExternalLinks 那道根本到不了，想手动控制导航，只设 openLinks(false) 就够了。您看示例，两条都设成 false，出处是 custom_resource_browser.cpp 第 31-32 行，anchorClicked 的槽里按 scheme 分流，内部页面手动装载，http 与 https 交 QDesktopServices::openUrl，自定义协议走自己的处理（第 228-264 行）。

槽里改文档还有个顺带的效果。文档一变，contentsChanged 把 textOrSourceChanged 置位，置位函数在 qtextbrowser.cpp 第 111-115 行，连接在第 680 行，activateAnchor 发完 anchorClicked 查这个标志（第 234-235 行），置位就不再自动导航。您在槽里调 setSource 或 setHtml 接管之后，Qt 不会替您再导一遍，两边不打架。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，QTextBrowser 在 QtWidgets，异步加载网络图片要补上 Qt6::Network，示例可在 Linux、macOS、Windows 桌面平台编译运行。

[Qt 文档 · QTextBrowser](https://doc.qt.io/qt-6/qtextbrowser.html) -- 只读富文本浏览器，loadResource、openLinks 与历史栈 API 都在这页

[Qt 文档 · QTextDocument](https://doc.qt.io/qt-6/qtextdocument.html) -- 底层文档模型，ResourceType 枚举与 addResource 的说明

[Qt 文档 · QTextDocument::ResourceType](https://doc.qt.io/qt-6/qtextdocument.html#ResourceType-enum) -- loadResource 的 type 参数取值

[Qt 文档 · QDesktopServices](https://doc.qt.io/qt-6/qdesktopservices.html) -- 系统服务，openUrl 把外部链接交给默认浏览器

---

输入类五连走到第四站，还剩 [3.26 QKeySequenceEdit 进阶](./26-qkeysequenceedit-advanced.md) 收尾，那是个只管一件事的小控件，录入快捷键、拦截冲突，思路与本篇完全两样。想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/25-qtextbrowser-advanced/，可执行文件 25-qtextbrowser-advanced），页面上三张 myapp:// 协议的图片出自模拟数据库，点外部链接会在您的默认浏览器里打开。两步实验顺着正文走，仪表提前备好，您在 handleAnchorClicked 的 openUrl 分支里，加一行 qDebug。第一步把 custom_resource_browser.cpp 第 31 行的 setOpenLinks(false) 注释掉，openLinks 回到默认的 true，示例原本 openLinks 是 false，openExternalLinks 那道关卡排在它后面，流程到第 211-213 行就返回了，不改这行实验不成立。这时点页面的外部链接，默认浏览器开，qDebug 照打，正文四组合表第 1 行的自动导航也照走，页面内容跟着被清空，不用意外。第二步把第 32 行改成 setOpenExternalLinks(true)，再点同一个链接，浏览器还是照样开，qDebug 却一声不响，anchorClicked 发不出来当场可见，这回开浏览器的是 qtextbrowser.cpp 第 225-228 行那道，第 228 行的 return 站在第 232 行的 emit 前面。接着试历史，前两步动过的第 31-32 两行要还原回来，不还原的话 openExternalLinks 开着，回程那颗拼好的绝对 URL 会在第 225 行命中那道关卡被送进系统浏览器，槽就收不到了。然后把 handleAnchorClicked 里 page1/page2 两处 setHtml 换成 setSource，myapp 演示分支里还有第三处 setHtml，那个不用动。loadResource 那边加一个 HtmlResource 分支，myapp://page1 与 myapp://page2 这样的文档 URL，也从内存里给出去。从第一页点进第二页，底部标签走到 0，历史计数就活了，首页出自 setHtml 本来就不在栈里，而 backwardAvailable 要等压栈之后才发，出处是 qtextbrowser.cpp 第 840 行，标签一次导航内就走到位。再从第二页点回第一页，这一下进槽的已经是 resolveUrl 拼好的绝对 URL myapp://page2/page1，page1/page2 的匹配认 path 照样命中，点满一来回，后退按钮亮起，标签走到 1。按下后退回到第二页，前进按钮跟着亮。完成标准咱们给您列好，自定义协议图片三张全显示，历史来回导航不断链，每次外部链接点击都有日志可查。
