# 现代Qt开发教程（新手篇）3.24——QPlainTextEdit：纯文本高性能编辑器

## 1. 前言 / 为什么 QTextEdit 不是万能的

上一篇我们花了整篇文章来拆解 QTextEdit 的富文本能力——QTextDocument 四层结构、QTextCursor 的光标操作、字符格式和段落格式。QTextEdit 确实强大，但它有一个绕不开的软肋：性能。QTextEdit 内部维护的是一棵完整的文档结构树，每一次内容追加都涉及 QTextFrame、QTextBlock 的创建和格式计算，哪怕你写入的只是一行没有任何格式的纯文本日志。当你的文档行数突破几万行的时候，你会发现滚动开始卡顿、追加内容开始出现肉眼可见的延迟，内存占用也会一路飙升——因为 QTextEdit 一直在默默地为那些你根本不需要的富文本能力买单。

这就是 QPlainTextEdit 存在的理由。QPlainTextEdit 继承自 QAbstractScrollArea（和 QTextEdit 是兄弟关系，不是父子关系），它在设计之初就只为一个目标服务：高性能地显示和编辑纯文本。它没有 QTextDocument 的四层结构，没有 QTextFrame，没有字符格式的概念，取而代之的是一种简化的文本块模型——每个段落就是一个 QTextBlock，仅此而已。这种简化让 QPlainTextEdit 在处理大文本时拥有显著的性能优势，也使它成为构建日志查看器、终端模拟器、代码编辑器的理想选择。我们今天就来把 QPlainTextEdit 的四个核心能力拆清楚：它和 QTextEdit 的本质差异、appendPlainText 追加日志的正确姿势、setMaximumBlockCount 防内存溢出的机制、以及实现当前行高亮效果的方法。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QPlainTextEdit 属于 QtWidgets 模块，和 QTextEdit 一样链接 Qt6::Widgets 即可。QPlainTextEdit 虽然不使用完整的 QTextDocument 富文本引擎，但它内部仍然使用 QTextDocument 来存储纯文本内容——只不过这个 QTextDocument 被配置为"纯文本模式"，不处理格式信息。所以你会看到 QPlainTextEdit 依然可以通过 `document()` 方法获取 QTextDocument 指针，也可以使用 QTextCursor 进行光标操作，但能力相比 QTextEdit 有所缩减（比如没有字符格式化、没有 HTML 渲染）。

## 3. 核心概念讲解

### 3.1 与 QTextEdit 的核心区别

我们先从继承关系上把这两个类的关系理清楚。QTextEdit 和 QPlainTextEdit 不是父子关系，它们各自继承自不同的基类：QTextEdit 继承自 QAbstractScrollArea（通过 QFrame 中转），QPlainTextEdit 也继承自 QAbstractScrollArea。两者的共同父类是 QAbstractScrollArea，这意味着它们都是"带滚动条的文档视图"，但在文档模型的实现上走了完全不同的路线。

QTextEdit 使用完整的 QTextDocument 富文本引擎。这个引擎支持段落格式（QTextBlockFormat）、字符格式（QTextCharFormat）、列表格式（QTextListFormat）、表格格式（QTextTableFormat）、帧格式（QTextFrameFormat）——一个完整的"文档-帧-文本块-字符"四层结构。每次你调用 `append()` 或 `setHtml()` 时，QTextEdit 都需要解析内容、创建对应的格式对象、更新布局。这些操作在小文本上几乎无感，但在大文本场景下（比如持续追加的程序日志、上十万行的代码文件）就会成为性能瓶颈。

QPlainTextEdit 使用简化的纯文本文档模型。它虽然内部仍然是 QTextDocument，但禁用了帧（QTextFrame）的嵌套结构，也不维护字符格式信息。每个段落就是一个扁平的 QTextBlock，段落之间没有额外的结构层次。这种简化带来了几个直接的收益：内存占用更低（不需要存储格式信息），追加内容的速度更快（不需要格式解析和布局计算），滚动渲染的性能更好（布局计算更简单）。

