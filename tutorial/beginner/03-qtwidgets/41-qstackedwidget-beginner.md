# 现代Qt开发教程（新手篇）3.41——QStackedWidget：堆叠页面控件

## 1. 前言 / 那个没有标签头却撑起了半个 Qt 导航体系的控件

QStackedWidget 大概是 Qt 里最"低调"的多页容器。它不像 QTabWidget 那样自带一排醒目的标签栏，也不像 QToolBox 那样有可折叠的分区标题——它什么导航控件都没有，就只是一堆 QWidget 叠在一起，你在代码里通过索引切换当前显示哪一个，其余的全部隐藏。听起来简陋得不像话，但 QStackedWidget 的出场频率远比你想象的高。QTabWidget 内部的页面区域就是它，QWizard（向导对话框）的页面管理也靠它，各种设置界面中左侧是导航列表、右侧是切换页面的经典布局还是它。它之所以被广泛使用，恰恰是因为它不绑定任何导航 UI——你可以自由选择用 QComboBox、QListWidget、QTreeWidget、按钮组甚至自定义的手势操作来充当导航，QStackedWidget 只负责页面的显示和隐藏。

今天的内容分四个部分：addWidget 添加页面和 setCurrentIndex 切换页面的基本用法，QStackedWidget 与 QComboBox 或 QListWidget 组合构建导航菜单的实战模式，currentChanged 信号的响应机制，以及它和 QTabWidget 的根本区别与各自的适用场景。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QStackedWidget 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。示例代码中用到了 QStackedWidget、QComboBox、QListWidget、QLabel、QLineEdit、QSpinBox、QCheckBox、QPushButton、QVBoxLayout、QHBoxLayout 和 QSplitter。

## 3. 核心概念讲解

### 3.1 addWidget 添加页面与 setCurrentIndex 切换页面

QStackedWidget 的核心概念极为简单——它维护一个 QWidget 列表，同一时刻只显示其中一个，其余全部隐藏。addWidget(QWidget *page) 把一个 QWidget 添加到列表末尾，insertWidget(int index, QWidget *page) 在指定位置插入，removeWidget(QWidget *page) 从列表中移除（但不 delete）。切换当前显示页面的方法有两个：setCurrentIndex(int index) 按索引切换，setCurrentWidget(QWidget *widget) 按 QWidget 指针切换。

```cpp
auto *stack = new QStackedWidget;

auto *page1 = new QWidget;
auto *page2 = new QWidget;
auto *page3 = new QWidget;

stack->addWidget(page1);  // 索引 0
stack->addWidget(page2);  // 索引 1
stack->addWidget(page3);  // 索引 2

// 切换到第 2 个页面
stack->setCurrentIndex(1);
// 或者按指针切换
stack->setCurrentWidget(page3);
```

currentIndex() 返回当前显示页面的索引（从 0 开始），currentWidget() 返回当前显示页面的 QWidget 指针。如果 QStackedWidget 中没有任何页面，currentIndex() 返回 -1。

addWidget 的返回值是这个页面被分配的索引。大多数情况下索引就是 addWidget 的调用顺序——第 N 次 addWidget 得到索引 N-1。但如果你在中间用了 insertWidget，后续 addWidget 的索引仍然是追加到末尾。indexOf(QWidget *page) 可以随时查询某个页面的索引，返回 -1 说明这个页面不在 QStackedWidget 中。

removeWidget(QWidget *page) 只是从 QStackedWidget 的管理列表中移除页面，页面控件本身不会被 delete。这和 QTabWidget 的 removeTab 行为一致——你需要自己管理页面的生命周期。一个常见的模式是：removeWidget 之后紧接着 delete 页面指针。

```cpp
QWidget *page = stack->currentWidget();
stack->removeWidget(page);
delete page;
```

count() 返回页面总数。widget(int index) 返回指定索引的页面指针，index 超出范围时返回 nullptr。

一个值得注意的细节：QStackedWidget 在切换页面时，会把旧页面调用 hide()，把新页面调用 show()。这意味着如果你的页面控件有需要在显示时刷新的内容（比如重新读取数据、更新列表），你不能依赖构造函数里的一次性初始化——应该监听 QStackedWidget 的 currentChanged 信号，在页面被切换到前台时执行刷新逻辑。

