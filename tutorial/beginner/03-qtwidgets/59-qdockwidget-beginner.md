# 现代Qt开发教程（新手篇）3.59——QDockWidget：可停靠浮动面板

## 1. 前言 / Dock 面板是专业级应用的标配

如果你用过 Qt Creator、Visual Studio、Eclipse 这类 IDE，或者 GIMP、Blender 这类专业工具，一定会注意到它们都有一个共同特征：界面中存在大量可以拖拽、停靠、浮动、关闭的面板——项目浏览器、代码大纲、属性面板、输出窗口、调试控制台、颜色选择器……这些面板就是 QDockWidget 的典型应用场景。没有 Dock 面板的桌面应用只能把所有功能塞进一个固定的布局中，用户无法根据自己的工作流自定义界面——这在简单应用中没问题，但在复杂的专业工具中是不可接受的。

QDockWidget 是 QMainWindow 体系中负责"可停靠浮动面板"的类。它可以作为一个子面板停靠在 QMainWindow 的四个边缘（左、右、上、下），也可以被拖出窗口变成一个独立的浮动窗口。用户可以自由地调整 Dock 的位置、大小、显示/隐藏状态——这种"让用户自定义工作区"的能力是专业级桌面应用的标配。Qt Creator 本身就是 QMainWindow + 大量 QDockWidget 构建的，你拖动面板时的所有交互体验都来自 QDockWidget 内置的行为。

今天我们从四个方面展开。先看 setAllowedAreas 如何限制 Dock 面板的停靠位置，然后研究 setFeatures 控制的关闭/浮动/移动行为，接着讨论 topLevelChanged / visibilityChanged 信号的使用场景，最后实现多 Dock 的 tabify 标签化合并。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QDockWidget 在 QtWidgets 模块中，链接 Qt6::Widgets 即可。示例代码涉及 QDockWidget、QMainWindow、QPlainTextEdit、QTextEdit、QTreeWidget、QListWidget、QLabel、QTimer 和 QAction。

## 3. 核心概念讲解

### 3.1 setAllowedAreas：限制停靠位置

QDockWidget 默认可以停靠在 QMainWindow 的四个边缘：左侧（Qt::LeftDockWidgetArea）、右侧（Qt::RightDockWidgetArea）、顶部（Qt::TopDockWidgetArea）、底部（Qt::BottomDockWidgetArea）。默认值是 Qt::AllDockWidgetAreas，即四个边缘都可以停靠。

通过 setAllowedAreas(Qt::DockWidgetAreas areas) 可以限制 Dock 面板只能停靠在指定的区域。这个限制作用于用户的拖拽行为——当用户尝试把 Dock 拖到不允许的区域时，QMainWindow 不会显示停靠预览（那个半透明的蓝色矩形），Dock 也不会停靠。

```cpp
// 只允许停靠在左侧或右侧（典型的侧边栏面板）
auto *outlineDock = new QDockWidget("大纲", this);
outlineDock->setAllowedAreas(Qt::LeftDockWidgetArea |
                              Qt::RightDockWidgetArea);
addDockWidget(Qt::LeftDockWidgetArea, outlineDock);

// 只允许停靠在底部（典型的输出/日志面板）
auto *outputDock = new QDockWidget("输出", this);
outputDock->setAllowedAreas(Qt::BottomDockWidgetArea);
addDockWidget(Qt::BottomDockWidgetArea, outputDock);
```

限制停靠位置的逻辑很好理解——不同功能的面板有不同的最佳位置。文件浏览器、代码大纲这类"辅助浏览"面板适合放在左右两侧（垂直空间大，适合列表/树形展示）；编译输出、调试日志这类"横向阅读"的面板适合放在底部（水平空间大，适合长文本行）；工具面板（颜色选择、图层管理）则可能允许停靠在任意位置。

setAllowedAreas 的限制不仅影响用户拖拽，也影响程序化的停靠操作。如果你在代码中调用 addDockWidget(Qt::TopDockWidgetArea, dock) 但 dock 的 allowedAreas 不包含 Qt::TopDockWidgetArea，Dock 仍然会被添加到指定位置（Qt 不会阻止你），但用户无法通过拖拽把它移回那个位置。所以在 addDockWidget 时确保指定的区域在 allowedAreas 范围内。

