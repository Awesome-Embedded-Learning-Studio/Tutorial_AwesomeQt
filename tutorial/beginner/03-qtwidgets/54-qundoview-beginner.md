# 现代Qt开发教程（新手篇）3.54——QUndoView：撤销历史视图

## 1. 前言 / 让撤销重做变得可视化

之前我们接触过不少文本编辑、表格操作类的控件，在这些场景里，撤销（Undo）和重做（Redo）几乎是不可或缺的功能。Qt 为此提供了一整套 QUndoFramework——核心类是 QUndoStack，它负责管理所有的操作命令（QUndoCommand），并且在内部维护一个操作历史栈，支持 push、undo、redo 等基本操作。这个框架在工程实践中非常成熟，但有一个问题一直困扰着我们：用户在执行了一系列操作之后，往往不记得自己到底做了几步，也不知道 undo 会回退到哪个状态。光靠 Ctrl+Z / Ctrl+Y 一次一次试探，效率实在太低。

QUndoView 就是用来解决这个问题的。它继承自 QListView，绑定了 QUndoStack 之后会自动把栈中的所有操作命令以列表的形式展示出来。用户可以一眼看到完整的操作历史，当前处于哪一步、上面还有多少步可以 redo、下面有多少步可以 undo——全部一目了然。更实用的是，QUndoView 支持直接点击历史条目来跳转到任意一个历史状态，相当于给操作历史做了一次"时间线跳转"。这个交互在很多专业软件中都有：Photoshop 的历史面板、Visual Studio 的撤销历史、Blender 的操作历史栈，本质上都是同一个东西。

今天我们从四个方面展开。先看 QUndoView 如何与 QUndoStack 绑定并自动显示操作历史，然后研究 setCleanIcon 如何在历史列表中标记"已保存"状态，接着深入实现点击历史条目的时间线跳转机制，最后在一个文档编辑器中搭建完整的撤销重做系统。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QUndoView 和 QUndoStack 在 QtWidgets 模块中，链接 Qt6::Widgets 即可。示例代码涉及 QUndoView、QUndoStack、QUndoCommand、QPlainTextEdit、QLabel、QSplitter、QToolBar、QMainWindow 和 QAction。

## 3. 核心概念讲解

### 3.1 与 QUndoStack 绑定显示操作历史

QUndoView 的核心工作机制其实很简单——它内部就是一个 QListView，数据源是一个自定义的内部 Model，这个 Model 直接读取 QUndoStack 的状态来生成显示条目。当你调用 setStack(QUndoStack *stack) 把一个 QUndoStack 绑定到 QUndoView 上之后，QUndoView 会自动监听这个 stack 的 indexChanged 信号，每当 stack 的状态发生变化（push 了一个新命令、undo 了一步、redo 了一步、跳转到了某个历史位置），QUndoView 立刻刷新列表内容。

列表中的每一项对应一个 QUndoCommand。条目的显示文本取自 QUndoCommand::text()——所以你在创建 QUndoCommand 子类的时候，一定要在构造函数中通过 setText() 设置一个有意义的描述文本，比如"插入文本"、"删除第 3 行"、"修改字体大小"。如果 text() 返回空字符串，QUndoView 中就会显示一个空行，用户完全无法理解这一步操作做了什么。

```cpp
auto *undoStack = new QUndoStack(this);

auto *undoView = new QUndoView;
undoView->setStack(undoStack);

// 每次向 stack push 命令时，QUndoView 自动更新
undoStack->push(new InsertTextCommand(document, "Hello", 0));
// QUndoView 中会出现一项 "插入文本"
```

QUndoView 还支持同时监控多个 QUndoStack。通过 setGroup(QUndoGroup *group) 可以绑定一个 QUndoGroup——QUndoGroup 管理多个 QUndoStack（比如多文档编辑器中每个文档一个 stack），当活跃 stack 切换时，QUndoView 自动切换显示对应的操作历史。这个机制在多文档应用中非常实用，我们后面会详细讨论。

当前被选中的条目（高亮显示的那一行）代表 QUndoStack 当前的 index 位置。这个位置上方的所有条目是"已经 undo 的操作"（可以 redo），下方的所有条目是"已经执行的操作"（可以继续 undo）。选中条目的上方和下方在视觉上通常会有不同的渲染方式——上方的条目可能是灰色或斜体，表示它们处于"已撤销"状态。QUndoView 默认不做这种区分，但你可以通过自定义 delegate 来实现。

### 3.2 setCleanIcon 标记保存点

很多编辑器类应用都有一个"文档是否已保存"的状态指示。当用户执行了 Ctrl+S 保存操作后，标题栏上的"未保存"标记消失；当用户又做了一些修改后，标记再次出现。这个机制在 QUndoStack 中是通过"clean state"来实现的。