还有一个容易忽略的点是 QStackedWidget 的尺寸策略。QStackedWidget 的默认 sizePolicy 是 (Preferred, Preferred)，它在计算自身大小时会取所有页面中最大的那个作为建议尺寸——也就是说，如果第 1 个页面很小但第 2 个页面很大，QStackedWidget 会按照第 2 个页面的尺寸来汇报给父布局。这个行为在大多数情况下是合理的——你不会希望切换页面时窗口大小跟着跳来跳去。但如果你希望 QStackedWidget 的大小跟随当前页面而不是所有页面中的最大值，需要手动处理——比如在 currentChanged 槽函数中调整窗口大小。

### 3.2 与 QComboBox / QListWidget 组合做导航

QStackedWidget 没有自带的导航 UI，所以实际使用时它总是和某个导航控件搭配出现。最常见的两种搭配是 QComboBox 和 QListWidget。

QComboBox + QStackedWidget 的模式适合页面数量较少（3-8 个）且导航入口不需要占用太多空间的场景——比如一个设置对话框的顶部，用下拉框选择不同类别，下方切换对应的设置页面。实现方式很直接：QComboBox 中每一项的索引和 QStackedWidget 中页面的索引保持一致，QComboBox 的 currentIndexChanged 信号直接连接到 QStackedWidget 的 setCurrentIndex 槽。

```cpp
auto *combo = new QComboBox;
combo->addItems({"个人信息", "网络设置", "外观", "高级"});

auto *stack = new QStackedWidget;
stack->addWidget(createPersonalPage());
stack->addWidget(createNetworkPage());
stack->addWidget(createAppearancePage());
stack->addWidget(createAdvancedPage());

// 一行信号连接搞定导航
connect(combo, &QComboBox::currentIndexChanged,
        stack, &QStackedWidget::setCurrentIndex);
```

这就是全部代码了。QComboBox 的索引从 0 开始，QStackedWidget 的索引也从 0 开始，两者完全对齐，不需要任何转换逻辑。这种简洁性是 QStackedWidget 配合索引型导航控件的核心优势。

QListWidget + QStackedWidget 的模式适合页面数量较多或者需要更丰富的导航展示的场景。QListWidget 的每一项可以显示文字和图标，选中某一项后切换 QStackedWidget 到对应页面。信号连接方式和 QComboBox 一样——currentRowChanged(int) 直接连 setCurrentIndex(int)。

```cpp
auto *list = new QListWidget;
list->addItem("个人信息");
list->addItem("网络设置");
list->addItem("外观");
list->addItem("高级");

auto *stack = new QStackedWidget;
// ... 添加页面 ...

connect(list, &QListWidget::currentRowChanged,
        stack, &QStackedWidget::setCurrentIndex);
```

这两种模式的核心逻辑完全一样——导航控件发出索引变化的信号，QStackedWidget 接收索引并切换页面。区别只在于导航控件的视觉呈现：QComboBox 是下拉框，节省空间；QListWidget 是列表，可以展示更多信息（图标、描述文字、分组等）。

实际项目中更常见的是用 QTreeWidget 或者自定义的侧边栏导航来替代 QListWidget——逻辑是一样的，只是导航的层级结构更复杂。Qt Creator 的"选项"对话框、系统设置应用的主界面，都是这种"左侧导航 + 右侧 QStackedWidget"的布局。

有一个实用的布局技巧：把导航控件和 QStackedWidget 放在一个 QHBoxLayout 中，导航控件在左侧设置固定宽度，QStackedWidget 在右侧占据剩余空间。可以用 QSplitter 来让用户自由调节左右比例，也可以直接用 QHBoxLayout + setStretch 因子来固定比例。

```cpp
auto *layout = new QHBoxLayout;
layout->addWidget(list, 1);      // 导航列表占 1 份
layout->addWidget(stack, 3);     // 内容区域占 3 份
```

### 3.3 currentChanged 信号响应页面切换

QStackedWidget 在当前页面切换时发出 currentChanged(int index) 信号，参数是新页面的索引。这个信号在你需要在页面切换时执行额外逻辑的场景下很重要——比如切换到某个页面时刷新数据、保存前一个页面的状态、更新窗口标题等。