还有一个相关的设置是 QMainWindow::setCorner。QMainWindow 的四个角落是 Dock 区域的交叉地带——左上角是顶部 Dock 和左侧 Dock 的重叠区域。默认情况下，左上角和左下角属于左侧 Dock 区域，右上角和右下角属于右侧 Dock 区域。你可以通过 setCorner 来改变角落的归属。

```cpp
// 把左上角分配给顶部 Dock 区域，左下角分配给底部 Dock 区域
setCorner(Qt::TopLeftCorner, Qt::TopDockWidgetArea);
setCorner(Qt::BottomLeftCorner, Qt::BottomDockWidgetArea);
setCorner(Qt::TopRightCorner, Qt::TopDockWidgetArea);
setCorner(Qt::BottomRightCorner, Qt::BottomDockWidgetArea);
```

这种配置在顶部和底部 Dock 面板比较多、需要横向空间的场景中有用——它让顶部/底部 Dock 可以延伸到窗口的最左端和最右端，不被左右两侧的 Dock 截断。

### 3.2 setFeatures：控制关闭/浮动/移动行为

QDockWidget 提供了一组特性标志，通过 setFeatures(QDockWidget::DockWidgetFeatures features) 来控制用户可以对 Dock 面板执行哪些操作。这些特性包括三个基本标志和一个组合标志。

DockWidgetClosable 控制 Dock 面板是否可以被关闭。当这个标志启用时，Dock 的标题栏上会显示一个关闭按钮（X），用户点击后 Dock 被隐藏（不是被删除——它仍然存在于 QMainWindow 的 Dock 列表中，只是不可见了）。如果禁用这个标志，关闭按钮消失，用户无法通过 UI 操作隐藏这个 Dock。这在"核心面板不允许被关闭"的场景中有用——比如一个应用的中央辅助面板。

```cpp
// 不允许用户关闭这个面板
outlineDock->setFeatures(QDockWidget::DockWidgetMovable |
                          QDockWidget::DockWidgetFloatable);
// DockWidgetClosable 被排除了，所以没有关闭按钮
```

DockWidgetMovable 控制 Dock 面板是否可以在 QMainWindow 的各个 Dock 区域之间移动。禁用后，用户无法把 Dock 拖到其他位置，但可以关闭和浮动它。

DockWidgetFloatable 控制 Dock 面板是否可以拖出窗口变成浮动窗口。禁用后，用户只能在 QMainWindow 的 Dock 区域内移动 Dock，不能把它拖出来。

```cpp
// 只允许在窗口内移动，不能浮动、不能关闭
outlineDock->setFeatures(QDockWidget::DockWidgetMovable);
```

DockWidgetFloatable 有一个容易混淆的地方：禁用浮动后，Dock 仍然可以被关闭（如果 DockWidgetClosable 启用）。关闭和浮动是两个独立的操作——关闭是隐藏面板，浮动是把面板拖出窗口。如果你想完全锁定一个面板（不能移动、不能关闭、不能浮动），需要 setFeatures(QDockWidget::NoDockWidgetFeatures)。

```cpp
// 完全锁定的面板：用户无法进行任何操作
auto *lockPanel = new QDockWidget("锁定面板", this);
lockPanel->setFeatures(QDockWidget::NoDockWidgetFeatures);
addDockWidget(Qt::RightDockWidgetArea, lockPanel);
```

Qt 6.4 之后还引入了 DockWidgetDrawerMode 特性。当启用这个模式时，Dock 面板会以一种"抽屉"的方式显示——面板覆盖在中央区域上方而不是推开中央区域。这在需要临时查看信息又不想改变布局的场景中有用。不过这个特性在 Qt 6.9 中仍然相对较新，使用时需要确认目标平台的渲染效果。

一个经常被问到的问题是：setFeatures 和 setAllowedAreas 的关系是什么？setAllowedAreas 限制 Dock 可以停靠在哪些位置，setFeatures 限制用户可以执行哪些操作（移动、浮动、关闭）。它们是互补的：setAllowedAreas 说"去哪里"，setFeatures 说"做什么"。如果你 setFeatures 排除了 DockWidgetMovable，那 setAllowedAreas 的限制就没有意义了——因为用户根本无法移动 Dock。

### 3.3 topLevelChanged / visibilityChanged：信号响应

QDockWidget 提供了几个有用的信号来监控面板的状态变化。

