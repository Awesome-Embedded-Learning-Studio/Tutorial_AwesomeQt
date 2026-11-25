---
title: "3.24 QPlainTextEdit 进阶"
description: "入门篇把 appendPlainText 追加日志、setMaximumBlockCount 限制行数和当前行高亮过了一遍，进阶篇从 QTextEdit 与 QPlainTextEdit 两个类的分工讲起，摸清 block 布局与坐标换算，看清行号侧栏的信号联动，捋清大日志淘汰发生在哪一步与代价记在哪儿，末尾收拾自动缩进与括号匹配，配套示例在 examples/advanced/03-qtwidgets/24-qplaintextedit-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.24——QPlainTextEdit 进阶

[3.23 QTextEdit 进阶](./23-qtextedit-advanced.md) 收尾把话递到了这里，纯文本兄弟登场，咱们这就把分工接住。两个类共用同一个 QTextDocument，frame 与 block 的文档树照旧，分歧出在排版这一层：QTextEdit 挂全套的 QTextDocumentLayout，表格、嵌套 frame、图文混排都给排，QPlainTextEdit 换成 QPlainTextDocumentLayout，Qt 给这个布局类的定位说得很直白，排版不按垂直像素算，按段落（内部叫 block）算，文档的高度等于段落数，表格与嵌套 frame 一概不支持。入门篇说它俩是兄弟关系，这句话落到源码层面，就落在布局类的差别上。

笔者把落差摆在前面。appendPlainText 追加日志、setMaximumBlockCount 限行数、ExtraSelection 当前行高亮，入门篇这几样够您搭一个日志窗口。需求一走到代码编辑器，哪怕轻量的脚本编辑器，行号、括号、缩进这些问题立刻冒出来，大日志场景里淘汰机制的细节也值得从头捋一遍。本篇按下面的次序走，看看 block 布局给了咱们什么底子，再逐路看行号侧栏，然后是大日志的淘汰路径与代价，末尾收自动缩进与括号匹配。

## 布局按 block 走，坐标换算是头一道关

QPlainTextEdit 的文档里，一个段落就是一个 QTextBlock，全部 block 线性串成一条，没有 frame 嵌套也没有表格，排版代价跟着段落走，这正是它扛得住大文档的原因。咱们要做行号，手上需要三样东西，firstVisibleBlock() 给出视口里第一个可见的 block，blockBoundingRect() 给出 block 自身坐标系里的矩形（原点在 block 左上角），contentOffset() 给出滚动偏移。这三样全是 protected 成员，子类和友元才摸得着，示例把侧栏声明成编辑器的友元类（line_number_editor.h 第 28 行），根由就在这儿。

blockBoundingRect 给出的矩形以 block 自己的左上角为原点，问哪一块 top 拿回来都是 0，它不携带这块在文档里的纵向位置，要文档坐标系里的矩形得换 blockBoundingGeometry。首块的视口位置正好等于 0 减去滚动偏移，所以拿首块的 top 减去 contentOffset() 的 y 分量，后面的块按各自高度累加往下走，咱们把换算这两行单独拎出来：

```cpp
QTextBlock block = editor->firstVisibleBlock();
qreal viewportY = editor->blockBoundingRect(block).top()
                  - editor->contentOffset().y();   // 首块专用换算，后续块按高度累加
```

有两处容易想岔，咱们摆在前面说。头一个是空文档的 blockCount() 拿回来就是 1，官方属性文档写明这个默认值，侧栏算宽度时示例用 qMax(1, ...) 托住位数（line_number_sidebar.cpp 第 33 行）。另一个是 blockCount 数的是段落，自动换行把一段折成三行，计数还是 1，行号也是一段只画一个。编号落在折行块的哪条可视行上，由 drawText 怎么摆框说了算，示例拿整块高度当框高、配 AlignVCenter 垂直居中，您把窗口压窄造出一个折行段就能看见，编号落在中间的可视行上。官方 Code Editor Example 是另一种摆法，框高取 fontMetrics().height() 的单行高度，编号贴着段落首行。要拿折行后的可视行数，得问 block.lineCount()，折出来的这些可视行由 block 自带的 QTextLayout 排版，要细看单行的几何，问它。

## 行号侧栏：三路联动各管一段，断光两路才露馅

