# 现代Qt开发教程（新手篇）3.23——QTextEdit：富文本多行编辑器

## 1. 前言 / QTextEdit 不只是"多行的 QLineEdit"

如果你认为 QTextEdit 就是"可以输入多行文字的 QLineEdit"，那你的认知需要大幅更新了。QLineEdit 是一个纯文本单行输入控件，它的内部数据模型非常简单——就是一个 QString。而 QTextEdit 是一个完整的富文本编辑引擎，它的内部是一个 QTextDocument，这个 document 采用了"文档-帧-文本块-字符"四层结构来管理内容，支持段落格式、字符格式、图片嵌入、表格、列表、甚至 HTML 和 Markdown。QTextEdit 可以用来做代码编辑器、日志查看器、富文本编辑器、聊天消息展示区——这些场景的核心差异就在于你如何配置和使用 QTextEdit 的富文本能力。

当然，QTextEdit 的富文本能力也带来了性能上的代价——它需要在内存中维护完整的文档结构树，每一次按键都涉及到光标移动、格式计算、布局重排。如果你只是需要一个高性能的纯文本编辑器（比如日志终端或代码编辑器），Qt 提供了 QPlainTextEdit 作为轻量替代，我们会在后面的文章中专门讲它。这篇文章我们先聚焦 QTextEdit 本身的四个核心维度：纯文本与富文本模式的切换、HTML 内容的读写、QTextCursor 的光标操作、以及修改状态的追踪。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QTextEdit 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。QTextDocument、QTextCursor、QTextCharFormat 等 QTextEdit 内部使用的类都在 QtGui 模块中定义——因为 Widgets 依赖 Gui，所以不需要额外链接。QTextEdit 支持的富文本格式包括 HTML 4 的子集和 Markdown（Qt 5.14 开始支持），但不支持完整的 HTML5 或 CSS3——它实现的是 Qt 自己定义的富文本标记子集。

## 3. 核心概念讲解

### 3.1 纯文本 vs 富文本模式切换

QTextEdit 默认就是富文本模式——用户可以通过快捷键（Ctrl+B 加粗、Ctrl+I 斜体、Ctrl+U 下划线）来格式化文字，也可以粘贴带格式的内容。但很多时候你可能不希望用户在编辑器中使用富文本——比如代码编辑器、笔记应用中的纯文本模式、或者日志展示区域。这时候你可以通过 `setAcceptRichText(bool)` 来控制是否接受富文本输入。

```cpp
// 富文本模式（默认）
auto *richEditor = new QTextEdit();
richEditor->setAcceptRichText(true);

// 纯文本模式——不接受格式化，粘贴时自动去除格式
auto *plainEditor = new QTextEdit();
plainEditor->setAcceptRichText(false);
```

`setAcceptRichText(false)` 之后，用户无法通过快捷键应用格式，粘贴的内容会自动被转换为纯文本（所有格式信息被剥离）。但这里有一个微妙的地方：`setAcceptRichText` 只控制"输入端"——它影响的是用户的交互行为，不影响程序通过 API 设置的内容。也就是说，即使你设置了 `setAcceptRichText(false)`，你仍然可以在代码中调用 `setHtml()` 来设置带格式的 HTML 内容， QTextEdit 会正常渲染这些格式。它只是阻止了用户通过键盘和粘贴操作引入格式。

如果你想要一个真正"只能输入纯文本"的编辑器，QPlainTextEdit 是更好的选择。QPlainTextEdit 是 QTextEdit 的轻量版本，它在内部使用简化的数据结构（没有 QTextFrame 的开销），对大文本的性能表现更好。但 QPlainTextEdit 不支持富文本渲染——你无法在其中显示带格式的 HTML 内容。

QTextEdit 还有一个跟模式相关的方法：`setAutoFormatting(QTextEdit::AutoFormattingFlag)`。这个方法控制 QTextEdit 是否自动应用一些格式规则。`QTextEdit::AutoAll` 会启用所有自动格式化——包括自动创建有序/无序列表（当用户输入 "1. " 或 "* " 开头的行时）、自动替换引号字符等。如果你不希望这种自动行为，保持默认的 `QTextEdit::AutoNone` 即可。

