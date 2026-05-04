# 现代Qt开发教程（新手篇）3.57——QToolBar：工具栏

## 1. 前言 / 工具栏不是菜单的翻版

工具栏这个东西，新手写 Qt 的时候往往对它缺乏敬畏心——往 addToolBar 里塞几个 QAction 就完事了，和菜单栏操作一回事嘛。但真到实战里你会发现，工具栏的行为远比菜单栏复杂：它可以被用户拖到窗口的任意一侧停靠、可以拖出来变成浮动窗口、可以调整按钮的显示风格（只显示图标、只显示文字、或者两者都显示）、当控件太多放不下时还会自动生成溢出菜单。这些交互细节如果你不在设计阶段就考虑清楚，等到用户抱怨"工具栏拖不动""按钮太小看不清文字""窄屏上工具栏直接截断了一半"的时候再去修，改动量会相当大。

QToolBar 是 QMainWindow 体系中专门管理工具栏的类。它和 QMenuBar 最大的区别在于，QMenuBar 是固定在窗口顶部的静态容器，而 QToolBar 是一个可以动态移动、浮动、调整大小的交互组件。QToolBar 的内容也不局限于 QAction——你可以通过 addWidget 向工具栏中塞入任意的 QWidget（比如 QComboBox、QSpinBox、QLineEdit），这给了工具栏极大的灵活性，但同时也带来了一些布局管理上的陷阱。

今天我们从四个方面展开。先看 addAction / addWidget / addSeparator 这三种往工具栏中添加内容的方式，然后研究 setMovable / setFloatable 控制的停靠行为，接着讨论 setIconSize / setToolButtonStyle 如何调整按钮的视觉呈现，最后处理工具栏宽度不足时的溢出菜单问题。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QToolBar 在 QtWidgets 模块中，链接 Qt6::Widgets 即可。示例代码涉及 QToolBar、QMainWindow、QAction、QLabel、QComboBox、QSpinBox、QLineEdit、QPlainTextEdit、QToolButton、QStyle、QKeySequence 和 QMenu。

## 3. 核心概念讲解

### 3.1 addAction / addWidget / addSeparator：工具栏的三种内容类型

QToolBar 内部的内容可以归纳为三类：动作项（QAction）、自定义控件（QWidget）和分隔线。三种内容通过不同的方法添加，在工具栏中的布局行为各有差异。

addAction 是最常用的添加方式。它把一个 QAction 以按钮的形式显示在工具栏中——用户点击这个按钮，就触发了 QAction 的 triggered 信号。和菜单栏一样，同一个 QAction 可以同时添加到菜单和工具栏中，两处的状态（enabled、checked、图标等）自动同步。这是 QAction 共享设计的核心优势：你不需要分别管理菜单项和工具栏按钮的状态。

```cpp
// 创建共享的 QAction
auto *newAction = new QAction("新建", this);
newAction->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
newAction->setShortcut(QKeySequence::New);

// 同时添加到菜单和工具栏
fileMenu->addAction(newAction);
fileToolBar->addAction(newAction);
```

addWidget 允许你向工具栏中插入任意的 QWidget。这个功能在需要放置非按钮类型的交互控件时非常有用——比如字体选择的 QComboBox、缩放比例的 QSpinBox、搜索框 QLineEdit。addWidget 会把传入的 QWidget 直接嵌入到工具栏的布局中，控件的大小由其 sizeHint 决定（但会被工具栏的布局约束压缩）。

```cpp
// 在工具栏中嵌入一个字体大小选择器
auto *sizeLabel = new QLabel(" 字号: ");
toolBar->addWidget(sizeLabel);

auto *fontSizeCombo = new QComboBox;
fontSizeCombo->addItems({"10", "12", "14", "16", "18"});
fontSizeCombo->setCurrentText("14");
fontSizeCombo->setMaximumWidth(65);
toolBar->addWidget(fontSizeCombo);

connect(fontSizeCombo, &QComboBox::currentTextChanged,
        this, [this](const QString &text) {
    bool ok = false;
    int size = text.toInt(&ok);
    if (ok && size > 0) {
        auto font = m_editor->font();
        font.setPointSize(size);
        m_editor->setFont(font);
    }
});
```

