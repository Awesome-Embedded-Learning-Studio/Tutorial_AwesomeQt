# 现代Qt开发教程（新手篇）3.39——QTabWidget：标签页控件

## 1. 前言 / 桌面应用里出场率最高的多页容器

如果你的应用有"设置"界面，十有八九你会看到标签页——"常规"、"外观"、"高级"、"关于"，每个标签背后藏着一整页控件。浏览器更不用说了，整个窗口就是一个巨型标签页管理器。标签页这种交互范式之所以如此普及，根本原因在于它能在不增加导航层级的前提下把大量控件平铺在不同的页面上——用户一眼就能看到所有分类标签，点一下就切换，不需要像向导对话框那样一步一步点"下一步"。

Qt 提供的 QTabWidget 就是这个范式的标准实现。它把一个标签栏（QTabBar）和一个堆叠页面区域（QStackedWidget）打包在一起，对外提供 addTab、insertTab、removeTab 这套 API 来动态管理标签页的增删，用 setTabPosition 控制标签栏放在上、下、左、右哪个位置，用 currentChanged 信号通知你用户切换了标签页。此外，setTabIcon 和 setTabToolTip 提供了标签的视觉美化手段——给每个标签加上小图标和悬浮提示，让界面更专业。

今天的内容分四个部分：QTabWidget 的基本创建和标签页的动态增删管理，标签栏位置和形状的配置，currentChanged 信号的响应机制，以及通过 setTabIcon 和 setTabToolTip 对标签进行视觉美化。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QTabWidget 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。示例代码中用到了 QTabWidget、QLabel、QLineEdit、QTextEdit、QSpinBox、QComboBox、QPushButton、QVBoxLayout、QHBoxLayout 以及标准对话框相关的控件。

## 3. 核心概念讲解

### 3.1 标签页的动态增删管理

QTabWidget 最核心的操作就是在运行时动态添加、插入和删除标签页。这三个操作分别对应 addTab、insertTab 和 removeTab 三个方法。

addTab(QWidget *page, const QString &label) 是最常用的添加方式——传入一个 QWidget 作为页面内容，再传入标签上显示的文字。QTabWidget 会把这个页面添加到最后一个标签的位置。你可以在创建 QTabWidget 后连续调用多次 addTab 来构建出完整的标签页结构。

```cpp
auto *tabs = new QTabWidget;

auto *generalPage = new QWidget;
// ... 给 generalPage 设置布局和控件 ...

auto *networkPage = new QWidget;
// ... 给 networkPage 设置布局和控件 ...

tabs->addTab(generalPage, "常规");
tabs->addTab(networkPage, "网络");
```

insertTab(int index, QWidget *page, const QString &label) 的行为和 addTab 类似，但它允许你指定插入位置。比如你有一个标签页管理器，用户可以新建标签页，新标签页需要插到当前标签的后面而不是末尾——这时就用 insertTab。index 参数是从 0 开始的，插入后原来 index 及之后的标签页都会往后移一位。如果 index 等于 count()（即当前标签页总数），效果等同于 addTab。

removeTab(int index) 删除指定位置的标签页。这里有一个非常关键的细节：removeTab 只是把页面从 QTabWidget 的管理中移除，但不会 delete 这个 QWidget。也就是说，如果你在堆上 new 了一个页面，removeTab 之后这个页面的内存仍然存在，你需要自己负责 delete。如果你希望移除的同时销毁页面，得手动 delete——或者在 removeTab 之前先取出页面指针，removeTab 之后再 delete。

```cpp
// 移除第 2 个标签页并销毁其内容
QWidget *doomed = tabs->widget(2);
tabs->removeTab(2);
delete doomed;
```

和增删相关的还有 count() 返回当前标签页总数，widget(int index) 返回指定位置的 QWidget 指针，indexOf(QWidget *page) 返回指定页面所在的索引。这三个方法在动态管理标签页时经常用到——比如你想知道某个页面是否还在标签页中，就调用 indexOf，返回 -1 说明已经不在了。

