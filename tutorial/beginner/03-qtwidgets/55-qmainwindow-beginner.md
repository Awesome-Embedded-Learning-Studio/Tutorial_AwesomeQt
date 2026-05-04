# 现代Qt开发教程（新手篇）3.55——QMainWindow：主窗口完整配置

## 1. 前言 / 为什么 QMainWindow 是桌面应用的骨架

写 Qt 桌面应用写到一定程度，你会发现绝大多数项目的主窗口都是 QMainWindow。这不是什么约定俗成的规矩，而是因为 QMainWindow 把一个桌面应用最常用的四块区域——中央内容区、菜单栏、工具栏、状态栏——全部封装好了，并且提供了一个成熟的布局管理机制。你不需要手动计算工具栏的高度、菜单栏的位置、状态栏是否被遮挡、Dock 窗口拖出来之后中央区域怎么重新分配空间——QMainWindow 在内部把这些事情全部处理完了。用一个不太精确但很形象的类比：QMainWindow 就像一个已经搭好框架的毛坯房，你只需要往里面搬家具（塞控件），不用操心承重墙和水电管路。

如果你之前一直用 QWidget 做主窗口，大概率遇到过这些问题：手动用 QVBoxLayout 把菜单区、工具区、内容区、状态区分层管理，结果发现工具栏不能拖动、不能浮动、不能停靠到其他边缘；想要加一个可拖拽的侧边面板，发现 QWidget 根本没有这种能力；用户调整了窗口大小之后下次打开又恢复了默认值，得自己写 QSettings 存储窗口几何信息。这些问题 QMainWindow 全部内置了解决方案。

今天我们从四个方面展开。先看 setCentralWidget 如何设置 QMainWindow 的中央内容区域，然后完整搭建菜单栏、工具栏、状态栏和 Dock 窗口这四大组件，接着研究 saveGeometry / restoreGeometry 如何实现窗口尺寸的持久化存储，最后讨论多 Dock 窗口的布局策略——包括嵌套、标签化合并、嵌套联动等高级技巧。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QMainWindow 在 QtWidgets 模块中，链接 Qt6::Widgets 即可。示例代码涉及 QMainWindow、QMenuBar、QToolBar、QStatusBar、QDockWidget、QPlainTextEdit、QLabel、QSettings、QAction、QKeySequence 和 QCloseEvent。

## 3. 核心概念讲解

### 3.1 setCentralWidget：中央内容区域

QMainWindow 的布局结构和 QWidget 最大的区别在于，它有一个预定义的"中央区域"概念。这个中央区域被菜单栏（顶部）、工具栏（四边均可）、状态栏（底部）和 Dock 窗口（四边均可）包围，是应用的主要内容展示区域。你通过 setCentralWidget(QWidget *widget) 来设置中央控件——这个控件会占据中央区域的所有可用空间。

```cpp
auto *editor = new QPlainTextEdit;
setCentralWidget(editor);
```

就这么一行代码，QPlainTextEdit 就会被放到 QMainWindow 的中央区域，并且在窗口缩放时自动调整大小。中央控件只能有一个——如果你再次调用 setCentralWidget，之前的中央控件会被替换（并且被自动 delete，注意它的生命周期）。如果你需要在一个中央区域内放多个控件，应该在 setCentralWidget 中传入一个容器 QWidget，然后在容器内部用 Layout 来管理子控件。

一个不太显眼但很重要的细节：QMainWindow 在析构时会自动 delete 中央控件。这意味着你不需要手动管理中央控件的内存——setCentralWidget 之后，它的 parent 就变成了 QMainWindow，QMainWindow 的析构函数会处理清理工作。这个行为和其他 Qt 控件的 parent-child 内存管理一致，但如果你在代码中保留了指向中央控件的裸指针，在 QMainWindow 析构后访问它就是 use-after-free。

```cpp
// 正确：用容器包裹多个子控件
auto *container = new QWidget;
auto *layout = new QHBoxLayout(container);

auto *treeView = new QTreeView;
auto *textEdit = new QPlainTextEdit;
auto *splitter = new QSplitter;
splitter->addWidget(treeView);
splitter->addWidget(textEdit);

layout->addWidget(splitter);
setCentralWidget(container);
```

中央区域的大小由 QMainWindow 的整体大小和四周组件的占用空间共同决定。当你往 QMainWindow 的顶部或底部添加工具栏时，中央区域的可用高度会减少；当你往左右两侧停靠 Dock 窗口时，中央区域的可用宽度会减少。QMainWindow 的内部布局引擎会自动计算这些空间的分配，你不需要手动处理。

### 3.2 菜单栏/工具栏/状态栏/Dock 完整搭建

