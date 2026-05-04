# 现代Qt开发教程（新手篇）3.43——QToolBox：工具箱折叠面板

## 1. 前言 / 那个和 Photoshop 工具栏原理相同的折叠面板

如果你打开过 Photoshop、GIMP 或者任何一款功能丰富的桌面工具，大概率见过侧边栏上那种"点开一个分类、其他分类自动收起"的折叠面板——"画笔工具"展开后显示各种笔刷，"形状工具"展开后显示矩形椭圆多边形，同时前一组的面板自动折叠回去，保证视觉焦点始终在当前选中的工具上。Adobe 系列软件、各种 CAD 工具、甚至 KDE 的系统设置界面都在大量使用这种交互模式。它的好处很明确：在有限的侧边栏空间里塞下大量分类内容，同时不让用户感到信息过载——任何时刻只有一组内容展开。

Qt 的 QToolBox 就是这个功能的标准实现。它是一个容器控件，内部管理着一组"页面"，每个页面有一个标题栏和一段内容区域。同一时刻只有一个页面的内容区域是展开的——点击某个页面的标题栏，该页面展开，之前展开的页面自动折叠。这个行为模式和 QTabWidget 有些相似——都是"同一时刻只显示一个"——但 QToolBox 的视觉呈现完全不同：所有页面的标题栏始终可见（竖着排列），只是内容区域在展开和折叠之间切换。这使得它特别适合侧边栏导航场景——用户不需要点开任何菜单就能看到所有分类名称，一键切换。

今天的内容分四个部分：addItem/insertItem 添加面板以及 setCurrentIndex 切换当前展开的面板，currentChanged 信号响应面板切换事件，setItemEnabled 禁用某个面板，以及 QToolBox 在侧边栏导航中的典型应用场景和布局模式。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QToolBox 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。示例代码中用到了 QToolBox、QLabel、QPushButton、QLineEdit、QListWidget、QTextEdit、QVBoxLayout、QHBoxLayout 和 QSplitter。

## 3. 核心概念讲解

### 3.1 addItem / insertItem 添加面板

QToolBox 添加面板的核心方法有两个。addItem(QWidget *widget, const QString &text) 把一个控件作为新面板追加到末尾，text 参数是该面板标题栏上显示的文字。insertItem(int index, QWidget *widget, const QString &text) 在指定位置插入面板。还有一个带图标的重载版本 addItem(QWidget *widget, const QIcon &icon, const QString &text)，可以给标题栏加上图标。

```cpp
auto *toolbox = new QToolBox;

auto *brushPage = new QWidget;
auto *shapePage = new QWidget;
auto *textPage = new QWidget;

toolbox->addItem(brushPage, "画笔工具");
toolbox->addItem(shapePage, "形状工具");
toolbox->addItem(textPage, "文字工具");
```

addItem 的返回值是新添加面板的索引——从 0 开始，依次递增。QToolBox 会在标题栏区域自动画一个可点击的按钮条，上面显示你传入的文字（和图标）。用户点击标题栏，对应面板展开，其余面板折叠。这个切换行为是 QToolBox 内部自动处理的，不需要你写任何信号连接代码。

setCurrentIndex(int index) 可以在代码中程序化地切换当前展开的面板。和用户手动点击标题栏的效果完全一样——指定索引的面板展开，其他面板折叠。currentIndex() 返回当前展开面板的索引。

```cpp
toolbox->setCurrentIndex(0);  // 展开第 1 个面板（画笔工具）
int current = toolbox->currentIndex();  // 返回 0
```

每个面板的内容区域就是一个普通的 QWidget，你可以在里面放任何布局和控件。需要注意的是，QToolBox 在面板折叠时会对该面板的 QWidget 调用 hide()，展开时调用 show()。如果你的面板内有需要在每次展开时刷新的数据（比如从数据库重新读取列表），不能依赖构造函数里的一次性初始化——需要监听 currentChanged 信号在面板展开时执行刷新。

removeItem(int index) 从 QToolBox 中移除指定面板。和 QTabWidget 的 removeTab 一样，removeItem 只是从 QToolBox 的管理中移除面板控件，不会 delete 它。你需要自己管理被移除面板的生命周期。

```cpp
QWidget *removed = toolbox->widget(1);  // 先取出指针
toolbox->removeItem(1);                 // 从 QToolBox 中移除
delete removed;                         // 手动销毁
```

count() 返回面板总数，widget(int index) 返回指定索引的面板 QWidget 指针，indexOf(QWidget *widget) 返回指定面板的索引。这些方法和 QStackedWidget、QTabWidget 中的同名方法行为完全一致。

setItemText(int index, const QString &text) 和 setItemIcon(int index, const QIcon &icon) 可以在运行时修改已有面板的标题栏文字和图标。setItemToolTip(int index, const QString &tip) 设置鼠标悬浮在标题栏上时显示的提示文字。