这里有一个不太显眼但很重要的细节：addWidget 返回一个 QAction 指针。这个 QAction 是一个"控件动作"——它不是真正的 QAction，而是 Qt 内部为了把 QWidget 统一到 QToolBar 的 QAction 列表中而创建的包装对象。你可以用这个返回值来动态移除控件（removeAction），或者调整控件在工具栏中的位置。

addSeparator 在工具栏中插入一条垂直分隔线（当工具栏水平排列时）或水平分隔线（当工具栏垂直排列时）。分隔线的用途是视觉分组——把逻辑上相关的按钮聚在一起，不同组之间用分隔线隔开。比如"新建/打开/保存"是一组操作文件的动作，和后面的"撤销/重做"之间用分隔线隔开。

```cpp
auto *fileToolBar = addToolBar("文件");
fileToolBar->addAction(newAction);
fileToolBar->addAction(openAction);
fileToolBar->addAction(saveAction);
fileToolBar->addSeparator();
fileToolBar->addAction(undoAction);
fileToolBar->addAction(redoAction);
```

工具栏中内容的排列顺序完全由添加顺序决定。addAction 和 addWidget 都是追加到末尾的。如果你需要在特定位置插入，可以使用 insertAction 和 insertWidget。removeAction 可以移除指定的 QAction（包括 separator action 和 widget action）。

### 3.2 setMovable / setFloatable：停靠行为控制

QToolBar 默认是可移动的（movable）——用户可以拖拽工具栏的把手（一条竖向的虚线或点状区域），把它拖到 QMainWindow 的任意一侧停靠，或者拖到窗口外面让它变成一个独立的浮动窗口。这个默认行为在大多数应用中是合理的，但某些场景下你可能需要限制它。

setMovable(bool) 控制工具栏是否可以被用户拖动。设为 false 后，工具栏固定在 addToolBar 时指定的位置（或者 restoreState 恢复的位置），用户无法拖动它。这在工具栏位置对布局至关重要的场景中有用——比如一个包含关键导航控件的顶部工具栏，你不希望用户不小心把它拖到底部去。

```cpp
auto *primaryToolBar = addToolBar("主工具栏");
primaryToolBar->setMovable(false);  // 固定在顶部，不允许拖动
```

setFloatable(bool) 控制工具栏是否可以浮动。设为 false 后，工具栏仍然可以被拖动到窗口的其他边缘停靠，但不能拖出窗口变成浮动窗口。如果你的应用有多个工具栏，不希望用户把它们拖得到处都是散落的浮动窗口，可以禁用浮动。

```cpp
auto *editToolBar = addToolBar("编辑");
editToolBar->setFloatable(false);  // 可以在窗口内移动，但不能浮出窗口
```

一个容易混淆的点：setFloatable(false) 不等于 setMovable(false)。setFloatable 只限制浮动行为，工具栏仍然可以在窗口的四个边缘之间拖动停靠。如果你完全不希望工具栏被移动，应该用 setMovable(false)，它会同时禁止停靠移动和浮动。

工具栏的初始停靠位置由 addToolBar(Qt::ToolBarArea area, QToolBar *toolbar) 指定。Qt::ToolBarArea 有四个值：Qt::TopToolBarArea、Qt::BottomToolBarArea、Qt::LeftToolBarArea、Qt::RightToolBarArea。如果你不指定区域（使用 addToolBar(const QString &title)），默认停靠在顶部。

```cpp
// 明确指定停靠位置
auto *topBar = addToolBar(Qt::TopToolBarArea, "主工具栏");
auto *leftBar = addToolBar(Qt::LeftToolBarArea, "工具箱");
auto *bottomBar = addToolBar(Qt::BottomToolBarArea, "输出");
```