QTabWidget 在标签页数量为 0 时不会崩溃，但标签栏区域会变成一块空白。如果你的应用有"关闭所有标签页"的场景，建议在 count() 变为 0 时显示一个占位提示或者隐藏整个 QTabWidget。

有一个容易踩的坑：当你在槽函数中响应某个标签页的按钮点击来 removeTab 时，如果删除的是当前活动的标签页，QTabWidget 会自动切换到相邻的标签页（优先切换到前一个，如果没有前一个则切换到后一个），同时触发 currentChanged 信号。如果你的槽函数逻辑依赖于"当前标签页"的状态，需要意识到这个自动切换行为——删除操作结束后"当前标签页"已经不是你预期的那个了。

### 3.2 setTabPosition 与 setTabShape

默认情况下，标签栏显示在 QTabWidget 的上方。但你可以通过 setTabPosition(QTabWidget::TabPosition) 把标签栏挪到下方、左侧或右侧。四个可选值分别是 QTabWidget::North（上方，默认）、QTabWidget::South（下方）、QTabWidget::West（左侧）和 QTabWidget::East（右侧）。

```cpp
tabs->setTabPosition(QTabWidget::North);   // 标签在上方（默认）
tabs->setTabPosition(QTabWidget::South);   // 标签在下方
tabs->setTabPosition(QTabWidget::West);    // 标签在左侧
tabs->setTabPosition(QTabWidget::East);    // 标签在右侧
```

当标签栏放在左侧或右侧时，标签文字默认是垂直排列的——每个标签的图标在上、文字在下，标签整体水平宽度会比较窄。如果你觉得这种布局不够好看，可以通过 QSS 给 QTabBar::tab 设置固定宽度或者通过 setStyle 自定义标签外观。左右侧标签栏在一些工具型应用中比较常见，比如 Photoshop 的属性面板、IDE 的侧边工具栏——标签文字短、图标明确的情况下，左右布局比上下布局更节省垂直空间。

setTabShape(QTabWidget::TabShape) 控制标签的外形。默认值是 QTabWidget::Rounded（圆角标签），另一个选项是 QTabWidget::Triangular（三角形标签）。Rounded 是绝大多数应用的标准外观——标签的四个角是圆弧形的。Triangular 会把标签画成三角形或者梯形的形状，视觉上更棱角分明。实际上 Triangular 样式在日常应用中几乎见不到，它主要出现在一些特殊的主题或者嵌入式设备的 UI 中。

```cpp
tabs->setTabShape(QTabWidget::Rounded);     // 圆角（默认）
tabs->setTabShape(QTabWidget::Triangular);  // 三角形
```

还有一个相关的属性是 tabsClosable()。通过 setTabsClosable(true) 可以在每个标签的右侧显示一个关闭按钮（小叉号）。用户点击关闭按钮时，QTabWidget 会发出 tabCloseRequested(int index) 信号。注意，这个信号只是通知你"用户想关闭这个标签"，QTabWidget 不会自动删除标签页——你需要在槽函数中自己调用 removeTab。

```cpp
tabs->setTabsClosable(true);

connect(tabs, &QTabWidget::tabCloseRequested,
        this, [tabs](int index) {
    QWidget *page = tabs->widget(index);
    tabs->removeTab(index);
    delete page;
});
```

setMovable(bool) 控制用户是否可以通过拖拽来重新排列标签的顺序。设为 true 后，用户可以按住标签拖动来改变标签页的位置。这个功能在浏览器式标签页管理中特别常见。

```cpp
tabs->setMovable(true);   // 允许拖拽排序
```

### 3.3 currentChanged 信号响应标签切换

QTabWidget 在用户切换标签页时会发出 currentChanged(int index) 信号，参数是切换后新标签页的索引。这个信号在你需要根据当前标签页状态更新其他 UI 元素时非常有用——比如菜单栏的可用操作随标签页变化，或者状态栏显示当前标签页的上下文信息。

```cpp
connect(tabs, &QTabWidget::currentChanged,
        this, [this](int index) {
    if (index < 0) return;  // 所有标签被移除时会触发 index == -1

    QString tabText = tabs->tabText(index);
    m_statusLabel->setText(
        QString("当前页面: %1 (第 %2 页)").arg(tabText).arg(index + 1));
});
```

