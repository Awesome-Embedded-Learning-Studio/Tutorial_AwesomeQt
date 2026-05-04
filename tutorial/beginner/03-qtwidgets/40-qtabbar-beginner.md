# 现代Qt开发教程（新手篇）3.40——QTabBar：独立标签栏

## 1. 前言 / 当你需要的是一个标签栏而不是标签页容器

上一讲我们完整拆解了 QTabWidget——它把标签栏和页面区域打包在一起，用起来很方便。但 QTabWidget 的封装也意味着你没法自由控制标签栏和内容区域的布局关系——标签栏永远在上方（或者下方/左右），内容区域永远是 QStackedWidget，标签栏的样式定制也只能通过 QTabWidget 暴露出来的有限接口来操作。有些场景下这种封装反而成了束缚。

比如你想做一个 IDE 风格的界面——顶部是一个独立的标签栏，下方分成左右两个面板，标签切换同时影响两个面板的内容。QTabWidget 做不到这一点，因为它内部只有一个 QStackedWidget。再比如你想把标签栏放在窗口的任意位置，或者标签栏和内容区域之间需要插入工具栏、分隔线之类的控件，QTabWidget 的固定布局也无法满足。

这就是 QTabBar 登场的场景。QTabBar 是 QTabWidget 内部使用的那个标签栏控件的独立版本——你可以单独创建它，放在任何布局位置，搭配任何内容展示方式。标签栏本身只负责标签的显示和选择逻辑，不关心标签背后展示什么内容。这种解耦给了你完全的自由度：你可以用 QTabBar 配合 QStackedWidget 复刻 QTabWidget 的行为，也可以用 QTabBar 配合自定义面板实现 QTabWidget 做不到的布局。

今天的内容分四个部分：QTabBar 与 QTabWidget 的本质区别和使用场景，自定义标签栏加自定义内容区域的组合方式，tabCloseRequested 信号实现可关闭标签页，以及 setMovable 实现可拖动标签排序。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QTabBar 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。示例代码中用到了 QTabBar、QStackedWidget、QLabel、QLineEdit、QTextEdit、QPushButton、QVBoxLayout、QHBoxLayout 以及 QSplitter。

## 3. 核心概念讲解

### 3.1 QTabBar 与 QTabWidget 的区别

QTabWidget 和 QTabBar 的关系可以用一句话概括：QTabWidget 是一个"标签栏 + 堆叠页面"的组合容器，而 QTabBar 只是那个"标签栏"本身。

QTabWidget 内部持有一个 QTabBar 实例（通过 tabBar() 方法可以访问）和一个 QStackedWidget 实例。当你调用 addTab 时，QTabWidget 同时往 QTabBar 里添加一个标签、往 QStackedWidget 里添加一个页面，再把两者通过内部信号连接起来——用户点击 QTabBar 上的标签时，QTabWidget 自动切换 QStackedWidget 到对应页面。

QTabBar 去掉了那层封装。它只管理标签的显示、选择、增删——没有内嵌的页面容器，不会帮你切换任何内容。你需要自己监听 QTabBar 的 currentChanged 信号，在槽函数里决定内容区域显示什么。这听起来多了一步手工活，但换来的好处是内容区域完全由你控制。

```cpp
// QTabWidget 的方式——一行搞定标签和页面
auto *tabs = new QTabWidget;
tabs->addTab(page1, "标签1");
tabs->addTab(page2, "标签2");

// QTabBar + 自定义内容的方式——需要手动连接
auto *tabBar = new QTabBar;
tabBar->addTab("标签1");
tabBar->addTab("标签2");

auto *stackedWidget = new QStackedWidget;
stackedWidget->addWidget(page1);
stackedWidget->addWidget(page2);

connect(tabBar, &QTabBar::currentChanged,
        stackedWidget, &QStackedWidget::setCurrentIndex);
```

上面这段代码用 QTabBar + QStackedWidget 复刻了 QTabWidget 的基本行为。看起来代码量多了一点，但你在标签栏和堆叠页面之间获得了完全的控制权——你可以在两者之间插入任何控件，可以用 QSplitter 把内容区域拆分成多个面板，甚至可以让一个标签栏同时控制多个内容区域。

