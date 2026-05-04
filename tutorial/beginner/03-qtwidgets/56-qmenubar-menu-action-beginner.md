# 现代Qt开发教程（新手篇）3.56——QMenuBar / QMenu / QAction：菜单系统

## 1. 前言 / 菜单系统远不止一行文字加一个回调

每个桌面应用都有菜单栏，这件事看起来平淡无奇——点击"文件"弹出下拉列表，再点"新建"触发一个操作，如此而已。但当我们真正动手去搭一个完整的菜单系统时，会发现它背后的细节远比想象中多：快捷键绑定、图标显示、禁用状态、勾选状态、子菜单嵌套、分隔线组织、右键上下文菜单、跨平台快捷键差异……这些事情单独看都不复杂，但全部组装到一个真实的应用菜单中时，很容易搞出一堆零散的代码和难以维护的菜单创建逻辑。

Qt 的菜单系统由三个核心类组成。QMenuBar 是菜单栏容器，通常占据窗口顶部的一条水平区域，里面包含若干个 QMenu（顶级菜单）。每个 QMenu 是一个下拉列表，里面包含若干个 QAction（菜单项）。QAction 不是一个可见控件——它是一个抽象的"动作"对象，可以被菜单项、工具栏按钮、快捷键、右键菜单等多种触发方式共享。这种设计的好处是：你只需要创建一个 QAction，设置好它的文本、图标、快捷键、回调函数，然后把它同时添加到菜单和工具栏——两处的状态自动同步（比如禁用了 QAction 之后，菜单项和工具栏按钮都变灰）。

今天我们从四个方面展开。先看 menuBar()->addMenu() 如何添加顶级菜单并组织菜单层级，然后研究 QAction 的创建和配置（文本、图标、快捷键、状态提示），接着讨论 setCheckable 和 addSeparator 这两个常用的菜单组织手段，最后实现 contextMenuEvent 中的右键上下文菜单。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QMenuBar、QMenu、QAction 在 QtWidgets 模块中，链接 Qt6::Widgets 即可。示例代码涉及 QMenuBar、QMenu、QAction、QPlainTextEdit、QMainWindow、QToolBar、QKeySequence、QCursor 和 QContextMenuEvent。

## 3. 核心概念讲解

### 3.1 menuBar()->addMenu() 添加顶级菜单

在 QMainWindow 中获取菜单栏只需要调用 menuBar()。这个方法在第一次调用时会自动创建一个 QMenuBar 实例，后续调用返回同一个对象。有了 QMenuBar 之后，通过 addMenu(const QString &title) 添加顶级菜单——每个顶级菜单在菜单栏中显示为一个可点击的文本项，点击后弹出一个下拉列表。

```cpp
auto *fileMenu = menuBar()->addMenu("文件(&F)");
auto *editMenu = menuBar()->addMenu("编辑(&E)");
auto *viewMenu = menuBar()->addMenu("视图(&V)");
auto *helpMenu = menuBar()->addMenu("帮助(&H)");
```

标题中的 & 符号定义了键盘助记符（mnemonic）。"文件(&F)" 表示在菜单栏获得焦点后，按下 F 键可以直接打开"文件"菜单。这个助记符在显示时会被渲染成带下划线的 F（根据平台主题不同可能有差异）。macOS 上的菜单栏由系统全局管理，助记符在 macOS 上通常不可见也不可操作，但设置它不会有任何副作用。

addMenu 的返回值是 QMenu 指针，你可以继续往 QMenu 中添加子项——QAction 或嵌套的子 QMenu。QMenu 的 addMenu(const QString &title) 创建子菜单，在父菜单中显示为一个带右箭头的条目，鼠标悬停时展开。

```cpp
auto *fileMenu = menuBar()->addMenu("文件(&F)");

// 子菜单："最近打开"
auto *recentMenu = fileMenu->addMenu("最近打开(&R)");
recentMenu->addAction("file1.txt");
recentMenu->addAction("file2.txt");
recentMenu->addAction("file3.txt");
```