| 联动入口 | 在侧栏里承担什么 |
| ---- | ---- |
| blockCountChanged(int newBlockCount) | 行数变动时重算侧栏宽度，两位数长成三位数那一刻主要靠它 |
| updateRequest(const QRect &rect, int dy) | 编辑器请求重绘时侧栏跟着重绘，dy 带垂直滚过的像素，槽尾一道保险顺带重算宽度 |
| resizeEvent | 窗口尺寸变化时把侧栏的几何位置重新摆正 |

前两路是 QPlainTextEdit 的信号，咱们把官方注释里的说明摆出来，updateRequest 的用途就是给行号、断点这类附属控件供绘制时机，滚动时 rect 覆盖整个视口，dy 给垂直滚过的像素。示例在构造函数里连上这两路（line_number_editor.cpp 第 22 与 26 行），第三路走 resizeEvent 重载（第 37 行）。

侧栏本体是一枚普通 QWidget 子控件，固定在编辑器左侧。构造里有一行不能省，它把鼠标事件让给下面的编辑器，靠的是 setAttribute(Qt::WA_TransparentForMouseEvents)，落在 line_number_sidebar.cpp 第 23 行。省掉它的后果很具体，行号区域上的按下与拖动全被侧栏吃掉，您在那儿划选区划不动，根因是子控件占住视口旁边那块边距区，鼠标事件落到它头上，轮不到编辑器的视口。

宽度按位数算。示例的 calculateWidth 拿单个字符 9 的宽度当单位，fontMetrics().horizontalAdvance(QLatin1Char('9')) 乘位数，再加左右边距，落在 line_number_sidebar.cpp 第 30-38 行。编辑器那头要让出这块空间，用的是 setViewportMargins，落在 line_number_editor.cpp 第 80 行，它同样是 protected，友元或子类是第二重前提。官方的 Code Editor Example 与示例是同一个路数，想看官方写法您顺着文末链接走。

咱们看示例 paintEvent 的主干，从 firstVisibleBlock 起步，block.next() 一块块走，top 累加每块高度，出了重绘区域就停，可见且落在区域内的才画，行号取 blockNumber + 1（给人看的行号从 1 数起，block 编号从 0 数起）：

```cpp
// LineNumberSidebar::paintEvent 的主干（示例第 68-92 行）
while (block.isValid()) {
    if (blockTop > event->rect().bottom())
        break;                                       // 出了重绘区就停
    if (block.isVisible() && (blockTop + blockHeight) >= event->rect().top())
        painter.drawText(0, static_cast<int>(blockTop), width() - 4,
                         static_cast<int>(blockHeight),
                         Qt::AlignRight | Qt::AlignVCenter,
                         QString::number(blockNumber + 1));
    block = block.next();
    blockTop += blockHeight;
    blockHeight = editor->blockBoundingRect(block).height();  // 下一块高度重取，折行块不一样高
    ++blockNumber;
}
```

咱们再看 updateRequest 带来的 rect，它是编辑器视口的坐标，跟侧栏自己的坐标系对不上号。所以侧栏的 paintEvent 按自己的 event->rect() 算可见范围，示例的 onUpdateRequest 拿 rect 的纵向位置，折算成侧栏自己的 update（line_number_editor.cpp 第 64 行），滚动时 dy 不为 0，直接 scroll(0, dy) 整体平移，省一次全量重绘（第 61 行）。槽的末尾还埋着一道保险，rect 把编辑器视口整个盖住时，顺手重算一次侧栏宽度（第 68-70 行），官方 Code Editor Example 同款。

这道保险平时不出声，兜的就是行数悄悄过线的场合。blockCountChanged 只在行数变动那一刻发一次，而窗口高度有限，一屏也就二三十行，您一路敲回车，光标早越过了视口下缘，换行键被 Qt 收下之后必调 ensureCursorVisible 把光标挪回可见区，文档跟着往下滚，这次滚动发出的 updateRequest 携带的 rect 恰好盖住整个视口，保险必中。宽度重算因此有两道入口，blockCountChanged 走明面，保险藏在 updateRequest 槽尾，断掉明面那道，两位数长成三位数的活照样有人接。