选择 QTabWidget 还是 QTabBar 的判断标准很简单：如果你的需求就是标准的"点标签切页面"，直接用 QTabWidget，省事；如果你需要对标签栏和内容区域分别做定制，或者标签栏的布局位置不在 QTabWidget 支持的范围内，就用 QTabBar 自己组合。

### 3.2 自定义标签栏 + 自定义内容区域组合

这一节我们来做一个比 QTabWidget 更灵活的布局：顶部一个 QTabBar，中间用 QSplitter 把内容区域分成左右两个面板，标签切换时左面板显示详情编辑区、右面板显示预览区。这种布局在编辑器、IDE、文档管理器中很常见。

```cpp
auto *tabBar = new QTabBar;
tabBar->addTab("文档 1");
tabBar->addTab("文档 2");
tabBar->addTab("文档 3");

// 左侧: 编辑区域
auto *editorStack = new QStackedWidget;
editorStack->addWidget(new QTextEdit);
editorStack->addWidget(new QTextEdit);
editorStack->addWidget(new QTextEdit);

// 右侧: 预览区域
auto *previewStack = new QStackedWidget;
previewStack->addWidget(new QLabel("预览: 文档 1"));
previewStack->addWidget(new QLabel("预览: 文档 2"));
previewStack->addWidget(new QLabel("预览: 文档 3"));

auto *splitter = new QSplitter(Qt::Horizontal);
splitter->addWidget(editorStack);
splitter->addWidget(previewStack);

// 标签切换同步驱动两个堆叠页面
connect(tabBar, &QTabBar::currentChanged,
        this, [editorStack, previewStack](int index) {
    if (index < 0) return;
    editorStack->setCurrentIndex(index);
    previewStack->setCurrentIndex(index);
});
```

这段代码的核心在于 QTabBar 的 currentChanged 信号同时驱动了两个 QStackedWidget——一个负责编辑区，一个负责预览区。用 QTabWidget 你没法实现这种"一个标签栏控制两个内容面板"的需求，因为 QTabWidget 内部只持有一个 QStackedWidget。

组合使用时需要注意 QTabBar 和 QStackedWidget 的索引同步问题。addTab 和 addWidget 都是从 0 开始编号的，只要你按相同顺序添加标签和页面，索引就自然对齐。但如果你做了 insertTab 或者 removeTab 操作，就需要确保 QStackedWidget 也同步插入或移除对应的页面，否则索引会错位。一个好的实践是写一个同步函数来统一管理增删操作。

```cpp
/// @brief 同步添加标签页和堆叠页面
void addDocumentTab(QTabBar *bar, QStackedWidget *stack,
                    const QString &title, QWidget *editorPage,
                    QWidget *previewPage)
{
    int index = bar->addTab(title);
    // insertWidget 保证索引和标签对齐
    stack->insertWidget(index, editorPage);
}
```

QTabBar 的布局位置完全由你决定——不像 QTabWidget 只能选上/下/左/右。你可以把它放在 QVBoxLayout 的顶部、放在工具栏里、放在状态栏旁边，甚至可以用 QHBoxLayout 把它和按钮、搜索框排在同一行。这种布局自由度是 QTabBar 相比 QTabWidget 最大的优势。

### 3.3 tabCloseRequested 可关闭标签页

和 QTabWidget 一样，QTabBar 也支持在标签上显示关闭按钮。调用 setTabsClosable(true) 后，每个标签的右侧会出现一个小的关闭图标。用户点击关闭图标时，QTabBar 发出 tabCloseRequested(int index) 信号。和 QTabWidget 一样，QTabBar 不会自动删除标签——你需要在槽函数里自己处理。

```cpp
tabBar->setTabsClosable(true);

connect(tabBar, &QTabBar::tabCloseRequested,
        this, [tabBar](int index) {
    tabBar->removeTab(index);
    // 如果有对应的内容页面，也需要同步移除
});
```