### 3.2 currentChanged 信号响应面板切换

QToolBox 在当前展开的面板发生变化时发出 currentChanged(int index) 信号，参数是新展开面板的索引。这个信号在你需要根据当前展开面板执行额外逻辑时很有用——比如展开"网络设置"面板时自动刷新网络接口列表、展开"用户信息"面板时重新读取用户数据。

```cpp
connect(toolbox, &QToolBox::currentChanged,
        this, [this](int index) {
    if (index < 0) return;

    QString pageName = toolbox->itemText(index);
    m_statusLabel->setText(
        QString("当前面板: %1 (索引 %2)").arg(pageName).arg(index));

    // 展开特定面板时执行刷新
    if (index == 2) {
        refreshTextToolOptions();
    }
});
```

currentChanged 在用户点击标题栏切换面板时会触发，在代码中通过 setCurrentIndex 切换时也会触发。如果你需要在初始化阶段切换面板但不希望触发信号——比如窗口启动时先设好面板状态再统一刷新——可以用 blockSignals(true) 临时屏蔽。

```cpp
toolbox->blockSignals(true);
toolbox->setCurrentIndex(0);
// ... 其他初始化操作 ...
toolbox->blockSignals(false);
```

有一个容易忽略的场景：当 QToolBox 只有一个面板时，setCurrentIndex(0) 不会触发 currentChanged——因为"当前面板"没有发生任何变化。这和 QStackedWidget、QTabWidget 的行为完全一致。如果你的初始化逻辑依赖 currentChanged 来做首次数据加载，需要确保初始化逻辑在信号触发路径之外也有执行的机会。

### 3.3 setItemEnabled 禁用面板

有时候你需要让某个面板暂时不可访问——比如某个功能模块需要高级许可才能使用，或者某个配置分类在当前条件下不适用。setItemEnabled(int index, bool enabled) 就是做这个的。禁用后该面板的标题栏文字会变灰，用户点击标题栏不会有任何效果——面板不会展开。

```cpp
toolbox->addItem(advancedPage, "高级选项");
toolbox->setItemEnabled(3, false);  // 禁用"高级选项"面板
```

isItemEnabled(int index) 查询某个面板是否启用。被禁用的面板在标题栏上仍然可见——它的标题文字和图标都在，只是变灰且不可点击。这和 removeItem 的区别在于：removeItem 是彻底移除，面板从列表中消失；setItemEnabled(false) 是"还在但不能用"，用户能看到它的存在但无法访问。

一个典型的场景是：你的应用有"免费版"和"专业版"两个级别，"高级选项"面板在免费版中 setItemEnabled(false)，标题栏文字后面加上 "(专业版)" 的提示，让用户知道这个功能存在但需要升级。升级后在代码中 setItemEnabled(index, true) 就能解锁。

```cpp
// 根据许可证状态启用/禁用面板
toolbox->setItemText(3, isPro ? "高级选项" : "高级选项 (专业版)");
toolbox->setItemEnabled(3, isPro);
```

setItemEnabled 对当前已经展开的面板也有效——如果你禁用的是当前展开的面板，该面板会立刻被折叠（因为用户无法继续操作它了），QToolBox 会自动切换到前一个可用的面板。这个行为可能会触发 currentChanged 信号，需要注意。

### 3.4 侧边栏导航的典型应用场景

QToolBox 最常见的使用场景是侧边栏导航——窗口左侧是一个窄长的面板，用 QToolBox 把功能分类组织成可折叠的区块。这种布局在各种工具型应用中大量出现。

一个标准的实现模式是把 QToolBox 和主内容区放在一个水平 QSplitter 中。QToolBox 在左侧充当导航面板，右侧放一个 QStackedWidget 或者直接放内容控件。用户在 QToolBox 中选择不同的工具分类，右侧的内容区跟着变化。

```cpp
auto *splitter = new QSplitter(Qt::Horizontal);

// 左侧：QToolBox 导航
auto *toolbox = new QToolBox;
toolbox->addItem(createBrushPage(), "画笔");
toolbox->addItem(createShapePage(), "形状");
toolbox->addItem(createTextPage(), "文字");
toolbox->setMinimumWidth(180);

// 右侧：画布区域
auto *canvas = new QTextEdit;
canvas->setPlaceholderText("画布区域...");

splitter->addWidget(toolbox);
splitter->addWidget(canvas);
splitter->setSizes({200, 600});
```

另一种常见的模式是把 QToolBox 放在一个可折叠的 QDockWidget 中。这样用户可以通过菜单栏或右键菜单来显示/隐藏整个工具箱面板。Qt Creator 的"项目"面板、"文件系统"面板就是这种模式——它们可以停靠在窗口侧边、可以隐藏、可以拖出来变成独立窗口。