QMainWindow 的四大组件分别对应四个方法：menuBar() 返回菜单栏（顶部），addToolBar() 创建并添加工具栏，statusBar() 返回状态栏（底部），addDockWidget() 添加 Dock 窗口。它们的创建和添加方式各有特点，我们逐一看过去。

菜单栏通过 menuBar() 获取。这个方法在第一次调用时会自动创建一个 QMenuBar 并设置为 QMainWindow 的菜单栏，之后每次调用都返回同一个对象。QMenuBar 本身不做什么事情，你需要往上面添加 QMenu（顶级菜单），再往 QMenu 里添加 QAction（菜单项）。

```cpp
auto *fileMenu = menuBar()->addMenu("文件(&F)");
auto *editMenu = menuBar()->addMenu("编辑(&E)");
auto *viewMenu = menuBar()->addMenu("视图(&V)");

// 文件菜单
auto *newAction = fileMenu->addAction("新建(&N)");
newAction->setShortcut(QKeySequence::New);

auto *openAction = fileMenu->addAction("打开(&O)");
openAction->setShortcut(QKeySequence::Open);

fileMenu->addSeparator();

auto *quitAction = fileMenu->addAction("退出(&Q)");
quitAction->setShortcut(QKeySequence::Quit);
```

工具栏通过 addToolBar(const QString &title) 创建。它返回一个 QToolBar 指针，你可以往上面添加 QAction（工具按钮）、QWidget（任意控件）和分隔线。工具栏默认可以被用户拖动——拖到窗口的其他边缘会自动停靠，拖到窗口外面会变成浮动窗口。这个行为可以通过 setMovable(false) 来禁用。

```cpp
auto *fileToolBar = addToolBar("文件");
fileToolBar->addAction(newAction);
fileToolBar->addAction(openAction);
fileToolBar->addSeparator();
fileToolBar->addAction(quitAction);

auto *editToolBar = addToolBar("编辑");
editToolBar->addAction(/* undoAction */);
editToolBar->addAction(/* redoAction */);
```

状态栏通过 statusBar() 获取，和 menuBar() 一样是懒创建的。状态栏有两种类型的消息：临时消息（showMessage，在指定时间后自动消失）和永久控件（addPermanentWidget，一直显示在状态栏右侧）。临时消息和永久控件不冲突——showMessage 的文本显示在状态栏左侧，永久控件固定在右侧。

```cpp
statusBar()->showMessage("就绪", 3000);

// 添加永久控件：一个显示当前光标位置的 QLabel
auto *positionLabel = new QLabel("行 1, 列 1");
statusBar()->addPermanentWidget(positionLabel);
```

Dock 窗口通过 addDockWidget(Qt::DockWidgetArea area, QDockWidget *dock) 添加。Dock 窗口是一个可以拖拽、浮动、关闭的子面板。初始停靠位置通过 Qt::DockWidgetArea 参数指定（Qt::LeftDockWidgetArea、Qt::RightDockWidgetArea、Qt::TopDockWidgetArea、Qt::BottomDockWidgetArea）。

```cpp
auto *outlineDock = new QDockWidget("大纲", this);
outlineDock->setWidget(new QTreeView);
addDockWidget(Qt::LeftDockWidgetArea, outlineDock);

auto *outputDock = new QDockWidget("输出", this);
outputDock->setWidget(new QPlainTextEdit);
addDockWidget(Qt::BottomDockWidgetArea, outputDock);
```

这四个组件在 QMainWindow 中的空间分配遵循一个优先级顺序：菜单栏固定在顶部，状态栏固定在底部，工具栏和 Dock 窗口竞争剩余的四边空间，中央区域占据所有剩余空间。当多个工具栏停靠在同一侧时，它们会纵向排列（如果停靠在左/右侧）或横向排列（如果停靠在顶/底部）。

### 3.3 saveGeometry / restoreGeometry 窗口尺寸持久化

用户调整了窗口大小、拖动了工具栏的位置、把某个 Dock 窗口拖出来浮动了——这些布局状态在应用重启后通常会丢失，除非你主动保存。QMainWindow 提供了 saveGeometry() 和 restoreState() 两个方法，配合 QSettings，可以一行代码搞定窗口状态的持久化。

saveGeometry() 返回一个 QByteArray，包含了 QMainWindow 的位置、大小和最大化状态。saveState() 返回的 QByteArray 包含了工具栏和 Dock 窗口的布局状态（停靠位置、浮动状态、是否可见、大小比例）。这两个 QByteArray 通常存储在 QSettings 中。

