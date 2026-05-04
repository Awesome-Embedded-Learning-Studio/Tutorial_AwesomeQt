# 现代Qt开发教程（新手篇）3.10——QMdiArea 多文档界面基础

## 1. 前言 / 什么时候需要 MDI

如果你用过 GIMP、Excel 早期版本、或者某些老式的 CAD 软件，你应该对"一个主窗口里面套着好几个子窗口"这种界面布局不陌生。这就是 MDI（Multiple Document Interface）——多文档界面。和"每个文档开一个独立窗口"的 SDI（Single Document Interface）不同，MDI 把所有子窗口限定在一个主窗口的范围内，子窗口不会跑到主窗口外面去，最小化的时候也是缩到主窗口底部而不是任务栏上。

说实话，现在新做的桌面应用用 MDI 布局的已经不太多了。大多数场景下，标签页（QTabWidget）或者拆分面板（QSplitter）就能解决问题，而且交互上更现代。但 MDI 依然有它不可替代的场景——比如你需要子窗口可以在主区域内自由拖拽、调整大小、互相重叠、级联或平铺排列，这些是标签页和拆分面板做不到的。如果你的需求正好是"在一个区域内管理多个自由浮动的子窗口"，那 QMdiArea 就是你需要的工具。

这篇文章我们聚焦四个核心议题：QMdiArea 子窗口的创建和基本管理、QMdiSubWindow 的标题图标和关闭行为配置、子窗口的级联和平铺排列模式、以及激活子窗口与信号监听机制。掌握这些之后，你就能够搭建起一个功能完整的 MDI 应用骨架。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QMdiArea 和 QMdiSubWindow 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。QMdiArea 在不同平台上的渲染行为基本一致——子窗口默认使用 Qt 自己的窗口装饰（而不是操作系统的原生窗口边框），所以在 Windows、macOS、Linux 上的外观是统一的。如果你需要原生子窗口外观（不推荐，因为会引入平台相关的问题），可以通过 `QMdiArea::setViewMode` 切换。

## 3. 核心概念讲解

### 3.1 QMdiArea 子窗口创建与管理

QMdiArea 本质上是一个特殊的 QWidget，它的内部区域用来放置多个子窗口。你可以把它想象成一个桌面——QMdiArea 是桌面，QMdiSubWindow 是桌面上的窗口。每个子窗口都是一个独立的 QWidget，被 QMdiSubWindow 包装后添加到 QMdiArea 中。

创建子窗口的方式有两种。第一种是用 `QMdiArea::addSubWindow(QWidget *widget)`，传入一个你自定义的 widget，QMdiArea 会自动为它创建一个 QMdiSubWindow 包装器并添加到区域中。这是最常用的方式——你只需要关注子窗口的"内容"（即那个自定义 widget），窗口边框、标题栏、最小化/最大化/关闭按钮等都由 QMdiSubWindow 自动提供。

```cpp
auto *mdiArea = new QMdiArea(this);
setCentralWidget(mdiArea);

// 创建第一个子窗口
auto *editor1 = new QTextEdit("文档 1 的内容");
auto *subWin1 = mdiArea->addSubWindow(editor1);
subWin1->setWindowTitle("文档 1");
subWin1->show();

// 创建第二个子窗口
auto *editor2 = new QTextEdit("文档 2 的内容");
auto *subWin2 = mdiArea->addSubWindow(editor2);
subWin2->setWindowTitle("文档 2");
subWin2->show();
```

你会发现整个过程非常线性：创建内容 widget、调用 `addSubWindow()` 获取包装器、配置包装器属性、调用 `show()` 显示。`addSubWindow()` 返回的是 `QMdiSubWindow*`，你可以通过这个指针进一步设置子窗口的标题、图标、大小等属性。

第二种方式是直接创建 `QMdiSubWindow` 对象，设置它的内部 widget，然后调用 `QMdiArea::addSubWindow()`。这种方式在你需要对 QMdiSubWindow 做更精细的初始化时使用，效果和第一种方式等价。

