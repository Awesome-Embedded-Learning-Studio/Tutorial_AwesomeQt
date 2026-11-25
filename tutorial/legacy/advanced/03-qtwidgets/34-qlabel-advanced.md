---
title: "3.34 QLabel 进阶"
description: "入门篇用 QLabel 显示了文本图片动画与链接，进阶篇对着 6.9.1 源码把行为细节认清：AutoText 的判定发生在 setText 里并记进 effectiveTextFormat，mightBeRichText 只查第一行，跳过前导空白与 xml 序言、见到 doc 类型前缀直接判富、途中遇到小于号转义也判富，然后取第一个尖括号对的标签名查 QTextHtmlParser 元素表，script 标签在表内且 DisplayNone 整段隐藏，查不到的标签名判回纯文本，纯文本无交互时连 QWidgetTextControl 都不建，绘制走 drawItemText 原文直画、测量走 boundingRect，富文本才建 control 加 QTextDocument 走 setHtml 解析与文档布局，setBuddy 的注册链在 updateShortcut，文本无 & 直接返回不注册，快捷键事件命中后 buddy 以 ShortcutFocusReason 拿焦点且按钮类会被 animateClick，setPixmap、setPicture、setNum、setMovie 都会经 clearContents 释放 buddy 快捷键，textInteractionFlags 默认是 LinksAccessibleByMouse，链接可点时 linkActivated 由 control 转发，openExternalLinks 为真时链接交给 QDesktopServices 打开且信号不再发，选中态住在 control 的 QTextCursor 里，可选中联动 ClickFocus 焦点并附带右键菜单，wordWrap 只拨对齐位，无人给宽度时 sizeHint 按平均字符宽乘 80 挑目标，minimumSizeHint 取最长单词，QLabel 无 elide 接口，省略号要用 QFontMetrics 的 elidedText 自己截，setPixmap 包 QIcon、setPicture 按值存 QPicture、setMovie 挂 QPointer 不接管所有权，配套示例在 examples/advanced/03-qtwidgets/34-qlabel-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.34——QLabel 进阶

[3.33 QDial 进阶](./33-qdial-advanced.md) 收尾把咱们从滑动条家族送进显示控件，交棒时记下的 setTextFormat 解析模式、setBuddy 快捷键注册与文本交互权限，这篇逐一接住。QDial 难在几何与钳位，QLabel 难在分工，同一段文字走哪条管线、建不建文档、收不收鼠标事件，全由几个标志位决定，咱们对着源码把这几处判定认清。setPixmap、setPicture、setMovie 的分界，顺路收掉。

默认值照 33 篇的样子记下。不带参数构造出来，textformat 是 AutoText，出处 qlabel_p.h 第 106 行，textInteractionFlags 是 LinksAccessibleByMouse，出处第 108 行，openExternalLinks 为假，出处 qlabel.cpp 第 39 行，对齐默认左对齐垂直居中并展开制表符，margin 0、indent 负 1，出处 qlabel_p.h 第 110-112 行。正文行号都带出处，Qt 内部实现主要指 qlabel.cpp，交互事件下沉到 qwidgettextcontrol.cpp，AutoText 的判定函数在 qtextdocument.cpp 与 qtexthtmlparser.cpp，快捷键解析在 qkeysequence.cpp。笔者把这几个文件对着源稿断言逐条过了一遍，交互权限的默认值与 wordWrap 两处偏得最远，正文按源码改准。

## AutoText 只查第一行，script 在元素表里且整段隐藏

判定发生在 setText 里，结果记进 effectiveTextFormat，咱们后面看到的管线全由它定：

```cpp
// qlabel.cpp 第 245-252 行
if (d->textformat == Qt::AutoText) {
    if (Qt::mightBeRichText(d->text))
        d->effectiveTextFormat = Qt::RichText;
    else
        d->effectiveTextFormat = Qt::PlainText;
} else {
    d->effectiveTextFormat = d->textformat;
}
```

