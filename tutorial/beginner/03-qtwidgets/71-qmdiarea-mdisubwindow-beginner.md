# 现代Qt开发教程（新手篇）3.71——QMdiArea / QMdiSubWindow：多文档界面

## 1. 前言 / 当你的应用需要同时打开十几个文档时

如果你用过早期的 Windows 应用——Visual Studio 6.0、Microsoft Office 2003、Photoshop CS2——你一定记得那种多窗口的工作方式：主窗口里面嵌套着一堆子窗口，每个子窗口是一个独立的文档，子窗口可以在主窗口内部拖动、最小化、最大化、层叠排列。这种界面模式叫做 MDI（Multiple Document Interface，多文档界面）。虽然从 Windows XP 时代开始微软就逐渐转向了标签页（Tabbed Interface）模式——VS Code 用标签页管理文件、Chrome 用标签页管理网页、Excel 用标签页管理工作表——但 MDI 模式在特定场景下依然有它的价值：文本编辑器同时打开多个文件、CAD 软件同时查看多个图纸、数据分析工具同时展示多个图表窗口、IDE 同时编辑多个源代码文件。这些场景的共同特点是用户需要同时看到多个文档的内容，并且需要在它们之间快速切换——标签页每次只能看到一个文档，而 MDI 允许多个文档并排显示。

Qt 提供了 QMdiArea 和 QMdiSubWindow 来实现 MDI 界面。QMdiArea 是 MDI 的工作区域——它是一个 QWidget，通常作为 QMainWindow 的中央控件（central widget）。QMdiSubWindow 是子窗口——每个子窗口可以包含任意 QWidget 作为内容。QMdiArea 负责管理所有子窗口的排列、切换、焦点管理，QMdiSubWindow 则提供每个子窗口的窗口装饰（标题栏、最小化/最大化/关闭按钮）和拖动调整大小的行为。

今天我们从四个方面展开。先看 addSubWindow 添加子窗口的基本操作，然后讨论 tileSubWindows / cascadeSubWindows 两种排列方式，接着研究 activeSubWindowChanged 信号追踪活动窗口的机制，最后看看如何让菜单项自动跟随子窗口列表更新。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+， C++17 标准。QMdiArea 和 QMdiSubWindow 都在 QtWidgets 模块中，链接 Qt6::Widgets 即可。示例代码涉及 QMdiArea、QMdiSubWindow、QMainWindow、QTextEdit、QMenuBar、QMenu、QAction、QToolBar、QLabel、QApplication 和 QDebug。

## 3. 核心概念讲解

### 3.1 addSubWindow：添加子窗口

QMdiArea 的核心 API 非常简洁——addSubWindow(QWidget *widget) 方法接受一个 QWidget 指针，为它创建一个 QMdiSubWindow 包装器并添加到 MDI 区域中。返回值就是创建的 QMdiSubWindow 指针，你可以通过它来设置子窗口的标题、图标、大小等属性。

```cpp
// 创建 MDI 区域并设为主窗口的中央控件
auto *mdiArea = new QMdiArea;
setCentralWidget(mdiArea);

// 添加第一个子窗口
auto *editor1 = new QTextEdit;
editor1->setPlainText("文档 1 的内容...");
auto *subWindow1 =
    mdiArea->addSubWindow(editor1);
subWindow1->setWindowTitle("文档 1");
subWindow1->show();

// 添加第二个子窗口
auto *editor2 = new QTextEdit;
editor2->setPlainText("文档 2 的内容...");
auto *subWindow2 =
    mdiArea->addSubWindow(editor2);
subWindow2->setWindowTitle("文档 2");
subWindow2->show();
```

addSubWindow 的参数是你想在子窗口中显示的"内容控件"——可以是任何 QWidget 子类。上面用了 QTextEdit 作为示例，但你可以放任何控件：QLabel 显示图片、QTableView 显示表格数据、自定义的 QWidget 子类显示图表。QMdiSubWindow 会自动为内容控件提供窗口装饰（标题栏、边框、按钮），用户可以拖动标题栏移动子窗口、拖动边框调整大小、点击最小化/最大化/关闭按钮执行对应操作。

addSubWindow 有一个重载版本 addSubWindow(QWidget *widget, Qt::WindowFlags flags)，可以指定子窗口的窗口标志。比如你不希望子窗口被最大化，可以传 Qt::WindowMinMaxButtonsHint & ~Qt::WindowMaximizeButtonHint 来禁用最大化按钮。

添加子窗口后必须调用 show()——addSubWindow 只是创建了子窗口并注册到 MDI 区域中，但不会自动显示。这给了你在显示之前做额外配置的机会（设置大小、位置、初始状态等）。如果你忘记调用 show()，子窗口存在但用户看不到。