```cpp
auto *editor = new QTextEdit();
// 启用自动列表格式化——输入 "* " 或 "1. " 会自动变成列表
editor->setAutoFormatting(QTextEdit::AutoAll);
```

### 3.2 内容读写：setHtml / toHtml / toPlainText

QTextEdit 提供了三种内容读写方式，分别对应不同的格式需求。

`setPlainText(const QString &)` 设置纯文本内容。所有文字都以同一种格式显示，不包含任何富文本格式。如果之前的内容是 HTML 格式的，调用 setPlainText 会清除所有格式。

`setHtml(const QString &)` 设置 HTML 格式的内容。QTextEdit 支持的 HTML 标签包括 `<b>`（加粗）、`<i>`（斜体）、`<u>`（下划线）、`<s>`（删除线）、`<font>`（字体/颜色/大小）、`<h1>`-`<h6>`（标题）、`<p>`（段落）、`<br>`（换行）、`<a>`（超链接）、`<img>`（图片）、`<table>`（表格）、`<ul>`/`<ol>`/`<li>`（列表）、`<pre>`（预格式化文本）等。它不支持 JavaScript、CSS 样式表、或者 DOM 操作——QTextEdit 不是浏览器。

`setMarkdown(const QString &)` 设置 Markdown 格式的内容。Qt 从 5.14 开始支持 Markdown 的子集，包括标题、加粗、斜体、链接、列表、代码块等基础语法。

对应的读取方法有三个：`toPlainText()` 返回纯文本（所有格式信息被剥离），`toHtml()` 返回 HTML 格式的字符串（包含所有格式标签），`toMarkdown()` 返回 Markdown 格式的字符串。

```cpp
auto *editor = new QTextEdit();

// 方式 1: 设置纯文本
editor->setPlainText("Hello World\n这是一行纯文本");

// 方式 2: 设置 HTML 内容
editor->setHtml(
    "<h2>欢迎</h2>"
    "<p>这是一个 <b>富文本</b> 编辑器的示例。</p>"
    "<p>支持 <i>斜体</i>、<u>下划线</u>、<s>删除线</s> 等格式。</p>"
    "<ul>"
    "  <li>列表项 1</li>"
    "  <li>列表项 2</li>"
    "</ul>");

// 方式 3: 设置 Markdown 内容
editor->setMarkdown(
    "## 欢迎\n"
    "\n"
    "这是一个 **富文本** 编辑器的示例。\n"
    "\n"
    "- 列表项 1\n"
    "- 列表项 2\n");
```

有一个非常容易踩的坑：`setHtml()` 和 `setPlainText()` 的调用会互相覆盖。如果你先调了 `setHtml()` 然后调了 `setPlainText()`，HTML 的所有格式信息都会丢失。反过来也一样。这两个方法不是"追加"，而是"完全替换"。如果你需要在现有内容后面追加文字，应该使用 QTextCursor 的 insertText 方法，后面我们会讲。

`toHtml()` 的返回值有一些需要注意的地方。QTextEdit 生成的 HTML 包含了大量的内联样式和文档元信息——它不是你手写的简洁 HTML。比如一段简单的加粗文字，`toHtml()` 的返回可能是这样的：

```html
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0//EN" "http://www.w3.org/TR/REC-html40/strict.dtd">
<html><head><meta name="qrichtext" content="1" /><style type="text/css">
p, li { white-space: pre-wrap; }
</style></head><body style=" font-family:'Noto Sans'; font-size:10pt;">
<p style=" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;">
<span style=" font-weight:700;">加粗文字</span></p></body></html>
```

你可以看到它包含了完整的 HTML 文档结构、DOCTYPE 声明、内联样式等。所以 `toHtml()` 的返回值通常不适合直接展示给用户或者存入需要人类可读的配置文件中。如果你需要保存和恢复编辑器内容，`toHtml()` 是最可靠的方式；如果你需要更简洁的格式，考虑 `toMarkdown()` 或 `toPlainText()`。