mightBeRichText 是一套快启发式，函数注释写明只保证常见情况，出处 qtextdocument.cpp 第 63-66 行。咱们跟着走：一进来就跳过前导空白，出处第 79-80 行，接着跳过 `<?xml` 序言，出处第 83-96 行，见到 `<!doc` 前缀直接判富文本，出处第 98-99 行。然后往前扫，扫到第一个 `<` 或换行符为止，途中遇到 `&lt;` 转义也判富文本，出处第 101-106 行。扫到 `<` 才取标签，从 `<` 到下一个 `>` 之间抽出字母数字当标签名，拿去查 QTextHtmlParser 的元素表，查得到判富文本，查不到判纯文本，出处第 107-127 行，表在 qtexthtmlparser.cpp 第 355-415 行，查询入口 lookupElement 在第 437-443 行。

只查第一行是判定链最容易被忽略的性质，扫描在换行符处停，出处第 101-102 行。您把 `<b>` 写在第二行开头，AutoText 看不见它，整段按纯文本渲染，尖括号原样印出来。反过来，用户评论里出现 `<script>` 标签，script 在元素表里，出处 qtexthtmlparser.cpp 第 396 行，笔者数到它那一列的 displayMode 是 DisplayNone，解析成富文本后标签连同内容一起被藏掉。类文档把这事写在构造警告里，来源不可控的文本要么消毒要么显式 PlainText，出处 qlabel.cpp 第 81-87 行。查表失败的例子当场记一个，`1 < 2 > 0` 这串抽出的标签名是 2，表里没有，判回纯文本，含尖括号的算式不至于全误伤。格式枚举里还有 MarkdownText，填充函数会走 setMarkdown，出处第 1472-1475 行，AutoText 的判定不产出它，显式设才进得去。

## 纯文本连 QWidgetTextControl 都不建，贵的都在富文本管线

要不要建文本控件，咱们把判据摆进 needTextControl：文本标签且格式不是纯文本、flags 里带鼠标或键盘可选中、焦点策略不是 NoFocus，条件满足其一才建，出处 qlabel_p.h 第 62-68 行。纯文本加默认 flags 的 QLabel 一条都不占，control 是空指针。此时的绘制极省，paintEvent 直接把原文交给 style 的 drawItemText 画出去，出处 qlabel.cpp 第 1018-1027 行，测量也只有 QFontMetrics 的 boundingRect 一口气，出处第 588-598 行。

换到富文本，路就重了。ensureTextControl 起手建 QWidgetTextControl 与 QTextDocument，关撤销、设默认字体，出处 qlabel.cpp 第 1534-1537 行，填充时 setHtml 整段解析，出处第 1469-1470 行，布局按文档来，出处第 1503-1526 行，绘制交给 control 的 drawContents，出处第 1016 行。sizeHint 的差异也落在这两套测量上：control 路径要 setTextWidth 或 adjustSize 把整个文档排一遍才拿得到尺寸，出处第 556-575 行，纯文本路径一个 boundingRect 就出。您在列表里铺的标签一多，用户输入又常带尖括号，AutoText 会把它们成批拖进重管线。能确定是纯文本的，就显式 setTextFormat(Qt::PlainText)。setTextFormat 自己也不神秘，改完标志位把旧文本清掉重走一遍 setText，出处第 1339-1350 行。

## setBuddy 的快捷键长在文本的 & 上，换文本就重抓

setBuddy 把伙伴的 QPointer 存下来并挂上 destroyed 通知，文本标签路径再把旧 shortcutId 释放、置零、有新伙伴就重抓，出处 qlabel.cpp 第 1133-1156 行。快捷键从哪来，咱们到 updateShortcut 里看：

```cpp
// qlabel.cpp 第 1179-1185 行
hasShortcut = false;
if (!text.contains(u'&'))
    return;
hasShortcut = true;
shortcutId = q->grabShortcut(QKeySequence::mnemonic(text));
```

文本里没有 `&`，函数原样返回，setBuddy 建立了关联也注册不出快捷键，您排查 Alt 不生效时头一眼看当前文本里有没有 `&`，这段代码就是全部来路。有 `&` 时 mnemonic 取它后面第一个可打印字符，转大写配成 Alt 加键，出处 qkeysequence.cpp 第 932-963 行，`&&` 表示字面上的 `&`，出处 qlabel.cpp 第 1115-1116 行。按下后的事件也直白，QEvent::Shortcut 命中 shortcutId 就把焦点以 Qt::ShortcutFocusReason 交给 buddy，buddy 是 QAbstractButton 还会替您 animateClick，出处 qlabel.cpp 第 925-941 行。注释里留了一个平台注脚，macOS 上助记符默认关，mnemonic 返回空序列，`&` 照样隐藏，出处第 1175-1178 行。