setAllowedAreas(Qt::ToolBarAreas areas) 可以限制工具栏允许停靠的区域。比如你希望某个工具栏只能停靠在左侧或右侧，不能到顶部和底部。

```cpp
leftBar->setAllowedAreas(Qt::LeftToolBarArea | Qt::RightToolBarArea);
```

当用户尝试把工具栏拖到不被允许的区域时，QToolBar 不会停靠——它会弹回原来的位置。这个机制在保持布局一致性方面很有用，比如你不希望侧边栏式的垂直工具栏被拖到顶部变成水平的。

关于工具栏的拖拽交互还有一个细节：工具栏的"拖拽把手"在 Qt 中默认是可见的（显示为一排点状图案）。如果你觉得它影响美观，可以通过 QSS 隐藏它，但这会让用户失去拖拽工具栏的视觉提示——如果你同时设置了 setMovable(true)，用户可能不知道这个工具栏是可以拖动的。

### 3.3 setIconSize / setToolButtonStyle：视觉呈现调整

工具栏按钮的显示风格由两个属性控制：图标大小和按钮样式。

setIconSize(const QSize &size) 设置工具栏中所有按钮的图标尺寸。默认值取决于平台主题——在大多数桌面环境中是 22x22 或 24x24。如果你使用了高分辨率的图标或者希望工具栏看起来更大/更小，可以手动设置。

```cpp
toolBar->setIconSize(QSize(32, 32));  // 使用更大的图标
```

setIconSize 影响的是工具栏中通过 addAction 添加的 QToolButton 的图标大小。对于通过 addWidget 添加的自定义控件，setIconSize 不起作用——自定义控件的大小由其自身的 sizeHint 和 sizePolicy 决定。这意味着如果你的工具栏中混合了 QAction 和 QWidget，可能会出现大小不一致的问题。解决方法是在 addWidget 之前手动设置自定义控件的固定高度，使其和图标大小匹配。

setToolButtonStyle(Qt::ToolButtonStyle style) 控制工具栏按钮的显示模式。它决定了每个按钮是只显示图标、只显示文字、还是两者都显示。

Qt::ToolButtonStyle 有四个主要取值。Qt::ToolButtonIconOnly 只显示图标，不显示文字——这是大多数应用的默认模式，节省空间。Qt::ToolButtonTextOnly 只显示文字，不显示图标——适合没有设计图标的场景。Qt::ToolButtonTextBesideIcon 在图标右侧显示文字——适合按钮数量较少、需要明确标注功能的工具栏。Qt::ToolButtonTextUnderIcon 在图标下方显示文字——适合大图标模式，常见于文件管理器、图片查看器等应用。

```cpp
// 小图标 + 无文字（紧凑模式）
toolBar->setIconSize(QSize(20, 20));
toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);

// 大图标 + 文字在下方（类似文件管理器的工具栏）
toolBar->setIconSize(QSize(32, 32));
toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

// 中图标 + 文字在旁边（类似浏览器工具栏）
toolBar->setIconSize(QSize(24, 24));
toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
```

这里有一个容易踩的坑：setToolButtonStyle 是 QToolBar 级别的设置，它会统一应用到工具栏中的所有 QAction 按钮。如果你希望某个按钮用不同的样式（比如一个"保存"按钮只显示图标，旁边嵌入一个 QComboBox 显示文字），应该通过 addWidget 添加那个特殊的控件，而不是用 addSeparator 把两种样式的按钮分到两个工具栏中——后者的做法虽然可行，但会在工具栏之间产生多余的间距和分隔线。

还有一个值得注意的细节：当 Qt::ToolButtonIconOnly 模式下某个 QAction 没有设置图标（icon 为空），QToolBar 会自动退化为显示文本——一个空白的按钮对用户来说毫无意义，Qt 的这个回退行为避免了这个问题。但这也意味着如果你忘了设置图标，可能不会立即发现问题——按钮显示了文字看起来"正常"，但在其他工具栏（比如正确设置了图标的工具栏）中共享同一个 QAction 时，行为就不一致了。