这里有一个 QTabBar 独有的细节需要注意：QTabBar 本身不管理任何页面控件，所以 removeTab 之后不需要 delete 任何 QWidget——因为 QTabBar 里根本没有 QWidget。你只需要调用 removeTab 把标签从栏中移除即可。但如果你的代码里用 QStackedWidget 或其他容器和 QTabBar 配合使用，别忘了同步从那些容器中移除对应的页面并 delete。

关闭最后一个标签后，QTabBar 会变成一个空栏——高度还在，但没有标签可以点击。currentChanged 会发出一次 index == -1。和 QTabWidget 的处理方式一样，你的槽函数必须检查 index < 0。

一个实用的交互模式是在关闭标签前弹出确认对话框，特别是当标签对应的编辑器有未保存内容时。你可以在 tabCloseRequested 的槽函数中检查文档是否有修改，如果有则弹出 QMessageBox 让用户确认。

```cpp
connect(tabBar, &QTabBar::tabCloseRequested,
        this, [this, tabBar](int index) {
    if (hasUnsavedChanges(index)) {
        auto result = QMessageBox::question(
            this, "关闭确认",
            "当前文档有未保存的修改，确定关闭吗？",
            QMessageBox::Yes | QMessageBox::No);
        if (result != QMessageBox::Yes) return;
    }
    tabBar->removeTab(index);
    // 同步清理内容区域...
});
```

另外，setTabButton(int index, QTabBar::ButtonPosition position, QWidget *widget) 允许你在标签的左侧或右侧放置自定义控件——不只是默认的关闭按钮，还可以是进度条、状态指示器、下拉菜单等等。这个功能在自定义标签外观时非常强大，我们示例代码里也会演示。

### 3.4 setMovable 可拖动标签排序

QTabBar 的 setMovable(bool) 控制用户是否可以通过拖拽标签来重新排列标签顺序。设为 true 后，用户可以按住一个标签向左或向右拖动来改变它在标签栏中的位置。

```cpp
tabBar->setMovable(true);
```

拖动排序在浏览器式标签页管理中是标配功能。实现原理上，QTabBar 内部在检测到鼠标拖动超过一定阈值后，会计算目标插入位置，然后调用 moveTab(int from, int to) 在内部数据结构中交换标签位置，并触发 tabMoved(int from, int to) 信号。

tabMoved 信号在标签被拖动到新位置后发出，参数是原始索引和目标索引。如果你用 QTabBar 配合 QStackedWidget，需要在 tabMoved 的槽函数中同步移动 QStackedWidget 里的页面——否则标签和页面会错位。

```cpp
connect(tabBar, &QTabBar::tabMoved,
        this, [this](int from, int to) {
    // 同步移动堆叠页面中的控件
    QWidget *page = m_stackedWidget->widget(from);
    m_stackedWidget->removeWidget(page);
    m_stackedWidget->insertWidget(to, page);
    // 确保当前显示的页面仍然是正确的
    m_stackedWidget->setCurrentIndex(tabBar->currentIndex());
});
```

这段代码的逻辑是先从 QStackedWidget 中取出原始位置的页面控件，删除它，然后插入到新位置。最后用 setCurrentIndex 确保堆叠页面显示的是当前标签对应的页面。这里有一个容易出错的地方：removeWidget 之后 QStackedWidget 的页面索引会重新排列——如果 from < to，移除 from 位置的页面后，to 位置的页面实际上变成了 to - 1。insertWidget 中的 to 参数应该用移动后的目标索引。幸运的是，QTabBar::tabMoved 的 to 参数已经是调整后的正确目标索引，所以直接用就行。

setMovable 默认值为 false——也就是说标准 QTabBar 不允许拖动排序。如果你在用 QTabWidget，QTabWidget::setMovable 实际上就是在调用内部 QTabBar 的 setMovable。两套 API 行为完全一致。

拖动排序和 setTabsClosable(true) 可以同时使用，但有一个已知的交互问题：如果用户在关闭按钮上按下鼠标后稍微移动了一下，QTabBar 可能会把这次操作识别为拖动而不是点击关闭按钮。Qt 6.9 中这个问题有所改善但没有完全解决。如果你的应用对关闭操作要求精确，可以考虑把关闭按钮做成自定义的 QTabBar::RightSide widget 来获得更精确的事件控制。

