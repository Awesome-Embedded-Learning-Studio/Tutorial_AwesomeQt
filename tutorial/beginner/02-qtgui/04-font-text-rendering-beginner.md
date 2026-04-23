# 现代Qt开发教程（新手篇）2.4——QFont 与文本渲染基础

## 1. 前言 / 为什么文本渲染值得单独学

很多初学者觉得文字显示有什么好学的？给 QLabel 设个文本不就行了。说实话，我一开始也是这么想的。直到有一天我需要在一个自定义的 Widget 上精确排列多行文字——要在指定位置画出标题、正文、注释，每段的字体大小还不一样，还要在文字之间留出精确的间距。那会儿我才意识到，Qt 的文本渲染远不只是 `drawText()` 一行代码那么简单。

QPainter 的 `drawText()` 确实能画文字，但如果你需要知道一段文字画出来之后占多大空间、需要在哪里换行、多行文字的行间距怎么算——这些都需要 QFont 和 QFontMetrics 的配合。而如果你需要渲染带格式的富文本——加粗、斜体、不同颜色混排——那就得请出 QTextDocument 了。

这篇文章我们一起来把 Qt 的文本渲染体系搞清楚：QFont 怎么设置字体属性、drawText 的各种用法和坐标含义、QFontMetrics 怎么帮我们计算文字尺寸做精确布局、以及 QTextDocument 的富文本渲染基础。搞懂这些之后，你在自定义控件里排列文字就跟玩积木一样顺手。

## 2. 环境说明

本篇代码适用于 Qt 6.5+ 版本（示例基于 Qt 6.9.1 验证），CMake 3.26+，C++17 标准。示例代码依赖 QtGui 和 QtWidgets 模块——QFont 和 QFontMetrics 在 QtGui 中，QTextDocument 也在 QtGui 中，但我们的展示窗口需要 QWidget，所以两个模块都要链接。关于字体：示例中使用了一些通用字体名称如 "Arial"、"Sans"、"Monospace"，Qt 会根据平台自动映射到可用的实际字体，你不需要额外安装什么字体包。所有代码在 Linux、Windows、macOS 上都可以编译运行。

## 3. 核心概念讲解

### 3.1 QFont 构造与属性设置

QFont 是 Qt 里描述字体的类。它本身不包含任何文字数据，它只是告诉 Qt "我想用什么字体来显示文字"。创建 QFont 最直接的方式就是指定字体族名和大小：

```cpp
// 基本构造：族名 + 大小
QFont font1("Arial", 12);

// 更完整的构造：族名 + 大小 + 加粗 + 斜体
QFont font2("Times New Roman", 16, QFont::Bold, true);  // 加粗 + 斜体
```

字体族名是一个容易搞混的概念。你传入的 "Arial" 或者 "Times New Roman" 不一定在当前系统上存在——特别是在 Linux 上，可能根本没有 Arial 这个字体。Qt 会按照一定的规则做字体替换：先找你指定的族名，找不到就找同类的替代字体（比如 sans-serif 类型的字体），再找不到就用系统默认字体。这个替换过程是自动的，你不会收到任何警告，所以如果发现文字显示出来的字体跟你想的不一样，大概率就是系统上没有你指定的字体。

如果你希望跨平台使用统一的字体外观，可以用 Qt 提供的通用字体族名：

```cpp
QFont sansFont("Sans", 12);          // 无衬线字体，Linux 映射到 Sans Serif
QFont serifFont("Serif", 12);        // 衬线字体
QFont monoFont("Monospace", 12);     // 等宽字体
```

QFont 有很多属性可以调整，日常开发中最常用的就是族名、大小、加粗、斜体这四个。其他如字间距、字母间距、下划线、删除线等属于精细调整，用到的时候查文档就行：

```cpp
QFont font("Arial", 14);
font.setBold(true);                   // 加粗
font.setItalic(true);                 // 斜体
font.setUnderline(true);              // 下划线
font.setStrikeOut(true);              // 删除线
font.setPointSize(16);                // 设置字号（点大小）
font.setPixelSize(20);                // 设置字号（像素大小）
font.setLetterSpacing(QFont::AbsoluteSpacing, 2);  // 字母间距 2 像素
font.setWordSpacing(5);               // 词间距 5 像素
```

这里有个值得注意的区别：`setPointSize()` 和 `setPixelSize()`。点大小是跟 DPI 无关的绝对单位——一个 12pt 的字体在 1x 屏幕和 2x 屏幕上看起来物理大小一样（Qt 会自动做 DPI 缩放）。像素大小则是跟 DPI 相关的——一个 20px 的字体在 2x 屏幕上看起来只有 1x 屏幕上的一半大。日常开发建议用 `setPointSize()`，这样在不同 DPI 的屏幕上显示效果更一致。