```cpp
auto *subWindow = new QMdiSubWindow(mdiArea);
subWindow->setWindowTitle("自定义子窗口");
subWindow->setWidget(new QTextEdit("通过 QMdiSubWindow 直接创建"));
mdiArea->addSubWindow(subWindow);
subWindow->show();
```

不管用哪种方式，子窗口必须调用 `show()` 才会显示。这一点和普通的 QWidget 一样——添加到 QMdiArea 只是建立了归属关系，不会自动让子窗口可见。

QMdiArea 提供了 `subWindowList()` 方法来获取当前所有子窗口的列表，这在实现"窗口"菜单（列出所有打开的文档）或者批量操作子窗口时非常有用。你可以传入 `QMdiArea::WindowOrder` 参数来指定排序方式——`CreationOrder`（默认）按创建顺序排列，`StackingOrder` 按层叠顺序排列（最上面的在列表最前面），`ActivationHistoryOrder` 按最近激活的历史排列。

```cpp
// 获取所有子窗口，按最近激活顺序排列
auto windows = mdiArea->subWindowList(QMdiArea::ActivationHistoryOrder);
for (auto *subWin : windows) {
    qDebug() << "子窗口:" << subWin->windowTitle();
}
```

### 3.2 QMdiSubWindow 标题、图标与关闭行为

QMdiSubWindow 提供了和普通窗口类似的属性配置接口，但它是嵌入在 QMdiArea 内部的，所以行为上有些区别。

设置标题和图标是最基本的配置。标题通过 `setWindowTitle()` 设置，会显示在子窗口的标题栏上。图标通过 `setWindowIcon()` 设置，会显示在标题栏的左侧。

```cpp
auto *subWin = mdiArea->addSubWindow(editor);
subWin->setWindowTitle("README.md");
subWin->setWindowIcon(QIcon::fromTheme("text-x-generic"));
subWin->resize(500, 400);
subWin->show();
```

关闭行为是一个需要特别注意的地方。默认情况下，当用户点击子窗口标题栏上的关闭按钮时，子窗口会被隐藏（`hide()`），但不会被删除。这意味着如果你再次调用 `subWin->show()`，它又会出现——包括之前的内容状态都还在。如果你的应用需要"关闭子窗口就销毁它"的行为，需要连接 `QMdiSubWindow::aboutToClose` 信号或者设置 `Qt::WA_DeleteOnClose` 属性。

```cpp
// 关闭时自动删除子窗口及其内容 widget
subWin->setAttribute(Qt::WA_DeleteOnClose);
```

设置了 `WA_DeleteOnClose` 之后，用户关闭子窗口时它会自动被 `delete`，包括其内部的 widget 也会一起被销毁。这是最常见的配置方式——除非你需要"关闭但保留状态"的功能。但要注意，一旦设置了 `WA_DeleteOnClose`，关闭后你就不能再访问这个子窗口指针了，否则会触发野指针。如果你需要在关闭后做清理工作，应该连接 `QMdiSubWindow::destroyed` 信号或者 QMdiArea 的 `subWindowActivated` 信号来检测。

另一个有用的属性是 `setWindowFlags()`，你可以通过它控制子窗口标题栏上显示哪些按钮。

```cpp
// 子窗口只有关闭按钮，禁用最小化和最大化
subWin->setWindowFlags(Qt::SubWindow | Qt::WindowSystemMenuHint);
```

`Qt::SubWindow` 标志表示这是一个子窗口（不是顶级窗口），`Qt::WindowSystemMenuHint` 表示显示系统菜单（包含关闭选项）。通过组合不同的 WindowFlags，你可以定制子窗口的标题栏外观。

### 3.3 子窗口排列模式：级联与平铺

当子窗口数量多了之后，手动一个个调整位置和大小是非常痛苦的。QMdiArea 内置了两种自动排列模式——级联（Cascade）和平铺（Tile），一行代码就能搞定。

级联排列（Cascade）把所有子窗口按照阶梯状依次排列——每个子窗口相对于前一个向右下方偏移一小段距离，形成类似扑克牌展开的效果。这种排列方式的好处是你能看到每个子窗口的标题栏，方便快速切换到目标窗口。