这种嵌套结构可以无限层叠，但实际上超过两层的菜单嵌套在用户体验上就非常糟糕了——大多数桌面应用的菜单结构控制在两到三层以内。一级是菜单栏上的顶级分类（文件、编辑、视图、工具、帮助），二级是功能分组（新建/打开/保存/退出），三级偶尔出现在"最近打开"或"工具 > 外部工具"这种场景中。

QMenu 还支持通过 insertMenu、insertAction、insertSeparator 在指定位置插入菜单项，而不是追加到末尾。这在需要动态调整菜单顺序的场景中很有用。removeAction 可以移除一个 QAction，但这只是从 QMenu 中移除它的显示——QAction 对象本身不会被 delete。

### 3.2 QAction 创建菜单项/图标/快捷键

QAction 是 Qt 菜单系统的核心抽象。它封装了一个"可以被触发的动作"——不管这个动作最终是通过菜单点击、工具栏按钮点击、还是键盘快捷键触发的，信号都从同一个 QAction 发出。这意味着你只需要连接一次 triggered 信号，无论用户用什么方式触发这个动作，回调函数都会被执行。

创建 QAction 最直接的方式是通过 QMenu 的 addAction(const QString &text) 便捷方法。它创建一个 QAction 并添加到菜单中，返回 QAction 指针。但如果你需要在多个地方共享同一个 QAction（比如菜单和工具栏共用），应该先单独创建 QAction，然后分别添加到菜单和工具栏。

```cpp
// 方式一：便捷方法（创建并添加到菜单）
auto *newAction = fileMenu->addAction("新建(&N)");

// 方式二：显式创建（可共享到工具栏等）
auto *openAction = new QAction("打开(&O)", this);
openAction->setIcon(
    style()->standardIcon(QStyle::SP_DirOpenIcon));
openAction->setShortcut(QKeySequence::Open);
openAction->setStatusTip("打开已有文件");
fileMenu->addAction(openAction);
toolBar->addAction(openAction);
```

QAction 的几个关键属性我们逐个过一遍。setText 设置菜单项的显示文本，同样支持 & 助记符。setIcon 设置图标——图标会显示在菜单项的左侧，同时也会出现在工具栏按钮上。图标的最佳来源是 QStyle::standardIcon，它提供了一套跨平台的标准图标（打开、保存、撤销、重做、对话框按钮等），在不同操作系统上自动适配为对应平台的风格。如果你使用自定义图标，建议使用 SVG 格式（通过 QIcon 加载 .svg 文件），这样可以自动适配不同 DPI 的显示器。

setShortcut 设置快捷键。QKeySequence 提供了大量标准快捷键常量（QKeySequence::New、QKeySequence::Open、QKeySequence::Save、QKeySequence::Undo 等），这些常量在不同平台上会自动映射为对应平台的快捷键组合——比如 QKeySequence::Copy 在 Windows/Linux 上是 Ctrl+C，在 macOS 上是 Cmd+C。永远不要硬编码 Ctrl+C 这样的快捷键，使用 QKeySequence::Copy。

```cpp
// 跨平台正确写法
openAction->setShortcut(QKeySequence::Open);

// 跨平台错误写法（macOS 上无效）
// openAction->setShortcut(QKeySequence("Ctrl+O"));

// 自定义快捷键
auto *refreshAction = new QAction("刷新(&R)", this);
refreshAction->setShortcut(QKeySequence(Qt::Key_F5));
```

setStatusTip 设置状态提示文本。当用户把鼠标悬停在菜单项上时，QMainWindow 的状态栏会自动显示这个提示文本。这个行为是 QMainWindow 内置的——它会监听 QAction 的 hovered 信号，然后调用 statusBar()->showMessage(action->statusTip())。

setEnabled 控制菜单项是否可用。禁用的菜单项显示为灰色，不可点击。这个属性在 QAction 层面设置——如果一个 QAction 被添加到了菜单和工具栏两处，setEnabled 一次就能同时影响两处。这是 QAction 共享设计的核心优势之一。