### 3.2 QPainter::drawText() 绘制文字与对齐

有了 QFont 之后，我们在 paintEvent 里就可以用 drawText 把文字画出来了。drawText 有好几个重载版本，用法和坑都不一样，我们一个一个来看。

最简单的版本是 `drawText(x, y, text)`，这里的 x 是文字左边缘的位置，y 是文字基线的位置。基线是什么？就是英文字母 "a"、"b"、"c" 底部对齐的那条线，字母 "g"、"p"、"y" 有部分会延伸到基线以下。注意 y 不是文字矩形的顶部，这一点非常关键，也是新手最容易踩的坑：

```cpp
void paintEvent(QPaintEvent *) override
{
    QPainter painter(this);
    painter.setFont(QFont("Arial", 20));
    painter.setPen(Qt::black);

    // y = 30 是基线位置，不是顶部位置
    painter.drawText(10, 30, "Hello Qt!");

    // 如果 y 设成 0，文字大部分会在 Widget 上方被裁剪掉
    painter.drawText(10, 0, "这段文字大部分看不见");
}
```

实际开发中更推荐使用 QRect 版本的 drawText，配合对齐标志，这样你不用操心基线的问题：

```cpp
void paintEvent(QPaintEvent *) override
{
    QPainter painter(this);
    painter.setFont(QFont("Arial", 16));

    // 在整个 Widget 区域内居中显示
    painter.drawText(rect(), Qt::AlignCenter, "居中文字");

    // 左上角对齐
    painter.drawText(rect(), Qt::AlignTop | Qt::AlignLeft, "左上角");

    // 指定一个矩形区域，右下角对齐
    QRect area(50, 50, 300, 100);
    painter.drawText(area, Qt::AlignBottom | Qt::AlignRight, "右下角");

    // 自动换行：需要加上 Qt::TextWordWrap 标志
    painter.drawText(QRect(10, 200, 200, 200),
                     Qt::TextWordWrap,
                     "这是一段比较长的文字，当它超出矩形宽度时会自动换行显示。");
}
```

QRect 版本的 drawText 会自动把文字放在矩形内按对齐标志定位，不需要你手动计算基线位置。而且加上 `Qt::TextWordWrap` 之后，文字超出矩形宽度会自动换行，非常方便。我个人的经验是：除非你有非常精确的像素级定位需求，否则永远用 QRect 版本而不是 x/y 版本。

drawText 还有一个不太常用但很有用的返回值：它返回一个 QRect，表示文字实际占据的矩形区域。这在做后续布局的时候很有用：

```cpp
QRect boundingRect = painter.drawText(100, 100, 300, 50,
                                       Qt::AlignLeft | Qt::AlignVCenter,
                                       "Hello World");
// boundingRect 就是文字实际占据的区域，可以用它来定位下一个元素
int nextX = boundingRect.right() + 10;  // 文字右侧留 10px 间距
```

### 3.3 QFontMetrics 计算文字尺寸

当我们需要做精确的文字布局——比如把一段文字和另一个控件并排排列，或者根据文字宽度来决定控件大小——就需要 QFontMetrics 出场了。QFontMetrics 能告诉你给定字体下一段文字的精确像素尺寸。

获取 QFontMetrics 最简单的方式是从 QPainter 或者 QFont 直接拿：

```cpp
// 方式 1：从当前 painter 的字体获取
QPainter painter(this);
painter.setFont(QFont("Arial", 14));
QFontMetrics fm = painter.fontMetrics();

// 方式 2：直接从 QFont 构造
QFont font("Arial", 14);
QFontMetrics fm(font);
```

QFontMetrics 最常用的几个函数：

```cpp
QFontMetrics fm(QFont("Arial", 14));

// 单个字符的宽度
int charWidth = fm.horizontalAdvance('A');

// 字符串的总宽度
int textWidth = fm.horizontalAdvance("Hello World");

// 字体的高度（ ascent + descent）
int fontHeight = fm.height();

// ascent：基线到文字顶部的距离
int ascent = fm.ascent();

// descent：基线到文字底部的距离
int descent = fm.descent();

// 两行文字之间的推荐行间距
int lineSpacing = fm.lineSpacing();

// 包围某个字符串的紧密矩形（比 boundingRect 更精确）
QRect tightRect = fm.boundingRect("Hello World");
```

我们来做一个实际的应用场景：在一个自定义 Widget 上画多行文字，每行用不同的字体大小，行间距精确控制。这就是 QFontMetrics 的经典用武之地：

