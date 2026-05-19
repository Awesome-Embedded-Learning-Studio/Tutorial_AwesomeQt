---
title: "3.24 QPlainTextEdit 进阶"
description: "入门篇我们把 QPlainTextEdit 和 QTextEdit 的区别、appendPlainText 追加日志、setMaximumBlockCount 限制行数、当前行高亮这四个基础维度过了一遍。进阶篇要把视角从'怎么用'转到'怎么构建真正的代码编辑器'。"
---

# 现代Qt开发教程（进阶篇）3.24——QPlainTextEdit 进阶

## 1. 前言 / 从日志查看器到代码编辑器

入门篇我们把 QPlainTextEdit 的基础能力过了一遍——appendPlainText 追加日志、setMaximumBlockCount 限制行数、ExtraSelection 做当前行高亮。说实话，如果你只是做日志终端那些知识完全够用了。但如果你要做代码编辑器——哪怕轻量级的脚本编辑器——你就得面对一组全新的问题：左侧怎么画行号、大日志文件怎么保证不卡、缩进怎么自动处理、括号怎么匹配高亮。这些问题入门篇一个都没碰，而它们恰恰是区分"能用"和"好用"的分水岭。

这篇文章我们就把 QPlainTextEdit 的四个进阶维度拆清楚：基于 block 的布局模型与行号侧边栏实现、maximumBlockCount 在高性能日志场景中的深度用法、以及代码编辑器功能（缩进和括号匹配）。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17 标准。QPlainTextEdit 属于 QtWidgets，链接 Qt6::Widgets 即可。行号侧边栏的实现涉及 QPainter（QtGui）的基本绘图操作。

## 3. 核心概念讲解

### 3.1 Block 布局模型——QPlainTextEdit 的渲染单位

QPlainTextEdit 的文档模型比 QTextEdit 简单得多，核心概念只有一个：QTextBlock。每个段落就是一个 block，所有 block 组成一个线性双向链表——没有 QTextFrame 嵌套，没有 QTextTable 二维结构。QPlainTextEdit 的渲染就是逐 block 从上到下绘制，只画可见 block，这是它性能优于 QTextEdit 的核心原因之一。

理解 block 布局模型的关键是三个方法：`blockCount()` 返回文档 block 总数，`blockBoundingRect(const QTextBlock &)` 返回某个 block 在文档坐标系中的矩形区域，`firstVisibleBlock()` 返回当前视口第一个可见 block。这三个方法构成了"从 block 索引到屏幕坐标"的映射基础。

```cpp
// 获取第一个可见 block 及其几何信息
QTextBlock firstBlock = plainEdit->firstVisibleBlock();
int blockNumber = firstBlock.blockNumber();
qreal blockTop = plainEdit->blockBoundingRect(firstBlock).top();
qreal blockHeight = plainEdit->blockBoundingRect(firstBlock).height();
```

blockBoundingRect 返回的坐标是文档坐标系（以文档左上角为原点），不是视口坐标系。你需要减去 contentOffset()（滚动偏移量）才能转换为视口坐标。contentOffset() 返回的是一个 QPointF，y 分量表示垂直方向的滚动偏移：

```cpp
QPointF offset = plainEdit->contentOffset();
qreal viewportY = blockTop - offset.y();
```

### 3.2 行号侧边栏——blockCountChanged / updateRequest / blockBoundingRect 三件套

行号侧边栏是代码编辑器的标配功能。Qt 没有提供内置的行号组件，但 QPlainTextEdit 提供了三个信号和方法让你自己实现：`blockCountChanged(int)` 在文档 block 数量变化时发射，用来更新行号区域的宽度；`updateRequest(const QRect &, int)` 在编辑器需要重绘某个区域时发射，用来同步更新行号的绘制；`blockBoundingRect(const QTextBlock &)` 用来获取指定 block 的几何信息，把 block 编号映射到屏幕位置。

实现行号侧边栏的核心思路是创建一个自定义 QWidget 作为 QPlainTextEdit 的子控件，固定在编辑器左侧。它的宽度根据最大行号位数动态计算——blockCount 为 100 时按三位数算宽度。绘制时遍历从 firstVisibleBlock 开始的所有可见 block，在对应位置画出行号文字。

行号的绘制逻辑分三步。第一步是 sizeHint 中根据 blockCount 的位数计算行号区域的宽度——blockCount 为 100 时按三位数算，`fontMetrics().horizontalAdvance('9') * digits` 加上左右边距。第二步是 paintEvent 中从 firstVisibleBlock 开始遍历，逐 block 计算它在视口中的 Y 坐标（blockBoundingRect 的 top 减去 contentOffset 的 y），在重绘区域内就画出行号。第三步是三个信号的联动：blockCountChanged 更新行号区域宽度，updateRequest 触发行号区域重绘，resizeEvent 调整行号区域的几何位置。这三个联动缺一不可——如果你只连了 updateRequest 没连 blockCountChanged，当行数从 99 变成 100 时行号区域宽度不会增加，三位数字的行号就会被截断。