```cpp
connect(stack, &QStackedWidget::currentChanged,
        this, [this](int index) {
    if (index < 0) return;

    // 根据当前页面更新窗口标题
    QString title = m_pageNames.value(index, "未知");
    setWindowTitle(QString("设置 — %1").arg(title));

    // 切换到"网络设置"页面时自动刷新 DNS 列表
    if (index == 1) {
        refreshDnsList();
    }
});
```

currentChanged 在 setCurrentIndex 和 setCurrentWidget 切换时都会触发。如果你需要在某些场景下切换页面但不触发信号——比如初始化阶段批量设置页面状态——可以用 blockSignals(true) 临时屏蔽。

```cpp
stack->blockSignals(true);
stack->setCurrentIndex(0);
// ... 其他初始化操作 ...
stack->blockSignals(false);
```

和 currentChanged 配合使用的还有 widgetRemoved(int index) 信号（Qt 5.15+ 引入），在 removeWidget 执行后发出。这个信号在需要维护索引映射的场景下很有用——比如你有一个 QMap 记录每个索引对应的页面名称，当某个页面被移除后需要更新映射关系。

还有一个容易忽略的场景：当 QStackedWidget 只有一个页面时，setCurrentIndex(0) 不会触发 currentChanged——因为"当前页面"没有发生变化。如果你依赖 currentChanged 来做初始化，需要确保初始化逻辑不是只在信号触发时才执行。

### 3.4 区别于 QTabWidget（无标签头）

QStackedWidget 和 QTabWidget 的核心区别可以归结为一点：QStackedWidget 没有导航头。

QTabWidget = QTabBar（导航标签栏） + QStackedWidget（页面容器） + 内部连接逻辑。它把导航和内容绑定在一起，优点是开箱即用——addTab 一行代码同时添加了标签和页面，currentChanged 同时响应了标签切换和页面切换。缺点是灵活性受限于 QTabBar 的能力——标签栏只能在四个固定位置（上/下/左/右），标签的样式定制需要通过 QTabBar 的有限接口或者 QSS。

QStackedWidget 去掉了导航层，只保留页面容器。这意味着你可以选择任何控件来充当导航——QComboBox、QListWidget、QTreeWidget、QPushButton 组成的按钮组、甚至是一个自定义的手势识别器。导航控件和页面容器之间的连接通过信号槽实现，你完全掌控连接逻辑。

选择 QTabWidget 的场景很明确：需要标准标签页导航，标签在上方/下方/左侧/右侧，没有特殊的导航定制需求。一行 addTab 搞定一切，开发效率高。

选择 QStackedWidget 的场景包括：导航控件不是标签栏（比如下拉框、侧边列表、树形导航），导航控件和页面区域之间需要插入其他控件（比如工具栏、搜索框），一个导航控件需要同时控制多个区域的内容，或者导航逻辑比较复杂（比如某些页面之间有前置条件、跳转逻辑）。

从实现复杂度来说，QStackedWidget 配合导航控件的代码量比 QTabWidget 多几行——你需要手动创建导航控件、添加导航项、连接信号。但这几行代码换来的是完全的布局自由度和导航灵活性。在复杂应用中，这种灵活性几乎是必需的。

另外一个细微但重要的区别是 QStackedWidget 在页面切换时的动画支持。QStackWidget 本身不提供切换动画，但你可以利用 QPropertyAnimation 或者 QGraphicsOpacityEffect 来自己实现淡入淡出、滑动等过渡效果。QTabWidget 在 Qt 6.x 中也不提供内置的标签页切换动画。所以在这方面两者没有本质区别——想要动画都得自己写。

## 4. 踩坑预防

第一个坑是 removeWidget 不会 delete 页面控件。这和 QTabWidget 的 removeTab 行为一致——如果你 new 了 QWidget 放进 QStackedWidget，之后 removeWidget 只是从管理列表中移除，控件仍然存在于内存中。如果你忘了 delete，就是内存泄漏。正确做法是 removeWidget 之后紧接着 delete 取出的 widget 指针，或者一开始就用 Qt 的父子对象树来管理生命周期——把 QStackedWidget 设为页面控件的 parent，但注意 removeWidget 并不会触发 deleteLater，你还是得手动 delete。

