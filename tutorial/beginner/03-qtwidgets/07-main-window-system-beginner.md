# 现代Qt开发教程（新手篇）3.7——QMainWindow 主窗口体系基础

## 1. 前言 / 为什么 QMainWindow 是"正经应用"的骨架

前面我们写的所有示例都是继承 QWidget 做主窗口，拉几个控件上去就完事了。对于一个只有两三个按钮的小工具来说，QWidget 完全够用。但当你开始做"像一个正经桌面软件"的东西——有菜单栏、有工具栏、有状态栏、有可拖拽的侧边面板——你就需要 QMainWindow 了。

QMainWindow 不是什么神奇的魔法类，它本质上就是对一个窗口做了区域划分。你可以把它想象成一栋房子的户型图：顶部是菜单栏（厨房）、菜单栏下面是工具栏（灶台）、底部是状态栏（地脚线）、左右两侧是可以停靠的面板（储物柜）、中间最大的那块区域是中央控件（客厅）。每个区域都有专门的 API 来管理，而且这些区域之间的协作——比如菜单项和工具栏按钮共享同一个 QAction、状态栏自动显示鼠标悬停的提示文字——都已经由 Qt 帮你处理好了。

说句实在话，如果你之前只用过 QWidget 做主窗口，第一次打开 QMainWindow 的文档可能会被它那一堆区域管理 API 吓到——menuBar()、addToolBar()、statusBar()、addDockWidget()、setCentralWidget()——感觉每个区域都有自己的一套方法。但实际上这些 API 的设计非常一致，每个区域的用法都遵循同一个模式：创建组件、配置内容、添加到主窗口。这篇文章我们一个一个来，从菜单栏到工具栏到状态栏再到停靠面板，把 QMainWindow 的四大区域全部走一遍。

## 2. 环境说明

本篇代码基于 Qt 6.5+，CMake 3.26+，C++17 标准。QMainWindow 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。菜单栏在不同平台上的渲染方式有差异——macOS 上菜单栏会被移到系统顶部菜单栏区域，Windows 和 Linux 上菜单栏在窗口内部。这种差异是 Qt 自动处理的，你的代码不需要针对平台做任何适配。

## 3. 核心概念讲解

### 3.1 QMainWindow 的五大区域

在动手写代码之前，我们需要先搞清楚 QMainWindow 把窗口划分成了哪几个区域，以及每个区域的用途和特性。QMainWindow 的布局是由 Qt 内部管理的，你不应该自己给 QMainWindow 设置布局（调用 `setLayout()` 会在运行时输出警告并可能导致布局混乱）。

顶部区域是菜单栏（Menu Bar），通过 `menuBar()` 获取。菜单栏是一个水平条，里面包含若干个下拉菜单（QMenu），每个下拉菜单里包含若干个菜单项（QAction）。菜单栏在整个窗口的最顶部，macOS 上会被移到系统菜单栏。

菜单栏下面是工具栏区域（Toolbars），通过 `addToolBar()` 添加。工具栏是水平的（也可以设置为垂直的）条状区域，里面放各种快捷操作按钮。你可以添加多个工具栏，它们会自动排列。工具栏支持拖拽——用户可以把工具栏拖到窗口的四个边缘或者浮动出来成为一个独立窗口。

窗口最底部是状态栏（Status Bar），通过 `statusBar()` 获取。状态栏用来显示一些临时的提示信息（比如"就绪""正在加载..."），也可以在右侧放置永久性的信息组件（比如一个显示当前行号的 QLabel）。

窗口的左右两侧和底部（在状态栏上方）是停靠区域（Dock Widget Area），用来放置 QDockWidget。停靠面板可以像 IDE 里的侧边栏一样吸附在主窗口的边缘，也可以拖拽出来成为浮动窗口，还可以在多个停靠区域之间切换。

最中间最大的那块区域是中央控件（Central Widget），通过 `setCentralWidget()` 设置。中央控件占据所有其他区域剩下的空间。通常你会在中央控件里放一个 QTextEdit、QTableView 或者自定义的编辑器组件。