```cpp
// paintEvent 核心绘制循环（简化）
QTextBlock block = m_editor->firstVisibleBlock();
int blockNumber = block.blockNumber();
int top = static_cast<int>(
    m_editor->blockBoundingRect(block).top()
    - m_editor->contentOffset().y());

while (block.isValid() && top <= event->rect().bottom()) {
    int height = static_cast<int>(
        m_editor->blockBoundingRect(block).height());
    if (block.isVisible() && (top + height) >= event->rect().top()) {
        painter.drawText(0, top, width() - 5, height,
            Qt::AlignRight | Qt::AlignVCenter,
            QString::number(blockNumber + 1));
    }
    top += height;
    block = block.next();
    ++blockNumber;
}
```

完整的 LineNumberArea 实现还需要在构造函数中设置 `setAttribute(Qt::WA_TransparentForMouseEvents)` 让鼠标事件穿透到编辑器，以及通过 `setViewportMargins` 给行号区域留出左侧空间。Qt 官方的 Code Editor Example 提供了完整的参考实现。

### 3.3 maximumBlockCount 深度——高性能日志场景的隐藏代价

入门篇我们讲了 setMaximumBlockCount 的基本用法——超过上限就删旧行。进阶篇要搞清楚它删行的时机和代价。QPlainTextEdit 在每次内容变更后检查 blockCount 是否超过 maximumBlockCount，超过了就从文档开头删除多余的 block。删除本身是 O(1) 的（修改链表头指针），但会触发布局重算和视口重绘。在高性能日志场景中（每秒追加上百行），触发频率非常高——上限 1000、速率每秒 200 行的话，每 5 秒就触发一次批量删除，blockCountChanged 信号发射后如果你的行号区域在监听它做更新，会有额外开销。

有一个容易忽略的细节：maximumBlockCount 触发删除时，被删除 block 上的 QTextBlock::userData 也会被一起销毁。如果你在每个 block 上存储了日志级别或时间戳等元数据，旧行删除后就丢失了。实战中推荐的做法是把 maximumBlockCount 设为"用户期望回溯的行数"加 20% 缓冲。另外，遍历文档时要在遍历前保存 blockCount 快照，因为遍历过程中 maximumBlockCount 可能触发删除导致迭代器失效。

现在有一道调试题给大家。下面这段代码在追加日志的同时手动清理旧行，但运行一段时间后程序崩溃——为什么？

```cpp
logView->appendPlainText(msg);
if (logView->blockCount() > logView->maximumBlockCount()) {
    QTextCursor cursor(logView->document());
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor,
                        logView->blockCount() - logView->maximumBlockCount());
    cursor.removeSelectedText();
}
```

问题出在手动清理逻辑上。appendPlainText 在 maximumBlockCount > 0 时已经会自动删除旧行，你再手动删一遍等于双倍删除——cursor 的 removeSelectedText 和 appendPlainText 的自动删除冲突，导致删多了或删除了正在使用的 block。解决方案很简单：不要手动删，让 maximumBlockCount 自动处理。

### 3.4 代码编辑器功能——缩进处理与括号匹配

QPlainTextEdit 不提供内置的代码编辑功能，但它的 block 模型和 QTextCursor 操作为实现这些功能提供了基础。这里讲两个最常见的代码编辑功能：自动缩进和括号匹配。

自动缩进的核心思路是在 keyPressEvent 中拦截 Enter 键，根据当前行的缩进级别自动插入相同数量的空格或制表符。判断缩进级别的方法是获取当前 block 的 text，找到前导空白字符的长度：

```cpp
void CodeEditor::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        QString currentText = textCursor().block().text();
        QString indent;
        for (auto c : currentText) {
            if (c == ' ' || c == '\t') {
                indent.append(c);
            } else {
                break;
            }
        }
        // 如果行末是 '{'，下一行额外缩进
        QString trimmed = currentText.trimmed();
        if (trimmed.endsWith('{')) {
            indent.append("    ");  // 追加一级缩进
        }
        QPlainTextEdit::keyPressEvent(event);  // 先让 Qt 处理换行
        textCursor().insertText(indent);       // 再插入缩进
        return;
    }
    QPlainTextEdit::keyPressEvent(event);
}
```

