---
title: "2.4 字体进阶：富文本与 QTextDocument"
description: "入门篇我们聊了 QFont 的基本用法——设置字体族、大小、粗体斜体，然后用 QPainter::drawText 画文字。说实话，画个标签文字确实够用了。"
---

# 现代Qt开发教程（进阶篇）2.4——字体进阶：富文本与 QTextDocument

## 1. 前言 / 文本渲染远不只是 drawText

入门篇我们聊了 QFont 的基本用法——设置字体族、大小、粗体斜体，然后用 QPainter::drawText 画文字。说实话，画个标签文字确实够用了。但当你需要显示格式化的富文本（加粗、斜体、颜色、超链接混排）、需要让用户编辑富文本、需要实现简单的语法高亮——drawText 就完全不够用了。Qt 提供了一套完整的富文本框架：QTextDocument 作为文档模型、QTextCursor 作为编辑游标、QTextCharFormat/QTextBlockFormat 控制格式。

我之前在一个日志查看器项目里用 QTextDocument 实现了简单的语法高亮，一开始觉得挺简单的，后来发现 QTextCursor 的 position 和 QTextBlock 的边界处理到处是坑——在空行上设置格式会飘到下一行，选中文字后设置颜色有时候整段都被改了。这些坑，我们今天一个一个填。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17 标准。QTextDocument 和相关类属于 QtGui 模块。QTextEdit 控件属于 QtWidgets 模块。所有示例需要 GUI 环境。

## 3. 核心概念讲解

### 3.1 QTextDocument——富文本的文档模型

QTextDocument 是 Qt 富文本框架的核心。它是一个文档对象模型，内部将文本内容组织为块（QTextBlock）和片段（QTextFragment）。每个块可以有自己的格式（QTextBlockFormat），每个片段可以有自己的字符格式（QTextCharFormat）。

QTextEdit 控件内部持有一个 QTextDocument 实例。你可以通过 `textEdit->document()` 获取它。你也可以创建独立的 QTextDocument（不依赖 QTextEdit），用于纯内存的文本处理。

```cpp
QTextDocument doc;
doc.setPlainText(QStringLiteral("Hello World"));

// 设置整个文档的默认字体
QTextCharFormat defaultFormat;
defaultFormat.setFont(QFont(QStringLiteral("Sans"), 12));
doc.setDefaultStyleSheet(QStringLiteral("body { font-family: Sans; }"));
```

### 3.2 QTextCursor——编辑操作的核心工具

QTextCursor 是操作 QTextDocument 的核心工具。它代表了文档中的一个位置或一个选中范围。几乎所有对文档的编辑操作（插入文字、设置格式、删除内容）都通过 QTextCursor 完成。

```cpp
QTextCursor cursor(textEdit->document());

// 在文档末尾插入加粗文字
cursor.movePosition(QTextCursor::End);

QTextCharFormat boldFormat;
boldFormat.setFontWeight(QFont::Bold);
boldFormat.setForeground(Qt::red);

cursor.insertText(QStringLiteral("Important: "), boldFormat);
cursor.insertText(QStringLiteral("check the log file."));  // 使用默认格式
```

QTextCursor 的关键概念是 position（光标位置）和 anchor（锚点位置）。当 position 和 anchor 相同时，光标是一个简单的插入点。当它们不同时，光标选中了从 anchor 到 position 之间的文本。movePosition 可以移动光标（带 MoveMode 参数控制是否扩展选中范围）。

### 3.3 QTextCharFormat 与 QTextBlockFormat

QTextCharFormat 控制字符级别的格式：字体、颜色、背景色、下划线、删除线等。QTextBlockFormat 控制段落级别的格式：对齐方式、缩进、行距、段前段后间距等。

```cpp
// 设置选中文字为红色加粗
QTextCursor cursor = textEdit->textCursor();
if (cursor.hasSelection()) {
    QTextCharFormat format;
    format.setFontWeight(QFont::Bold);
    format.setForeground(QColor(Qt::red));
    cursor.mergeCharFormat(format);  // merge 而不是 set，保留其他格式属性
}

// 设置当前段落为居中对齐
QTextBlockFormat blockFormat = cursor.blockFormat();
blockFormat.setAlignment(Qt::AlignCenter);
cursor.setBlockFormat(blockFormat);
```