### 3.3 光标操作：QTextCursor 插入/选中/格式化

QTextEdit 的内容编辑不是通过直接操作字符串来完成的——你需要通过 QTextCursor 来完成。QTextCursor 是 QTextEdit 的编辑接口，它代表了文档中的一个"光标位置"（也就是插入点），同时也可以表示一个选区（选中了一段文字）。QTextCursor 提供了插入文字、插入图片、选中文字、应用格式等操作。

你可以通过 `textCursor()` 获取当前编辑器的光标对象。注意这个方法返回的是一个副本（value type），不是引用——你对返回的 QTextCursor 做的修改不会自动反映到编辑器上。你需要修改完之后调用 `setTextCursor()` 把修改后的光标写回编辑器。

```cpp
// 在当前光标位置插入文字
QTextCursor cursor = editor->textCursor();
cursor.insertText("插入的文字");

// 不需要 setTextCursor，因为 insertText 已经修改了文档
// 但如果你想移动光标位置，就需要 setTextCursor
cursor.movePosition(QTextCursor::End);
editor->setTextCursor(cursor);
```

`movePosition(QTextCursor::MoveOperation, QTextCursor::MoveMode, int n)` 是光标移动的核心方法。MoveOperation 指定了移动方向和粒度：`QTextCursor::Start` 移动到文档开头，`QTextCursor::End` 移动到文档末尾，`QTextCursor::Up` / `Down` / `Left` / `Right` 按字符或行移动，`QTextCursor::WordLeft` / `WordRight` 按单词移动，`QTextCursor::StartOfLine` / `EndOfLine` 移动到行首/行尾。MoveMode 指定了是否保留选区：`QTextCursor::MoveAnchor`（默认）是移动光标位置，不选中文字；`QTextCursor::KeepAnchor` 是保持锚点不动、移动光标位置，结果就是选中了从锚点到新位置之间的文字。

```cpp
QTextCursor cursor = editor->textCursor();

// 选中整行
cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);

// 选中整个文档
cursor.select(QTextCursor::Document);

// 选中当前单词
cursor.select(QTextCursor::WordUnderCursor);

editor->setTextCursor(cursor);
```

QTextCursor 配合 QTextCharFormat 可以实现字符级别的格式化。QTextCharFormat 继承自 QTextFormat，用来设置字符的外观属性——字体、颜色、背景色、加粗、斜体、下划线、删除线等等。

```cpp
// 在光标位置插入一段带格式的文字
QTextCursor cursor = editor->textCursor();

QTextCharFormat boldFormat;
boldFormat.setFontWeight(QFont::Bold);
boldFormat.setForeground(QColor("#1976D2"));

cursor.insertText("蓝色加粗标题", boldFormat);

QTextCharFormat normalFormat;
normalFormat.setFontWeight(QFont::Normal);
normalFormat.setForeground(QColor("#333333"));

cursor.insertText(" — 这是普通的说明文字", normalFormat);
```

如果你需要对已经存在的文字应用格式（而不是插入新文字），需要先选中目标文字，然后调用 `mergeCharFormat()`。`mergeCharFormat` 只修改你指定的属性，保留其他属性不变——这比 `setCharFormat`（完全替换所有属性）更安全。

```cpp
QTextCursor cursor = editor->textCursor();

// 选中当前行
cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);

// 把选中文字设为红色加粗
QTextCharFormat highlightFormat;
highlightFormat.setForeground(QColor("#D32F2F"));
highlightFormat.setFontWeight(QFont::Bold);
cursor.mergeCharFormat(highlightFormat);

editor->setTextCursor(cursor);
```

段落级别的格式通过 QTextBlockFormat 来设置——对齐方式、缩进、行距、段前段后间距等。

```cpp
QTextCursor cursor = editor->textCursor();

QTextBlockFormat centerFormat;
centerFormat.setAlignment(Qt::AlignCenter);
cursor.mergeBlockFormat(centerFormat);

cursor.insertText("居中标题\n");
```