```cpp
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr)
        : QMainWindow(parent)
    {
        setWindowTitle("QMainWindow 区域演示");
        resize(800, 600);

        // 中央控件
        auto *central = new QTextEdit("这是中央控件区域");
        setCentralWidget(central);

        // 菜单栏
        setupMenuBar();

        // 工具栏
        setupToolBar();

        // 状态栏
        statusBar()->showMessage("就绪");
    }
};
```

你会发现 QMainWindow 的使用方式非常线性——构造函数里依次设置中央控件、创建菜单栏、创建工具栏、配置状态栏，没有任何需要特别处理的顺序依赖。唯一的要求是中央控件必须设置，否则中间会是一个空白区域。

### 3.2 QMenuBar / QMenu / QAction 菜单系统构建

Qt 的菜单系统由三层对象组成：QMenuBar 是整个菜单栏容器，QMenu 是一个下拉菜单，QAction 是菜单中的一个操作项。这三者的关系是：QMenuBar 包含多个 QMenu，每个 QMenu 包含多个 QAction。

```cpp
void MainWindow::setupMenuBar()
{
    // menuBar() 第一次调用时会自动创建菜单栏
    QMenuBar *bar = menuBar();

    // ---- 文件菜单 ----
    QMenu *fileMenu = bar->addMenu("文件(&F)");

    QAction *newAction = fileMenu->addAction("新建(&N)");
    newAction->setShortcut(QKeySequence::New);  // Ctrl+N
    newAction->setStatusTip("创建一个新文件");
    connect(newAction, &QAction::triggered,
            this, &MainWindow::onNewFile);

    QAction *openAction = fileMenu->addAction("打开(&O)");
    openAction->setShortcut(QKeySequence::Open);  // Ctrl+O
    openAction->setStatusTip("打开已有文件");
    connect(openAction, &QAction::triggered,
            this, &MainWindow::onOpenFile);

    fileMenu->addSeparator();  // 分隔线

    QAction *exitAction = fileMenu->addAction("退出(&Q)");
    exitAction->setShortcut(QKeySequence::Quit);
    exitAction->setStatusTip("退出应用程序");
    connect(exitAction, &QAction::triggered,
            this, &QWidget::close);

    // ---- 编辑菜单 ----
    QMenu *editMenu = bar->addMenu("编辑(&E)");

    QAction *copyAction = editMenu->addAction("复制(&C)");
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered,
            this, &MainWindow::onCopy);

    QAction *pasteAction = editMenu->addAction("粘贴(&V)");
    pasteAction->setShortcut(QKeySequence::Paste);
    connect(pasteAction, &QAction::triggered,
            this, &MainWindow::onPaste);

    // ---- 帮助菜单 ----
    QMenu *helpMenu = bar->addMenu("帮助(&H)");
    QAction *aboutAction = helpMenu->addAction("关于(&A)");
    connect(aboutAction, &QAction::triggered,
            this, &MainWindow::onAbout);
}
```

你会发现 QAction 是整个菜单系统的核心——它不仅表示一个菜单项，还承载了快捷键、状态栏提示文字、图标、是否可选中等元数据。更重要的是，同一个 QAction 可以同时出现在菜单和工具栏中，这意味着"新建"这个操作只需要一个 QAction 实例，菜单和工具栏共享它，用户无论从哪里点击触发的都是同一个信号。

`addSeparator()` 在菜单中插入一条水平分隔线，用来把功能相近的菜单项分组。比如"新建"和"打开"属于文件操作类，"退出"属于应用级操作，中间用分隔线隔开，视觉上更清晰。

菜单文字中的 `(&F)` 是 Qt 的快捷键标记。用户按 Alt+F 就可以打开"文件"菜单——这是桌面应用的标准交互模式。括号里的字母会在菜单文字下面显示一条下划线（取决于系统设置）。