这里要注意一个细节：当标签页被移除导致当前标签变化时，currentChanged 也会触发。如果所有标签页都被移除了，currentChanged 会发出一次 index == -1 的信号。你的槽函数必须处理 index < 0 的情况，否则调用 tabs->widget(-1) 会返回 nullptr，后续操作大概率会崩溃。

另一个相关的信号是 tabBarClicked(int index)，在用户点击标签栏上的标签时触发——即使用户点击的已经是当前标签页（也就是没有实际切换发生）。这个信号和 currentChanged 的区别在于：currentChanged 只在标签页真正切换时才触发，tabBarClicked 在任何点击标签时都触发。

setCurrentIndex(int index) 和 setCurrentWidget(QWidget *widget) 用于在代码中程序化地切换标签页。这两个方法会触发 currentChanged 信号。如果你需要在不触发信号的情况下切换标签页——虽然不太推荐，但确实有这种场景——可以用 blockSignals(true) 临时阻塞信号：

```cpp
tabs->blockSignals(true);
tabs->setCurrentIndex(0);
tabs->blockSignals(false);
```

currentIndex() 返回当前标签页的索引（从 0 开始），currentWidget() 返回当前标签页对应的 QWidget 指针。如果 QTabWidget 中没有标签页，currentIndex() 返回 -1，currentWidget() 返回 nullptr。

### 3.4 setTabIcon 与 setTabToolTip 标签美化

纯文字标签在标签页数量少的时候没问题，但当标签页多了或者你希望界面更专业时，给标签加上图标和悬浮提示会让交互体验好很多。

setTabIcon(int index, const QIcon &icon) 给指定标签设置图标。图标显示在标签文字的左侧，尺寸由 QTabBar 的 iconSize 属性决定，默认大约是 16x16 像素。你可以通过 tabBar()->setIconSize(QSize(24, 24)) 来调整图标大小。

```cpp
tabs->addTab(generalPage, "常规");
tabs->addTab(networkPage, "网络");
tabs->addTab(advancedPage, "高级");

tabs->setTabIcon(0, QIcon::fromTheme("preferences-system"));
tabs->setTabIcon(1, QIcon::fromTheme("network-wired"));
tabs->setTabIcon(2, QIcon::fromTheme("preferences-other"));
```

QIcon::fromTheme() 会从系统主题图标中加载指定名称的图标。在 Linux 上这通常能正常工作（系统有 freedesktop.org 图标主题），但在 Windows 和 macOS 上可能找不到对应的图标名。如果你需要跨平台一致的图标，建议把图标资源打包进 qrc 资源文件，用 QIcon(":/icons/general.png") 这种方式加载。

setTabToolTip(int index, const QString &tip) 设置鼠标悬浮在标签上时显示的提示文字。这个功能在标签文字被截断时特别有用——比如窗口太窄导致标签文字显示不全，用户把鼠标悬停在标签上就能看到完整的提示。

```cpp
tabs->setTabToolTip(0, "常规设置：语言、主题、启动选项");
tabs->setTabToolTip(1, "网络设置：代理、DNS、连接超时");
tabs->setTabToolTip(2, "高级设置：调试、日志、实验性功能");
```

setTabText(int index, const QString &text) 可以在运行时修改已有标签的文字，setTabWhatsThis(int index, const QString &text) 设置"这是什么"帮助文本（在 Windows 上按 Shift+F1 触发，现代应用中很少用到）。

还有一个实用的方法 setTabEnabled(int index, bool enabled)——它可以禁用某个标签页。禁用后标签文字变灰，用户无法点击切换到该标签页，但标签仍然可见。这比 removeTab 更温和：你不想让用户访问某个标签页，但又想让它保持可见，用 setTabEnabled(false) 就行。

```cpp
tabs->setTabEnabled(2, false);  // 禁用"高级"标签页
```

对应的 tabEnabled(int index) 查询某个标签是否启用，setTabVisible(int index, bool visible)（Qt 5.15+ 引入）可以隐藏标签但保留页面在内部管理中。

## 4. 踩坑预防