topLevelChanged(bool topLevel) 在 Dock 面板的浮动状态发生变化时发射。当 Dock 被拖出窗口变成浮动窗口时，topLevel 为 true；当 Dock 被重新停靠到 QMainWindow 中时，topLevel 为 false。这个信号在需要根据面板的浮动/停靠状态调整 UI 时有用——比如浮动窗口中可能需要显示额外的工具栏或标题栏信息。

```cpp
connect(outlineDock, &QDockWidget::topLevelChanged,
        this, [this](bool topLevel) {
    if (topLevel) {
        statusBar()->showMessage("大纲面板已浮动", 2000);
        // 浮动时可以给面板添加更多控件
    } else {
        statusBar()->showMessage("大纲面板已停靠", 2000);
    }
});
```

visibilityChanged(bool visible) 在 Dock 面板的可见性发生变化时发射。当用户关闭 Dock（点击关闭按钮）或通过代码调用 hide() 时，visible 为 false；当 Dock 重新显示时，visible 为 true。这个信号和 QWidget::setVisible / QWidget::hide 的关系是：visibilityChanged 在 Dock 的实际可见性变化后发射，而不仅仅是调用了 show()/hide() 方法。如果一个 Dock 被隐藏但因为父窗口也被隐藏了而没有实际变化，visibilityChanged 可能不会发射。

```cpp
connect(outlineDock, &QDockWidget::visibilityChanged,
        this, [this](bool visible) {
    // 更新菜单中的勾选状态
    m_showOutlineAction->setChecked(visible);
});
```

实际上 QDockWidget 已经内置了一个 toggleViewAction，这个 QAction 自动和 Dock 的可见性同步——勾选时显示 Dock，取消勾选时隐藏 Dock。所以上面的代码在大多数情况下不需要手写——直接把 toggleViewAction 添加到"视图"菜单就行了。但如果你需要在 Dock 可见性变化时执行额外的逻辑（比如记录日志、更新其他 UI 状态），visibilityChanged 信号是正确的接入点。

```cpp
// 把 Dock 的显示/隐藏控制权交给菜单
auto *viewMenu = menuBar()->addMenu("视图(&V)");
viewMenu->addAction(outlineDock->toggleViewAction());
viewMenu->addAction(outputDock->toggleViewAction());
viewMenu->addAction(propertiesDock->toggleViewAction());
```

dockLocationChanged(Qt::DockWidgetArea area) 在 Dock 的停靠位置发生变化时发射。area 是新的停靠位置。如果 Dock 变成浮动的，area 为 Qt::NoDockWidgetArea。这个信号在需要记录 Dock 布局或者根据 Dock 位置调整内容布局时有用。

```cpp
connect(outlineDock, &QDockWidget::dockLocationChanged,
        this, [this](Qt::DockWidgetArea area) {
    QString position;
    switch (area) {
        case Qt::LeftDockWidgetArea:   position = "左侧"; break;
        case Qt::RightDockWidgetArea:  position = "右侧"; break;
        case Qt::TopDockWidgetArea:    position = "顶部"; break;
        case Qt::BottomDockWidgetArea: position = "底部"; break;
        default: position = "浮动"; break;
    }
    statusBar()->showMessage(
        "大纲面板已移动到" + position, 2000);
});
```

featuresChanged(QDockWidget::DockWidgetFeatures features) 在 Dock 的特性标志发生变化时发射。这个信号比较少用，因为你通常知道自己什么时候调用了 setFeatures。

### 3.4 多 Dock tabify 标签化合并

当你有多个 Dock 面板需要放在同一侧时，如果让它们全部平铺排列，每个面板只能分到很小的空间。QMainWindow 提供了 tabifyDockWidget 方法，可以把多个停靠在同一侧的 Dock 合并成标签页形式——就像浏览器的多标签页一样，一次只显示一个 Dock 的内容，通过点击标签切换。

```cpp
auto *outlineDock = new QDockWidget("大纲", this);
outlineDock->setWidget(new QTreeWidget);
addDockWidget(Qt::LeftDockWidgetArea, outlineDock);

auto *symbolsDock = new QDockWidget("符号", this);
symbolsDock->setWidget(new QListWidget);
addDockWidget(Qt::LeftDockWidgetArea, symbolsDock);

auto *bookmarksDock = new QDockWidget("书签", this);
bookmarksDock->setWidget(new QListWidget);
addDockWidget(Qt::LeftDockWidgetArea, bookmarksDock);

// 把 symbolsDock 和 bookmarksDock 标签化到 outlineDock
tabifyDockWidget(outlineDock, symbolsDock);
tabifyDockWidget(outlineDock, bookmarksDock);

// 默认显示第一个标签页
outlineDock->raise();
```