文本会流动，注册跟着走。setText 头一步经 clearContents 释放旧 shortcutId，末尾有 buddy 就重抓，出处第 269-271 与 1281-1284 行，您换掉的文本丢了 `&` 标记，快捷键当场失效且无任何警告。富文本不背这个锅，快捷键取自原始文本，与格式无关，出处第 1184 行，格式只决定显示端的处理，control 路径在文档解析后删 `&` 加下划线，出处第 1482-1496 行。一个标签的 buddy 槽位是单个 QPointer，后一次设置覆盖前一次，出处 qlabel_p.h 第 103 行。还有一条容易漏，setPixmap、setPicture、setNum、setMovie 的文档都注明会禁用 buddy 快捷键，出处 qlabel.cpp 第 305-306、348、369、1233 行，机制同样是 clearContents 里的释放。

## 交互权限默认开着链接，选中态住在 QTextCursor 里

源稿把默认权限写成 NoTextInteraction，说鼠标点击选择拖拽全不响应。笔者在 qlabel_p.h 第 108 行翻到的初始化是 LinksAccessibleByMouse，属性文档同一句，出处 qlabel.cpp 第 660 行。默认的富文本标签里链接能点，linkActivated 能发，ensureTextControl 把 control 的同名信号转发给标签，出处第 1546-1547 行，富文本顺手打开 mouse tracking 给悬停用，出处第 263-264 行。纯文本默认也没有交互，来路在没建 control，判据回看上一节 needTextControl，与 flags 无关。

flags 一动，焦点策略跟着动。带 LinksAccessibleByKeyboard 设 StrongFocus，带可选中设 ClickFocus，其余情况压回 NoFocus，出处 qlabel.cpp 第 668-673 行。鼠标事件的闸门在 sendControlEvent，非文本标签、没有 control 或 flags 恰为 NoTextInteraction 时事件被 ignore 原路退回父控件，出处第 1555-1563 行。进了 control 之后各走各的检查，按下去要 TextSelectableByMouse 才立拖选标志，出处 qwidgettextcontrol.cpp 第 1591-1607 行，链接要 LinksAccessibleByMouse 才在松开时激活，出处第 1834-1865 行。所以您把可选择标签放进带拖拽的自定义父控件里，拖选会吃掉左键事件，父控件的拖拽收不到，想让事件漏下去就得在 flags 上让步。

激活后的分岔只有几行，咱们看准了再用：

```cpp
// qwidgettextcontrol.cpp 第 2968-2972 行
if (openExternalLinks)
    QDesktopServices::openUrl(href);
else
    emit q_func()->linkActivated(href);
```

openExternalLinks 为真时链接直接交给系统打开器，linkActivated 不再发，出处同段，属性文档还要求 flags 里带链接可访问，出处 qlabel.cpp 第 629-630 行。安全面就藏在这，href 来自文本、文本又来自用户时，本地路径与没审过的 scheme 都会原样送进 QDesktopServices，这类标签您别开 openExternalLinks，接 linkActivated 自己审。选中态全记在 control 的 QTextCursor 里，hasSelectedText 与 selectedText 直接问 textCursor，出处 qlabel.cpp 第 730-736 与 754-759 行，setSelection 也是摆 cursor 加 KeepAnchor，出处第 702-712 行。可选中还附带一份右键菜单，复制与复制链接由 contextMenuEvent 起，出处第 847-862 行，失焦时选区被清，出处第 881-893 行。

## wordWrap 只拨一个对齐位，省略号得自己用 elidedText 截

源稿说 wordWrap 在宽度没定下来时不生效，文本仍画在一行。机制上站不住：setWordWrap 只把 align 里的 TextWordWrap 位拨上或拨下，出处 qlabel.cpp 第 436-445 行，updateLabel 顺手把 sizePolicy 切成 heightForWidth，出处第 1093-1099 行。宽度没人给时 sizeHint 自己挑目标，纯文本路径取平均字符宽乘 80 与最大允许宽里较小的一个，还可能再折半两回，出处第 588-598 行，control 路径走 adjustSize，出处第 564-565 行，折行时 minimumSizeHint 的宽取最长单词，行内注释写得分明，出处第 811 行。您看到的现象由此而来，折行标签在布局定宽前报的宽约是一行 80 字，它从不静默失效，只是 hint 的宽与您预期的不一样。