```cpp
// 保存窗口状态（在 closeEvent 或析构函数中）
void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings("MyCompany", "MyApp");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    QMainWindow::closeEvent(event);
}

// 恢复窗口状态（在构造函数中，UI 初始化完成后）
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    initUi();  // 创建所有控件、工具栏、Dock 窗口...

    QSettings settings("MyCompany", "MyApp");
    if (settings.contains("geometry")) {
        restoreGeometry(
            settings.value("geometry").toByteArray());
    }
    if (settings.contains("windowState")) {
        restoreState(
            settings.value("windowState").toByteArray());
    }
}
```

这里有一个非常关键的调用顺序：你必须先完成所有 UI 初始化（包括 addToolBar、addDockWidget 等操作），然后再调用 restoreState。原因是 restoreState 通过对象名称（objectName）来匹配工具栏和 Dock 窗口——它遍历保存的状态数据，找到 objectName 匹配的控件，然后恢复其位置和状态。如果你在 restoreState 之后才创建某个工具栏或 Dock 窗口，它的状态就无法被恢复。所以务必给每个工具栏和 Dock 窗口设置一个唯一的 objectName。

```cpp
// 必须设置 objectName，否则 restoreState 无法匹配
auto *fileToolBar = addToolBar("文件");
fileToolBar->setObjectName("fileToolBar");

auto *outlineDock = new QDockWidget("大纲", this);
outlineDock->setObjectName("outlineDock");
```

restoreGeometry 只恢复窗口的位置和大小，不恢复工具栏和 Dock 的布局。restoreState 只恢复工具栏和 Dock 的布局，不恢复窗口的位置和大小。两者通常配合使用。如果你只需要保存窗口尺寸而不关心工具栏位置，只用 saveGeometry / restoreGeometry 就够了。

有一个容易踩的坑：在多显示器环境下，restoreGeometry 可能会恢复到一个已经不存在的显示器上（比如用户拔掉了外接显示器）。Qt 在 6.x 版本中对此做了一些处理——如果恢复的位置完全在所有屏幕之外，Qt 会自动调整到主屏幕上。但在某些边缘情况下（比如窗口只有一小部分在屏幕外），你可能需要手动检查 restoreGeometry 后的窗口位置是否在有效区域内。

### 3.4 多 Dock 窗口布局策略

当你有多个 Dock 窗口时，如何安排它们的初始位置和交互行为就变成一个需要仔细考虑的问题。QMainWindow 提供了几种机制来管理多 Dock 窗口的布局。

最基本的是嵌套停靠。当你把多个 Dock 窗口停靠在同一侧时，QMainWindow 会把它们纵向（左右侧）或横向（顶底部）平铺排列。用户可以通过拖拽来调整 Dock 窗口之间的分隔比例。你可以通过 resizeDocks 来程序化地设置 Dock 窗口的尺寸。

```cpp
// 设置多个 Dock 窗口的尺寸
QList<QDockWidget*> docks = {outlineDock, propertiesDock};
QList<int> sizes = {200, 150};
resizeDocks(docks, sizes, Qt::Vertical);
```

更强大的布局策略是标签化合并（tabify）。当你有两个或多个 Dock 窗口停靠在同一侧时，可以用 tabifyDockWidget 把它们合并成标签页形式——类似于浏览器中的多标签页，一次只显示一个 Dock 的内容，通过点击标签切换。

```cpp
// 把 propertiesDock 标签化到 outlineDock 旁边
tabifyDockWidget(outlineDock, propertiesDock);

// 默认显示 outlineDock
outlineDock->raise();
```

tabifyDockWidget 的两个参数没有顺序要求——它只是把第二个 Dock 作为第一个的"邻居"标签页添加进去。如果第一个 Dock 当前不是标签页形式，Qt 会自动把它转换成标签页模式。

setDockNestingEnabled(bool enable) 控制是否允许 Dock 窗口的嵌套停靠行为。默认情况下 Dock 嵌套是启用的，这意味着用户可以把一个 Dock 窗口拖到另一个已经停靠的 Dock 窗口的内部，形成更复杂的嵌套布局。如果你希望限制用户只能使用简单的上下左右平铺布局，可以禁用嵌套。

```cpp
setDockNestingEnabled(false);
```

setCorner(Qt::Corner corner, Qt::DockWidgetArea area) 是一个容易被忽略但很实用的方法。它控制 QMainWindow 四个角落区域的归属——当一个 Dock 停靠在顶部、另一个停靠在左侧时，左上角的区域属于谁？默认情况下左上角和左下角属于左侧 Dock 区域，右上角和右下角属于右侧 Dock 区域。但你可以通过 setCorner 来改变这个行为。

```cpp
// 把左上角分配给顶部 Dock 区域
setCorner(Qt::TopLeftCorner, Qt::TopDockWidgetArea);
// 把右下角分配给底部 Dock 区域
setCorner(Qt::BottomRightCorner, Qt::BottomDockWidgetArea);
```