```cpp
mdiArea->cascadeSubWindows();
```

平铺排列（Tile）把所有子窗口在 QMdiArea 内部等分排列——不重叠，每个子窗口占据相同大小的矩形区域。这种排列方式适合需要同时查看多个窗口内容的场景，比如对比两个文档。

```cpp
mdiArea->tileSubWindows();
```

两种排列方式都是即时生效的，调用之后子窗口会立即移动到新位置。你可以把它们绑定到菜单项或工具栏按钮上，让用户自由选择排列方式。

除了这两种排列方式之外，QMdiArea 还支持切换视图模式。默认模式下子窗口以"子窗口"的形式显示在 MDI 区域内部（类似传统 MDI），你也可以切换到标签页模式：

```cpp
mdiArea->setViewMode(QMdiArea::TabView);
```

切换到 `TabView` 模式后，所有子窗口会以标签页的形式排列在顶部，用户点击标签页切换——外观上和 QTabWidget 类似，但底层仍然是 QMdiArea 的子窗口管理机制。你可以随时切回子窗口模式：

```cpp
mdiArea->setViewMode(QMdiArea::SubWindowView);  // 默认模式
```

这个特性在"同一个应用同时支持传统 MDI 和标签页布局"的场景下非常方便，用户可以根据自己的偏好切换。

### 3.4 激活子窗口与信号监听

MDI 应用的一个核心交互是"当前活动的子窗口"。QMdiArea 在任意时刻最多只有一个活动子窗口（active subwindow），它的标题栏会高亮显示，键盘输入会发送给它。QMdiArea 提供了 `activeSubWindow()` 方法来获取当前活动的子窗口。

```cpp
QMdiSubWindow *active = mdiArea->activeSubWindow();
if (active) {
    qDebug() << "当前活动窗口:" << active->windowTitle();
}
```

当用户点击某个子窗口切换焦点时，或者通过代码调用 `QMdiArea::setActiveSubWindow()` 时，QMdiArea 会发出 `subWindowActivated(QMdiSubWindow *)` 信号。这个信号是你实现"窗口感知"功能的核心——比如在菜单中高亮当前文档名称、在状态栏显示当前文档路径、启用或禁用与当前文档相关的操作按钮等。

```cpp
connect(mdiArea, &QMdiArea::subWindowActivated,
        this, &MainWindow::onSubWindowActivated);

void MainWindow::onSubWindowActivated(QMdiSubWindow *subWin)
{
    if (!subWin) {
        // 所有子窗口都关闭了
        statusBar()->showMessage("没有打开的文档");
        return;
    }

    statusBar()->showMessage("当前文档: " + subWin->windowTitle());

    // 更新菜单中的窗口列表
    updateWindowMenu();
}
```

你也可以通过代码激活某个子窗口：

```cpp
// 激活指定子窗口
mdiArea->setActiveSubWindow(subWin);

// 切换到下一个子窗口（按创建顺序）
mdiArea->activateNextSubWindow();

// 切换到上一个子窗口
mdiArea->activatePreviousSubWindow();
```

`activateNextSubWindow()` 和 `activatePreviousSubWindow()` 适合绑定到快捷键上（比如 Ctrl+Tab 切换到下一个文档），让用户快速在子窗口之间切换。

在实现"窗口"菜单时，你通常需要列出所有打开的子窗口并在当前活动窗口旁边打勾。`subWindowList()` 配合 `activeSubWindow()` 就能做到：

```cpp
void MainWindow::updateWindowMenu()
{
    m_windowMenu->clear();
    auto windows = m_mdiArea->subWindowList();
    QMdiSubWindow *active = m_mdiArea->activeSubWindow();

    for (auto *subWin : windows) {
        QString title = subWin->windowTitle();
        auto *action = m_windowMenu->addAction(title);
        action->setCheckable(true);
        action->setChecked(subWin == active);
        connect(action, &QAction::triggered, this, [this, subWin]() {
            m_mdiArea->setActiveSubWindow(subWin);
        });
    }
}
```