QUndoStack 有一个 cleanIndex 属性。当调用 setClean() 时，stack 会记录当前的 index 作为 cleanIndex。之后如果你查询 isClean()，它会比较当前 index 是否等于 cleanIndex——相等则说明自上次保存以来没有未提交的修改，不等则说明有未保存的修改。这个逻辑非常巧妙：即使你 undo 回到了保存时的状态，isClean() 也会返回 true，因为当前 index 等于 cleanIndex。

QUndoView 通过 setCleanIcon(const QIcon &icon) 提供了对 clean state 的可视化支持。设置一个 cleanIcon 之后，QUndoView 会在历史列表中 cleanIndex 对应的那一行前面显示这个图标——通常是一个磁盘或保存图标。这样用户在浏览操作历史时，可以一眼看出"哪一步是最后一次保存"，以及"当前状态距离保存点有多少步操作"。

```cpp
// 设置保存点图标
undoView->setCleanIcon(
    QApplication::style()->standardIcon(
        QStyle::SP_DriveHDIcon));

// 在保存操作时标记 clean state
void saveDocument()
{
    // ... 执行文件保存逻辑 ...
    undoStack->setClean();
    // QUndoView 中当前 index 对应的行会显示保存图标
}
```

这里有一个需要注意的细节：如果你在 clean state 之后又 push 了新的命令，cleanIndex 不会变化，但当前 index 向前推进了。此时 isClean() 返回 false。如果你 undo 回到了 cleanIndex 的位置，isClean() 又会返回 true——即使你没有执行保存操作。这个行为在某些应用中可能是期望的（用户 undo 回到了已保存的状态），但在另一些应用中可能需要额外处理（比如在窗口标题上显示"已保存*"的标记，需要同时检查 isClean() 和实际的文件状态）。

另外，如果你调用了 undoStack->clear()，所有的历史记录被清空，cleanIndex 也被重置。此时 isClean() 返回 true。如果你调用了 undoStack->push() 并且新命令被压入，cleanIndex 可能会被丢弃（如果新命令不是紧跟在 cleanIndex 之后的话，因为 QUndoStack 在 push 新命令时会丢弃当前 index 之后的所有命令，如果 cleanIndex 在被丢弃的范围内，它就被设置为 -1）。

### 3.3 点击历史条目实现时间线跳转

QUndoView 最强大的交互能力是"时间线跳转"。当用户直接点击历史列表中的某个条目时，QUndoView 会计算目标条目和当前 index 之间的差值，然后调用 QUndoStack 的 setIndex(int index) 来跳转到目标状态。setIndex 内部会自动执行所需次数的 undo 或 redo 操作，把文档状态恢复到目标 index 对应的状态。

这个跳转过程是原子性的——用户看到的是从当前状态直接跳到目标状态，中间的状态虽然也被依次经过（QUndoCommand 的 undo/redo 依次被调用），但在用户感知上是一步到位的。这也是为什么 QUndoCommand 的 undo() 和 redo() 实现必须正确且幂等——因为时间线跳转会批量调用它们，任何状态管理上的疏忽都会在跳转时暴露无遗。

```cpp
// QUndoView 内部的跳转逻辑（简化版）
// 当用户点击了列表中的第 N 项：
undoStack->setIndex(N);
// setIndex 内部：
//   如果 N < 当前 index: 执行 (当前 index - N) 次 undo
//   如果 N > 当前 index: 执行 (N - 当前 index) 次 redo
```

在 QUndoView 中，点击跳转是默认行为——你不需要写任何额外的代码来支持它。只要你把 QUndoView 绑定到了一个 QUndoStack，用户点击列表中的任意一行，对应的 undo/redo 操作就会自动执行。你可以通过 QUndoView 的 canUndo 和 canRedo 信号（实际上是从 QUndoStack 转发过来的）来监控状态变化，也可以通过 QUndoStack 的 indexChanged(int idx) 信号来精确追踪当前处于历史中的哪一个位置。

如果你需要在跳转前后做一些额外处理（比如刷新某个预览、更新状态栏），连接 QUndoStack 的 indexChanged 信号就够了。不需要去拦截 QUndoView 的点击事件——那是 QUndoView 内部的事情。

时间线跳转的一个重要约束是：它只能在已有的历史记录范围内跳转。用户不能跳转到一个已经被丢弃的状态。在 QUndoStack 中，当你 push 一个新命令时，当前 index 之后的所有命令都会被丢弃。比如你执行了操作 A、B、C（index 为 3），undo 了一步回到 B（index 为 2），然后执行了操作 D——此时操作 C 被丢弃，历史记录变成了 A、B、D。用户无法再跳转回操作 C。QUndoView 会正确反映这个变化：列表中不再显示操作 C。