子窗口的移除使用 removeSubWindow(QWidget *widget) 方法。注意参数是内容控件（你传给 addSubWindow 的那个 widget），不是 QMdiSubWindow 本身。removeSubWindow 会销毁 QMdiSubWindow 包装器，但不会销毁内容控件——如果你想同时销毁内容控件，需要在 removeSubWindow 之后手动 delete。

subWindowList() 方法返回所有子窗口的列表（QList<QMdiSubWindow*>）。这个方法在实现"窗口"菜单（列出所有打开的文档）时非常有用。你可以遍历这个列表来构建菜单项：

```cpp
void updateWindowMenu()
{
    m_windowMenu->clear();
    const auto subWindows =
        m_mdiArea->subWindowList();
    for (auto *subWindow : subWindows) {
        auto *action =
            m_windowMenu->addAction(
                subWindow->windowTitle());
        connect(action, &QAction::triggered,
                this, [this, subWindow]() {
            m_mdiArea->setActiveSubWindow(
                subWindow);
        });
    }
}
```

QMdiArea 内部使用 QMdiSubWindow 作为实际的子窗口类。当你调用 addSubWindow(widget) 时，QMdiArea 创建一个新的 QMdiSubWindow，把你的 widget 设为 QMdiSubWindow 的内部控件（通过 setWidget）。所以 subWindowList() 返回的是 QMdiSubWindow 列表，而不是你传入的 widget 列表。你可以通过 QMdiSubWindow::widget() 获取到内容控件。

### 3.2 tileSubWindows / cascadeSubWindows：排列子窗口

当用户打开了很多子窗口后，桌面会变得混乱——窗口互相遮挡，找不到想看的那个。QMdiArea 提供了两种快速排列方式来解决这个问题。

cascadeSubWindows() 以级联方式排列所有子窗口——每个子窗口稍微偏移一点位置叠在前一个上面，标题栏都露出来，看起来像一叠纸牌。这种排列方式让用户能快速看到所有窗口的标题，点击任何一个标题栏就能把对应的窗口提到前面。

tileSubWindows() 以平铺方式排列所有子窗口——所有子窗口大小相同，像瓷砖一样铺满整个 MDI 区域，互不遮挡。这种排列方式适合用户需要同时查看多个窗口内容的场景——比如同时看两份文档做对比。

```cpp
// 工具栏按钮或者菜单项的槽函数
void onTile()
{
    m_mdiArea->tileSubWindows();
}

void onCascade()
{
    m_mdiArea->cascadeSubWindows();
}
```

这两种排列方式是"一键整理"——用户只需要点一下就能把混乱的桌面整理整齐。在大多数 MDI 应用中，"窗口"菜单都会提供"层叠"和"平铺"两个选项。

除了这两种自动排列方式，QMdiArea 还支持子窗口的 TabbedView 模式——所有子窗口以标签页的形式展示，类似浏览器的标签页。你可以通过 setViewMode(QMdiArea::TabbedView) 来切换到标签页模式。在 TabbedView 模式下，tileSubWindows 和 cascadeSubWindows 不生效，因为子窗口已经变成了标签页而不是浮动窗口。

```cpp
// 切换到标签页模式
m_mdiArea->setViewMode(QMdiArea::TabbedView);
// 切回子窗口模式
m_mdiArea->setViewMode(QMdiArea::SubWindowView);
```

TabbedView 模式是 Qt 4.5 引入的，它让 QMdiArea 能够同时支持传统的 MDI 子窗口模式和现代的标签页模式，用户可以根据自己的喜好切换。很多现代应用（比如 Qt Creator 本身）在编辑器部分使用标签页模式而不是传统的浮动子窗口。

还有一种排列操作是 closeActiveSubWindow() / closeAllSubWindows()，用于关闭当前活动窗口或关闭所有窗口。closeAllSubWindows 会逐个关闭所有子窗口——如果子窗口的内容有未保存的修改，你需要在 QMdiSubWindow 的 closeEvent 中拦截并弹出确认对话框。QMdiSubWindow 在关闭时会发射 aboutToClose 信号（实际上没有这个信号，你需要重写 closeEvent 或者监控 QMdiArea::subWindowActivated 来感知窗口关闭）。

### 3.3 activeSubWindowChanged：追踪活动窗口

在 MDI 应用中，用户的操作通常针对的是"当前活动的子窗口"——复制/粘贴操作作用于当前编辑器、保存操作保存当前文档、工具栏上的格式按钮应用于当前选中内容。所以你的应用需要知道"当前哪个子窗口是活动的"。

QMdiArea 提供了 subWindowActivated 信号来实现这个功能。每当活动子窗口发生变化时（用户点击了另一个子窗口、关闭了当前子窗口、通过菜单切换了活动窗口），QMdiArea 都会发射 subWindowActivated 信号，参数是新激活的 QMdiSubWindow 指针。如果所有子窗口都被关闭了，参数是 nullptr。

