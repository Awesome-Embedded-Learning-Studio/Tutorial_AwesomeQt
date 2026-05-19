---
title: "3.23 QTextEdit 进阶"
description: "入门篇我们学会了 QTextEdit 的基础操作——setHtml/toHtml、QTextCursor 的 movePosition 和 insertText、以及 document()->setModified 的修改追踪。进阶篇要把视角拉到 QTextDocument 的底层模型上，拆解富文本操作的真正机制。"
---

# 现代Qt开发教程（进阶篇）3.23——QTextEdit 进阶

## 1. 前言 / QTextEdit 的深度远不止 setHtml + toPlainText

入门篇我们把 QTextEdit 的基础能力过了一遍——setHtml/toHtml/toPlainText 三种内容格式、QTextCursor 的光标移动和选中、QTextCharFormat 的字符格式化、以及 document()->setModified 的修改状态追踪。这些知识够你做一个简易的文本编辑器了。但如果你要做的是富文本编辑器——用户可以调字体、变色、插表格、嵌列表——你就会发现入门篇的知识像是在用锤子修手表。你需要理解 QTextDocument 的四层结构模型，需要掌握 QTextCursor 的高级导航和选区操作，需要知道 QTextCharFormat/QTextBlockFormat/QTextTableFormat 各自管什么，还需要会做搜索替换。这篇文章我们就把 QTextEdit 的四个进阶维度拆清楚：QTextDocument 底层模型、富文本格式化 API 全景、QTextCursor 高级导航与选区、以及搜索替换的正确实现。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17 标准。QTextEdit 属于 QtWidgets，但其内部使用的 QTextDocument、QTextCursor、QTextCharFormat 等类定义在 QtGui 中。QtWidgets 依赖 QtGui，链接 Qt6::Widgets 即可。富文本相关的格式类（QTextFrameFormat、QTextTableFormat、QTextListFormat）同样在 QtGui 中。

## 3. 核心概念讲解

### 3.1 QTextDocument 四层结构——frame/block/fragment/table 的层级关系

QTextDocument 采用了经典的文档对象模型设计，内部是一个树形结构，顶层是 root frame，每个 frame 包含若干 block，每个 block 包含若干 fragment。frame 是最大的容器单元，用来实现分栏、页眉页脚等布局。block 对应一个段落（以换行符分隔的文本段），它是文本内容的主要载体。fragment 是 block 内的最小文本单元，每个 fragment 拥有统一的字符格式——也就是说，一个 block 内如果有三种不同格式的文字，它就包含三个 fragment。

```cpp
QTextDocument *doc = editor->document();
QTextFrame *rootFrame = doc->rootFrame();

// 遍历 root frame 下的所有 block
for (auto it = rootFrame->begin(); !(it == rootFrame->end()); ++it) {
    QTextBlock block = it.currentBlock();
    if (block.isValid()) {
        qDebug() << "Block text:" << block.text();
        // 遍历 block 内的 fragment
        for (auto fragIt = block.begin(); !(fragIt == block.end()); ++fragIt) {
            QTextFragment fragment = fragIt.fragment();
            if (fragment.isValid()) {
                qDebug() << "  Fragment:" << fragment.text()
                         << "Format index:" << fragment.charFormatIndex();
            }
        }
    }
}
```

这段代码展示了从 document -> rootFrame -> block -> fragment 的完整遍历路径。理解这个结构的关键在于：每次你通过 QTextCursor 插入带格式的文字，实际上是在当前 block 内创建了一个新的 fragment，这个 fragment 记录了文字内容和对应的 QTextCharFormat 索引。当两个相邻 fragment 的格式相同时，QTextDocument 会自动合并它们。这也是为什么你用 setHtml 加载一段内容后，`toHtml()` 的输出看起来和你写入的不一样——QTextDocument 在内部做了格式合并和结构优化。

QTextTable 是 frame 的一个特殊子类，它不直接出现在 block/fragment 的遍历路径中，而是作为 frame 的子 frame 存在。插入表格时，QTextCursor 的 insertTable 会在当前 block 的位置创建一个 QTextTable，表格内的每个单元格又是一个 frame，每个 frame 内包含自己的 block。

### 3.2 富文本格式化 API 全景——CharFormat / BlockFormat / TableFormat