QTextCursor 是 QTextEdit 最核心的编程接口，几乎所有"程序控制编辑器内容"的操作都需要通过它来完成。掌握 QTextCursor 的 movePosition / insertText / select / mergeCharFormat 这几个核心方法，你就能实现大部分编辑器操作。

### 3.4 document()->setModified() 追踪修改状态

QTextEdit 内部持有一个 QTextDocument 对象，你可以通过 `document()` 方法获取它。QTextDocument 除了管理文档内容和格式之外，还提供了一个非常实用的功能：修改状态追踪。

`QTextDocument::isModified()` 返回一个 bool 值，表示文档自上次"清除修改标记"以来是否被修改过。用户每次编辑内容（输入、删除、粘贴）都会把 modified 标记设为 true。你可以通过 `document()->setModified(false)` 来清除这个标记——通常在保存文件、加载新文件、或者初始化内容之后调用。

```cpp
auto *editor = new QTextEdit();

// 初始化内容后清除修改标记
editor->setHtml("<h1>初始内容</h1><p>一些文字...</p>");
editor->document()->setModified(false);

// 用户编辑内容后检查是否需要保存
if (editor->document()->isModified()) {
    qDebug() << "文档已修改，需要保存";
}
```

QTextDocument 还提供了一个 `modificationChanged(bool)` 信号，在 modified 状态发生变化时触发。你可以用这个信号来实现窗口标题中的"未保存"标记（文件名后面加一个星号），或者控制"保存"按钮的启用/禁用状态。

```cpp
connect(editor->document(), &QTextDocument::modificationChanged,
        this, [this](bool modified) {
    if (modified) {
        setWindowTitle(windowTitle() + " *");
    } else {
        // 移除标题末尾的 " *"
        QString title = windowTitle();
        title.remove(" *");
        setWindowTitle(title);
    }
});
```

修改状态追踪在文本编辑器类应用中几乎是标配功能。一个完整的"打开-编辑-保存"流程应该是这样的：加载文件内容到 QTextEdit，调用 `document()->setModified(false)` 清除标记；用户编辑过程中 modified 自动变为 true，窗口标题显示星号；用户点击保存时把内容写入文件，再次调用 `setModified(false)` 清除标记；用户关闭窗口时检查 `isModified()`，如果为 true 则弹出"是否保存"的确认对话框。

```cpp
void closeEvent(QCloseEvent *event) override
{
    if (m_editor->document()->isModified()) {
        auto ret = QMessageBox::question(
            this, "保存确认",
            "文档已修改但未保存，是否保存？",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        if (ret == QMessageBox::Save) {
            saveToFile();
            event->accept();
        } else if (ret == QMessageBox::Discard) {
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        event->accept();
    }
}
```

这里有一个需要特别注意的点：`setHtml()` 和 `setPlainText()` 等方法会触发内容变化，从而把 modified 标记设为 true。所以如果你在初始化内容或加载文件后忘了调用 `setModified(false)`，你的编辑器会一直显示"已修改"状态——即使实际上什么都没改。这看起来是个小事，但用户看到标题栏上一直有个星号，心里会很不踏实。

另外一个相关的信号是 `QTextEdit::textChanged()`——它在文档内容发生任何变化时触发。`textChanged` 和 `modificationChanged` 的区别在于：`textChanged` 在每次内容变化时都触发（即使 modified 之前已经是 true，再次变化也会触发 textChanged），而 `modificationChanged` 只在 modified 状态从 false 变为 true 或从 true 变为 false 时触发。

## 4. 踩坑预防

第一个坑是 `toHtml()` 返回的 HTML 非常冗长。QTextEdit 生成的 HTML 包含完整的文档结构和大量的内联样式，不适合作为简洁的 HTML 输出使用。如果你需要简洁的格式，考虑 `toMarkdown()` 或自己遍历 QTextDocument 的文本块来生成格式。

第二个坑是 QTextCursor 是值类型。`textCursor()` 返回的是副本，你对它做的位置移动不会自动反映到编辑器上。如果你移动了光标位置，必须调用 `setTextCursor()` 写回。但 `insertText()` 等修改文档内容的操作会直接反映到文档上，不需要额外的写回步骤。