另外一个重要的区别是内容操作 API。QTextEdit 提供了 `setHtml()`、`setMarkdown()`、`toHtml()`、`toMarkdown()` 等富文本相关的读写方法，而 QPlainTextEdit 只提供 `setPlainText()`、`toPlainText()` 和 `appendPlainText()`。QPlainTextEdit 没有 `append()` 方法——这是一个很容易混淆的细节。QTextEdit 有 `append(const QString &)`，QPlainTextEdit 有 `appendPlainText(const QString &)`。如果你在 QPlainTextEdit 上调用了 `append()`（继承自 QTextEdit 的方法），它实际上会以富文本模式追加内容，这会在纯文本编辑器中引入不必要的格式开销。所以对于 QPlainTextEdit，请始终使用 `appendPlainText()`。

```cpp
// QTextEdit: 富文本追加
auto *richEdit = new QTextEdit();
richEdit->append("<b>加粗文字</b>");      // 以 HTML 方式追加，会渲染格式
richEdit->appendHtml("<i>斜体</i>");       // 显式 HTML 追加

// QPlainTextEdit: 纯文本追加
auto *plainEdit = new QPlainTextEdit();
plainEdit->appendPlainText("[INFO] 服务启动完成");   // 纯文本追加，无格式开销
// 注意: 不要调用 append()，那会走富文本路径
```

在 API 层面还有一个细节值得注意：QPlainTextEdit 的 `setPlainText()` 会替换整个文档内容，而 `appendPlainText()` 是在文档末尾追加一行新的文本块。`appendPlainText()` 的行为是：在当前文档末尾添加一个新段落（QTextBlock），然后把参数中的文本设置到这个段落中。如果文档当前为空，它会直接创建第一个段落。

最后提一下选择标准。如果你的场景是富文本编辑器、聊天消息渲染、带格式的文档查看器——用 QTextEdit。如果你的场景是日志终端、代码编辑器、大文本查看器——用 QPlainTextEdit。两者之间没有谁好谁坏，只是设计目标不同。

### 3.2 appendPlainText() 追加日志的正确用法

`appendPlainText()` 是 QPlainTextEdit 在日志场景下最核心的方法，但它的正确用法比很多人想象的要讲究。

最基础的用法很简单——调用 `appendPlainText()` 传入一行日志文本，QPlainTextEdit 就会在文档末尾追加一个新段落并显示。追加完成后滚动条不会自动跳到底部，如果你在做日志终端，通常需要在追加之后手动把滚动条拉到最底下。

```cpp
auto *logView = new QPlainTextEdit();
logView->setReadOnly(true);
logView->appendPlainText("[INFO] 应用启动");
logView->appendPlainText("[INFO] 数据库连接成功");
logView->appendPlainText("[WARN] 配置文件不存在，使用默认值");

// 追加后自动滚动到底部
QTextCursor cursor = logView->textCursor();
cursor.movePosition(QTextCursor::End);
logView->setTextCursor(cursor);
```

这里有一个很容易踩的坑：`appendPlainText()` 会自动在追加内容的前面插入一个换行符（如果文档不为空的话）。也就是说，每次调用 `appendPlainText()` 都会产生一个新的段落（QTextBlock），段落之间用换行分隔。你不能用 `appendPlainText()` 来在同一行后面追加内容——如果你需要这种效果，应该通过 QTextCursor 来操作。

在日志场景中，一个常见的模式是用定时器模拟持续追加日志。我们来看一个完整的追加日志模式：

```cpp
class LogViewer : public QWidget
{
    Q_OBJECT

public:
    explicit LogViewer(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        m_logView = new QPlainTextEdit();
        m_logView->setReadOnly(true);
        m_logView->setMaximumBlockCount(1000);  // 限制最大行数

        // 模拟日志追加
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &LogViewer::appendLog);
        m_timer->start(500);  // 每 500ms 追加一行
    }

private slots:
    void appendLog()
    {
        static int counter = 0;
        QString logLine = QString("[%1] 日志条目 #%2")
            .arg(QTime::currentTime().toString("HH:mm:ss.zzz"))
            .arg(++counter);
        m_logView->appendPlainText(logLine);

        // 自动滚动到底部
        QTextCursor cursor = m_logView->textCursor();
        cursor.movePosition(QTextCursor::End);
        m_logView->setTextCursor(cursor);
    }

private:
    QPlainTextEdit *m_logView = nullptr;
    QTimer *m_timer = nullptr;
};
```