在实际项目中，一个比较好的多 Dock 布局策略是：根据功能区域分组——编辑相关的 Dock（大纲、符号列表）放在左侧，属性/输出相关的 Dock（属性面板、编译输出、调试控制台）放在底部，预览/工具类的 Dock（颜色选择、组件库）放在右侧。初始化时通过 tabifyDockWidget 把同一区域的多个 Dock 合并成标签页，避免界面过于拥挤。然后用 saveState / restoreState 记住用户的自定义布局——让用户自己调整到最舒服的状态。

## 4. 踩坑预防

第一个坑是 setCentralWidget 的中央控件在 QMainWindow 析构时会被自动 delete。如果你在代码中保留了指向中央控件的裸指针，在 QMainWindow 析构之后访问它就是未定义行为。如果你需要在 QMainWindow 生命周期之外继续使用中央控件，要么不要 setCentralWidget 而是通过 layout 管理，要么在 closeEvent 中先 takeCentralWidget() 把它取出来。

第二个坑是 restoreState 必须在所有工具栏和 Dock 窗口创建之后调用，并且每个控件必须设置了唯一的 objectName。否则 restoreState 找不到匹配的控件，所有布局状态都无法恢复。建议在 addToolBar 和 addDockWidget 之后立刻 setObjectName，形成习惯。

第三个坑是工具栏和 Dock 窗口在 macOS 上的行为和其他平台有差异。macOS 的全局菜单栏由系统管理，QMainWindow 的 menuBar() 在 macOS 上不会显示在窗口内——它出现在系统菜单栏中。工具栏的浮动行为在 macOS 上也受到系统全局工具栏样式的影响。如果你的应用需要跨平台，务必在 macOS 上做额外的测试。

第四个坑是 saveGeometry 保存的是窗口在屏幕上的绝对位置。在多显示器环境下，如果用户改变了显示器的排列顺序或者分辨率，恢复出来的窗口位置可能不正确。Qt 会做一些自动修正，但极端情况下（比如从三屏变成单屏）可能需要你手动处理。

第五个坑是当你在代码中频繁调用 addDockWidget 和 removeDockWidget 时，QMainWindow 的内部布局引擎会频繁重新计算。如果你需要一次性做大量的布局变更，可以考虑先 hide() 所有的 Dock 窗口，做完变更后再 show()，减少中间状态的布局计算。

## 5. 练习项目

我们来做一个综合练习：创建一个完整的 QMainWindow 应用。中央区域使用 QPlainTextEdit 作为文本编辑器。顶部菜单栏包含"文件"（新建、打开、保存、分隔线、退出）和"视图"（切换大纲面板可见性、切换输出面板可见性、恢复默认布局）。两个工具栏：文件工具栏（新建、保存按钮）和编辑工具栏（撤销、重做按钮）。底部状态栏包含一个临时消息区和一个永久显示的光标位置 QLabel。两个 Dock 窗口：左侧的"大纲"面板和底部的"输出"面板。使用 QSettings 在关闭时保存窗口几何信息和布局状态，在启动时恢复。重写 closeEvent 执行保存逻辑。所有工具栏和 Dock 窗口都设置了 objectName。

提示：菜单栏的"视图"菜单中，可以通过 QDockWidget::toggleViewAction() 获取一个现成的 QAction，点击它可以切换 Dock 窗口的显示/隐藏状态。不需要自己写显示隐藏逻辑。

## 6. 官方文档参考链接

[Qt 文档 -- QMainWindow](https://doc.qt.io/qt-6/qmainwindow.html) -- 主窗口类

[Qt 文档 -- QDockWidget](https://doc.qt.io/qt-6/qdockwidget.html) -- 可停靠面板

[Qt 文档 -- QToolBar](https://doc.qt.io/qt-6/qtoolbar.html) -- 工具栏

[Qt 文档 -- QStatusBar](https://doc.qt.io/qt-6/qstatusbar.html) -- 状态栏

[Qt 文档 -- QSettings](https://doc.qt.io/qt-6/qsettings.html) -- 应用设置持久化

[Qt 文档 -- Main Window Examples](https://doc.qt.io/qt-6/examples-mainwindow.html) -- 主窗口示例集

---

到这里，QMainWindow 的核心用法就全部讲完了。setCentralWidget 定义中央内容区域，menuBar / addToolBar / statusBar / addDockWidget 四个方法搭建出完整的窗口骨架，saveGeometry / saveState / restoreGeometry / restoreState 配合 QSettings 实现窗口状态的持久化，tabifyDockWidget 和 setDockNestingEnabled 让多 Dock 布局变得灵活可控。把这些组合起来，就是一个专业级桌面应用的主窗口基础。