括号匹配的实现思路是在 cursorPositionChanged 信号中检查光标前后的字符是否是括号。检查 `doc->characterAt(pos - 1)` 是否是开括号 `(`、`[`、`{`，如果是就向前搜索对应的闭括号；同时检查 `doc->characterAt(pos)` 是否是闭括号，如果是就向后搜索对应的开括号。搜索方法是从当前位置向对应方向逐字符扫描，维护一个深度计数器：遇到同类型括号就 depth++，遇到匹配的另一半就 depth--，depth 为 0 时就是匹配位置。匹配成功的括号对通过 ExtraSelection 设置背景色高亮——每个括号创建一个只选中单个字符的 ExtraSelection，背景色设为绿色表示匹配成功。

```cpp
// 括号匹配触发点（在 cursorPositionChanged 信号中调用）
void matchParentheses()
{
    QList<QTextEdit::ExtraSelection> selections;
    QTextDocument *doc = document();
    int pos = textCursor().position();

    // 检查光标左侧的开括号
    if (pos > 0) {
        QChar ch = doc->characterAt(pos - 1);
        if (ch == '(' || ch == '[' || ch == '{') {
            int matchPos = findForwardMatch(doc, pos - 1, ch);
            if (matchPos >= 0) {
                addMatchSelection(selections, pos - 1, matchPos);
            }
        }
    }
    setExtraSelections(selections);
}
```

## 4. 踩坑预防

第一个坑是 maximumBlockCount 静默删除旧行导致用户数据丢失。setMaximumBlockCount 的删除操作没有信号通知，被删除 block 上的 QTextBlock::userData 也会被清除。如果你的逻辑依赖遍历所有 block 的数据，要么不用 maximumBlockCount 自己管理，要么在外部维护独立的数据结构。

第二个坑是 updateRequest 的 rect 不一定覆盖行号区域的完整宽度。updateRequest 的 QRect 是编辑器视口中需要重绘的区域，而行号区域的绘制范围是它自己的宽度，两者可能不重合。解决方案是在行号区域的 paintEvent 中自行计算可见 block 范围，而不是直接用 updateRequest 的 rect。

第三个坑是行号宽度在 blockCount 从 99 变成 100 时没有及时更新。blockCountChanged 信号的异步处理可能导致短暂的一帧截断。更稳妥的做法是在 sizeHint 中预留一位数的余量——blockCount 是 95 时就按三位数计算宽度。

## 5. 练习项目

练习项目：带行号和括号匹配的迷你代码编辑器。我们要在 QPlainTextEdit 的基础上实现一个轻量级的代码编辑器组件。核心功能包括：左侧行号侧边栏（跟随滚动、跟随行数变化更新宽度）、当前行高亮（浅蓝色背景）、括号匹配高亮（匹配成功为绿色、失败为红色）、自动缩进（Enter 键后保持当前缩进级别，遇到 `{` 额外增加一级缩进）、以及状态栏显示当前光标的行号和列号。完成标准是行号滚动时正确对齐、行数从 99 变成 100 时宽度自动增加不截断、括号匹配能处理嵌套对、自动缩进不累加多余空格。提示几个关键点：行号区域通过 setViewportMargins 留出空间，bracket matching 在 cursorPositionChanged 中用 `QTextDocument::characterAt` 触发，行列号用 `blockNumber() + 1` 和 `positionInBlock() + 1` 计算。

## 6. 官方文档参考链接

[Qt 文档 · QPlainTextEdit](https://doc.qt.io/qt-6/qplaintextedit.html) -- 纯文本编辑器，blockCountChanged/updateRequest/firstVisibleBlock

[Qt 文档 · QTextBlock](https://doc.qt.io/qt-6/qtextblock.html) -- 文本块，blockNumber/layout/userData

[Qt 文档 · QTextCursor](https://doc.qt.io/qt-6/qtextcursor.html) -- 光标操作，positionInBlock/block

[Qt 文档 · QTextLayout](https://doc.qt.io/qt-6/qtextlayout.html) -- 文本排版，block 内的行布局信息

[Qt 示例 · Code Editor Example](https://doc.qt.io/qt-6/qtwidgets-widgets-codeeditor-example.html) -- Qt 官方带行号的代码编辑器示例

到这里，QPlainTextEdit 的进阶内容就过完了。Block 布局模型是理解渲染机制的基础——每个 block 是独立的排版单元，只有可见的 block 才参与绘制。行号侧边栏依赖 blockCountChanged/updateRequest/blockBoundingRect 三件套。maximumBlockCount 在高频日志场景中需要关注删除时机和用户数据丢失。代码编辑器功能通过 keyPressEvent 和 cursorPositionChanged 驱动实现。掌握这些，你就有了构建专业代码编辑器的基础。