有一点需要留意：`appendPlainText()` 默认不会触发滚动到文档底部。如果你在做日志终端，用户期望看到的是最新的日志条目，所以每次追加后都需要手动把光标移到文档末尾。上面代码中的三行（获取光标、移动到末尾、写回光标）就是实现自动滚动的标准模式。还有一种更简洁的写法是用 `ensureCursorVisible()`——但这个方法只在光标已经在末尾的时候才有效，如果用户手动把光标点到了文档中间，`ensureCursorVisible()` 就不会滚到底部。所以对于日志终端，显式地移动光标到末尾再调用 `ensureCursorVisible()` 是更稳妥的做法。

另外一个实战中很常见的技巧是日志着色。QPlainTextEdit 不支持字符级别的格式化（那是 QTextEdit 的活），但你可以通过 `appendHtml()` 方法来追加带 HTML 格式的日志行。是的，QPlainTextEdit 虽然叫"纯文本"编辑器，但它确实提供了 `appendHtml()` 这个方法——这是一个容易让人困惑的设计。`appendHtml()` 会解析 HTML 标签并按富文本方式渲染那一行内容，但不会启用完整的富文本编辑能力。所以在日志查看器中，你可以用 `appendHtml()` 来实现简单的彩色日志输出：

```cpp
// 追加红色错误日志
logView->appendHtml(
    QString("<span style='color:#D32F2F;'>[ERROR]</span> "
            "连接超时 (%1)")
        .arg(QTime::currentTime().toString("HH:mm:ss")));
```

不过要注意，一旦你开始使用 `appendHtml()`，性能优势就会打折扣——HTML 解析和格式渲染是 QPlainTextEdit 试图避免的开销。如果你的日志量非常大（比如每秒几十行），建议全部使用 `appendPlainText()` 配合 QSyntaxHighlighter 来实现着色，而不是每行都用 HTML。

### 3.3 setMaximumBlockCount() 限制行数防内存溢出

日志终端有一个绕不开的问题：如果你不停地追加日志行，内存迟早会被撑爆。程序跑了三天，日志积累了几百万行，QPlainTextEdit 内部的 QTextDocument 持续膨胀，最终系统 OOM killer 上门——这种事在生产环境中并不罕见。

`setMaximumBlockCount(int)` 就是 QPlainTextEdit 为这个场景提供的内置解决方案。它的工作原理非常直观：当文档中的文本块（段落）数量超过设定值时，QPlainTextEdit 会自动从文档开头删除最旧的文本块，保持文档中的文本块总数不超过设定值。

```cpp
auto *logView = new QPlainTextEdit();
logView->setMaximumBlockCount(5000);  // 最多保留 5000 行
```

设置了 `setMaximumBlockCount(5000)` 之后，当第 5001 行日志被追加进来时，第 1 行会自动被删除。文档始终只保留最新的 5000 行，内存占用被控制在一个可预测的范围内。这比你自己写一个"超过 N 行就截断前面内容"的逻辑要高效得多，因为 Qt 内部直接操作文本块链表，不需要重建整个文档。

`maximumBlockCount` 的默认值是 0，表示不限制。如果你不设置这个值，QPlainTextEdit 会保留所有追加的内容，内存会随时间线性增长。在生产环境的日志终端中，强烈建议设置一个合理的上限。

选择上限值的时候需要考虑几个因素：你的日志产生速率（每秒多少行），你希望用户能向上回溯多少历史记录，以及你能接受的内存开销。一般来说，几千到一万行是一个比较合理的范围——以每行平均 100 字节计算，一万行大约占 1MB 内存（加上 QTextDocument 的内部结构开销，实际可能在 3-5MB 左右）。对于桌面应用来说这个量级完全不是问题。

```cpp
// 根据场景选择合理的上限
logView->setMaximumBlockCount(1000);    // 轻量日志：1k 行足够
logView->setMaximumBlockCount(5000);    // 中等日志：5k 行平衡内存与回溯
logView->setMaximumBlockCount(0);       // 不限制：仅在日志量可控时使用
```

有一个细节需要注意：当 `setMaximumBlockCount` 触发旧行删除时，如果你有其他代码在访问文档的文本块（比如通过 `document()->begin()` 遍历），可能会遇到迭代器失效的问题。所以在日志终端中，尽量避免在追加日志的同时遍历文档内容。

另一个相关的 API 是 `maximumBlockCount()`（getter），用来获取当前的最大行数限制。如果你需要动态调整限制（比如用户在设置界面修改了日志保留行数），可以随时调用 `setMaximumBlockCount()` 修改——修改后的新值会立即生效，如果当前行数已经超过新设定的上限，多余的旧行会被立即删除。