```cpp
// 没有选中文本时禁用"复制"操作
copyAction->setEnabled(m_editor->textCursor().hasSelection());
```

### 3.3 setCheckable 与 addSeparator

setCheckable(bool) 让 QAction 变成一个可勾选的菜单项。勾选状态的菜单项在左侧显示一个对勾标记（或者一个单选按钮样式的标记，取决于 QMenu 的配置和平台主题），再次点击取消勾选。

```cpp
auto *showLineNumbers = new QAction("显示行号(&L)", this);
showLineNumbers->setCheckable(true);
showLineNumbers->setChecked(true);
viewMenu->addAction(showLineNumbers);

connect(showLineNumbers, &QAction::toggled,
        this, &MainWindow::toggleLineNumbers);
```

注意这里的信号选择：setCheckable 的 QAction 在被点击时发射 triggered(bool checked) 信号，参数是点击后的勾选状态。同时也发射 toggled(bool checked) 信号。两者的区别在于：triggered 在每次用户点击时都发射（不管状态有没有变化），toggled 只在状态确实发生变化时发射。绝大多数场景下用 toggled 更合适——你只关心状态的变化，不关心重复的点击。

对于"多选一"的场景（比如一组互斥的选项），QMenu 提供了一个 QActionGroup 机制。把多个 setCheckable 的 QAction 放进同一个 QActionGroup 中，组内的 QAction 就变成互斥的——选中一个自动取消其他。

```cpp
auto *sizeGroup = new QActionGroup(this);

auto *smallAction = viewMenu->addAction("小字体");
smallAction->setCheckable(true);
sizeGroup->addAction(smallAction);

auto *mediumAction = viewMenu->addAction("中字体");
mediumAction->setCheckable(true);
mediumAction->setChecked(true);  // 默认选中
sizeGroup->addAction(mediumAction);

auto *largeAction = viewMenu->addAction("大字体");
largeAction->setCheckable(true);
sizeGroup->addAction(largeAction);

// 点击组内任意一个 Action，自动取消其他
connect(sizeGroup, &QActionGroup::triggered,
        this, &MainWindow::onFontSizeChanged);
```

addSeparator 在 QMenu 中添加一条水平分隔线。它的用途是将菜单项按功能分组——比如"文件"菜单中，"新建/打开"是一组，"保存/另存为"是一组，"打印"是一组，"退出"单独一组，每组之间用分隔线隔开。分隔线在视觉上是一条淡灰色的水平线，不可点击。

```cpp
auto *fileMenu = menuBar()->addMenu("文件(&F)");

fileMenu->addAction("新建");
fileMenu->addAction("打开");

fileMenu->addSeparator();  // 分隔线

fileMenu->addAction("保存");
fileMenu->addAction("另存为");

fileMenu->addSeparator();  // 分隔线

fileMenu->addAction("打印");

fileMenu->addSeparator();  // 分隔线

fileMenu->addAction("退出");
```

addSeparator 的返回值是一个 QAction 指针（separator action）。你可以保存这个指针，然后通过 removeAction 来动态移除分隔线。这在需要根据运行时状态动态重组菜单结构的场景中有用。

关于菜单组织有一个值得强调的原则：菜单项的顺序应该遵循"使用频率 + 功能相关性"的排序。最常用的操作放在最上面，最危险的操作（比如"退出"）放在最下面，中间用分隔线分组。用户打开菜单后眼睛通常会从上往下扫——"新建/打开"比"退出"更常被点击，所以放在上面。这不是硬性规则，但几乎所有的桌面应用都遵循这个惯例，用户已经形成了肌肉记忆。

### 3.4 contextMenuEvent 右键菜单

右键上下文菜单（context menu）是菜单系统的另一个重要组成部分。和菜单栏中的固定菜单不同，上下文菜单是动态弹出的——用户在某个控件上点击鼠标右键，弹出一个和当前上下文相关的菜单。比如在文本编辑器中右键，弹出的菜单包含"剪切/复制/粘贴"；在文件列表中右键一个文件，弹出的菜单包含"打开/重命名/删除"。