`setStatusTip()` 设置的状态提示文字会在鼠标悬停到菜单项上时显示在状态栏中。这是一个很贴心的用户体验细节——用户不需要点击，只需要把鼠标移到菜单项上就能看到这个操作的简要说明。但 `setStatusTip()` 只在你设置了 `QAction::StatusTip` role 并且状态栏处于活跃状态时才会生效，`statusBar()->showMessage()` 显示的是"永久消息"，会覆盖状态栏的提示。

### 3.3 QToolBar 工具栏添加按钮与分隔线

工具栏是菜单栏的快捷方式——用户不需要点菜单找操作，直接在工具栏上点一下就行。Qt 的工具栏功能非常丰富：支持添加按钮、分隔线、任意 QWidget、支持拖拽重定位、支持浮动窗口、支持图标大小调整。

```cpp
void MainWindow::setupToolBar()
{
    // 创建工具栏
    QToolBar *toolBar = addToolBar("主工具栏");
    toolBar->setMovable(true);       // 允许拖拽移动
    toolBar->setFloatable(true);     // 允许浮动为独立窗口
    toolBar->setIconSize(QSize(24, 24));

    // 复用菜单中的 QAction——同一个操作，菜单和工具栏共享
    QAction *newAction = menuBar()->findChild<QAction*>();
    // 更实际的做法是在类中持有 QAction 成员变量
    // 这里为了演示简洁，重新创建

    auto *newBtn = toolBar->addAction("新建");
    newBtn->setShortcut(QKeySequence::New);
    newBtn->setStatusTip("新建文件");
    connect(newBtn, &QAction::triggered,
            this, &MainWindow::onNewFile);

    auto *openBtn = toolBar->addAction("打开");
    openBtn->setStatusTip("打开文件");
    connect(openBtn, &QAction::triggered,
            this, &MainWindow::onOpenFile);

    toolBar->addSeparator();  // 工具栏分隔线

    auto *copyBtn = toolBar->addAction("复制");
    connect(copyBtn, &QAction::triggered,
            this, &MainWindow::onCopy);

    auto *pasteBtn = toolBar->addAction("粘贴");
    connect(pasteBtn, &QAction::triggered,
            this, &MainWindow::onPaste);

    toolBar->addSeparator();

    // 工具栏中也可以放任意 QWidget，比如一个搜索框
    auto *searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText("搜索...");
    searchEdit->setMaximumWidth(200);
    toolBar->addWidget(searchEdit);
}
```

这里有一个非常重要的实践建议：在实际项目中，你应该把 QAction 定义为类的成员变量，然后在菜单和工具栏中共用同一个 QAction 实例。上面的代码为了简化演示在工具栏中重新创建了按钮，但正确做法是在 `setupMenuBar()` 中创建 QAction 并保存为成员变量，然后在 `setupToolBar()` 中用 `toolBar->addAction(m_newAction)` 把同一个 QAction 添加到工具栏。这样当你需要禁用"新建"操作时，只需要 `m_newAction->setEnabled(false)`，菜单和工具栏上的按钮会同时变灰。

`addToolBar()` 的参数是工具栏的名称（显示在右键菜单中，用户可以通过右键菜单隐藏或显示工具栏）。如果你添加了多个工具栏，用户可以拖拽调整它们的顺序和位置。`setMovable(true)` 允许用户拖拽工具栏到窗口的其他边缘，`setFloatable(true)` 允许用户把工具栏拖出来变成一个浮动的小窗口。

`addWidget()` 方法可以把任意的 QWidget 嵌入工具栏。这在需要放一个搜索框、下拉框、或者字体大小选择器的时候非常有用。注意 `addWidget()` 添加的控件会跟随工具栏移动和浮动，不需要你额外处理布局。

### 3.4 QDockWidget 可停靠面板基础

如果你用过 Qt Creator、Visual Studio 这类 IDE，一定对侧边栏可以自由拖拽、停靠、浮动的行为不陌生。这种可停靠面板在 Qt 中由 QDockWidget 实现。每个 QDockWidget 有一个标题栏和一个内容区域，用户可以拖拽标题栏把面板移到窗口的其他边缘、浮动出来、或者关闭面板。