### 3.4 highlightCurrentLine() 实现行高亮效果

如果你用过代码编辑器，一定对"当前行高亮"这个功能不陌生——光标所在行的背景色与其他行不同，让你在密集的文本中能快速定位自己正在编辑的行。QPlainTextEdit 没有内置的当前行高亮功能，但实现起来并不复杂。

QPlainTextEdit 提供了一个虚函数 `highlightCurrentLine()`——等等，严格来说它并没有这个虚函数。这是一个需要我们自己实现的模式，核心思路是监听光标位置变化，然后通过"行内选区 + 额外选区"机制来为当前行设置背景色。

QPlainTextEdit 提供了一个叫 `extraSelections` 的机制，允许你在不修改文档内容的情况下添加视觉高亮。`QTextEdit::ExtraSelection` 结构体包含一个 QTextCursor 和一个 QTextCharFormat——QTextCursor 指定要高亮的文本范围，QTextCharFormat 指定高亮的样式。你可以通过 `setExtraSelections(const QList<QTextEdit::ExtraSelection> &)` 来设置一组额外选区，它们会以半透明的方式叠加在正常文本上方。

```cpp
class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent = nullptr)
        : QPlainTextEdit(parent)
    {
        connect(this, &QPlainTextEdit::cursorPositionChanged,
                this, &CodeEditor::highlightCurrentLine);
    }

private:
    void highlightCurrentLine()
    {
        QList<QTextEdit::ExtraSelection> selections;

        QTextEdit::ExtraSelection lineSelection;
        lineSelection.format.setBackground(QColor("#E8F0FE"));
        lineSelection.format.setProperty(
            QTextCharFormat::FullWidthSelection, true);
        lineSelection.cursor = textCursor();
        lineSelection.cursor.clearSelection();

        selections.append(lineSelection);
        setExtraSelections(selections);
    }
};
```

我们来逐行拆解这段代码。首先是构造函数中的信号连接——`cursorPositionChanged` 在光标位置发生变化时触发（用户点击了新的位置、按了方向键、输入了文字等），我们把它连接到 `highlightCurrentLine` 槽函数上。然后是 `highlightCurrentLine` 的实现。我们创建了一个 `ExtraSelection` 对象，设置它的背景色为浅蓝色 `#E8F0FE`。`QTextCharFormat::FullWidthSelection` 是一个关键属性——它告诉 Qt 这个选区应该占据整行的宽度，而不是仅仅覆盖到文字结束的位置。没有这个属性的话，高亮只会出现在有文字的区域，行末的空白区域不会高亮，看起来会很不自然。然后我们获取当前光标并调用 `clearSelection()` 清除选区（我们只需要光标位置，不需要选中任何文字），最后通过 `setExtraSelections()` 应用这个高亮。

这里有一个需要注意的地方：`setExtraSelections()` 会替换之前的所有额外选区。如果你同时使用了多个额外选区（比如当前行高亮 + 搜索结果高亮），你需要在每次调用时把所有选区一起传入。一个常见的做法是维护一个成员变量来存储所有的额外选区，每次更新时先清空再重新添加：

```cpp
void highlightCurrentLine()
{
    m_extraSelections.clear();

    // 当前行高亮
    QTextEdit::ExtraSelection lineSelection;
    lineSelection.format.setBackground(QColor("#E8F0FE"));
    lineSelection.format.setProperty(
        QTextCharFormat::FullWidthSelection, true);
    lineSelection.cursor = textCursor();
    lineSelection.cursor.clearSelection();
    m_extraSelections.append(lineSelection);

    // 这里可以追加更多选区（比如搜索结果高亮、错误标记等）

    setExtraSelections(m_extraSelections);
}
```

如果你还想在左侧显示行号，QPlainTextEdit 提供了一个配套机制：`setLineNumberArea`——不过严格来说 Qt 没有提供内置的行号区域组件，你需要自己实现一个 QWidget 作为行号面板。Qt 官方示例中的 Code Editor Example 演示了完整的行号面板实现方式，核心思路是重写 `QPlainTextEdit::lineCountChanged` 和 `QPlainTextEdit::blockCountChanged` 信号，以及 `paintEvent` 和 `resizeEvent` 来绘制和管理行号区域。这个实现比较长，我们这里就不展开了，有兴趣的读者可以参考 Qt 官方的 Code Editor Example。

## 4. 踩坑预防