```cpp
void paintEvent(QPaintEvent *) override
{
    QPainter painter(this);
    painter.setPen(Qt::black);

    int y = 20;  // 起始 y 位置

    // 第一行：大标题
    QFont titleFont("Sans", 24, QFont::Bold);
    painter.setFont(titleFont);
    QFontMetrics titleFm(titleFont);
    painter.drawText(10, y + titleFm.ascent(), "文章标题");
    y += titleFm.lineSpacing() + 5;  // 标题后多留 5px

    // 第二行：副标题
    QFont subFont("Sans", 14);
    painter.setFont(subFont);
    QFontMetrics subFm(subFont);
    painter.setPen(Qt::darkGray);
    painter.drawText(10, y + subFm.ascent(), "作者：AwesomeQt | 2025-04-22");
    y += subFm.lineSpacing() + 10;  // 正文前多留 10px

    // 第三行开始：正文
    QFont bodyFont("Sans", 11);
    painter.setFont(bodyFont);
    painter.setPen(Qt::black);
    QFontMetrics bodyFm(bodyFont);

    QStringList lines = {
        "这是正文的第一行。",
        "这是正文的第二行。",
        "这是正文的第三行。"
    };

    for (const QString &line : lines) {
        painter.drawText(10, y + bodyFm.ascent(), line);
        y += bodyFm.lineSpacing();
    }
}
```

这里的关键点是：每一行文字的 y 坐标 = `当前基线位置 + ascent`，这样才能保证文字的顶部在基线位置对齐。每行之后 y 递增 `lineSpacing()` 而不是 `height()`，因为 `lineSpacing()` 包含了 Qt 推荐的行间距，视觉效果更好。

还有一个你可能遇到的问题：`horizontalAdvance()` 在 Qt 5.11 之前叫 `width()`。如果你在网上看到老代码用 `fm.width("text")`，在新版 Qt 里这个函数已经被标记为废弃了，用 `horizontalAdvance()` 替代即可。

### 3.4 富文本渲染：QTextDocument 基础

前面我们用的 drawText 只能画纯文本。如果你需要在一个区域内渲染带格式的文字——一部分加粗、另一部分变色、有的地方是链接——QTextDocument 就是专门干这个的。

QTextDocument 是 Qt 富文本框架的核心类，它内部维护一棵文档树，支持 HTML 子集的标记语法。你可以用 HTML 格式的字符串来创建富文本内容，然后用 QTextDocument 的 drawContents 方法画到 QPainter 上：

```cpp
void paintEvent(QPaintEvent *) override
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QTextDocument doc;
    doc.setHtml("<h1 style='color: #2C3E50;'>标题文字</h1>"
                "<p>这是正文段落，<b>这里加粗了</b>，"
                "<i>这里斜体了</i>，"
                "<span style='color: red;'>这里变红了</span>。</p>"
                "<p style='color: gray; font-size: 10pt;'>"
                "这是灰色的小字注释。</p>");

    // 设置文档的绘制宽度（超出了会自动换行）
    doc.setTextWidth(width() - 20);

    // 把文档内容画到 QPainter 上，从 (10, 10) 开始
    painter.translate(10, 10);
    doc.drawContents(&painter);
}
```

QTextDocument 支持的 HTML 标签包括 `<h1>` 到 `<h6>`、`<p>`、`<b>`、`<i>`、`<u>`、`<s>`、`<font>`、`<span>`、`<a>`、`<br>`、`<ul>`、`<ol>`、`<li>`、`<table>` 等。CSS 样式方面支持 `color`、`background-color`、`font-family`、`font-size`、`font-weight`、`font-style`、`text-align`、`margin`、`padding` 等常用属性。不过它不是完整的浏览器引擎，别指望它支持 CSS3 动画或者 Flexbox 布局。

QTextDocument 有一个很实用的功能：你可以用 `size()` 获取文档的实际内容尺寸。这在做动态布局的时候非常有用——比如你需要知道富文本渲染出来之后到底有多高，才能决定下方其他控件的位置：

```cpp
QTextDocument doc;
doc.setHtml("<h1>标题</h1><p>一段文字</p>");
doc.setTextWidth(400);  // 先设定宽度

// 获取文档的实际尺寸
QSizeF docSize = doc.size();
qDebug() << "文档高度:" << docSize.height();

// 根据文档高度设置 Widget 的固定高度
setFixedHeight(static_cast<int>(docSize.height()) + 20);
```

还有一个细节你可能需要注意：QTextDocument 默认会有一些页边距（margin），导致画出来的文字跟你预期的位置有偏移。你可以用 `setDocumentMargin()` 把它设成 0，然后自己控制偏移量：

```cpp
QTextDocument doc;
doc.setDocumentMargin(0);  // 去掉默认边距
doc.setHtml("<p>无边距的文字</p>");
```

到这里你可以思考一个实际场景：如果你要实现一个带格式化的通知气泡控件——标题加粗、正文普通、底部时间灰色小字——用 QTextDocument 应该怎么组织 HTML？气泡的背景矩形大小又该怎么根据文字内容动态计算？想清楚这个，QFont、QFontMetrics、QTextDocument 三者如何配合你就全通了。