实现右键菜单有两种方式。第一种是重写 QWidget::contextMenuEvent(QContextMenuEvent *event)。这个事件处理器在用户右键点击控件时被调用，event->globalPos() 给出了鼠标在屏幕上的全局坐标，event->pos() 给出了鼠标在控件内的相对坐标。你需要在 contextMenuEvent 中创建一个 QMenu，添加需要的 QAction，然后调用 QMenu::exec(const QPoint &pos) 在指定位置弹出菜单。

```cpp
void MyEditor::contextMenuEvent(
    QContextMenuEvent *event)
{
    auto *menu = new QMenu(this);

    auto *cutAction = menu->addAction("剪切");
    auto *copyAction = menu->addAction("复制");
    auto *pasteAction = menu->addAction("粘贴");

    menu->addSeparator();

    auto *selectAllAction = menu->addAction("全选");

    // 根据当前状态启用/禁用菜单项
    bool hasSelection = textCursor().hasSelection();
    cutAction->setEnabled(hasSelection);
    copyAction->setEnabled(hasSelection);

    // 在鼠标位置弹出菜单
    QAction *chosen = menu->exec(event->globalPos());

    // 处理选中的菜单项
    if (chosen == cutAction) {
        cut();
    } else if (chosen == copyAction) {
        copy();
    } else if (chosen == pasteAction) {
        paste();
    } else if (chosen == selectAllAction) {
        selectAll();
    }

    delete menu;
}
```

exec 是一个阻塞调用——它弹出菜单并进入一个局部事件循环，直到用户选择了一个菜单项或者点击了菜单外部。返回值是用户选择的 QAction 指针，如果用户取消了菜单则返回 nullptr。由于 exec 是阻塞的，你不应该在构造函数或其他初始化代码中调用它——它只在事件处理器中使用。

第二种方式是使用 QWidget::setContextMenuPolicy(Qt::CustomContextMenu) 配合 customContextMenuRequested 信号。这种方式不需要重写 contextMenuEvent，而是在信号槽中处理。好处是你可以把右键菜单的逻辑和控件的显示逻辑分离开来。

```cpp
editor->setContextMenuPolicy(Qt::CustomContextMenu);
connect(editor, &QWidget::customContextMenuRequested,
        this, &MainWindow::showEditorContextMenu);

void MainWindow::showEditorContextMenu(
    const QPoint &pos)
{
    auto *menu = new QMenu(this);
    // ... 添加菜单项 ...

    // 将控件内坐标转换为全局坐标
    menu->exec(editor->mapToGlobal(pos));
    delete menu;
}
```

这里有一个关键的坐标转换：customContextMenuRequested 信号的 pos 参数是控件内的相对坐标，而 QMenu::exec 需要全局坐标。所以你必须调用 mapToGlobal 把相对坐标转换成全局坐标。如果你在 contextMenuEvent 中处理，event->globalPos() 已经是全局坐标了，不需要转换。

右键菜单的一个重要设计原则是：只显示和当前上下文相关的操作。不要把所有可能的操作都塞进右键菜单——那会变成一个让人头疼的超长列表。在文本编辑器中，右键菜单最多包含 6-8 个操作（剪切/复制/粘贴/删除/全选/拼写检查等），超过这个数量就应该考虑用子菜单分组。

## 4. 踩坑预防

第一个坑是 QMenu::exec 的生命周期。每次 exec 之前创建的 QMenu，在 exec 返回之后应该被 delete。如果你把 QMenu 作为成员变量反复复用，可能会导致菜单项累积——每次 exec 前需要 clear() 旧的菜单项。推荐的做法是每次右键时 new 一个 QMenu，exec 之后立刻 delete，干净利落。

