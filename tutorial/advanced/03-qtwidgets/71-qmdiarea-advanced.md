---
title: "3.71 QMdiArea 进阶"
description: "入门篇我们学会了 QMdiArea 的基本用法：addSubWindow 添加子窗口、cascadeSubWindows / tileSubWindows 排列、subWindowList 遍历。进阶篇要深挖的是标签页模式的切换和配置、子窗口激活信号的正确用法、以及 cascade 和 tile 布局的实际行为边界。"
---

# 现代Qt开发教程（进阶篇）3.71——QMdiArea 进阶

## 1. 前言 / MDI 不仅仅是子窗口的容器

入门篇我们学会了 QMdiArea 的基本用法：addSubWindow 添加子窗口、cascadeSubWindows / tileSubWindows 排列、subWindowList 遍历。说实话，做到这一步你已经能搭出一个多文档编辑器的框架了。但如果你真正用过 Visual Studio、Excel 这类支持多文档的应用，你会发现它们在 MDI 的交互细节上做得远不止"添加子窗口"这么简单——标签页模式切换、子窗口激活状态追踪、菜单与活动窗口的联动、布局管理的精确控制，这些才是让 MDI 从"能用"变成"好用"的关键。

这篇文章要深挖的是四个核心议题：setViewMode 在 SubWindowView 和 TabbedView 之间的切换行为和注意事项，标签页模式下的可关闭/可拖动配置，subWindowActivated 信号的触发时机和常见误用，以及 cascadeSubWindows / tileSubWindows 的实际布局算法和边界情况。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17 标准。QMdiArea 和 QMdiSubWindow 属于 QtWidgets 模块，不需要额外的模块依赖。TabbedView 模式在 Qt 4.5 引入，Qt 6 全系列支持。

## 3. 核心概念讲解

### 3.1 setViewMode：SubWindowView 与 TabbedView 切换

QMdiArea 提供两种视图模式：QMdiArea::SubWindowView（默认值）是经典的 MDI 子窗口模式，每个子窗口在 MDI 区域内自由浮动、可独立拖动和缩放；QMdiArea::TabbedView 是标签页模式，所有子窗口以标签页的形式排列，同一时刻只显示一个子窗口的内容，类似于浏览器的多标签页。

切换模式很简单：

```cpp
auto *mdi_area = new QMdiArea;

// 标签页模式
mdi_area->setViewMode(QMdiArea::TabbedView);

// 切回子窗口模式
mdi_area->setViewMode(QMdiArea::SubWindowView);
```

这里有一个很重要的行为需要知道：切换模式时，QMdiArea 不会销毁或重建子窗口——它只是改变了子窗口的展示方式。从 SubWindowView 切到 TabbedView 时，所有子窗口的几何位置和大小会被保存到内部状态中（通过 QMdiSubWindow 的内部属性），子窗口被"折叠"成标签页。从 TabbedView 切回 SubWindowView 时，之前保存的几何位置会被恢复。这意味着用户在子窗口模式下精心排列的窗口布局不会因为临时切换到标签页模式而丢失。

但要注意：如果在 TabbedView 模式下关闭了一个标签页（删除了子窗口），切回 SubWindowView 后这个子窗口自然也不存在了——因为子窗口被删除了，不是被隐藏了。

另一个需要注意的点是：在 TabbedView 模式下，QMdiArea 内部会创建一个 QTabBar 来管理标签。这个 QTabBar 可以通过 findChild<QTabBar*>() 获取到，方便做进一步的自定义。但不建议对它做过多修改，因为 QMdiArea 内部对 QTabBar 的行为有假设，过度自定义可能导致不一致。

### 3.2 标签页行为配置：setTabsClosable 与 setTabsMovable

在 TabbedView 模式下，QMdiArea 提供了几个配置标签页行为的方法。setTabsClosable(true) 会在每个标签页上显示一个关闭按钮——点击关闭按钮会关闭对应的子窗口（等同于调用 closeActiveSubWindow()）。setTabsMovable(true) 允许用户通过拖拽来重新排列标签页的顺序。

```cpp
mdi_area->setViewMode(QMdiArea::TabbedView);
mdi_area->setTabsClosable(true);
mdi_area->setTabsMovable(true);
mdi_area->setTabPosition(QTabWidget::North);  // 标签位置：上下左右
mdi_area->setTabShape(QTabWidget::Rounded);   // 圆角或梯形标签
```

setTabPosition 控制标签栏的位置——QTabWidget::North（上方，默认）、South（下方）、West（左侧）、East（右侧）。setTabShape 控制标签的外观——Rounded（圆角，默认）或 Triangular（梯形）。这些配置只在 TabbedView 模式下生效，在 SubWindowView 模式下调用它们不会有任何效果，也不会报错。