第二个坑是 setCurrentIndex 在索引不变时不会触发 currentChanged。如果当前已经是第 0 个页面，再次调用 setCurrentIndex(0) 不会有任何效果——信号不会发出。如果你的初始化逻辑依赖 currentChanged 来刷新页面，在首次 setCurrentIndex 之前需要确认信号是否会被触发。一个稳妥的做法是在初始化完成后手动调用一次刷新函数，而不是完全依赖信号。

第三个坑是 QStackedWidget 的大小计算基于所有页面中的最大值。如果你的页面尺寸差异很大（比如第一个页面只有几行输入框，第二个页面有一整棵树），QStackedWidget 会按照大树页面的尺寸来汇报大小。在小页面上会看到大量留白。解决方法是在 currentChanged 槽中调用 adjustSize() 或者设置 QStackedWidget 的 sizePolicy 为 QSizePolicy::Minimum，让窗口大小跟随当前页面。

第四个坑是 addWidget 的顺序必须和导航控件的项顺序完全一致。如果你先往 QComboBox 里加了 4 个项目，再往 QStackedWidget 里 addWidget 了 4 个页面，两者的索引天然对齐。但如果中间做了一次 insertWidget 或者 insertItem，某一边的索引发生了偏移而另一边没有同步，就会出现"选导航项 A 却显示页面 B"的错位问题。建议封装一个 addPage(name, widget) 方法来保证两边同步添加。

第五个坑是 QStackedWidget 在没有任何页面时 currentIndex 返回 -1，widget(-1) 会返回 nullptr。如果你在代码中没有检查空容器就直接用 currentIndex() 去访问 widget()，程序可能崩溃。防御性的做法是在使用 widget(currentIndex()) 之前先检查 count() > 0。

## 5. 练习项目

我们来做一个综合练习：创建一个"系统偏好设置"窗口，采用经典的左侧导航 + 右侧内容区布局。左侧使用 QListWidget 作为导航列表，包含五个项目："个人信息"、"网络设置"、"外观偏好"、"快捷键"、"关于"。右侧使用 QStackedWidget，每个导航项对应一个内容页面。"个人信息"页面包含用户名 QLineEdit 和邮箱 QLineEdit；"网络设置"页面包含代理地址 QLineEdit 和端口 QSpinBox；"外观偏好"页面包含深色模式 QCheckBox 和字体大小 QSpinBox；"快捷键"页面显示一个说明标签"此功能开发中"；"关于"页面显示版本信息和版权声明。QListWidget 的 currentRowChanged 信号直接连接 QStackedWidget 的 setCurrentIndex 槽。顶部添加一个 QComboBox 允许快速跳转到指定页面，currentIndexChanged 同样连接到 setCurrentIndex。窗口标题通过 currentChanged 信号动态更新为"系统偏好设置 — [当前页面名]"。

提示：QComboBox 和 QListWidget 的索引变化信号都可以连到同一个 setCurrentIndex 槽，但两者之间需要互相同步——切换 QComboBox 时需要同步 QListWidget 的当前项，反之亦然。可以用 blockSignals 避免循环触发。

## 6. 官方文档参考链接

[Qt 文档 -- QStackedWidget](https://doc.qt.io/qt-6/qstackedwidget.html) -- 堆叠页面控件

[Qt 文档 -- QComboBox](https://doc.qt.io/qt-6/qcombobox.html) -- 下拉框控件（组合导航参考）

[Qt 文档 -- QListWidget](https://doc.qt.io/qt-6/qlistwidget.html) -- 列表控件（侧边导航参考）

---

到这里，QStackedWidget 的核心用法就全部讲完了。它可能是 Qt 里最纯粹的页面容器——没有任何导航装饰，只做一件事：把一堆 QWidget 叠在一起，按索引显示其中一个。这种极简设计让它成为自定义导航布局的理想基石。无论是 QComboBox + QStackedWidget 的紧凑设置对话框，还是 QListWidget + QStackedWidget 的经典左右分栏，或者 QTreeWidget + QStackedWidget 的层级导航，核心都是同一个模式：导航控件发出索引变化信号，QStackedWidget 接收信号切换页面。掌握了 QStackedWidget，你就掌握了 Qt 中所有自定义多页导航的基础构建块。
