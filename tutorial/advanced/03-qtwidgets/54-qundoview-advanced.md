---
title: "3.54 QUndoView 进阶"
description: "进阶篇把 QUndoView 与 QUndoStack 的镜像关系读明白：列表行 0 是空标签（qundoview.cpp 第 63、181-183 行），行 N 显示 text(N-1)，rowCount 为 count()+1（第 161 行），clean 图标走 DecorationRole 落在 cleanIndex 行（第 184-188 行），只读来自模型未重写 flags（qabstractitemmodel.cpp 第 2458-2464 行），点选经共享选中模型转成 setIndex(row)（qundoview.cpp 第 112-124、253-260 行），indexChanged 后整表重建（第 105-110 行），setStack 会清掉组（第 333-340 行），setGroup 连 activeStackChanged（第 353-374 行），push 立即执行 redo 并删掉 index 之上的重做历史（qundostack.cpp 第 570-571、583-584 行），干净点丢失时 clean_index 置 -1（第 585-586 行），合并需 id 相等且不在干净点（第 589-594 行），私有 setIndex 一次发五个信号再按需 cleanChanged（第 415-436 行），宏命令 beginMacro 压空父命令（第 1128-1143 行），默认子命令 redo 顺子序 undo 逆子序（第 197-219 行），setUndoLimit 只认空栈（第 1229-1242 行），QUndoGroup 切栈时六个信号摘旧挂新（qundogroup.cpp 第 151-203 行），示例 contentsChange 双层回环与守卫包 undo 的写法、删除分支占位串的精度边界，配套示例在 examples/advanced/03-qtwidgets/54-qundoview-advanced/"
---

# 现代Qt开发教程（进阶篇）3.54——QUndoView 进阶

3.53 收队时说好，本篇的视图是 QUndoStack 的镜子，栈里每一步操作在列表里各占一行。镜子怎么造，咱们从 qundoview.cpp 读起：QUndoView 是一只 QListView，内部装着一只叫 QUndoModel 的私有模型，栈的一切都被翻译成表里的行。读下去您会遇到一件意外的事，QUndoView 自己一个信号都没有。笔者把 qundoview.h 第 21 到 55 行翻了个遍，Q_SIGNALS 段根本不存在，镜面上的每次动静，信号全从 QUndoStack 一侧发出来。

引用文件咱们交代清楚。镜面读 qundoview.cpp 与 qundoview.h，它们住在 QtWidgets 一侧（src/widgets/util）。撤销框架的本体 QUndoStack、QUndoCommand、QUndoGroup 在 Qt 6 里搬去了 QtGui（src/gui/util），所以 CMake 里两头都得请到，Qt6::Widgets 与 Qt6::Gui 一个不能少。配套示例是一只带历史面板的文本编辑器，完整玩法放结语。

## 镜子比栈多一行，第 0 行是空标签

QUndoModel 的 rowCount 返回 count() + 1，多出的一行放在第 0 位（qundoview.cpp 第 161 行），显示 emptyLabel，出厂值是 “<empty>”（第 63 行），您用 setEmptyLabel 换成“初始状态”一类的话也行。第 N 行显示栈里第 N 条命令的 text（第 180-183 行），text 来自 QUndoCommand::setText，就是示例构造函数里那句 Insert “…”（undo_editor.cpp 第 21 行）。咱们把 data() 的分岔读一段：

```cpp
// 第 0 行给空标签，其余行给命令文字与干净标记（qundoview.cpp 第 180-188 行，摘录）
if (role == Qt::DisplayRole) {
    if (index.row() == 0)
        return m_emty_label;
    return m_stack->text(index.row() - 1);
} else if (role == Qt::DecorationRole) {
    if (index.row() == m_stack->cleanIndex() && !m_clean_icon.isNull())
        return m_clean_icon;
    return QVariant();
}
```

字段名 m_emty_label 拼错了，empty 少一个字母，这是 Qt 源码里的原样笔误，咱们照读。DecorationRole 只认干净点：行号等于 cleanIndex 的行拿图标，前提是您 setCleanIcon 过，默认空图标不显示（第 185-187 行，属性定义第 423-428 行）。图标尺寸听视图的 iconSize，那是 QListView 继承来的通用行为。行号本身带着“执行到这里”的语义，行 0 表示一切之前，行 3 表示已执行三条命令，下文的点选行为全靠它。

咱们再看看为什么改不了列表文字。QUndoModel 没有重写 flags()，QAbstractItemModel 的默认返回只有 Selectable 与 Enabled，出处 qabstractitemmodel.cpp 第 2458-2464 行。列表里没有 ItemIsEditable，双击也进不了编辑态。想换显示文字，改 emptyLabel 或给命令 setText，视图里直接改是没指望的。

## 点一行就是拨到那一步，方向盘是选中模型

QUndoViewPrivate::init 干了一件有性格的事：模型自建一只 QItemSelectionModel，直接装给视图（qundoview.cpp 第 253-260 行），选中模型与模型同生，外部塞不进来。点选的去向在 setStackCurrentIndex，咱们读实现：

