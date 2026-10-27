---
title: "3.23 QTextEdit 进阶"
description: "入门篇把 setHtml/toHtml/toPlainText 三种内容格式、QTextCursor 的移动与选中、QTextCharFormat 的字符格式化和 setModified 的修改追踪过了一遍，进阶篇把视角沉到 QTextDocument 的 frame/block/fragment 树上，拆 Format 类的分工与锚点点击的真正归属，过 position 与 anchor 的选区模型和 ExtraSelection 批量高亮，末尾收拾 find 的返回值语义、默认大小写与零宽匹配，配套示例在 examples/advanced/03-qtwidgets/23-qtextedit-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.23——QTextEdit 进阶

[3.22 QLineEdit 进阶](./22-qlineedit-advanced.md) 收尾留了话，输入类五连的第二篇从富文本文档结构讲起，本篇兑现这句，咱们从树根看起。QLineEdit 的世界只有一串纯文本，QTextEdit 的每个字却都住在一棵文档树上，控件自己只是这棵树的显示端。

笔者摆一个多数人都碰过的落差。入门篇的 setHtml 加 toPlainText，够您做一个能读写 HTML 的窗口，可需求一走到调字体、插表格、嵌列表、全文高亮这一带，就处处别扭：API 都认识，下手时却不知道该改树的哪一层。根子在 QTextDocument 的结构没进脑子，本篇按结构、格式类、光标选区、搜索替换的顺序把这几层过一遍。

## 树才是正文：frame、block 与 fragment

QTextDocument 内部是一棵树，咱们从上往下数。顶层是 rootFrame，document()->rootFrame() 拿到它，frame 下面还能套 frame，frame 里住着 block，block 里住着 fragment。frame 负责圈区域，表格这类嵌套排版就落在这层，它的子类下文单说。block 对应一个段落，以换行符分界。fragment 是 block 里格式一致的最小文本段，一段话里出现三种字体，它就有三个 fragment。

从根往下遍历的路径不长，咱们写成片段：

```cpp
QTextFrame *root = doc->rootFrame();
for (auto it = root->begin(); it != root->end(); ++it) {
    if (QTextFrame *child = it.currentFrame())        // 子 frame，表格在这里
        walkFrame(child);                             // 递归往下走
    else if (QTextBlock block = it.currentBlock(); block.isValid())
        for (auto fragIt = block.begin(); fragIt != block.end(); ++fragIt)
            qDebug() << fragIt.fragment().text();     // 每个格式一致的小段
}
```

迭代器每一站要么指着子 frame，要么指着 block。currentFrame() 与 currentBlock()，同一时刻只有一个非空。您要是只查 currentBlock()，跑起来不报错，表格里的字却一行都打不出来，因为子 frame 的位置上 currentBlock() 给回的是无效块，isValid() 把它滤掉了，遍历就这么安静地漏掉一整块。第一行那个 currentFrame() 分支是必需品。

插入的落点也在这棵树上。您用 QTextCursor 插入一段带格式的文字，文档里多出来的是一个 fragment，它记着文字内容和对应的字符格式索引。相邻两个 fragment 格式相同时会被并回去，同格式的连续文本合成一个片段。setHtml 存进去再 toHtml 读出来，标记串跟您手写的常有出入，一部分根由在这类归并，另一部分是序列化时补进去的内容：toHtml 给每个段落配上一整套 margin 与缩进样式，body 标签注入默认字体（Sans Serif 9pt），颜色归一成 `#rrggbb` 的六位写法，写 `red` 进去，出来是 `#ff0000`。

表格是 frame 的子类 QTextTable，光标调 insertTable 就在当前位置生成一枚。单元格咱们要单独说，它拿的是 QTextTableCell，一枚轻量的位置句柄，够不着 frame 的继承链，进一格子用 firstCursorPosition() 或 begin()，接着走 block 和 fragment。把单元格当 frame 去深挖的代码，会在继承层面落空。

示例把结构这件事做成了看得见的东西。状态栏一路显示光标的段落号和文档的段落总数，实现在 rich_text_navigator.cpp 的 updateStatusBar，238 行起用 blockCount 与 characterCount。您跑起示例，点一次插入富文本按钮（169 行的 insertRichText，插一张 2x2 表格），段落总数比肉眼数的多出五个：四个单元格各占一个 block，还有一块是表格落脚的普通块（插入片段首尾的 `<br/>` 把它折了出来），树的形状当场数出来了。

## Format 类各管一层：字符、段落、表格互不越权

| Format 类 | 管哪一层 |
| ---- | ---- |
| QTextCharFormat | 字体、字号、前景背景色、加粗斜体、下划线删除线、上下标、锚点 |
| QTextBlockFormat | 对齐、左右与首行缩进、行距、段前段后距、段落背景 |
| QTextListFormat | 列表样式与缩进层级 |
| QTextTableFormat | 边框、单元格间距、列宽约束 |