关于 setTabsClosable 有一个行为需要注意：标签页上的关闭按钮关闭的是子窗口（QMdiSubWindow），不是标签页。也就是说，点击关闭按钮会触发子窗口的 closeEvent，如果你在子窗口的 closeEvent 中做了条件判断（比如"文档未保存是否关闭"的确认对话框），这个逻辑在标签页关闭按钮上同样生效。这是正确的行为——关闭按钮不应该绕过子窗口的关闭验证逻辑。

现在有一道思考题给大家。如果我们在 TabbedView 模式下调用 setTabsClosable(false)，用户还能关闭子窗口吗？答案是可以——setTabsClosable(false) 只是隐藏了标签页上的关闭按钮，但用户仍然可以通过右键菜单（如果有的话）、键盘快捷键（Ctrl+W）或者代码调用 closeActiveSubWindow() 来关闭子窗口。setTabsClosable 只控制 UI 上的关闭按钮是否可见，不控制子窗口是否可关闭。

### 3.3 子窗口激活与 subWindowActivated 信号

QMdiArea 的 subWindowActivated 信号在活动子窗口切换时发射。这个信号在工程中非常常用——比如你需要在菜单栏中显示当前活动文档的名称、在状态栏中显示当前活动窗口的信息、或者根据活动窗口的类型动态启用/禁用工具栏按钮。

```cpp
QObject::connect(mdi_area, &QMdiArea::subWindowActivated, [](QMdiSubWindow* sub) {
    if (sub == nullptr) {
        // 所有子窗口都关闭了
        update_menu_for_no_document();
    } else {
        // 新的子窗口被激活
        update_menu_for_document(sub->widget());
    }
});
```

这个信号有几个行为细节需要搞清楚。第一，参数可以为 nullptr——当最后一个子窗口被关闭时，QMdiArea 会发射 subWindowActivated(nullptr)。如果你不检查 nullptr 就直接访问 sub->widget()，会收获一个漂亮的 segfault。

第二，subWindowActivated 的参数是 QMdiSubWindow 指针，不是你 addSubWindow 时传入的内部 widget 指针。QMdiArea 会自动把你的 widget 包裹在一个 QMdiSubWindow 中，所以你需要通过 sub->widget() 来获取原始的 widget。这是一个常见的混淆点。

第三，在 SubWindowView 模式下，点击一个子窗口的标题栏、或者用 setActiveSubWindow() 切换活动窗口都会触发这个信号。在 TabbedView 模式下，切换标签页也会触发。但有一个边界情况：如果在 TabbedView 模式下用代码调用 addSubWindow 并立即调用 setActiveSubWindow，信号可能不会触发——因为新添加的窗口已经是活动窗口了，没有发生"切换"。如果需要确保初始化时也触发一次，可以在 addSubWindow 后手动调用一次信号处理逻辑。

第四，如果你在 subWindowActivated 的槽函数中又调用了 setActiveSubWindow（比如根据某些条件自动切换活动窗口），会形成信号重入。Qt 的事件循环会处理这种情况，但可能导致非预期的窗口切换顺序。建议在槽函数中避免再次修改活动窗口，或者用一个 bool 标志防止重入。

### 3.4 cascadeSubWindows / tileSubWindows 布局管理

cascadeSubWindows() 将所有子窗口层叠排列——每个子窗口相对前一个子窗口偏移一定的水平和垂直距离，形成"瀑布"效果。tileSubWindows() 将所有子窗口平铺排列——MDI 区域被均分为 N 行 N 列的网格，每个子窗口占据一个格子。

```cpp
// 层叠排列
mdi_area->cascadeSubWindows();

// 平铺排列
mdi_area->tileSubWindows();
```

cascadeSubWindows 的行为比较直觉：它按照 subWindowList 的顺序（默认是 CreationOrder），依次排列子窗口。每个子窗口的大小会被重置为一个默认值（通常是 MDI 区域大小的某个比例），位置依次偏移约 20-30 像素。偏移量由平台风格决定，不可配置。层叠后用户可以点击任何子窗口的标题栏将它提升到最前面。

tileSubWindows 的行为需要仔细理解。它会将 MDI 区域的可用空间均分为 ceil(sqrt(N)) 行和 ceil(N / rows) 列（其中 N 是子窗口数量），然后将子窗口依次填入网格。比如 5 个子窗口会被排列成 2 行 3 列（2x3=6 格，最后一格为空），7 个子窗口会被排列成 3 行 3 列（3x3=9 格，最后两格为空）。

tileSubWindows 有一个重要的行为：它会忽略子窗口的 minimumSizeHint 和 minimumSize——所有子窗口被强制缩放到网格单元的大小。如果你的子窗口有最小尺寸要求（比如内部的 QTextEdit 需要至少 200x100 才能用），tile 后可能会被压得比最小尺寸还小，导致布局变形或控件重叠。