tabifyDockWidget(QDockWidget *first, QDockWidget *second) 把 second 作为一个新标签页添加到 first 所在的标签组中。如果 first 当前不是标签页形式（它独占一个位置），Qt 会自动把 first 转换成标签页模式，然后把 second 作为第二个标签页添加进去。tabifyDockWidget 可以被多次调用来添加更多标签页。

raise() 让指定的 Dock 成为当前活动标签页。这在初始化时确定默认显示哪个标签页很有用——如果你不调用 raise()，最后一个被 tabifyDockWidget 的 Dock 会自动成为活动标签页。

用户也可以通过拖拽来手动创建标签页——把一个 Dock 拖到另一个已停靠的 Dock 的标题栏上，Qt 会自动把它们合并成标签页。这个交互在 Qt 的默认样式中会显示一个标签页形状的预览指示。

标签页的文本默认使用 Dock 的 windowTitle。你可以通过 setWindowTitle 来设置标签页的显示文本。如果你想给标签页添加图标，目前 Qt 没有提供直接的 API——需要通过 QTabBar（内部使用的标签栏控件）来访问和自定义。

setDockNestingEnabled(bool enable) 控制是否允许 Dock 的嵌套停靠。默认是启用的，意味着用户可以把一个 Dock 拖到另一个 Dock 的内部区域，形成嵌套布局（左右上下均可）。如果你只希望使用简单的标签化和平铺布局，可以禁用嵌套。

```cpp
// 禁止嵌套，只允许平铺和标签化
setDockNestingEnabled(false);
```

在一个真实的应用中，多 Dock 布局的初始化通常是这样的：先 addDockWidget 把每个 Dock 添加到指定位置，然后用 tabifyDockWidget 把同一位置的多个 Dock 合并成标签页，最后调用 raise() 确定默认的活动标签页。用户拖拽调整后的布局通过 saveState / restoreState 来持久化。

```cpp
// 初始化 Dock 布局
void MainWindow::setupDocks()
{
    // 左侧：浏览面板组
    m_outlineDock = createDock("大纲", new QTreeWidget,
                                Qt::LeftDockWidgetArea);
    m_symbolsDock = createDock("符号", new QListWidget,
                                Qt::LeftDockWidgetArea);
    m_bookmarksDock = createDock("书签", new QListWidget,
                                  Qt::LeftDockWidgetArea);

    tabifyDockWidget(m_outlineDock, m_symbolsDock);
    tabifyDockWidget(m_outlineDock, m_bookmarksDock);
    m_outlineDock->raise();

    // 底部：输出面板组
    m_outputDock = createDock("输出", new QTextEdit,
                               Qt::BottomDockWidgetArea);
    m_problemsDock = createDock("问题", new QListWidget,
                                 Qt::BottomDockWidgetArea);
    m_consoleDock = createDock("控制台", new QTextEdit,
                                Qt::BottomDockWidgetArea);

    tabifyDockWidget(m_outputDock, m_problemsDock);
    tabifyDockWidget(m_outputDock, m_consoleDock);
    m_outputDock->raise();
}

QDockWidget* MainWindow::createDock(const QString &title,
                                      QWidget *widget,
                                      Qt::DockWidgetArea area)
{
    auto *dock = new QDockWidget(title, this);
    dock->setWidget(widget);
    addDockWidget(area, dock);
    return dock;
}
```

这种"分组 + 标签化"的布局模式是专业 IDE 和编辑器的标准做法——左侧放浏览类面板（大纲、符号、书签），底部放输出类面板（编译输出、问题列表、终端），右侧放属性类面板（属性编辑、颜色设置）。每组内部通过标签页切换，避免界面过于拥挤。

## 4. 踩坑预防

第一个坑是 Dock 面板的 widget 生命周期。QDockWidget 通过 setWidget(QWidget *widget) 设置面板内容。这个 widget 的 parent 会被设置为 QDockWidget，当 QDockWidget 被析构时 widget 也会被自动 delete。如果你需要在 Dock 关闭后继续使用这个 widget，需要在 Dock 析构前调用 setWidget(nullptr) 把它取出来，或者使用 QDockWidget::takeWidget() 方法。