这件事当场能验，实验分两步走。第一步，把 blockCountChanged 的连接注释掉，重新编译（line_number_editor.cpp 第 22 行），连敲回车把行数顶过 99，侧栏宽度照样从两位数长到三位数，笔者在离屏环境实测，视口左边距从 23px 长到 30px，此时 updateRequest 照样响、paintEvent 照样画，槽尾那道保险把宽度重算接了过去。第二步，把 onUpdateRequest 末尾的保险分支一并注释掉（第 68-70 行），再连敲回车把行数顶过 99，这回宽度才真停住，实测视口左边距恒在 16px，那是启动时文档还空着、按一位数算出来的值，此后再没有一路去重算它，行号过 99 之后三位数画不全，绘制右对齐，裁掉的是百位那一头，宽度停在几位数，超过的编号就只剩末几位。两步对照下来，blockCountChanged 与保险两道入口断一道另一道还顶着，断光两道才露馅。想更稳，宽度按当前位数加一预留，行数逼近 100 的时候就不会闪。

## 大日志不卡：淘汰发生在哪一步，代价记在哪儿

setMaximumBlockCount 的基本用法入门篇过了，进阶咱们把剪裁的时机讲清楚。进入稳态（blockCount 已到上限）之后，appendPlainText 每追加一段，就从文档头退掉一段，淘汰逐段进行。它的内部走一条专门路径，把上限临时摘成 0，追加新段，再用一枚光标从文档头把出头的那块 removeSelectedText 掉，最后把上限挂回去，appendHtml 走的是同一条路径。滚动跟随也在同一条路径里做完，追加前视口停在底部，追加完滚动条自动拉到底，您往上翻着看历史，它就不打扰。打字这类其余编辑不走 append 的私有路径，由 QTextDocument 在每次编辑收尾时统一检查淘汰，同样从文档头删。

照这个机制看，流传很广的兜底写法是在 append 之后再手动删一遍，咱们拿条件推一下，它根本不会执行：

```cpp
logView->appendPlainText(msg);
// 走到这里 blockCount 已经被剪回上限，下面的条件永远不成立
if (logView->blockCount() > logView->maximumBlockCount()) {
    // ...手动删除多余行
}
```

根因是剪裁内置在 append 的私有路径里，函数返回之前就做完了，轮不到外面补第二遍。您真想自己管淘汰，把上限设为 0 自己删，Qt 内部做的也是同一件事，光标选中文档头再 removeSelectedText。只是把上限设 0 的那一刻，撤销已经被顺手关掉了，既想自己管淘汰又想留着撤销，得补一次 setUndoRedoEnabled(true)，重开只在上限为 0 时走得通，缘由下一段展开。

咱们把代价看两处。明面上那笔是编辑成本，每次淘汰都是一次真实的文档编辑，contentsChange 照发，布局照算，视口照重绘，监听 blockCountChanged 做更新的行号侧栏跟着多干活，高频日志下这笔成本按行数线性付。隐蔽的一笔在撤销上，setMaximumBlockCount 会顺手把文档的撤销功能关掉，上限大于 0 时想再打开也不行，日志窗口里 Ctrl+Z 没反应属于设计如此，别当故障排查。

挂在被删 block 上的数据跟着退场。block 数据析构时它携带的 QTextBlockUserData 一并 delete，您在上面存的日志级别、时间戳随之消失，blockCountChanged 只告诉您数量变了，不告诉您哪一块没了。要么把上限设为 0 自己管理，要么在外部维护独立的环形缓冲，渲染最后几行时再回填元数据。上限的取值咱们给个经验值，设成用户期望回溯的行数再加两成余量，别压着刚好。

从外面遍历文档，咱们有一句要嘱咐，存 QTextCursor 比存 position 整数稳当，光标随文档编辑自动调整位置，整数不会，淘汰一发生，全文的 position 整体前移。性能上还有一枚现成的开关，centerOnScroll 按官方文档的说法能让日志查看器更快，滚动量小了，追加的代价跟着小。

## 缩进与括号匹配：一个拦按键，一个跟光标

QPlainTextEdit 不内置任何代码编辑功能，好在 block 模型与 QTextCursor 把地基打好了。QTextCursor 的选区与位置模型 3.23 刚讲过，这里不重讲，用着不熟的您回 3.23 补课。

自动缩进挂在 keyPressEvent 上，拦换行键。Key_Return 与 Key_Enter 都得拦，主键盘回车发前者，小键盘回车发后者，只拦一个就漏一半，您按小键盘回车，缩进没跟上的话，回这里查。缩进量取当前行的前导空白，行尾是大括号再进一级：