第一个坑是 removeTab 不会 delete 页面控件。如果你 new 了 QWidget 放进 QTabWidget，之后 removeTab 只是从管理列表中移除，内存泄漏就在那摆着。正确做法是 removeTab 之后手动 delete 取出的 widget 指针，或者一开始就用 setAttribute(Qt::WA_DeleteOnClose) 但这在 QTabWidget 场景下不太适用——最靠谱的还是手动管理生命周期。

第二个坑是 currentChanged 信号在所有标签被移除时会发出 index == -1。如果你的槽函数里直接用 index 去 widget(index) 或者 tabText(index)，必定越界或者拿到 nullptr。槽函数开头加一行 if (index < 0) return; 是最简单的防御。

第三个坑是 setTabPosition(QTabWidget::West) 或 QTabWidget::East 时标签文字的可读性问题。垂直标签栏默认会把文字旋转 90 度排列，如果标签文字较长，整个标签栏会变得很宽或者文字被截断。建议在使用左右标签栏时保持标签文字简短，或者配合图标使用——让图标承担主要的识别功能，文字作为辅助。

第四个坑是 setTabsClosable(true) 配合 setMovable(true) 的交互冲突。在 Qt 6.9 中这两个功能可以共存，但点击关闭按钮时手指稍微一移动就可能被识别为拖拽操作而不是点击。如果你的用户反馈"关闭按钮不好按"，可以考虑在 tabCloseRequested 槽函数中加一个确认对话框，或者干脆用右键菜单提供关闭功能。

第五个坑是标签页数量过多时的溢出问题。当标签数超出标签栏的可见宽度时，QTabBar 会自动显示滚动箭头。但如果你用的是 setTabPosition(West) 或 (East)，标签过多时滚动箭头出现在标签栏的上下端——视觉上可能不太明显，用户可能不知道还有更多标签。这种情况下考虑使用 elideMode 来控制文字截断方式：tabs->setElideMode(Qt::ElideRight) 会让超长标签文字在右侧显示省略号。

## 5. 练习项目

我们来做一个综合练习：创建一个"系统设置"窗口，使用 QTabWidget 组织四个标签页。"常规"标签页包含用户名 QLineEdit、语言选择 QComboBox、自动启动 QCheckBox；"网络"标签页包含代理地址 QLineEdit、端口 QSpinBox，并用 setTabIcon 给标签配上对应的系统主题图标；"外观"标签页包含深色模式 QCheckBox 和字体大小 QSpinBox；"高级"标签页默认被 setTabEnabled(false) 禁用，但通过一个 QCheckBox 可以解锁它。窗口底部有一个状态栏，通过 currentChanged 信号实时显示当前标签页的名称和索引。标签栏放在上方，使用 Rounded 形状，开启 tabsClosable 和 movable 功能。

提示：禁用标签页用 setTabEnabled(index, false)，解锁时通过 setTabEnabled(index, true) 恢复。关闭标签时在 tabCloseRequested 槽中 removeTab + delete 页面指针。

## 6. 官方文档参考链接

[Qt 文档 -- QTabWidget](https://doc.qt.io/qt-6/qtabwidget.html) -- 标签页控件

[Qt 文档 -- QTabBar](https://doc.qt.io/qt-6/qtabbar.html) -- 标签栏控件（QTabWidget 内部使用的标签栏）

[Qt 文档 -- QTabWidget::TabPosition](https://doc.qt.io/qt-6/qtabwidget.html#TabPosition-enum) -- 标签位置枚举

---

到这里，QTabWidget 的核心用法就全部讲完了。addTab/insertTab/removeTab 构成了标签页动态管理的三角支柱，setTabPosition 和 setTabShape 让标签栏的布局足够灵活，currentChanged 信号是标签页状态联动的核心，setTabIcon 和 setTabToolTip 则给标签赋予了更专业的视觉表现。QTabWidget 是 Qt 里最成熟的多页容器——如果你的界面需要把大量控件按功能分组呈现，标签页几乎永远是首选方案。掌握它的动态管理和信号机制，后面我们讲到 QTabBar 和 QStackedWidget 时你会发现很多概念是互通的。