注意 `mergeCharFormat` 和 `setCharFormat` 的区别。merge 会保留未指定的属性（比如你只设了颜色，字体不会变），而 set 会替换所有属性。大多数情况下应该用 merge。

现在有一道调试题。下面这段代码有什么问题？

```cpp
QTextCursor cursor(doc);
cursor.movePosition(QTextCursor::Start);
QTextCharFormat fmt;
fmt.setForeground(Qt::red);
cursor.setCharFormat(fmt);
```

问题在于 `setCharFormat` 在没有选中范围的情况下不会对已有文字生效——它只设置「后续插入的文字」的格式。如果你想让已有文字变红，需要先选中文字（设置 position 和 anchor），然后调用 `mergeCharFormat`。

### 3.4 语法高亮基础——QSyntaxHighlighter 的用法

Qt 提供了 QSyntaxHighlighter 类来简化语法高亮的实现。你继承它，实现 `highlightBlock(const QString& text)` 方法，在其中用正则表达式匹配关键词并设置格式。

```cpp
class JsonHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit JsonHighlighter(QTextDocument* parent)
        : QSyntaxHighlighter(parent)
    {
    }

protected:
    void highlightBlock(const QString& text) override
    {
        // 匹配 JSON key（引号内的字符串）
        QRegularExpression keyRe(R"("([^"]+)"\s*:)");
        auto it = keyRe.globalMatch(text);
        while (it.hasNext()) {
            auto match = it.next();
            QTextCharFormat keyFormat;
            keyFormat.setForeground(Qt::darkBlue);
            keyFormat.setFontWeight(QFont::Bold);
            setFormat(match.capturedStart(), match.capturedLength(), keyFormat);
        }
    }
};
```

QSyntaxHighlighter 在文档内容变化时自动重新高亮受影响的块，不需要手动管理刷新时机。

## 4. 踩坑预防

第一个坑是 QTextCursor 在空文档上的行为。当 QTextDocument 为空时，cursor 的 position() 返回 0，但 movePosition 大部分操作会失败（因为没有内容可以移动）。后果是插入文字的格式不生效或者插入位置不对。解决方案是在操作 cursor 之前检查 `cursor.isNull()`，或者在插入前确保文档至少有一个空块。

第二个坑是 QTextBlockFormat 的缩进单位。QTextBlockFormat 的 setIndent 使用的是「缩进级别」而不是像素值——indent=1 表示一级缩进，具体渲染宽度由 QTextDocument 的默认样式决定。如果你期望缩进 20 像素但设置了 indent=20，你会得到非常深的缩进。后果是段落布局完全不符合预期。解决方案是用 QTextBlockFormat 的 setLeftMargin/setRightMargin 来设置精确的像素级缩进。

第三个坑是 QSyntaxHighlighter 的性能。highlightBlock 在每次文档变化时被调用，如果正则表达式很复杂或者文档很长，高亮计算会阻塞 UI 线程。后果是大文件输入时明显卡顿。解决方案是限制高亮范围（只高亮可见区域），或者在正则匹配时使用非贪婪量词和更精确的模式。

## 5. 练习项目

练习项目：简易 Markdown 预览器。实现一个支持基本 Markdown 格式预览的控件。

具体要求是：左侧 QTextEdit 接受 Markdown 文本输入，右侧 QTextBrowser 显示格式化后的富文本。支持 `**粗体**`、`*斜体*`、`# 标题`、`- 列表` 四种格式。使用 QSyntaxHighlighter 在编辑区高亮 Markdown 语法。完成标准是编辑区输入实时更新预览区显示、格式转换正确、高亮不阻塞输入。

提示几个关键点：用 QTextDocument::contentsChanged 信号触发预览更新，QRegularExpression 匹配 Markdown 语法，QTextCursor 设置格式。

## 6. 官方文档参考链接

[Qt 文档 · QTextDocument](https://doc.qt.io/qt-6/qtextdocument.html) -- 富文本文档模型

[Qt 文档 · QTextCursor](https://doc.qt.io/qt-6/qtextcursor.html) -- 文本游标编辑器

[Qt 文档 · QSyntaxHighlighter](https://doc.qt.io/qt-6/qsyntaxhighlighter.html) -- 语法高亮基类

---

到这里，富文本和字体的进阶知识就拆完了。QTextDocument 的文档模型、QTextCursor 的编辑操作、QSyntaxHighlighter 的高亮机制——这些是构建文本编辑器和文档查看器的核心知识。