```cpp
// 点选行号直接喂给栈（qundoview.cpp 第 112-124 行，摘录）
void QUndoModel::setStackCurrentIndex(const QModelIndex &index)
{
    if (m_stack == nullptr)
        return;

    if (index == selectedIndex())
        return;

    if (index.column() != 0)
        return;

    m_stack->setIndex(index.row());
}
```

setIndex 会一路 undo 或 redo 到目标行，出处 qundostack.cpp 第 830-842 行。您在历史列表里点中间某一行，跳过的命令挨个回退或重放，文档状态直接落到那一步。selectedIndex 反过来起行号，用的是栈的 index()，出处 qundoview.cpp 第 126-129 行，两侧互为镜像。栈每动一步发 indexChanged，QUndoModel 听到就 beginResetModel、endResetModel，整表重建后再把选中行设回当前 index（第 105-110 行）。撤销一次就整表重建，几百条命令也不心疼，Qt 在这里选了简单。

## setStack 与 setGroup，镜子照谁由后设的赢

setStack 进门第一件事是 setGroup(nullptr)，出处 qundoview.cpp 第 333-340 行。单栈与组两种观察模式互斥，后设的一方接管。setGroup 反过来把组的 activeStackChanged 信号接到模型的 setStack 槽上（第 353-374 行），组里活跃栈一换，镜子自动转照新的栈，您什么都不用管。

多文档场景正需要这个。每个文档一只 QUndoStack，全部交给一只 QUndoGroup。栈的构造函数认 parent，父亲是组就自动入组（qundostack.cpp 第 477-480 行），所以 new QUndoStack(group) 之后再调 addStack 是重复劳动，addStack 里有查重（qundogroup.cpp 第 98-99 行），坏不了东西，只是多余。一只栈同时只属于一个组，addStack 会把它从旧组偷走（第 102-103 行）。setActiveStack 是切换总闸（第 151-203 行）：旧活跃栈连到组上的六个信号逐一摘下，挂到新栈，再把新栈的当前状态重发一遍。摘下与挂上的六个信号，canUndoChanged、undoTextChanged 一组，canRedoChanged、redoTextChanged 一组，indexChanged、cleanChanged 一组。组的 undo() 只有一行，活跃栈非空就转发（第 308-313 行）。组上 createUndoAction 造出的菜单项，因此永远跟着活跃文档走（第 237-260 行）。示例是单文档，撤销重做动作是自己造的 QAction，槽里把守卫标志一置一清（undo_editor.cpp 第 103-121 行），enabled 状态挂栈的 canUndoChanged 与 canRedoChanged 自动管理。您做多文档时把这组动作换成组上的 createUndoAction，菜单与快捷键一字不用改，守卫照样要自己包，组动作内部一样直连栈的 undo。

## push 立即执行 redo，也顺手销毁重做历史

栈的心脏在 push（qundostack.cpp 第 567-624 行）。命令一进来就调 redo()（第 570-571 行），push 没有“只存不执行”的用法。接着它把当前 index 之上的命令全部删掉（第 583-584 行）：您撤销了三步再输入新内容，那三步的重做历史当场销毁，撤销重做系统里这是通例。还有一个安静的副作用，被删的命令若含着干净点，clean_index 会被置成 -1（第 585-586 行），保存点从此不存在，文档会一直显示已修改，除非再存一次触发 setClean。示例正是拿 cleanChanged 驱动标题修改标记，出处 undo_editor.cpp 第 161-162 行。您自己写时窗口标题记得带 “[*]” 占位符，修改标记才有落脚的地方。

合并的判定也在 push 里，咱们把条件念一遍（qundostack.cpp 第 589-594 行）：栈顶命令与新命令的 id() 相等且不为 -1，还得满足一个容易被忽略的前提，当时不在干净点上（宏内或 index != clean_index）。判定通过就调栈顶的 mergeWith，返回 true 时新命令被删除（第 594-595 行）。拖动滑块想把三十次拖动并成一步，id 与 mergeWith 就是为它准备的。

index 每动一次，私有的 setIndex 就发信号：indexChanged 打头，canUndoChanged、undoTextChanged 跟上，canRedoChanged、redoTextChanged 收尾，跨过或回到干净点时再补一个 cleanChanged，出处第 415-436 行。菜单项的禁用态、QUndoView 的整表重建，您接信号刷新界面，靠的就是它们。栈的长度上限 setUndoLimit 只认空栈，栈里有命令时调用收到 qWarning 且什么都不做（第 1229-1236 行），因为截断可能砍到当前 index。超限时删的是栈底的命令（checkUndoLimit，第 445-464 行），干净点被删掉时同样置 -1，默认 0 表示不限长。

## 宏命令，beginMacro 压的是一只空父命令