### 3.4 在文档编辑器中的完整撤销重做系统

前面三节分别讲了 QUndoView 的各个独立能力，现在我们把它们组装到一个完整的文档编辑器场景中。这个编辑器的核心架构是：一个 QPlainTextEdit 作为编辑区域，一个 QUndoStack 管理所有编辑操作，一个自定义的 QUndoCommand 子类封装文本变更，一个 QUndoView 作为历史面板显示在窗口侧边。

对于文本编辑来说，最关键的 QUndoCommand 子类需要封装三个信息：变更发生的位置（cursor position）、被删除的文本（如果是删除操作）、被插入的文本（如果是插入操作）。redo() 在指定位置插入新文本（或删除旧文本），undo() 执行相反操作。这里有一个非常容易踩的坑：如果你的编辑器支持多光标或者选中区域替换，command 需要同时记录选区的起始和结束位置，否则 undo 后光标位置会错乱。

```cpp
class TextEditCommand : public QUndoCommand
{
public:
    TextEditCommand(QPlainTextEdit *editor,
                    const QString &removedText,
                    const QString &insertedText,
                    int position,
                    QUndoCommand *parent = nullptr)
        : QUndoCommand(parent)
        , m_editor(editor)
        , m_removedText(removedText)
        , m_insertedText(insertedText)
        , m_position(position)
    {
        // 设置命令描述文本（显示在 QUndoView 中）
        if (!insertedText.isEmpty() && removedText.isEmpty()) {
            setText(QString("插入 \"%1\"").arg(
                truncated(insertedText, 20)));
        } else if (insertedText.isEmpty()
                   && !removedText.isEmpty()) {
            setText(QString("删除 \"%1\"").arg(
                truncated(removedText, 20)));
        } else {
            setText(QString("替换 \"%1\" -> \"%2\"")
                        .arg(truncated(removedText, 15),
                             truncated(insertedText, 15)));
        }
    }

    void undo() override
    {
        // 撤销：删除插入的文本，恢复被删除的文本
        QTextCursor cursor = m_editor->textCursor();
        cursor.setPosition(m_position);
        cursor.setPosition(
            m_position + m_insertedText.length(),
            QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        if (!m_removedText.isEmpty()) {
            cursor.setPosition(m_position);
            cursor.insertText(m_removedText);
        }
        m_editor->setTextCursor(cursor);
    }

    void redo() override
    {
        // 重做：删除旧文本，插入新文本
        QTextCursor cursor = m_editor->textCursor();
        cursor.setPosition(m_position);
        if (!m_removedText.isEmpty()) {
            cursor.setPosition(
                m_position + m_removedText.length(),
                QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
        }
        cursor.setPosition(m_position);
        cursor.insertText(m_insertedText);
        m_editor->setTextCursor(cursor);
    }

private:
    static QString truncated(const QString &text, int maxLen)
    {
        if (text.length() <= maxLen) {
            return text;
        }
        return text.left(maxLen) + "...";
    }

    QPlainTextEdit *m_editor;
    QString m_removedText;
    QString m_insertedText;
    int m_position;
};
```

上面这个 TextEditCommand 是一个相对完整的文本变更命令。它同时处理插入、删除和替换三种操作，并且通过 setText() 为每种操作提供了清晰的描述文本。这些描述文本会直接显示在 QUndoView 的列表中，用户可以一目了然地看到每一步操作的具体内容。

在实际的编辑器中，你不需要为每一次按键都创建一个 QUndoCommand——那样历史列表会被撑爆。更好的做法是使用 QUndoStack 的 beginMacro() / endMacro() 来合并一系列相关的操作。比如用户连续输入了 "Hello" 五个字符，你可以把这五次插入合并成一个 macro，在 QUndoView 中只显示一项"插入 'Hello'"。或者使用 QPlainTextEdit 内置的 undo/redo 支持，它已经自动做了合并处理。

不过本篇示例中我们选择手动管理 QUndoCommand，原因是这样能更清晰地展示 QUndoView 和 QUndoStack 的交互机制。QPlainTextEdit 内置的 undo 框架虽然方便，但它的内部 QUndoStack 不直接暴露给外部的 QUndoView——所以如果你的编辑器需要显示撤销历史面板，通常需要自己管理 QUndoStack，禁用 QPlainTextEdit 内置的 undo/redo，然后通过 QUndoCommand 来封装每一次文本变更。

整个编辑器的撤销重做系统最终呈现出来的效果是：左侧是文本编辑区域，右侧是 QUndoView 历史面板。顶部工具栏有撤销、重做、保存三个按钮。每执行一次编辑操作，QUndoView 中新增一行；每点击一次 undo，QUndoView 中的选中行上移一行；直接点击历史面板中的某一行，编辑器内容立刻跳转到对应状态。保存时在历史面板中标记 cleanIcon，让用户知道哪一步是最后一次保存。