## 4. 踩坑预防

第一个坑就是前面反复强调的 drawText(x, y) 的 y 坐标问题。y 是基线位置不是顶部位置，这个概念说起来简单，但一到实际编码的时候特别容易忘。如果你用 x/y 版本的 drawText 发现文字显示不全或者位置偏了，第一反应就应该是检查 y 是不是设成了文字矩形的顶部。最简单的验证方式：改成 QRect 版本 + Qt::AlignTop，如果显示正常了，就说明是基线坐标的问题。

第二个坑是字体族名不存在的情况。你指定了 "Helvetica"，但系统上没有这个字体（常见于 Linux），Qt 会默默替换成系统默认的 sans-serif 字体，不会给你任何提示。如果你的界面在某个平台上字体看起来不一样，可以用 `QFontInfo(font).family()` 来查看实际使用的字体族名：

```cpp
QFont font("Helvetica", 14);
QFontInfo info(font);
qDebug() << "请求的字体:" << font.family()
         << "实际使用的字体:" << info.family();
```

第三个坑是 QFontMetrics 的函数行为在高 DPI 缩放下可能跟你想的不一样。Qt6 默认开启了高 DPI 缩放，`fontMetrics.height()` 返回的是逻辑像素高度。如果你在 2x 的屏幕上，一个 14pt 的字体 `height()` 可能返回 20，但实际物理像素是 40。日常使用中你不需要手动处理这个缩放——QPainter 在画的时候会自动缩放——但如果你要把文字尺寸跟 QImage 的像素坐标混在一起算，就要小心了。

第四个坑是 QTextDocument 在 paintEvent 里重复创建和解析 HTML 的性能问题。每次 paintEvent 都 new 一个 QTextDocument、setHtml、drawContents，这个开销在频繁重绘的场景下会非常明显。如果你的富文本内容不变，应该把 QTextDocument 存成成员变量，只在内容变化时重新 setHtml，paintEvent 里只做 drawContents。

## 5. 练习项目

我们来做一个实战练习：实现一个简易的富文本信息卡片控件。卡片中央显示一段格式化的内容——顶部是加粗的标题（20pt）、中间是正文段落（12pt，支持自动换行）、底部右对齐显示日期和作者信息（10pt 灰色）。卡片的背景是带圆角的白色矩形，外面加一圈浅灰色边框。卡片大小根据文字内容自适应：宽度固定 400 像素，高度随内容自动扩展。

完成标准是：继承 QWidget 实现自定义控件，对外提供 `setTitle()`、`setBody()`、`setFooter()` 方法设置内容；内部用 QTextDocument 渲染标题、正文、注释三部分；用 QFontMetrics 计算各部分高度，加上上下边距得到卡片总高度；在 paintEvent 里先画圆角矩形背景，再画文字内容；提供 `setCardContent(title, body, footer)` 一次性设置所有内容并自动刷新。几个提示：QTextDocument 的 `size().height()` 可以获取每段文字的实际渲染高度；圆角矩形用 `drawRoundedRect()`，半径 8-12 像素比较好看；背景填充用 `QColor(255, 255, 255)` 加浅灰边框 `QColor(220, 220, 220)` 是一种常见的信息卡片风格。

## 6. 官方文档参考链接

[Qt 文档 · QFont Class](https://doc.qt.io/qt-6/qfont.html) -- QFont 的完整 API，字体族名、大小、粗细、斜体等属性设置

[Qt 文档 · QFontMetrics Class](https://doc.qt.io/qt-6/qfontmetrics.html) -- QFontMetrics 的完整 API，文字宽度、高度、基线相关的所有计算函数

[Qt 文档 · QTextDocument Class](https://doc.qt.io/qt-6/qtextdocument.html) -- QTextDocument 富文本渲染的完整文档，HTML 子集支持和文档结构

[Qt 文档 · QPainter::drawText](https://doc.qt.io/qt-6/qpainter.html#drawText) -- drawText 所有重载版本的详细说明

[Qt 文档 · Rich Text Processing](https://doc.qt.io/qt-6/richtext.html) -- Qt 富文本框架的总览文档，Scribe 框架架构说明

---

到这里，Qt 文本渲染的几个核心工具你都已经上手了：QFont 负责描述字体、QFontMetrics 负责计算尺寸、drawText 负责绘制、QTextDocument 负责富文本。记住一个原则——凡是需要精确布局文字的地方，先用 QFontMetrics 量好尺寸再画，别靠猜。下一篇文章我们进入 OpenGL 的世界，用 QOpenGLWidget 把 GPU 渲染能力嵌入到 Qt 界面中。