第三个坑是 `setHtml()` 之后 modified 被设为 true。程序初始化内容或加载文件后，务必调用 `document()->setModified(false)` 来清除修改标记，否则编辑器会一直处于"已修改"状态。

第四个坑是 QTextEdit 不适合做高性能日志终端。QTextEdit 的富文本引擎每次追加内容都涉及文档结构更新和布局重排，当日志量达到几万行时性能会明显下降。如果你的场景是持续追加大量纯文本（比如程序日志、终端输出），应该用 QPlainTextEdit 配合 `setMaximumBlockCount()` 来限制内存使用。

第五个坑是 `setAcceptRichText(false)` 只控制用户输入端。即使设为 false，程序仍然可以通过 `setHtml()` 设置富文本内容。如果你需要一个完全禁止富文本的编辑器，要么自己在每次内容变化后检查并清除格式，要么使用 QPlainTextEdit。

## 5. 练习项目

我们来做一个综合练习：创建一个"迷你文本编辑器"窗口，展示 QTextEdit 的各项能力。窗口顶部有一个工具栏（用 QToolBar 或一组 QPushButton 实现），包含"加粗""斜体""下划线"三个格式按钮，以及一个"插入蓝色标题"按钮和一个"清除所有格式"按钮。窗口中央是一个 QTextEdit 作为编辑区域，初始化时加载一段 HTML 格式的示例内容。窗口底部有一个状态栏，显示当前文档的字符数、行数、以及修改状态（"已修改"或"未修改"）。点击格式按钮时，通过 QTextCursor 对当前选中的文字应用对应的 QTextCharFormat。点击"插入蓝色标题"时，在当前光标位置插入一段蓝色加粗的文字。点击"清除所有格式"时，调用 setPlainText(toPlainText()) 把所有格式剥离。关闭窗口时，如果文档处于 modified 状态，弹出"是否保存"的确认对话框。

几个提示：格式按钮用 `textCursor().mergeCharFormat(format)` 对选中文字应用格式，如果没有选中文字则对当前光标位置的格式做预设置（下次输入的文字会自动应用这个格式）；字符数和行数在 `textChanged` 信号中用 `document()->characterCount()` 和 `document()->blockCount()` 获取；修改状态用 `document()->modificationChanged` 信号驱动状态栏更新；关闭事件重写 `closeEvent` 检查 `document()->isModified()`。

## 6. 官方文档参考链接

[Qt 文档 · QTextEdit](https://doc.qt.io/qt-6/qtextedit.html) -- 富文本编辑器

[Qt 文档 · QTextDocument](https://doc.qt.io/qt-6/qtextdocument.html) -- 富文本文档模型

[Qt 文档 · QTextCursor](https://doc.qt.io/qt-6/qtextcursor.html) -- 文档光标（编辑接口）

[Qt 文档 · QTextCharFormat](https://doc.qt.io/qt-6/qtextcharformat.html) -- 字符格式

[Qt 文档 · QTextBlockFormat](https://doc.qt.io/qt-6/qtextblockformat.html) -- 段落格式

[Qt 文档 · QPlainTextEdit](https://doc.qt.io/qt-6/qplaintextedit.html) -- 纯文本编辑器（轻量替代）

---

到这里，QTextEdit 的四个核心维度我们就全部讲完了。纯文本和富文本模式通过 setAcceptRichText 来切换，但要注意它只控制输入端；setHtml / toHtml / toPlainText 提供了三种不同格式的内容读写方式，setHtml 生成的 HTML 比较冗长但能完整保存和恢复编辑器状态；QTextCursor 是编辑器的核心编程接口，所有的插入、选中、格式化操作都通过它完成，记住它是值类型需要 setTextCursor 写回；document()->setModified() 配合 modificationChanged 信号可以实现完整的"未保存"状态追踪。QTextEdit 的富文本引擎在小型到中型文档上表现很好，但如果你需要处理大量纯文本，QPlainTextEdit 是更合适的选择。