```cpp
connect(m_mdiArea,
        &QMdiArea::subWindowActivated,
        this, &MainWindow::onSubWindowActivated);

void MainWindow::onSubWindowActivated(
    QMdiSubWindow *subWindow)
{
    if (!subWindow) {
        // 所有子窗口都已关闭
        statusBar()->showMessage("没有打开的文档");
        return;
    }

    // 更新状态栏显示当前文档信息
    statusBar()->showMessage(
        QString("活动文档: %1")
            .arg(subWindow->windowTitle()));

    // 更新菜单状态（剪切/复制/粘贴的可用性等）
    updateMenuState(subWindow);
}
```

activeSubWindow() 方法返回当前活动的子窗口指针。如果没有活动窗口，返回 nullptr。你可以在任何需要操作当前文档的地方调用这个方法。

setActiveSubWindow(QMdiSubWindow *subWindow) 方法用于程序化地切换活动窗口——比如用户从"窗口"菜单中选择了一个文档，你需要把那个子窗口设为活动。setActiveSubWindow 会触发 subWindowActivated 信号，所以你的 onSubWindowActivated 槽函数会被调用。

这里有一个需要注意的细节：subWindowActivated 信号在子窗口关闭时也会触发。当用户关闭当前活动窗口时，QMdiArea 会自动激活下一个窗口并发射 subWindowActivated。如果被关闭的是最后一个窗口，参数为 nullptr。所以你的槽函数需要处理 subWindow 为 nullptr 的情况。

另外一个细节是 subWindowActivated 信号的名字用的是 "Activated" 而不是 "Changed"——这暗示了它不是在"窗口切换"时触发，而是在"窗口被激活"时触发。这意味着如果一个窗口已经是活动状态然后又被重新激活（比如用户点击了活动窗口的标题栏），信号不会重复触发。但如果你通过 setActiveSubWindow 强制设置当前活动窗口为自己，信号也不会触发——因为它已经是活动的了。

### 3.4 子窗口菜单项自动更新

一个完善的 MDI 应用通常有一个"窗口"菜单，列出所有打开的子窗口，让用户可以快速切换到任何一个。这个菜单需要动态更新——每次添加/关闭子窗口时都要刷新菜单项。

Qt 提供了一个便捷的机制来实现这个功能：QMdiArea::setupMenuConnections() 不存在，但你可以把 QMdiArea 和 QMenu 通过 QMdiArea 的信号手动关联。更方便的方式是使用 Qt 的 QActionGroup 配合 subWindowList 来构建可选择的窗口菜单：

```cpp
void MainWindow::updateWindowMenu()
{
    // 清空窗口菜单
    m_windowMenu->clear();

    // 添加固定的操作项
    m_windowMenu->addAction(
        "新建文档", this,
        &MainWindow::onNewDocument);
    m_windowMenu->addSeparator();
    m_windowMenu->addAction(
        "层叠排列", m_mdiArea,
        &QMdiArea::cascadeSubWindows);
    m_windowMenu->addAction(
        "平铺排列", m_mdiArea,
        &QMdiArea::tileSubWindows);
    m_windowMenu->addAction(
        "关闭所有", m_mdiArea,
        &QMdiArea::closeAllSubWindows);

    const auto subWindows =
        m_mdiArea->subWindowList();
    if (subWindows.isEmpty()) {
        return;
    }

    m_windowMenu->addSeparator();

    // 为每个子窗口创建菜单项
    for (int i = 0;
         i < subWindows.size(); ++i) {
        auto *subWindow = subWindows.at(i);
        QString text = QString("&%1 %2")
            .arg(i + 1)
            .arg(subWindow->windowTitle());

        auto *action = m_windowMenu->addAction(text);
        action->setCheckable(true);
        action->setChecked(
            subWindow ==
            m_mdiArea->activeSubWindow());

        connect(action, &QAction::triggered,
                this, [this, subWindow]() {
            m_mdiArea->setActiveSubWindow(
                subWindow);
        });
    }
}
```

这个函数在每次需要更新窗口菜单时调用——通常是在 subWindowActivated 信号的槽函数中调用，或者在新建/关闭文档后手动调用。菜单项使用 & 前缀加数字（&1, &2, ...）作为快捷键，用户可以通过 Alt+1、Alt+2 等组合键快速切换到对应的子窗口。setChecked 标记当前活动的窗口，让用户一眼就能看到哪个窗口是活动的。

如果你不想每次都重建菜单，也可以维护一个 QActionGroup，在添加/关闭子窗口时增量更新菜单项。但对于子窗口数量不会太多的场景（通常不会超过几十个），每次重建菜单的性能开销完全可以忽略。