QTextCharFormat 控制字符级别的外观——字体族、字号、颜色、背景色、加粗、斜体、下划线、删除线、上标下标、超链接。入门篇我们用它做过加粗和变色，进阶篇要补充几个容易忽略的属性。

```cpp
QTextCharFormat linkFormat;
linkFormat.setAnchor(true);
linkFormat.setAnchorHref("https://doc.qt.io");
linkFormat.setForeground(QColor("#1565C0"));
linkFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
cursor.insertText("Qt Documentation", linkFormat);
```

setAnchor(true) 把文字标记为超链接锚点，setAnchorHref 设置目标 URL。QTextEdit 默认不会响应锚点的点击——你需要自己连接 anchorClicked 信号，或者在 QTextEdit 上设置 openExternalLinks。另外 QTextCharFormat 支持 verticalAlignment 属性来设置上标（AlignSuperScript）和下标（AlignSubScript），这在公式编辑或脚注标注中会用到。

QTextBlockFormat 控制段落级别的布局——对齐方式、缩进（左/右/首行）、行距、段前段后间距、背景色。一个常见的进阶用法是设置列表格式：

```cpp
QTextListFormat listFormat;
listFormat.setStyle(QTextListFormat::ListDisc);  // 无序圆点列表
listFormat.setIndent(1);

QTextCursor cursor = editor->textCursor();
QTextList *list = cursor.insertList(listFormat);
cursor.insertText("第一项");
cursor.insertText("\n");
cursor.insertText("第二项");
```

QTextListFormat 是 QTextBlockFormat 的子类，专门用来控制列表样式。QTextTableFormat 控制表格的外观——边框宽度、单元格间距、列宽约束。这三套 Format 类各自管理不同层级的视觉属性，互不干扰。

### 3.3 QTextCursor 高级导航与 ExtraSelection 高亮

入门篇我们用了 movePosition 做光标移动和选中，进阶篇要搞清楚 QTextCursor 的 position 和 anchor 两个核心概念。QTextCursor 内部维护了两个整数位置：position 是光标所在位置（活跃端），anchor 是选区的起点位置。当 position == anchor 时没有选区，只是一个插入点；position != anchor 时从 anchor 到 position 之间的文字就是选区。movePosition 在 KeepAnchor 模式下只移动 position 而 anchor 不动，形成选区。

QTextCursor 是值类型，你可以随意复制而不影响编辑器状态——只有通过 setTextCursor 写回时才生效。这个特性在实现"搜索后高亮所有匹配项"时特别有用：你创建多个 QTextCursor 分别指向各个匹配位置，然后把它们转为 ExtraSelection 列表一次性设置给 QTextEdit。

```cpp
QList<QTextEdit::ExtraSelection> highlights;
QTextDocument *doc = editor->document();
QTextCursor searchCursor(doc);

while (!searchCursor.isNull() && !searchCursor.atEnd()) {
    searchCursor = doc->find("Qt", searchCursor);
    if (!searchCursor.isNull()) {
        QTextEdit::ExtraSelection sel;
        sel.cursor = searchCursor;
        sel.format.setBackground(QColor("#FFF176"));
        highlights.append(sel);
    }
}
editor->setExtraSelections(highlights);
```

QTextDocument::find 返回的是一个已经选好区的 QTextCursor——position 指向匹配文本末尾，anchor 指向开头。下一次 find 从 position 继续搜索，避免死循环。find 支持纯文本和 QRegularExpression 两种模式，配合 FindFlags 控制方向和选项（FindBackward / FindCaseSensitively / FindWholeWords）。找不到匹配时返回 isNull() 为 true 的 cursor，循环搜索时用 isNull() 判断终止。

现在有一道调试题给大家。下面这段代码试图高亮文档中所有正则匹配，但运行后只高亮了第一个——为什么？

```cpp
QTextCursor cursor(doc);
while (!cursor.atEnd()) {
    cursor = doc->find(QRegularExpression("Qt\\w+"), cursor);
    if (!cursor.isNull()) {
        QTextEdit::ExtraSelection sel;
        sel.format.setBackground(QColor("#FFEB3B"));
        sel.cursor = cursor;
        highlights.append(sel);
    }
}
```

问题出在零宽匹配的陷阱——如果正则可以匹配空字符串（比如 `Qt*`），find 会在同一位置反复返回零宽匹配，position 不前进，死循环。解决方案是在每次 find 之后检查 position 是否推进了，必要时手动 `cursor.setPosition(cursor.position() + 1)`。