有一个说法流传很广，说 QTextListFormat 继承自 QTextBlockFormat，咱们来修正，两类是平级的，全都直接继承 QTextFormat，互不干扰是设计如此，不靠继承关系。

字符层入门篇玩过加粗变色，进阶咱们补两个容易漏的属性。头一个是锚点：

```cpp
QTextCharFormat link;
link.setAnchor(true);
link.setAnchorHref(QStringLiteral("https://doc.qt.io"));
link.setForeground(QColor(QStringLiteral("#1565C0")));
link.setUnderlineStyle(QTextCharFormat::SingleUnderline);
cursor.insertText(QStringLiteral("Qt 文档"), link);
```

setAnchor(true) 只把文字标记成锚点，QTextEdit 不会替您响应点击，它的信号表里没有 anchorClicked，openExternalLinks 也是别家的属性，这两样都长在 QTextBrowser 上，那是输入类五连的第四站。QTextEdit 里插了链接点着没反应，属于设计如此。另一个是 verticalAlignment，AlignSuperScript 与 AlignSubScript 摆出上标下标，公式和脚注用得上。

段落层归 QTextBlockFormat，咱们常用的对齐、缩进、行距、段间距都在它身上。列表样式走它的同层伙伴 QTextListFormat，setStyle 给 ListDisc 摆圆点，setIndent 定层级，cursor.insertList(listFormat) 落下列表并返回 QTextList*，之后光标续写的文字挂在同一枚列表上，敲换行自动续下一项。

这里咱们要当场说清 setHtml 的脾气。它把整棵文档树推倒重析，之前用光标摆好的格式、表格、列表全部随之消失，一次 setHtml 就是一次整篇重写。想往现有内容里塞一段 HTML，走光标的 insertHtml()，在指定位置插入，周围格式原样保留。示例两种都有：loadSampleContent 在 116 行用 setHtml 装载演示内容，插入按钮在 180 行用 insertHtml 往光标处塞表格。您做个对照实验，把 116 行的 setHtml 紧跟着复制一份再调一次（换成另一段 HTML），跑起来只见第二段，第一段的演示内容连同格式一起没了。再点一次插入按钮对照着看，insertHtml 塞进来的表格落在光标处，周围的字一个都没动。

## position 与 anchor：选区的形状由光标两端决定

QTextCursor 内部是两个整数位置，咱们挨个看。position 是活跃端，光标所在。anchor 是另一端，选区的固定脚。两端重合就是一个插入点，分开了才有选区。您用 movePosition 配 KeepAnchor，挪的只有 position，anchor 留在原地，选区随之成形，示例的选中行按钮就是这么拼的（rich_text_navigator.cpp 第 148 行的 selectLine，StartOfLine 配 MoveAnchor，EndOfLine 配 KeepAnchor）。懒一点的场合 select() 更省事，BlockUnderCursor 整段，WordUnderCursor 整词，140 与 157 行各有一枚按钮伺候。

QTextCursor 是值类型，随手复制不会惊动编辑器，改动要 setTextCursor 写回去才生效。咱们后面做搜索高亮正沾这个光，一份光标改到哪算哪，不碰编辑器本身。

anchor 可以大于 position。用户从右往左划选区，position 在前 anchor 在后，内容照样能取（selectedText() 按文档顺序提取选区文本，与划动方向无关），可拿 position() 当起点算偏移的代码，在反向选区上就算错了。要起止位置走 selectionStart() 与 selectionEnd()，比自己拿 qMin qMax 现算稳。再补一个会拦住人的细节，selectedText() 里的段落分隔以 U+2029 形式回来，直接拿它跟纯文本比对会落空。验证的法子现成，您在 selectBlock（140 行）里补一行打点，把 cursor.selectedText() 打出来，选中含多段的内容，输出长度比肉眼数的字符多，多出来的就是段间的 U+2029。

## find 与 ExtraSelection：高亮、大小写与倒序替换

不动文档就能给一片文字上色，靠的是 ExtraSelection，一组带格式的光标，setExtraSelections 一次给齐。咱们看示例的 performSearch，实现从 rich_text_navigator.cpp 第 189 行起：

```cpp
QList<QTextEdit::ExtraSelection> highlights;
QTextCursor searchCursor(doc);
while (!searchCursor.isNull() && !searchCursor.atEnd()) {
    searchCursor = doc->find(keyword, searchCursor);   // keyword 来自搜索框
    if (!searchCursor.isNull()) {
        QTextEdit::ExtraSelection sel;
        sel.cursor = searchCursor;
        sel.format.setBackground(QColor(QStringLiteral("#FFF176")));
        highlights.append(sel);
    }
}
setExtraSelections(highlights);
```