QToolBox 和 QTabWidget 在侧边栏导航场景下的选择标准很直接。如果导航分类数量较少（三五个）且每个分类的内容不多，QToolBox 的折叠面板风格更合适——所有分类名称同时可见，一键切换，不需要点开下拉框或者在标签之间寻找。如果分类数量很多（七八个以上），QTabWidget 的标签栏或者 QComboBox + QStackedWidget 的组合可能更节省空间。

QToolBox 还有一个特点使它特别适合工具面板场景：每个面板的标题栏可以带图标。在实际项目中，给每个工具分类配上一个小图标（16x16 或 24x24），视觉识别效率比纯文字高得多。

```cpp
toolbox->addItem(brushPage, QIcon::fromTheme("draw-brush"), "画笔");
toolbox->addItem(shapePage, QIcon::fromTheme("draw-rectangle"), "形状");
toolbox->addItem(textPage, QIcon::fromTheme("draw-text"), "文字");
```

有一点需要注意：QToolBox 在面板数量较多时会自动显示一个内部的滚动区域——当所有标题栏的高度之和超过 QToolBox 的可见区域时，会出现滚动条。这意味着即使你往 QToolBox 里塞十几个面板，也不会导致界面溢出。但考虑到"所有标题栏始终可见"是 QToolBox 的核心交互特征，面板数量过多会削弱这个优势——用户需要滚动才能看到所有分类名称，这就和"一目了然"的初衷矛盾了。

## 4. 踩坑预防

第一个坑是 removeItem 不会 delete 被移除的控件。这一点和 QTabWidget 的 removeTab、QStackedWidget 的 removeWidget 行为一致——QToolBox 只是从管理列表中移除面板，控件仍然存在于内存中。如果你忘了 delete，就是内存泄漏。正确做法是 removeItem 之后手动 delete 取出的 widget 指针。

第二个坑是 setItemEnabled(false) 作用在当前展开面板上时会触发 currentChanged。当你在代码中禁用当前展开的面板时，QToolBox 会自动折叠它并切换到前一个可用面板，同时发出 currentChanged 信号。如果你的 currentChanged 槽函数中有依赖于"当前面板索引"的逻辑，需要意识到这个自动切换行为——禁用操作结束后"当前面板"已经不是你禁用的那个了。

第三个坑是 QToolBox 中面板的标题栏高度在不同平台和风格下差异较大。在 Fusion 风格下标题栏比较紧凑，但在某些平台上可能非常高。如果你在一个有限的侧边栏空间中使用 QToolBox，需要考虑标题栏高度对可用内容区域的挤压。可以通过 QSS 来控制标题栏的高度。

```css
QToolBox::tab {
    padding: 6px 8px;
    min-height: 24px;
}
```

第四个坑是 QToolBox 的面板内容控件在折叠时会被 hide()，这意味着如果你的面板中有需要持续运行的动画或者定时器，它们在面板折叠期间不会显示但仍然在后台运行。如果这些后台操作消耗资源，应该在 currentChanged 槽函数中检测面板是否被折叠，在折叠时暂停不必要的操作。

## 5. 练习项目

我们来做一个综合练习：创建一个"系统控制面板"窗口，使用 QSplitter 分成左右两栏。左侧使用 QToolBox 组织四个折叠面板："显示设置"面板包含分辨率 QComboBox 和刷新率 QComboBox；"网络配置"面板包含 IP 地址 QLineEdit 和子网掩码 QLineEdit；"声音设置"面板包含音量滑块 QSlider 和静音 QCheckBox；"高级选项"面板默认被 setItemEnabled(false) 禁用。窗口底部有一个 QCheckBox "启用高级选项"，勾选后通过 setItemEnabled(true) 解锁高级面板。右侧是一个 QTextEdit 作为日志区域，通过 currentChanged 信号在每次面板切换时记录"用户切换到 XXX 面板"。关闭窗口时使用 QSettings 保存当前展开的面板索引，下次启动时恢复。

提示：保存当前索引用 toolbox->currentIndex()，恢复时用 toolbox->setCurrentIndex()。注意 setCurrentIndex 需要在所有面板都 addItem 完毕之后再调用。

## 6. 官方文档参考链接

[Qt 文档 -- QToolBox](https://doc.qt.io/qt-6/qtoolbox.html) -- 工具箱折叠面板控件

[Qt 文档 -- QTabWidget](https://doc.qt.io/qt-6/qtabwidget.html) -- 标签页控件（对比参考）

---

到这里，QToolBox 的核心用法就全部讲完了。addItem/insertItem 构成了面板的动态管理能力，currentChanged 信号让你能响应面板切换事件，setItemEnabled 提供了按需禁用面板的手段，而侧边栏导航场景是 QToolBox 的主战场。如果你在设计一个工具型应用的侧边栏，需要在有限的空间里展示多个分类的功能面板，QToolBox 的"标题栏始终可见、内容按需展开"的交互模式几乎是最自然的选择。