省略号这块 QLabel 完全不管，qlabel.h 里没有任何 elide 接口，活归 QFontMetrics。elidedText 按给定宽返回已含省略号的串，出处 qfontmetrics.h 第 70 行，方向有 ElideLeft、ElideRight、ElideMiddle、ElideNone 四档，出处 qnamespace.h 第 190-193 行，文件路径类内容用 ElideMiddle 保两头。它是静态计算，窗口 resize 不会自动重截，您要么在 resizeEvent 里重算，要么认下这个限制。

## setPixmap、setPicture、setMovie 分工不同，换内容就断快捷键

三种内容共用一张清空流程。clearContents 删 control、清文本、断动画连接，末尾释放 shortcutId，出处 qlabel.cpp 第 1266-1290 行，任何一次换内容都会头一步把 buddy 快捷键断掉，上一节的清单就是从这来的。setPixmap 把位图包进 QIcon 存下尺寸，出处第 308-318 行，开 scaledContents 时缩放结果进 QPixmapCache，避免每帧重缩，出处第 1055-1075 行。setPicture 按值存一份 QPicture，绘制用 drawPicture 回放，scaledContents 用 painter 的 scale 拉伸，出处第 1030-1052 行。setMovie 的所有权咱们要单独记，QPointer 存指针，文档明说不接管，出处第 1231 行，接着挂 resized 与 updated 两条连接，出处第 1246-1250 行，绘制时画 currentPixmap，出处第 975-981 行，动画跑起来后重画全靠信号推着走，出处第 1252-1255 行。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，QLabel 在 QtWidgets，文本测量用的 QFontMetrics 在 QtGui，链接 Qt6::Widgets 即可。

[Qt 文档 · QLabel](https://doc.qt.io/qt-6/qlabel.html) -- 文本与图像显示控件，setTextFormat、setBuddy、textInteractionFlags 的属性说明

[Qt 文档 · QFontMetrics](https://doc.qt.io/qt-6/qfontmetrics.html) -- 字体度量类，elidedText 的方向与宽度参数

[Qt 文档 · Qt::TextInteractionFlag](https://doc.qt.io/qt-6/qt.html#TextInteractionFlag-enum) -- 交互标志枚举与 TextBrowserInteraction 的组合

[Qt 文档 · QKeySequence](https://doc.qt.io/qt-6/qkeysequence.html) -- mnemonic 的 & 解析与 Alt 组合规则

[Qt 文档 · QMovie](https://doc.qt.io/qt-6/qmovie.html) -- 动画播放，QLabel 不接管其所有权

---

显示控件头一站到这收住，下一程还在显示家族里等咱们，入口在 [3.35 QProgressBar 进阶](./35-qprogressbar-advanced.md)。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/34-qlabel-advanced/，可执行文件 34-qlabel-advanced）。窗口第一区三只标签并排，输入框预填一段含 `<b>`、`<i>` 与 `<script>` 的文本，出处 label_mode_demo.cpp 第 51 行，三只标签的格式固定为 PlainText、RichText、AutoText，出处第 66-102 行，下拉框只负责高亮当前对比的模式，不改渲染。第二区一只带 `&U` 标记的标签配输入框，出处第 136-138 行，按钮在含标记与不含标记两种文本间切换，出处第 246-260 行。第三区长路径按标签实际字体现截，ElideLeft、ElideMiddle、ElideRight 三档可选，出处第 187-200 行，计算在 updateElideDisplay，出处第 262-280 行。

实验补两步。您在第一区输入框敲 `1 < 2 > 0`，三只标签原样显示，标签名 2 查不到元素表，AutoText 判回纯文本，来路在 qtexthtmlparser.cpp 第 437-443 行。再到第二区点掉 `&` 标记按 Alt+U，焦点不动了，updateShortcut 见文本无 `&` 原样返回，出处 qlabel.cpp 第 1181-1182 行。

完成标准照旧。AutoText 判定链查到换行为止、标签名要过元素表，您说得出在哪几行。默认交互权限是什么、它为什么不影响纯文本标签，您分得开。换 setPixmap 后 Alt 快捷键失效的释放发生在哪一段，您指得出来。