另一个需要注意的点是：这两个方法都只在 SubWindowView 模式下有意义。在 TabbedView 模式下调用它们不会产生任何效果——因为标签页模式下子窗口的几何位置由标签页控件管理，不受 cascade/tile 的影响。

## 4. 踩坑预防

第一个坑是 subWindowActivated(nullptr) 导致的崩溃。当最后一个子窗口被关闭时，QMdiArea 发射 subWindowActivated(nullptr)。如果你的槽函数直接对参数调用 widget()、windowTitle() 等方法而不检查 nullptr，程序会崩溃。解决方案是在槽函数开头做 nullptr 检查，这是最基本的防御性编程，但真的很多人忘了。

第二个坑是 TabbedView 模式下切换回 SubWindowView 后窗口位置混乱。虽然 QMdiArea 会保存子窗口的几何位置，但如果你在 TabbedView 模式下 addSubWindow 了新窗口，这个新窗口没有保存过几何位置——切回 SubWindowView 后它会被放在 MDI 区域的 (0,0) 位置，可能和其他恢复位置的窗口重叠。解决方案是在 TabbedView 模式下添加新窗口后，手动设置一个合理的初始几何位置，或者在切回 SubWindowView 后调用一次 cascadeSubWindows() 来重新排列。

第三个坑是 tileSubWindows 忽略子窗口的 minimumSize 导致布局变形。如果你的子窗口有最小尺寸约束，tile 后可能被压缩到比最小尺寸还小。这在子窗口数量较多时特别明显——10 个子窗口 tile 后每个可能只有 200x100 像素。解决方案是在调用 tileSubWindows 前检查 MDI 区域的大小和子窗口数量，如果空间不够就不要 tile，改用 cascade 或者提示用户调整窗口大小。也可以在 tile 后遍历子窗口检查实际尺寸是否小于 minimumSize，如果小于就自动调整 MDI 区域的最小尺寸。

第四个坑是频繁切换 ViewMode 导致的性能问题。每次调用 setViewMode，QMdiArea 内部会重建子窗口的展示方式——在 SubWindowView 模式下为每个子窗口创建独立的 QMdiSubWindow 框架（标题栏、边框等），在 TabbedView 模式下更新 QTabBar 的标签。如果子窗口数量很多（比如 50 个以上），频繁切换模式会导致明显的卡顿。解决方案是避免在运行时频繁切换，或者在切换前关闭不需要的子窗口减少数量。

## 5. 练习项目

练习项目：多文档编辑器，支持模式切换和窗口菜单。我们要实现一个 QMainWindow，中心区域是 QMdiArea。菜单栏有两个菜单：View 和 Window。View 菜单提供 SubWindowView / TabbedView 两个选项（用 QActionGroup 互斥），切换时保持子窗口不丢失。Window 菜单提供 Cascade、Tile、Close Active 三个操作，以及一个动态生成的子窗口列表（每个子窗口对应一个 QAction，点击激活对应窗口）。工具栏上有一个 QComboBox 显示所有子窗口的标题，选择后激活对应窗口。

完成标准是：模式切换后所有子窗口内容保持不变；subWindowActivated 正确更新 Window 菜单和工具栏 ComboBox 的当前选项；tile 时子窗口不被压缩到小于合理尺寸；关闭最后一个子窗口后 Window 菜单和 ComboBox 正确清空。提示几个关键点：用 subWindowActivated 的槽函数更新 Window 菜单和 ComboBox；Window 菜单的子窗口列表需要在每次 subWindowActivated 时重建（先清空再 addAction）；ComboBox 的 currentIndexChanged 和 subWindowActivated 之间避免循环触发（用 blockSignals 或标志位）。

## 6. 官方文档参考链接

[Qt 文档 · QMdiArea](https://doc.qt.io/qt-6/qmdiarea.html) -- MDI 区域控件，包含 ViewMode、标签页配置、布局方法说明

[Qt 文档 · QMdiSubWindow](https://doc.qt.io/qt-6/qmdisubwindow.html) -- MDI 子窗口类，包含窗口状态、最小尺寸、关闭行为说明

[Qt 文档 · QTabBar](https://doc.qt.io/qt-6/qtabbar.html) -- 标签栏控件，QMdiArea 在 TabbedView 模式内部使用的标签管理器

---

到这里，QMdiArea 的进阶内容就过了一遍。SubWindowView 和 TabbedView 的切换不会丢失子窗口，但新添加的窗口在切回子窗口模式后可能位置混乱。标签页的 closable 和 movable 配置只影响 UI 表现，不改变子窗口的底层行为。subWindowActivated 信号必须检查 nullptr，槽函数中不要再次修改活动窗口。cascade 和 tile 的布局算法有明确的行为边界——tile 会忽略 minimumSize，cascade 的偏移量不可配置。把这些搞清楚了，你的多文档编辑器就能在两种模式之间自如切换了。