第二个坑是 restoreState 时 Dock 的匹配机制。QMainWindow::restoreState 通过 objectName 来匹配 Dock——它遍历保存的状态数据，找到 objectName 匹配的 QDockWidget，然后恢复其位置、大小、可见性。如果你没有给 Dock 设置 objectName，或者 objectName 发生了变化，restoreState 就找不到对应的 Dock，所有布局状态都无法恢复。务必在 addDockWidget 之后立刻 setObjectName。

```cpp
auto *outlineDock = new QDockWidget("大纲", this);
outlineDock->setObjectName("outlineDock");  // 必须！
addDockWidget(Qt::LeftDockWidgetArea, outlineDock);
```

第三个坑是浮动 Dock 窗口在应用关闭时的行为。当 QMainWindow 被关闭时，浮动的 Dock 窗口也会被自动关闭和析构——它们是 QMainWindow 的子窗口。但如果你的 Dock 中有长时间运行的操作（比如一个正在运行的进程），关闭 Dock 会中断操作。如果你需要在 Dock 关闭时执行清理逻辑，可以重写 QDockWidget 的 closeEvent 或者监听 aboutToHide 信号。

第四个坑是 tabifyDockWidget 的调用顺序。tabifyDockWidget 要求两个 Dock 已经通过 addDockWidget 添加到 QMainWindow 中。如果你在 addDockWidget 之前调用 tabifyDockWidget，它不会做任何事情。同时，两个 Dock 必须停靠在同一侧——如果 outlineDock 在左侧而 outputDock 在底部，tabifyDockWidget(outlineDock, outputDock) 会先把 outputDock 移动到左侧，然后再合并成标签页。这个隐式的位置移动可能不是你期望的。

第五个坑是 Dock 面板的最小尺寸。如果 Dock 中的 widget 设置了很大的 minimumSize，用户可能无法把 Dock 拖拽缩小到那个尺寸以下。在 Dock 面板中使用的控件应该设置合理的 minimumSize 或者使用 sizePolicy 来控制缩放行为——不要让 minimumSize 过大，否则会挤压中央区域的可用空间。

## 5. 练习项目

我们来做一个综合练习：创建一个 QMainWindow 应用，模拟一个简易 IDE 的布局。中央区域使用 QPlainTextEdit 作为代码编辑器。左侧创建三个 Dock 面板："文件浏览器"（QTreeWidget，模拟文件树）、"符号列表"（QListWidget）、"书签"（QListWidget），通过 tabifyDockWidget 合并成标签页组，默认显示"文件浏览器"。底部创建两个 Dock 面板："编译输出"（QTextEdit）和"问题列表"（QListWidget），同样标签化合并。"文件浏览器"只允许停靠在左右两侧，"编译输出"只允许停靠在底部。所有 Dock 都不允许程序化关闭（移除关闭按钮中的 X），但可以通过"视图"菜单的 toggleViewAction 控制显示/隐藏。监听 topLevelChanged 信号，在 Dock 浮动时通过状态栏显示提示信息。监听 dockLocationChanged 信号，在状态栏显示 Dock 的新位置。

提示：tabifyDockWidget 之后记得调用 raise() 设置默认活动标签页。给每个 Dock 设置唯一的 objectName 以便未来集成 saveState/restoreState。

## 6. 官方文档参考链接

[Qt 文档 -- QDockWidget](https://doc.qt.io/qt-6/qdockwidget.html) -- 可停靠面板类

[Qt 文档 -- QMainWindow](https://doc.qt.io/qt-6/qmainwindow.html) -- 主窗口类（Dock 管理）

[Qt 文档 -- Main Window Examples](https://doc.qt.io/qt-6/examples-mainwindow.html) -- 主窗口示例集

[Qt 文档 -- Dock Widgets Example](https://doc.qt.io/qt-6/qtwidgets-mainwindows-dockwidgets-example.html) -- Dock 面板示例

---

到这里，QDockWidget 的核心用法就全部讲完了。setAllowedAreas 限制停靠位置保证布局的合理性，setFeatures 控制关闭/浮动/移动行为锁定关键面板，topLevelChanged / visibilityChanged / dockLocationChanged 信号让你在面板状态变化时执行自定义逻辑，tabifyDockWidget 把多个 Dock 合并成标签页避免界面拥挤。把这些组合起来，就能搭建出一个灵活、专业、可定制的面板系统——这正是用户对桌面专业工具的基本期望。