循环能停得下来，靠的是 find 的返回值语义。返回的光标已经替您选好匹配，anchor 压在匹配开头，position 落在末尾，下一轮正向搜索从 selectionEnd() 接着走，反向从 selectionStart() 往回走，每轮必进。找不到给一枚 isNull() 为真的光标，查询串为空时直接返回空光标。您看示例开头的关键词判空（192 行），跟文档层这道理是同一道守卫。

方向与大小写，您在 FindFlags 里挑：

| FindFlag | 作用 |
| ---- | ---- |
| FindBackward | 反向搜，从 selectionStart 往回走 |
| FindCaseSensitively | 区分大小写，不给就默认不区分 |
| FindWholeWords | 整词匹配，边界按字母与数字判定 |

默认不区分，咱们在示例上一验就着。搜索框敲小写 qtext，照样一片黄。正则这层更干脆，表达式自己带的大小写选项会被 find 忽略，它按 FindCaseSensitively 在内部重设一遍 patternOptions，您想区分只能加这个标志。实验就把 204 行补上第三个参数 QTextDocument::FindCaseSensitively，小写 qtext 当场搜空，QText 恢复满屏高亮。

find 另有一组 QRegularExpression 的重载，用之前有一件事要想清楚。能匹配空串的模式（`Q*`、`\\b`、`(?=Q)` 都算）会让循环出不去：find 实现里返回光标的推进量取自匹配长度，长度为零，下一轮还从 selectionEnd() 原地起步，循环条件永远满足。`\\b` 按 C++ 字面量的写法是两个反斜杠，要是裸写一个 `\b`，粘进 QStringLiteral 是合法转义，编译器把它读成退格符 0x08，能编译不报错，正则拿到的却已经是另一个字符，含义整个跑偏。兜底要么把模式写成不可能空匹配，要么每轮 find 后看 position 推没推进，没推进就手动调一次 setPosition(position() + 1)，把光标顶过去一格。您把 204 行换成 `doc->find(QRegularExpression(QStringLiteral("Q*")), searchCursor)` 跑一把，界面当场卡住（终端 Ctrl+C 退出），换成 `Q\\w*` 程序照常跑完，零宽不零宽一试便知。

替换顺着同一套光标走。find 返回的 cursor 已经选中目标，insertText() 就地替换。替换全部必须从文档末尾往开头 FindBackward 倒着来，正向边替换边改文档长度，后面的位置全跟着偏。您把示例的搜索栏补成完整的查找替换吧，查找下一个、查找上一个、替换、全部替换，再加一枚大小写复选框，高亮列表在关键词变动时整批重算，全部替换按倒序的路子走，做完这套东西能直接搬进自己的项目。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17。QTextEdit 在 QtWidgets，QTextDocument、QTextCursor 与全部格式类都在 QtGui，链接 Qt6::Widgets 一并带上，示例可在 Linux、macOS、Windows 桌面平台编译运行。

[Qt 文档 · QTextEdit](https://doc.qt.io/qt-6/qtextedit.html) -- 富文本编辑器，ExtraSelection 结构体与 setExtraSelections 的用法

[Qt 文档 · QTextDocument](https://doc.qt.io/qt-6/qtextdocument.html) -- 文档模型，find 的各档重载、rootFrame 与 blockCount

[Qt 文档 · QTextCursor](https://doc.qt.io/qt-6/qtextcursor.html) -- 编辑与选区接口，position、anchor、selectionStart 与 selectionEnd

[Qt 文档 · QTextCharFormat](https://doc.qt.io/qt-6/qtextcharformat.html) -- 字符格式，锚点属性与 verticalAlignment

[Qt 文档 · QTextBlockFormat](https://doc.qt.io/qt-6/qtextblockformat.html) -- 段落格式，对齐、缩进与行距

[Qt 文档 · QTextListFormat](https://doc.qt.io/qt-6/qtextlistformat.html) -- 列表格式，样式枚举与缩进层级

[Qt 文档 · QTextTable](https://doc.qt.io/qt-6/qtexttable.html) -- 表格对象，QTextTableCell 单元格的进入方式

---

输入类五连走到第二篇，树、格式、光标这三样本篇备齐，下一篇接着用。[3.24 QPlainTextEdit 进阶](./24-qplaintextedit-advanced.md) 请出 QTextEdit 的纯文本兄弟，从两个类的分工讲起，聊到行号侧栏与大日志不卡。想再动手的，请您把示例（examples/advanced/03-qtwidgets/23-qtextedit-advanced/，可执行文件 23-qtextedit-advanced）完整跑一轮：搜索框敲小写关键词看默认大小写，204 行加 FindCaseSensitively 再看一遍，setHtml 连调两次看整篇重写，插入富文本后数一数状态栏的段落。加码的话，您照查找替换那一段的清单一口气补齐，全部替换记得倒序。