### 3.4 溢出菜单：工具栏宽度不足时的自救方案

当工具栏中的控件宽度总和超过了工具栏本身的可用宽度时（比如用户把窗口缩小了、或者工具栏被拖到了一个很窄的位置），QToolBar 会自动把放不下的按钮折叠到一个溢出菜单中。这个溢出菜单在工具栏的右端显示为一个向右的小箭头（当工具栏水平排列时），点击后会弹出一个下拉菜单，里面包含了所有放不下的 QAction 和 QWidget。

这个机制是 QToolBar 内置的，你不需要写任何代码来处理——它会自动发生。但理解它的工作方式有助于你设计更好的工具栏布局。

溢出菜单的触发条件是工具栏的宽度小于其内容的 sizeHint 之和。当用户缩放窗口时，工具栏的宽度会实时变化，溢出菜单的内容也会动态增减——放大窗口时部分按钮从溢出菜单中"回来"了，缩小窗口时又有按钮"挤进"溢出菜单。整个过程是实时的，不需要手动刷新。

对于通过 addAction 添加的 QAction，溢出菜单会把它们显示为普通的菜单项（带图标和文字）。对于通过 addWidget 添加的 QWidget，溢出菜单会把整个控件作为一个菜单项显示——但效果通常不理想，因为 QWidget 的布局是按水平工具栏设计的，塞进一个垂直的下拉菜单中往往显得很别扭。

所以如果你知道某些控件可能会频繁进入溢出菜单，有几种应对策略。第一种是把不太常用的控件单独放到另一个工具栏中，让它们在不需要时被用户手动隐藏。第二种是在设计阶段就控制工具栏的内容数量——一般建议一个工具栏的按钮数量控制在 8 个以内，超过这个数量就应该考虑拆分。

```cpp
// 不要把所有按钮塞进一个工具栏
auto *fileBar = addToolBar("文件操作");
fileBar->addAction(newAction);
fileBar->addAction(openAction);
fileBar->addAction(saveAction);
fileBar->addSeparator();
fileBar->addAction(printAction);

auto *editBar = addToolBar("编辑操作");
editBar->addAction(undoAction);
editBar->addAction(redoAction);
editBar->addSeparator();
editBar->addAction(cutAction);
editBar->addAction(copyAction);
editBar->addAction(pasteAction);
```

第三种策略是利用 QToolBar 的 toggleViewAction。每个 QToolBar 都有一个内置的 QAction，调用 toggleViewAction() 可以获取它。这个 QAction 是一个可勾选项，勾选状态对应工具栏的可见性。你可以把这个 QAction 添加到"视图"菜单中，让用户自己决定显示哪些工具栏。

```cpp
auto *viewMenu = menuBar()->addMenu("视图(&V)");
viewMenu->addAction(fileBar->toggleViewAction());
viewMenu->addAction(editBar->toggleViewAction());
```

当工具栏被拖到侧边变成垂直排列时，溢出菜单的触发逻辑也一样——只不过方向变成了纵向，溢出菜单的箭头出现在底部。此时工具栏按钮会从下往上依次被折叠到溢出菜单中。

## 4. 踩坑预防

第一个坑是 addWidget 嵌入的 QWidget 在工具栏拖到侧边时的布局问题。当你把一个水平工具栏拖到窗口左侧变成垂直工具栏时，工具栏内部的布局会从水平变成垂直——通过 addAction 添加的按钮会自动适应垂直排列，但通过 addWidget 添加的 QWidget 可能不会正确调整大小。如果自定义控件的宽度超过了垂直工具栏的宽度，它会被截断。解决方法是在 addWidget 之前给控件设置合理的 sizePolicy 和最大宽度。