```cpp
void MainWindow::setupDockWidgets()
{
    // ---- 文件浏览器停靠面板（左侧）----
    auto *fileDock = new QDockWidget("文件浏览器", this);
    fileDock->setAllowedAreas(Qt::LeftDockWidgetArea |
                               Qt::RightDockWidgetArea);

    auto *fileTree = new QTreeWidget;
    fileTree->setHeaderLabel("文件");
    // 添加一些示例文件项
    auto *rootItem = new QTreeWidgetItem(fileTree, {"项目根目录"});
    new QTreeWidgetItem(rootItem, {"main.cpp"});
    new QTreeWidgetItem(rootItem, {"CMakeLists.txt"});
    new QTreeWidgetItem(rootItem, {"README.md"});
    fileTree->expandAll();

    fileDock->setWidget(fileTree);
    addDockWidget(Qt::LeftDockWidgetArea, fileDock);

    // ---- 属性面板停靠面板（右侧）----
    auto *propDock = new QDockWidget("属性面板", this);
    propDock->setAllowedAreas(Qt::LeftDockWidgetArea |
                               Qt::RightDockWidgetArea);

    auto *propTable = new QTableWidget;
    propTable->setColumnCount(2);
    propTable->setHorizontalHeaderLabels({"属性", "值"});
    propTable->horizontalHeader()->setStretchLastSection(true);
    propTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    int row = 0;
    propTable->insertRow(row);
    propTable->setItem(row, 0, new QTableWidgetItem("文件名"));
    propTable->setItem(row, 1, new QTableWidgetItem("main.cpp"));
    propTable->insertRow(++row);
    propTable->setItem(row, 0, new QTableWidgetItem("大小"));
    propTable->setItem(row, 1, new QTableWidgetItem("2.4 KB"));

    propDock->setWidget(propTable);
    addDockWidget(Qt::RightDockWidgetArea, propDock);

    // 把文件浏览器和属性面板加入"视图"菜单，让用户可以控制显示/隐藏
    QMenu *viewMenu = menuBar()->addMenu("视图(&V)");
    viewMenu->addAction(fileDock->toggleViewAction());
    viewMenu->addAction(propDock->toggleViewAction());
}
```

`setAllowedAreas()` 限制了这个停靠面板允许被拖拽到哪些区域。比如文件浏览器只允许在左右两侧，不允许拖到顶部或底部。如果你不设置这个限制，默认是允许所有区域。`addDockWidget()` 的第二个参数指定面板的初始停靠区域。

`toggleViewAction()` 是 QDockWidget 提供的一个现成的 QAction，用于控制面板的显示和隐藏。你只需要把它添加到"视图"菜单中，用户就可以通过菜单来控制各个面板的可见性。这个 QAction 的文本就是 QDockWidget 的标题，选中状态对应面板是否可见——非常方便。

多个 QDockWidget 可以停靠在同一个区域，它们会自动以标签页的形式堆叠。用户可以通过拖拽标题栏来重新排列。你也可以通过 `tabifyDockWidget()` 方法把两个停靠面板手动叠加成标签页。

关于停靠面板还有一个有用的属性——`QDockWidget::setFeatures()`。你可以控制面板是否可关闭、是否可移动、是否可浮动。比如一个"导航面板"你希望用户可以移动但不允许关闭，就设置 `DockWidgetMovable | DockWidgetFloatable`，去掉 `DockWidgetClosable`。

## 4. 踩坑预防

第一个坑是给 QMainWindow 设置布局。QMainWindow 有自己内部的布局管理器来管理菜单栏、工具栏、停靠面板和中央控件的位置。如果你调用 `setLayout()` 试图给 QMainWindow 设置自定义布局，Qt 会在运行时输出一条警告信息，并且你的布局会和内部布局冲突，导致窗口显示异常。中央控件的内容布局应该在 centralWidget 内部设置，而不是在 QMainWindow 上设置。