beginMacro 做的事直白，咱们顺着 qundostack.cpp 第 1128-1143 行读：new 一只 QUndoCommand，setText 成宏名，压进栈。此后到 endMacro 之间的 push 不进主列表，全塞进父命令的 child_list，出处第 616-617 行。endMacro 结束编排，把 index 加一，出处第 1162-1176 行。宏在栈里就是一条命令，QUndoView 里占一行，count() 也只算一条（第 803-805 行的文档写明）。手动嵌套与它等价，QUndoCommand 的默认 redo() 顺着子命令从前往后执行，默认 undo() 从后往前回退（第 197-219 行），顺序是写死的，父命令不重写就继承。示例把替换操作包成 Replace text 宏，删除与插入两条命令一次撤销（undo_editor.cpp 第 211-216 行）。

宏进行中，您调 setClean、undo、redo 与 setIndex 一律被拒，只回一句 qWarning，出处分散在第 639-648、740-743 行，以及第 782-785、838-842 行。beginMacro 与 endMacro 必须配对，漏了 endMacro 栈就卡在宏态，撤销重做全部失灵。Clear History 菜单项调的 clear() 倒是随时可用，它不回退任何命令，直接删光历史并回到干净态（第 513-537 行）。

## 示例：contentsChange 的三要素与信号回环

配套示例里，QPlainTextEdit 自带的撤销栈被关掉，出处 undo_editor.cpp 第 84 行，命令的来源是 QTextDocument，具体走 contentsChange 信号。信号自带 position、charsRemoved、charsAdded 三要素（第 156 行），咱们照三要素决定造哪种命令：只删是 DeleteTextCommand，只增是 InsertTextCommand，替换走宏。

咱们把 push 立即执行 redo() 的语义带进示例，就能看见一层回环。contentsChange 发出时，用户敲的文本已经在文档里，此时 push 一条插入命令，它的 redo() 会把同一份文本再插一遍。示例的做法是把已插入的文本删掉，让 redo() 重演一遍（第 201-203 行），push 前后还把信号断开再重连（第 175、225 行），免得命令里的文本操作再进这个槽。

回环还有另一副面孔，undo 与 redo 自己也动文档。命令的 undo() 改回文档，contentsChange 照样发，不拦的话撤销动作本身会被压成一条新命令，回环就这么转起来。示例的拦法是 m_isUndoing 标志位（第 79、170 行），置位的活交给撤销重做动作的槽：进槽置上 true，调完栈的 undo 或 redo 再清回 false（第 103-121 行），守卫在信号到达之前就位。您要是改用 QUndoStack 自带的 createUndoAction，可得留个心眼，它内部直连栈的 undo，标志位没机会置上，守卫同样落空，示例自己造动作就是为这一步。删除分支还有一处占位串的痕迹，被删原文在 undo 侧拿不回来，存的是一个形如 “<%1 chars removed at pos %2>” 的占位串，出处第 184-190 行，撤销长删除时文本按占位串长度回退，精确恢复原文得把原文塞进命令里存着。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，行为咱们在 6.9.1 源码核对过，示例的回环行为笔者在离屏环境实测。QUndoView 在 QtWidgets，其余三个类在 QtGui。

[Qt 文档 · QUndoView](https://doc.qt.io/qt-6/qundoview.html) -- 撤销历史列表视图，emptyLabel 与 cleanIcon 的定义处

[Qt 文档 · QUndoStack](https://doc.qt.io/qt-6/qundostack.html) -- push、undo、redo、setIndex、setUndoLimit 与信号一览

[Qt 文档 · QUndoCommand](https://doc.qt.io/qt-6/qundocommand.html) -- redo、undo、id、mergeWith 与子命令遍历顺序

[Qt 文档 · QUndoGroup](https://doc.qt.io/qt-6/qundogroup.html) -- 多栈管理与活跃栈切换

[Qt 文档 · QTextDocument](https://doc.qt.io/qt-6/qtextdocument.html) -- contentsChange 信号，示例的命令来源

---

撤销框架到本篇读完，条目视图家族也在 53 篇收了队。下一站 [3.55 QMainWindow 进阶](./55-qmainwindow-advanced.md)，接得正好：本篇示例的壳子就是 QMainWindow，菜单栏、工具栏、可停靠的历史面板，咱们已经用了它一整篇，下一篇把 QMainWindow 的机制讲清楚。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/54-qundoview-advanced/，可执行文件 54-qundoview-advanced）。做三个实验。连续输入几段文字，历史面板逐行增加，第 0 行始终是 <empty>。点历史列表中间任意一行，看编辑器状态直接跳到那一步，再点另一行，文档跟着跳。输入一段长文本后按一次 Ctrl+Z，历史列表不添新行，文档精确退回输入前的样子，守卫在工作。您把撤销槽里的置位与清位两行（undo_editor.cpp 第 108、110 行，重做槽对应 119、121 行）注释掉再按 Ctrl+Z，历史列表当场多出一行 Delete，回环的痕迹就露出来了。

完成标准照旧。行号与命令序号的错位对应哪几行代码，您指得出。点一行列表项经谁走到 setIndex，您说得出。push 销毁重做历史、干净点丢失，后果您讲得明。宏进行中被拒绝的四个操作，您数得清。