```cpp
// keyPressEvent 里拦到换行键后，算好要补的缩进
QString indent;
const QString line = textCursor().block().text();
for (const QChar c : line) {
    if (c == ' ' || c == '\t')
        indent.append(c);
    else
        break;                                  // 前导空白到头为止
}
if (line.trimmed().endsWith(QLatin1Char('{')))
    indent.append(QStringLiteral("    "));      // 大括号行尾，下一行再进一级
```

顺序得讲究，放行基类让 Qt 把换行做完，然后再把缩进 insertText 进去。倒过来插缩进，缩进落在上一行行尾，每敲一次回车，上一行的尾巴就多出一截空白。示例专心做行号，没收录缩进，您把这段片段补进自己的编辑器子类就能用。

括号匹配挂在 cursorPositionChanged 上，光标动一次查两格。characterAt(pos - 1) 查左邻，左邻是开括号就向右搜配对，characterAt(pos) 查右邻，右邻是闭括号就向左搜。搜索用深度计数器，从当前位置逐字符扫，遇同类括号加一，遇配对的另一半减一，计数归零即命中，嵌套的括号对照这个规则也能配出来。高亮走 ExtraSelection，每个括号一枚只选中单个字符的选区，配上了给绿底，拼装方式咱们在 3.23 刚过一遍，这里不重复。

```cpp
// cursorPositionChanged 的槽里，两格各查一次
const int pos = textCursor().position();
const QChar before = doc->characterAt(pos - 1);   // 左邻是开括号则向右搜
const QChar after  = doc->characterAt(pos);       // 右邻是闭括号则向左搜
```

边界顺带交代，pos 为 0 时您去摸 pos - 1，characterAt 给回空字符，匹配自然落空，程序不会崩，守卫还是写上，读代码的人一眼看到边界在哪。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17。QPlainTextEdit 在 QtWidgets，行号绘制用 QPainter（QtGui），链接 Qt6::Widgets 一并带上，示例可在 Linux、macOS、Windows 桌面平台编译运行。

[Qt 文档 · QPlainTextEdit](https://doc.qt.io/qt-6/qplaintextedit.html) -- 纯文本编辑器，blockCountChanged、updateRequest 与 maximumBlockCount

[Qt 文档 · QTextBlock](https://doc.qt.io/qt-6/qtextblock.html) -- 文本块，blockNumber、lineCount 与 userData

[Qt 文档 · QTextCursor](https://doc.qt.io/qt-6/qtextcursor.html) -- 编辑与选区接口，removeSelectedText 的用法

[Qt 文档 · QTextLayout](https://doc.qt.io/qt-6/qtextlayout.html) -- block 内的可视行布局信息

[Qt 源码 · Code Editor Example](https://code.qt.io/cgit/qt/qtbase.git/tree/tests/manual/textrendering/codeeditor?h=6.9) -- 官方带行号的代码编辑器示例，Qt 6 文档页已撤，完整例子在源码树里

---

输入类五连走到第三篇。[3.25 QTextBrowser 进阶](./25-qtextbrowser-advanced.md) 是第四站，3.23 埋的那颗种子在这儿发芽，QTextBrowser 补上 anchorClicked 信号与 openExternalLinks 属性，纯文本这边点链接没反应的事到它那儿有个交代。想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/24-qplaintextedit-advanced/，可执行文件 24-qplaintextedit-advanced），连敲回车把行数顶过 99，看侧栏自己加宽。再把 line_number_editor.cpp 第 22 行的连接注释掉，重新编译，行数过 99 宽度照样加宽，onUpdateRequest 末尾那道保险（第 68-70 行）把它接住了，把保险分支也注释掉，宽度才停住，三位行号被裁掉百位那一头。把 line_number_sidebar.cpp 第 23 行的穿透属性注释掉，在行号区划两下，选区动不了。最后在 main.cpp 里加个 QTimer，每 50ms 追加一行，配上 setMaximumBlockCount(100)，然后打印 blockCount()，前 100 次追加里数字一路爬，稳态之后它恒为 100，isUndoRedoEnabled() 给的是 false。加码的话，把本篇的自动缩进与括号匹配补进这个编辑器，再配当前行高亮，状态栏的行列号也好算，blockNumber() 加一，positionInBlock() 加一。完成标准咱们给您列好，行号滚动要对得齐，嵌套括号要配得上，缩进不能累加多余空白。