## 4. 踩坑预防

第一个坑是 QTabBar 和 QStackedWidget 的索引同步问题。每次你对 QTabBar 做 addTab、insertTab、removeTab 操作时，必须同步对 QStackedWidget 做对应的 addWidget、insertWidget、removeWidget 操作。如果忘记同步，标签和页面就会错位——点击"标签 2"显示的是"页面 3"的内容。建议封装一个统一的同步管理方法来避免遗漏。

第二个坑是 tabMoved 信号触发后 QStackedWidget 的同步移动。removeWidget 会改变后续页面的索引，moveTab 不会。两者组合时需要小心索引偏移。最稳妥的做法是先取出 QWidget 指针，removeWidget，再 insertWidget 到目标位置。

第三个坑是 QTabBar 为空时的 currentChanged(-1)。所有标签被关闭后 QTabBar 变成空栏，currentChanged 发出 index == -1。如果你的代码中有 widget(index) 之类的调用而没有检查 index < 0，程序会崩溃。

第四个坑是 setTabButton 放置的自定义控件在标签被 removeTab 后不会被自动 delete。QTabBar::removeTab 会把标签从栏中移除，但标签上通过 setTabButton 放置的自定义 QWidget 仍然存在于内存中。你需要在 removeTab 之前先取出并 delete 这些自定义控件，否则会有内存泄漏。

第五个坑是 QTabBar 的 setDrawBase(bool)。默认情况下 QTabBar 会绘制一个基线（base line）——在标签下方的一条水平线，用来在视觉上区分标签栏和内容区域。如果你把 QTabBar 嵌在工具栏或者自定义布局中，这条基线可能显得多余。调用 setDrawBase(false) 可以隐藏它。

## 5. 练习项目

我们来做一个综合练习：创建一个简易文档编辑器窗口，顶部用 QTabBar 作为文档标签栏，下方用 QSplitter 把窗口分成左右两栏——左栏是 QTextEdit 编辑区，右栏是 QLabel 预览区。初始包含三个文档标签。标签栏开启 tabsClosable 和 movable。点击关闭按钮时弹出确认对话框，确认后同步移除标签和对应的堆叠页面。标签栏下方添加一排按钮——"新建文档"按钮调用 insertTab + insertWidget 在当前标签之后插入新标签和页面，"关闭全部"按钮循环 removeTab 清空所有标签。拖动标签时通过 tabMoved 信号同步移动堆叠页面中的页面控件。窗口右下角显示当前标签的索引和总数。

提示：拖动同步的关键是 tabMoved 槽函数中先 removeWidget 再 insertWidget，并最后 setCurrentIndex(currentIndex())。

## 6. 官方文档参考链接

[Qt 文档 -- QTabBar](https://doc.qt.io/qt-6/qtabbar.html) -- 标签栏控件

[Qt 文档 -- QTabWidget](https://doc.qt.io/qt-6/qtabwidget.html) -- 标签页控件（对比参考）

[Qt 文档 -- QTabBar::ButtonPosition](https://doc.qt.io/qt-6/qtabbar.html#ButtonPosition-enum) -- 标签按钮位置枚举

---

到这里，QTabBar 的核心用法就全部讲完了。它和 QTabWidget 的本质区别在于"标签栏和内容区域是否耦合"——QTabWidget 打包了一切，简单但不灵活；QTabBar 只管标签，内容区域由你决定。当你需要自定义标签栏和内容区域之间的布局关系、一个标签栏同时控制多个内容面板、或者标签栏的位置不在 QTabWidget 支持的四种固定位置时，QTabBar 就是正确的选择。tabCloseRequested 和 setMovable 提供了标签级别的交互能力，配合 setTabButton 你甚至可以在标签上嵌入自定义控件。掌握了 QTabBar，你就拥有了比 QTabWidget 更精细的标签页控制权。