这段代码每次菜单打开时都会重建窗口列表，把当前活动窗口标记为勾选状态，点击某个菜单项时激活对应的子窗口。这是一个非常经典的 MDI 应用模式。

## 4. 踩坑预防

第一个坑是忘记调子窗口的 `show()`。`addSubWindow()` 只是把子窗口添加到 MDI 区域中，但不会自动显示它。如果你发现 MDI 区域是空白的，大概率是忘了 `show()`。

第二个坑是 `WA_DeleteOnClose` 和 `subWindowList()` 的配合问题。如果子窗口设置了 `WA_DeleteOnClose`，关闭后它会被删除，但 `subWindowList()` 在子窗口被 `hide()` 之后、`delete` 之前可能还会返回这个子窗口。所以在遍历 `subWindowList()` 时，应该检查 `subWin->isVisible()` 或者用 `qobject_cast` 验证指针有效性。

第三个坑是在 `subWindowActivated` 信号处理函数中直接操作子窗口列表。当关闭一个子窗口时，Qt 会先激活另一个子窗口，然后才删除被关闭的那个。这意味着在 `subWindowActivated` 的处理函数中，`subWindowList()` 可能还包含即将被删除的子窗口。如果你需要在关闭时做清理，更安全的做法是连接子窗口的 `destroyed` 信号。

第四个坑是平铺模式下子窗口数量太多。`tileSubWindows()` 会对所有子窗口做等分排列，如果你有 20 个子窗口，每个窗口会被压成非常小的面积，基本不可用。平铺适合 2-4 个子窗口的场景，再多就应该用级联或标签页模式了。

第五个坑是子窗口的最大化行为。在 MDI 区域中，一个子窗口最大化后不会覆盖整个屏幕，而是占满整个 QMdiArea 区域。当你在最大化状态下打开新子窗口时，新窗口也会以最大化状态出现。这种行为和普通窗口的最大化不同，可能会让用户困惑。

## 5. 练习项目

我们来做一个综合练习：实现一个简易的"多文档文本编辑器"。主窗口使用 QMainWindow，中央控件是 QMdiArea。工具栏上有三个按钮——"新建文档"创建一个新的子窗口（内含 QTextEdit）、"级联排列"和"平铺排列"。菜单栏有一个"窗口"菜单，动态列出所有打开的子窗口，当前活动的子窗口前面打勾。状态栏显示当前活动子窗口的标题和子窗口总数。每个子窗口设置 `WA_DeleteOnClose`，关闭后自动销毁。

几个提示：新建文档时用 `addSubWindow(new QTextEdit)` 创建子窗口，设置递增的标题如"文档 1""文档 2"；窗口菜单在每次 `subWindowActivated` 信号触发时重建；状态栏用 `activeSubWindow()` 获取当前窗口标题，用 `subWindowList().size()` 获取总数；子窗口关闭后如果 MDI 区域为空，状态栏应显示"没有打开的文档"。

## 6. 官方文档参考链接

[Qt 文档 · QMdiArea](https://doc.qt.io/qt-6/qmdiarea.html) -- MDI 区域控件，子窗口管理和排列

[Qt 文档 · QMdiSubWindow](https://doc.qt.io/qt-6/qmdisubwindow.html) -- MDI 子窗口类，标题栏和关闭行为配置

[Qt 文档 · SDI vs MDI](https://doc.qt.io/qt-6/qmdiarea.html#details) -- 官方对 SDI 和 MDI 模式的对比说明

[Qt 文档 · QMainWindow](https://doc.qt.io/qt-6/qmainwindow.html) -- 主窗口类，MDI 通常作为 QMainWindow 的中央控件

---

到这里，QMdiArea 的基础你就掌握了。创建子窗口、配置标题和关闭行为、级联平铺排列、激活窗口与信号监听——这四个核心操作覆盖了 MDI 应用的绝大多数需求。虽然现在新应用用 MDI 的场景在减少，但当你的确需要"一个区域内管理多个自由浮动的子窗口"时，QMdiArea 是最现成、最稳定的方案。下一篇我们回到 QWidget 基类本身，看看所有控件的"根"都提供了哪些基础能力。