第二个坑是工具栏按钮的图标缺失。在 Qt::ToolButtonIconOnly 模式下，没有设置图标的 QAction 会退化为显示文字——这在开发阶段不容易被发现，因为按钮看起来"有内容"。但如果你在多个工具栏中共享这个 QAction，或者用户把工具栏样式切换为只显示图标，问题就会暴露出来。建议在创建 QAction 时就检查是否设置了图标，养成条件编译或运行时检查的习惯。

第三个坑是 setMovable(false) 的工具栏在 restoreState 时的行为。restoreState 会尝试恢复工具栏的停靠位置——如果一个被 restoreState 恢复到左侧的工具栏在代码中被设置为 setMovable(false)，它仍然会出现在左侧，但此时用户无法把它拖回顶部。如果你的工具栏在某个版本中从可移动变成了不可移动，用户旧版保存的 restoreState 数据可能导致工具栏出现在意外的位置。解决方法是在 restoreState 之后手动调用 addToolBar(Qt::TopToolBarArea, toolBar) 把不可移动的工具栏归位。

第四个坑是 macOS 上 QToolBar 的行为差异。macOS 的工具栏样式由系统统一管理，某些 QToolBar 的外观属性（比如分隔线的绘制方式、按钮的间距）在 macOS 上可能和其他平台不一致。如果你的应用需要跨平台，务必在 macOS 上做额外的视觉测试。

第五个坑是多个工具栏停靠在同一侧时的顺序问题。当你通过多次 addToolBar 向同一侧添加多个工具栏时，它们的排列顺序默认是添加顺序——先添加的在上面（或左面），后添加的在下面（或右面）。用户可以通过拖拽来调整顺序，但程序化的顺序控制只能通过 insertToolBar(QToolBar *before, QToolBar *toolbar) 来实现——它把 toolbar 插入到 before 前面。

## 5. 练习项目

我们来做一个综合练习：创建一个 QMainWindow 应用，包含三个工具栏。"文件"工具栏包含新建、打开、保存三个按钮（使用标准图标），通过 addSeparator 和"编辑"区域的撤销、重做按钮分开。"格式"工具栏通过 addWidget 嵌入一个 QComboBox（字体选择）和一个 QSpinBox（字号），以及一个加粗的 QAction 按钮。"绘图"工具栏设置为只能停靠在左侧或右侧，使用 Qt::ToolButtonTextUnderIcon 风格，包含画笔、矩形、圆形三个按钮。"视图"菜单提供 toggleViewAction 让用户控制各工具栏的可见性。中央区域使用 QPlainTextEdit。尝试缩小窗口观察溢出菜单的效果。

提示：使用 style()->standardIcon() 获取跨平台标准图标。QStyle 提供了 SP_FileIcon、SP_DirOpenIcon、SP_DialogSaveButton、SP_ArrowBack（撤销）、SP_ArrowForward（重做）等常用图标。

## 6. 官方文档参考链接

[Qt 文档 -- QToolBar](https://doc.qt.io/qt-6/qtoolbar.html) -- 工具栏类

[Qt 文档 -- QToolButton](https://doc.qt.io/qt-6/qtoolbutton.html) -- 工具栏按钮

[Qt 文档 -- QAction](https://doc.qt.io/qt-6/qaction.html) -- 动作抽象类

[Qt 文档 -- QStyle](https://doc.qt.io/qt-6/qstyle.html) -- 标准图标和样式

[Qt 文档 -- QMainWindow](https://doc.qt.io/qt-6/qmainwindow.html) -- 主窗口类

[Qt 文档 -- Tool Button Style Example](https://doc.qt.io/qt-6/qtwidgets-mainwindows-application-example.html) -- 工具栏风格示例

---

到这里，QToolBar 的核心用法就全部讲完了。addAction / addWidget / addSeparator 三种方法覆盖了工具栏的全部内容需求，setMovable / setFloatable / setAllowedAreas 控制停靠行为，setIconSize / setToolButtonStyle 调整视觉呈现，溢出菜单在空间不足时自动兜底。把这些组合起来，就能设计出既灵活又专业的工具栏系统。