## 4. 踩坑预防

第一个坑是 addSubWindow 后忘记调用 show()。这是一个非常容易犯的错误——addSubWindow 创建了子窗口但不会自动显示，你必须手动调用 subWindow->show()。如果你发现子窗口"添加了但看不到"，检查一下是不是忘了 show()。

第二个坑是 QMdiSubWindow 的内存管理。QMdiArea 在 addSubWindow 时接管了 QMdiSubWindow 的所有权——当 QMdiArea 被销毁时，它会自动销毁所有子窗口及其内容控件。所以不要手动 delete 你传给 addSubWindow 的内容控件——它的生命周期已经由 QMdiArea 管理了。如果你想在运行时手动关闭某个子窗口，调用 subWindow->close() 而不是 delete。close() 会触发关闭流程，QMdiArea 会在适当的时候清理资源。

第三个坑是 subWindowActivated 信号中的空指针处理。当最后一个子窗口被关闭时，subWindowActivated 的参数是 nullptr。如果你的槽函数没有检查空指针就访问 subWindow 的方法（比如 windowTitle()），程序会崩溃。始终在槽函数开头检查 subWindow 是否为 nullptr。

第四个坑是在子窗口的 closeEvent 中处理"未保存修改"的逻辑。如果你希望用户关闭有未保存修改的文档时弹出确认对话框，需要创建 QMdiSubWindow 的子类并重写 closeEvent，或者给内容控件安装事件过滤器拦截 close 事件。直接给内容控件（比如 QTextEdit）设置 close 回调是没用的——因为关闭事件是发送给 QMdiSubWindow 的，不是发送给内容控件的。

第五个坑是 TabbedView 模式下子窗口的行为变化。切换到 TabbedView 模式后，子窗口不再支持拖动、调整大小、层叠/平铺操作。如果你的应用在两种模式之间切换，需要注意菜单项的启用/禁用状态——TabbedView 模式下"层叠排列"和"平铺排列"操作没有意义，应该被禁用。

## 5. 练习项目

我们来做一个综合练习：创建一个 QMainWindow 应用，中央是 QMdiArea。菜单栏有三个菜单——"文件""编辑""窗口"。

"文件"菜单包含"新建文档"（创建一个新的 QMdiSubWindow，内含 QTextEdit，标题为"未命名 N"）、"关闭当前"（关闭活动子窗口）和"退出"。"编辑"菜单包含"复制"和"粘贴"（操作作用于活动子窗口中的 QTextEdit，如果没有活动窗口则禁用这两个菜单项）。

"窗口"菜单包含"层叠排列""平铺排列""关闭所有"三个固定项，然后是分割线和所有子窗口的列表（带数字快捷键和勾选标记）。每次添加或关闭子窗口时，窗口菜单自动更新。

工具栏上有"新建"按钮和"排列方式"下拉按钮（层叠/平铺）。状态栏显示当前活动文档的标题和子窗口总数。subWindowActivated 信号连接到更新函数，更新状态栏文字、刷新窗口菜单、更新编辑菜单的启用状态。

提示：使用 QSignalMapper 或者 lambda 捕获来处理窗口菜单中动态创建的 QAction 的点击事件。在 onSubWindowActivated 槽函数中调用 updateWindowMenu 和 updateEditMenu 来刷新菜单状态。

## 6. 官方文档参考链接

[Qt 文档 -- QMdiArea](https://doc.qt.io/qt-6/qmdiarea.html) -- MDI 区域类

[Qt 文档 -- QMdiSubWindow](https://doc.qt.io/qt-6/qmdisubwindow.html) -- MDI 子窗口类

[Qt 文档 -- QMdiArea::addSubWindow](https://doc.qt.io/qt-6/qmdiarea.html#addSubWindow) -- 添加子窗口方法

[Qt 文档 -- QMdiArea::subWindowActivated](https://doc.qt.io/qt-6/qmdiarea.html#subWindowActivated) -- 子窗口激活信号

[Qt 文档 -- QMdiArea::ViewMode](https://doc.qt.io/qt-6/qmdiarea.html#ViewMode-enum) -- 视图模式枚举

---

到这里，QMdiArea 和 QMdiSubWindow 的核心用法就全部讲完了。addSubWindow 添加内容控件到 MDI 区域，tileSubWindows / cascadeSubWindows 提供一键排列功能，subWindowActivated 信号让应用能追踪活动窗口并同步更新菜单和工具栏状态，动态构建的"窗口"菜单则给用户提供了快速切换文档的入口。虽然标签页模式在现代应用中越来越流行，但 MDI 在需要同时查看多个文档内容的场景下依然是不可替代的布局方案。