第二个坑是忘记设置中央控件。如果你创建了一个 QMainWindow 但没有调用 `setCentralWidget()`，窗口中间会是一个巨大的空白区域。即使你只需要菜单栏和工具栏而不需要中央内容区，也应该设置一个占位的 QWidget 作为中央控件。

第三个坑是 QAction 的内存管理。通过 `QMenu::addAction()` 创建的 QAction 的父对象是对应的 QMenu，所以当 QMenu 被销毁时 QAction 也会被销毁。但如果你需要在菜单和工具栏之间共享 QAction，应该确保 QAction 的生命周期不会被菜单的销毁提前终止。最安全的做法是把 QAction 作为 QMainWindow 的子对象创建（`new QAction(this)`），这样它的生命周期和主窗口一致。

第四个坑是状态栏的 `showMessage()` 和 `addAction()` 的冲突。如果你用 `statusBar()->showMessage("就绪")` 设置了永久消息，那么 `QAction::setStatusTip()` 的提示文字就不会显示——因为永久消息会一直占据状态栏。正确做法是用 `statusBar()->showMessage("就绪", 2000)` 的第二个参数设置消息显示时长（毫秒），超时后消息自动消失，状态栏恢复到可以显示 StatusTip 的状态。

## 5. 练习项目

我们来做一个综合练习：用 QMainWindow 实现一个简易文本编辑器。要求包含以下功能：菜单栏包含"文件"（新建、打开、保存、退出）和"编辑"（复制、粘贴、全选）两个菜单；工具栏包含新建、打开、保存三个快捷按钮和一个字体大小选择框 QComboBox；状态栏左侧显示当前光标位置（行号:列号），右侧永久显示字符总数；左侧停靠一个"文件信息"面板，显示当前文件的路径、字符数、行数、最后修改时间；中央控件是一个 QTextEdit。

几个提示：光标位置通过 `QTextEdit::textCursor()` 获取，`cursor.blockNumber() + 1` 是行号，`cursor.positionInBlock() + 1` 是列号；连接 QTextEdit 的 `cursorPositionChanged` 信号来实时更新状态栏；文件信息面板用一个 QFormLayout 或者 QLabel 展示数据即可；QComboBox 的 `currentTextChanged` 信号连接到设置字体大小的逻辑，通过 `QFont::setPointSize()` 修改 QTextEdit 的字体。

## 6. 官方文档参考链接

[Qt 文档 · QMainWindow](https://doc.qt.io/qt-6/qmainwindow.html) -- 主窗口类，包含五大区域管理 API

[Qt 文档 · QMenuBar](https://doc.qt.io/qt-6/qmenubar.html) -- 菜单栏，包含菜单的添加和管理

[Qt 文档 · QMenu](https://doc.qt.io/qt-6/qmenu.html) -- 下拉菜单，包含菜单项和分隔线的添加

[Qt 文档 · QAction](https://doc.qt.io/qt-6/qaction.html) -- 操作项，可同时用于菜单、工具栏和快捷键

[Qt 文档 · QToolBar](https://doc.qt.io/qt-6/qtoolbar.html) -- 工具栏，支持按钮、分隔线和自定义 Widget

[Qt 文档 · QDockWidget](https://doc.qt.io/qt-6/qdockwidget.html) -- 可停靠面板，支持拖拽重定位和浮动

[Qt 文档 · QStatusBar](https://doc.qt.io/qt-6/qstatusbar.html) -- 状态栏，支持临时消息和永久组件

---

到这里，QMainWindow 主窗口体系的基础你就掌握了。菜单栏用 QMenu + QAction 组织操作、工具栏用同样的 QAction 提供快捷入口、状态栏显示上下文信息、停靠面板提供可拖拽的侧边区域——这四块区域拼在一起，就构成了一个完整的桌面应用框架。下一篇我们进入图形视图框架，那是一个完全不同的世界：场景、视图、图元三层架构，坐标系的转换，以及鼠标事件的分发机制。