第二个坑是 QAction 的 shortcut 在菜单不可见时依然生效。如果你给一个 QAction 设置了 QKeySequence::Save 快捷键，即使包含这个 QAction 的菜单当前没有展开，快捷键仍然可以触发 QAction。这在大多数情况下是期望行为（Ctrl+S 任何时候都应该能保存），但如果你有条件地禁用某些操作（比如没有打开文档时不能保存），需要同步更新 QAction 的 enabled 状态。

第三个坑是 macOS 上 QMenuBar 的行为差异。macOS 的菜单栏由系统全局管理，不显示在窗口内部。QMenuBar 在 macOS 上会被自动整合到系统的全局菜单栏中。"关于"和"退出"菜单项会被自动移到系统的应用菜单下，无论你把它们放在 Qt 菜单的什么位置。这是 Qt 在 macOS 上的特殊处理，在其他平台上不会发生。

第四个坑是 QActionGroup 的互斥行为默认不排斥"不选中"状态——也就是说，用户可以点击当前已选中的项来取消选中，导致所有项都没有被选中。如果你不希望出现这种"没有选中任何项"的状态，需要调用 QActionGroup::setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive) 并确保至少有一个项被 setChecked(true)。

第五个坑是 contextMenuEvent 的调用时机。如果你的控件上面覆盖了另一个控件（比如一个透明的 overlay），右键事件可能被上层控件拦截，不会传递到你的控件。如果发现 contextMenuEvent 不被调用，检查事件过滤链是否有其他控件先处理了右键事件。

## 5. 练习项目

我们来做一个综合练习：创建一个带有完整菜单系统的 QMainWindow 应用。菜单栏包含"文件"（新建、打开、保存、分隔线、退出）、"编辑"（撤销、重做、分隔线、剪切、复制、粘贴、分隔线、全选）、"格式"（一个子菜单"字体大小"包含小/中/大三个互斥选项，一个可勾选的"自动换行"，一个可勾选的"显示行号"）和"帮助"（关于）。工具栏包含"文件"工具栏（新建、保存按钮）和"编辑"工具栏（撤销、重做、剪切、复制、粘贴按钮），使用和菜单共享的 QAction。中央区域使用 QPlainTextEdit。重写 contextMenuEvent，根据编辑器当前状态（是否有选中文本、剪贴板是否有内容）动态创建右键菜单，包含剪切/复制/粘贴/全选操作。菜单项的 enabled 状态根据编辑器的实际状态动态更新——比如"撤销"在编辑器没有可撤销操作时应该被禁用。

提示：你可以通过 QPlainTextEdit 的 undoAvailable(bool) 和 redoAvailable(bool) 信号来动态更新撤销/重做菜单项的 enabled 状态。copyAvailable(bool) 信号可以用来更新剪切/复制菜单项的 enabled 状态。

## 6. 官方文档参考链接

[Qt 文档 -- QMenuBar](https://doc.qt.io/qt-6/qmenubar.html) -- 菜单栏控件

[Qt 文档 -- QMenu](https://doc.qt.io/qt-6/qmenu.html) -- 菜单控件

[Qt 文档 -- QAction](https://doc.qt.io/qt-6/qaction.html) -- 动作抽象类

[Qt 文档 -- QActionGroup](https://doc.qt.io/qt-6/qactiongroup.html) -- 动作分组（互斥选择）

[Qt 文档 -- QKeySequence](https://doc.qt.io/qt-6/qkeysequence.html) -- 快捷键序列

[Qt 文档 -- QContextMenuEvent](https://doc.qt.io/qt-6/qcontextmenuevent.html) -- 右键菜单事件

---

到这里，Qt 菜单系统的核心用法就全部讲完了。QMenuBar 通过 addMenu 构建顶级菜单，QMenu 通过 addAction 添加菜单项和子菜单，QAction 把文本、图标、快捷键、状态提示和回调函数封装成一个可共享的动作对象，setCheckable 和 QActionGroup 处理勾选和互斥选择，addSeparator 组织菜单项分组，contextMenuEvent 或 customContextMenuRequested 实现右键上下文菜单。把这些组合起来，就能搭建出一个功能完整、交互细腻的菜单系统。