## 4. 踩坑预防

第一个坑是 QUndoView 的条目文本完全依赖 QUndoCommand::text()。如果你在 push 命令时忘记调用 setText()，QUndoView 中就会出现一堆空白行。这不是 bug，是 QUndoView 的设计——它只负责显示，不负责生成描述文本。养成一个好习惯：在每一个 QUndoCommand 子类的构造函数中必须调用 setText()，哪怕只是一个简短的描述。

第二个坑是 QUndoStack 的 push 操作会丢弃当前 index 之后的所有命令。这意味着如果用户 undo 了几步然后执行了新操作，被 undo 的那些操作就彻底丢失了。QUndoView 会正确反映这一点——列表中不再显示被丢弃的条目。但在某些应用场景中（比如需要"保留所有历史"的审计日志），这个行为可能不符合需求，需要额外设计。

第三个坑是关于 QUndoView 的选中项和 QUndoStack 的 index 同步。当你直接操作 QUndoStack（比如调用 undo()、redo()、setIndex()）时，QUndoView 的选中项会自动同步。但如果你在代码中通过其他方式修改了文档状态（绕过 QUndoStack），QUndoView 不会知道——它只监听 QUndoStack 的信号。所以务必保证所有文档变更都通过 QUndoStack::push() 来执行。

第四个坑是 setCleanIcon 的图标只在 cleanIndex 对应的行上显示。如果你多次调用了 setClean()（比如用户保存了多次），只有最后一次 setClean() 对应的 index 会被标记。之前的保存标记会消失。这是合理的行为——你只关心"最后一次保存点"——但如果你需要标记多个保存点，就需要自己实现一个自定义的 Model 或 Delegate。

第五个坑是在多文档编辑器中使用 QUndoGroup 时，QUndoView 只显示当前活跃 stack 的历史。切换活跃文档时，QUndoView 的内容会整体刷新——之前的选中项、滚动位置都会丢失。如果这对用户体验有影响，可以考虑为每个文档维护一个独立的 QUndoView，在切换文档时同时切换显示对应的 QUndoView。

## 5. 练习项目

我们来做一个综合练习：创建一个"文档编辑器"窗口，左侧是 QPlainTextEdit 编辑区域，右侧是 QUndoView 撤销历史面板。使用 QSplitter 分隔左右两个区域。自定义一个 TextEditCommand 类（继承 QUndoCommand）来封装文本的插入和删除操作，在构造函数中根据操作类型设置描述文本。编辑器顶部有一个 QToolBar，包含撤销、重做、保存三个按钮。撤销和重做按钮通过 QUndoStack::createUndoAction() 和 createRedoAction() 创建，保存按钮调用 QUndoStack::setClean() 并在 QUndoView 中标记保存图标。禁用 QPlainTextEdit 内置的 undo/redo，所有文本变更都通过 QUndoStack::push() 管理。窗口标题实时显示当前文档的修改状态——isClean() 为 true 时显示"已保存"，为 false 时显示"未保存*"。

提示：拦截 QPlainTextEdit 的按键事件不太现实（按键会产生不确定的文本变更），一个更实际的方案是在编辑器下方放几个操作按钮（插入示例文本、删除选中、追加一行），每个按钮的点击操作通过 QUndoCommand 封装后 push 到 QUndoStack。这样既能完整演示 QUndoView 的所有功能，又避免了拦截按键的复杂性。

## 6. 官方文档参考链接

[Qt 文档 -- QUndoView](https://doc.qt.io/qt-6/qundoview.html) -- 撤销历史视图控件

[Qt 文档 -- QUndoStack](https://doc.qt.io/qt-6/qundostack.html) -- 撤销命令栈

[Qt 文档 -- QUndoCommand](https://doc.qt.io/qt-6/qundocommand.html) -- 撤销命令基类

[Qt 文档 -- QUndoGroup](https://doc.qt.io/qt-6/qundogroup.html) -- 多文档撤销栈管理

[Qt 文档 -- Undo Framework Overview](https://doc.qt.io/qt-6/qundogroup.html) -- 撤销框架概览

---

到这里，QUndoView 的核心用法就全部讲完了。它和 QUndoStack 的绑定是一行代码的事，setCleanIcon 让保存状态可视化，点击历史条目的时间线跳转是零代码的内置能力。在一个完整的文档编辑器中，QUndoView 配合自定义 QUndoCommand 和 QUndoStack，就能搭建出一个专业级的撤销重做系统——用户不仅可以 Ctrl+Z / Ctrl+Y，还能直接在历史面板里浏览和跳转。这是单纯靠 QPlainTextEdit 内置 undo/redo 做不到的。