实现替换功能时，find 配合 insertText 就够了——find 返回的 cursor 已经选中目标文本，直接 insertText 替换选区。替换全部要从文档末尾向开头反向搜索（FindBackward），因为每次替换后文档长度变了，正向搜索的位置会偏移。

## 4. 踩坑预防

第一个坑是 QTextCursor 的 position 和 anchor 混淆导致选区方向错误。QTextCursor 的选区方向是从 anchor 到 position，但 anchor 可以大于 position（反向选区）。如果你在代码中假设选区总是从左到右的，用 `qMin(cursor.anchor(), cursor.position())` 和 `qMax(...)` 来获取起止位置会更安全。selectedText() 总是返回从 anchor 到 position 的文本，不管方向如何——所以内容是对的，但如果你用 position() 当起点来计算偏移，方向反了就会算错。

第二个坑是 setHtml 会完全覆盖之前的所有格式和结构。调用 setHtml 时 QTextDocument 会清空整个文档树并重新解析 HTML，之前通过 QTextCursor 精心设置的格式、表格、列表全部丢失。这不是"追加"而是"重写"。如果你需要在保留现有内容的基础上追加 HTML 片段，应该用 QTextCursor::insertHtml() 在指定位置插入。

第三个坑是 QTextDocument::find 配合正则表达式时，如果正则可以匹配空字符串（比如 `Qt*` 匹配零个 Qt），会导致死循环——find 在同一位置反复返回零宽匹配，position 不前进。解决方案是在正则中避免零宽匹配，或者在每次 find 后检查 position 是否推进了。

## 5. 练习项目

练习项目：简易富文本搜索工具。我们要实现一个带搜索高亮和替换功能的 QTextEdit 编辑器。窗口中央是 QTextEdit，初始化时加载一段包含多种格式的 HTML 内容。底部搜索栏包含搜索输入框、查找下一个/上一个按钮、替换输入框、替换/全部替换按钮、大小写敏感复选框。查找下一个从当前光标位置向后搜索第一个匹配项，选中并滚动到该位置，同时用 ExtraSelection 黄色背景高亮所有匹配项。替换当前选中项并自动跳到下一个。全部替换从文档末尾向开头反向操作，避免位置偏移。完成标准是高亮正确标记所有匹配、查找能循环跳转、替换不丢失周围格式、全部替换不遗漏。提示：高亮用 `QList<QTextEdit::ExtraSelection>` 统一管理，搜索内容变化时重新计算；全部替换用 FindBackward 从末尾向前替换。

## 6. 官方文档参考链接

[Qt 文档 · QTextEdit](https://doc.qt.io/qt-6/qtextedit.html) -- 富文本编辑器，ExtraSelection 结构体定义

[Qt 文档 · QTextDocument](https://doc.qt.io/qt-6/qtextdocument.html) -- 文档模型，find 方法、rootFrame、blockCount

[Qt 文档 · QTextCursor](https://doc.qt.io/qt-6/qtextcursor.html) -- 文档编辑接口，position/anchor/selection 操作

[Qt 文档 · QTextCharFormat](https://doc.qt.io/qt-6/qtextcharformat.html) -- 字符格式，anchor/verticalAlignment/font 属性

[Qt 文档 · QTextBlockFormat](https://doc.qt.io/qt-6/qtextblockformat.html) -- 段落格式，alignment/indent/lineHeight

[Qt 文档 · QTextDocument::find](https://doc.qt.io/qt-6/qtextdocument.html#find) -- 文本搜索方法，FindFlags 说明

---

到这里，QTextEdit 的进阶内容就过完了。QTextDocument 的 frame/block/fragment 四层结构是理解所有富文本操作的基础——知道数据长什么样，才能正确操作它。QTextCharFormat/QTextBlockFormat/QTextTableFormat 三套格式类分别管理字符、段落、表格三个层级的视觉属性。QTextCursor 的 position/anchor 双位置模型是精确控制选区的关键，配合 ExtraSelection 可以实现搜索高亮等高级效果。QTextDocument::find 是内置的搜索引擎，但正则搜索要注意零宽匹配的陷阱，替换操作要注意方向选择。掌握这些，你就具备了构建完整富文本编辑器的能力。