第一个坑是在 QPlainTextEdit 上误用 `append()`。QPlainTextEdit 虽然"继承"了 QTextEdit 的部分接口（通过各自的基类实现），但调用 `append()` 会走富文本路径，带来不必要的格式解析开销。日志追加请始终使用 `appendPlainText()`。

第二个坑是追加日志后忘记滚动到底部。`appendPlainText()` 不会自动把视图滚动到最新内容。如果你的场景是日志终端或聊天窗口，每次追加后需要手动将光标移到文档末尾。

第三个坑是 `setMaximumBlockCount(0)` 是默认值，表示不限制行数。如果你在做持续追加日志的场景而不设置上限，长时间运行后内存会持续增长。建议在创建 QPlainTextEdit 时就设置一个合理的上限，不要等到内存报警了才想起来加。

第四个坑是 `extraSelections` 会被完全替换而非追加。每次调用 `setExtraSelections()` 时传入的列表会完全替换之前的所有额外选区。如果你有多个来源的选区（行高亮、搜索高亮、错误标记），必须在一个列表中统一管理，不要分多次调用。

第五个坑是在高频率日志追加时使用 `appendHtml()`。HTML 解析和格式渲染的开销在单行追加时不明显，但如果你的日志速率是每秒几十行甚至上百行，累积的解析开销会导致界面卡顿。高频率场景请使用 `appendPlainText()` 配合 QSyntaxHighlighter 来实现着色。

## 5. 练习项目

我们来做一个综合练习：创建一个"模拟日志终端"窗口，展示 QPlainTextEdit 的核心能力。窗口顶部有一排控制按钮：一个"开始日志"按钮用于启动定时器（每 300ms 追加一行带时间戳的模拟日志），一个"停止日志"按钮用于停止定时器，一个"清空日志"按钮清除所有内容，以及一个下拉选择框让用户切换最大保留行数（选项为 100 / 500 / 1000 / 无限制）。窗口中央是一个 QPlainTextEdit 作为日志显示区域，设置为只读模式。日志内容格式为 `[HH:mm:ss.zzz] [LEVEL] 消息内容`，其中 LEVEL 随机从 INFO、WARN、ERROR 中选取。窗口底部显示当前日志行数和最大行数限制。关闭定时器后用户可以点击日志区域查看任意位置——此时当前行高亮功能生效，光标所在行的背景色变为浅蓝色。

几个提示：定时器用 QTimer 配合 lambda 或槽函数来追加日志，日志级别用 `QRandomGenerator` 随机选取；行数限制通过 `setMaximumBlockCount()` 设置，下拉框的 `currentIndexChanged` 信号连接到一个槽函数中根据选项值设置上限（无限制对应值为 0）；当前行高亮通过 `cursorPositionChanged` 信号驱动，使用 `QTextEdit::ExtraSelection` 配合 `FullWidthSelection` 属性来实现整行背景色。

## 6. 官方文档参考链接

[Qt 文档 · QPlainTextEdit](https://doc.qt.io/qt-6/qplaintextedit.html) -- 纯文本编辑器

[Qt 文档 · QTextDocument](https://doc.qt.io/qt-6/qtextdocument.html) -- 底层文本文档模型

[Qt 文档 · QTextCursor](https://doc.qt.io/qt-6/qtextcursor.html) -- 文档光标（编辑接口）

[Qt 文档 · QTextEdit::ExtraSelection](https://doc.qt.io/qt-6/qtextedit.html#ExtraSelection-struct) -- 额外选区结构体

[Qt 示例 · Code Editor Example](https://doc.qt.io/qt-6/qtwidgets-widgets-codeeditor-example.html) -- 带行号的代码编辑器示例

---

到这里，QPlainTextEdit 的四个核心维度我们就全部讲完了。它和 QTextEdit 的核心区别在于简化的文档模型——没有 QTextFrame 嵌套、没有字符格式，换来的是更好的大文本性能和更低的内存占用。`appendPlainText()` 是日志追加的标准方法，追加后记得手动滚动到底部。`setMaximumBlockCount()` 提供了内置的行数上限机制，防止长时间运行的日志终端把内存吃光。`extraSelections` 机制让你可以在不修改文档内容的情况下添加视觉高亮，当前行高亮就是它最经典的应用场景。如果你的场景不需要富文本，QPlainTextEdit 是比 QTextEdit 更好的选择。
