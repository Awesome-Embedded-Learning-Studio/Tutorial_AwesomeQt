---
title: "3.57 QToolBar 进阶"
description: "入门篇我们搭了工具栏的基本骨架——addAction、setMovable、信号槽连接。这次我们解决工具栏在窗口变窄时的溢出折叠、浮动/可移动行为控制、运行时动态增删 Action 和自定义 Widget、以及 QSS 深度定制。"
---

# 现代Qt开发教程（进阶篇）3.57——QToolBar 进阶

## 1. 前言 / 当工具栏不再是"一排按钮排到天荒地老"

入门篇我们把 QToolBar 的基本用法过了一遍——addAction 添加按钮、setMovable 允许拖拽、setOrientation 控制方向，连上 triggered 信号就能用了。如果你的工具栏只有四五个按钮，入门篇的用法完全够用。但现实中的工具栏往往比想象中复杂——一个 IDE 的工具栏上可能有十几个按钮加搜索框加下拉框，窗口变窄时这些控件不能挤成一团甚至被截断，需要一个优雅的溢出机制把多出来的项折叠到扩展菜单里。还有些工具栏需要嵌入自定义控件（比如字体选择 ComboBox、缩放滑块），需要在运行时动态添加或移除按钮，需要用 QSS 做深度视觉定制。

本篇我们从四个方向深入 QToolBar：溢出折叠机制、可移动和浮动行为、动态增删内容、以及 QSS 样式定制。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17 标准。涉及 QtWidgets 模块（QToolBar、QToolButton、QAction）和 QtGui 模块（QIcon）。QToolBar 的溢出机制是 QMainWindow 内部布局引擎的一部分，独立使用 QToolBar（不在 QMainWindow 中）时溢出行为不会自动生效——这一点后面会详细讲。

## 3. 核心概念讲解

### 3.1 工具栏溢出机制

QToolBar 在 QMainWindow 中有一个内置的溢出（overflow）机制。当工具栏的宽度不足以显示所有 action 和 widget 时，多出来的项会被自动移到一个扩展菜单（extension menu）中。这个扩展菜单通过工具栏右端的一个小箭头按钮（`>>`）触发。

溢出机制由几个 API 协同控制。首先，QToolBar::setToolButtonStyle(Qt::ToolButtonStyle) 决定工具栏按钮的显示方式——是只显示图标、只显示文字、还是图标加文字。显示方式直接影响每个按钮占用的宽度，从而影响何时触发溢出。

```cpp
// 五种工具栏按钮样式
toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);      // 只显示图标（最省空间）
toolbar->setToolButtonStyle(Qt::ToolButtonTextOnly);      // 只显示文字
toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon); // 文字在图标右侧
toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);  // 文字在图标下方
toolbar->setToolButtonStyle(Qt::ToolButtonFollowStyle);   // 跟随系统风格
```

QToolBar 提供了 overflow() 信号，在工具栏发生溢出时触发。但说实话这个信号的实际用途不大——它没有告诉你哪些 action 溢出了。更实用的做法是关注 toolbar 内 action 的实际可见性，或者在 extension 按钮被点击时做自定义处理。

如果你想在某个特定的 action 处强制断行（让这个 action 之后的所有项溢出到扩展菜单），可以用 QToolBar::insertSeparator 在合适的位置插入分隔符。分隔符本身不会触发溢出，但它改变了 action 的排列方式。

```cpp
// 监听窗口大小变化，在工具栏溢出时调整显示模式
connect(toolbar, &QToolBar::overflow, this, [this]() {
    // 溢出时切换为只显示图标以节省空间
    toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
});
```

这里有一个很重要的行为细节：QToolBar 的溢出机制只在 QToolBar 处于 QMainWindow 的布局管理中时才生效。如果你把 QToolBar 作为独立窗口（不在 QMainWindow 中使用），或者通过 setParent 把它从 QMainWindow 中移走了，溢出机制就失效了——所有 action 会挤在一起，多出来的部分直接被截断不可见。所以，如果你的工具栏需要溢出功能，一定要确保它是通过 QMainWindow::addToolBar 添加到主窗口中的。

另外，QToolBar::widgetForAction(action) 可以获取某个 action 对应的 QToolButton 指针。这个方法在需要直接操作按钮控件时很有用——比如给某个按钮设置自定义的弹出模式（QToolButton::setPopupMode），或者修改按钮的样式。

```cpp
QAction* zoom_action = toolbar->addAction("缩放");
QToolButton* zoom_button =
    qobject_cast<QToolButton*>(toolbar->widgetForAction(zoom_action));
if (zoom_button) {
    // 设置为按下时立即弹出菜单
    zoom_button->setPopupMode(QToolButton::InstantPopup);
    zoom_button->setMenu(create_zoom_menu());
}
```

### 3.2 setMovable / setFloatable 与工具栏定位

QToolBar 有两个相关的定位属性。setMovable(bool) 控制用户能否拖拽工具栏到 QMainWindow 的其他停靠区域（上/下/左/右）。setFloatable(bool) 控制工具栏能否被拖出主窗口变成一个独立的浮动窗口。

这两个属性是独立控制的。一个工具栏可以 movable 但不 floatable——用户能在主窗口内拖拽工具栏换位置，但不能把它拖出来变成独立窗口。这在某些布局敏感的应用中很有用，比如你想让用户能调整工具栏在上/下区域的排列顺序，但不允许工具栏脱离主窗口。

```cpp
// 允许在主窗口内移动，但不允许浮动
toolbar->setMovable(true);
toolbar->setFloatable(false);
```

如果你完全不想让用户动工具栏的位置（固定在顶部），就把 movable 也设为 false。此时工具栏会固定在 addToolBar 时指定的区域，不允许拖拽。

```cpp
// 完全固定，不可拖拽
toolbar->setMovable(false);
```

QMainWindow::addToolBar(Qt::ToolBarArea area, QToolBar* toolbar) 指定工具栏的初始停靠区域。可选值是 Qt::TopToolBarArea、Qt::BottomToolBarArea、Qt::LeftToolBarArea、Qt::RightToolBarArea。同一个区域可以有多个工具栏——它们会从左到右（或从上到下）依次排列。用户可以通过拖拽调整同一区域内多个工具栏的排列顺序（前提是 movable 为 true）。

如果你需要在代码中（而不是用户拖拽）移动工具栏的位置，用 QMainWindow::addToolBarBreak 在某个区域插入一个换行断点，或者用 QMainWindow::insertToolBar(before_toolbar, new_toolbar) 在指定工具栏之前插入。removeToolBar 则从主窗口中移除工具栏但不 delete 它——和 takeCentralWidget 类似，你可以之后再用 addToolBar 把它加回来。

```cpp
// 运行时把工具栏从顶部移到底部
main_window->removeToolBar(toolbar);
main_window->addToolBar(Qt::BottomToolBarArea, toolbar);
toolbar->show();  // removeToolBar 会隐藏工具栏，需要重新 show
```

这里要注意 removeToolBar 之后必须手动 show()。removeToolBar 会把工具栏从主窗口的布局中移除，工具栏变成一个隐藏的独立窗口。addToolBar 会把它重新放入布局，但不会自动显示。

### 3.3 动态添加/移除 Action 和自定义 Widget

QToolBar 的内容不只是按钮。它支持三种类型的内容：QAction（自动包装为 QToolButton）、QWidget（通过 addWidget 添加）、以及分隔符。这三种内容可以混合排列。

addWidget 是往工具栏里塞自定义控件的关键接口。它接受一个 QWidget*，返回一个 QAction*。这个返回的 QAction 代表这个 widget 在工具栏中的"占位"，你可以用它来控制 widget 的可见性。

```cpp
// 在工具栏中嵌入一个搜索框
QLineEdit* search_edit = new QLineEdit(toolbar);
search_edit->setPlaceholderText("搜索...");
search_edit->setMaximumWidth(200);
QAction* search_action = toolbar->addWidget(search_edit);

// 可以通过 action 控制搜索框的可见性
search_action->setVisible(false);  // 隐藏搜索框

// 搜索框的信号连接
connect(search_edit, &QLineEdit::returnPressed, this, [this, search_edit]() {
    perform_search(search_edit->text());
});
```

addWidget 返回的 QAction 有一个特殊行为：它的 triggered 信号永远不会触发——因为点击事件被内部的 QWidget 拦截了。这个 QAction 的唯一用途是控制 widget 在工具栏中的可见性和位置。

动态添加和移除 action 的接口和 QMenu 基本一致。addAction 在末尾添加，insertAction 在指定位置前插入，removeAction 移除但不 delete。但 QToolBar 多了一个 insertWidget，它可以在指定 action 之前插入一个自定义 widget。

```cpp
// 动态添加一组操作按钮
void MainWindow::add_format_actions()
{
    QAction* bold = m_format_toolbar->addAction(QIcon::fromTheme("format-text-bold"), "粗体");
    QAction* italic = m_format_toolbar->addAction(QIcon::fromTheme("format-text-italic"), "斜体");

    bold->setCheckable(true);
    italic->setCheckable(true);

    connect(bold, &QAction::toggled, this, &MainWindow::toggle_bold);
    connect(italic, &QAction::toggled, this, &MainWindow::toggle_italic);
}

// 动态移除
void MainWindow::remove_format_actions()
{
    // removeAction 不会 delete，需要自己管理
    for (QAction* action : m_format_toolbar->actions()) {
        m_format_toolbar->removeAction(action);
        delete action;
    }
}
```

现在有一道调试题给大家。下面这段代码试图在工具栏里嵌入一个 ComboBox 并在选中项改变时响应，但用户反馈选中项后什么都没发生。问题出在哪里？

```cpp
QComboBox* font_combo = new QComboBox(toolbar);
font_combo->addItems({"宋体", "黑体", "楷体"});
toolbar->addWidget(font_combo);

connect(font_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &MainWindow::change_font);
```

问题出在 QComboBox 的信号签名上。QComboBox 有两个 currentIndexChanged 信号重载——一个接受 int，一个接受 const QString&。QOverload<int>::of 是正确的写法。但这里真正的问题是 addWidget 返回的 QAction 接管了 ComboBox 的鼠标事件处理。实际上这段代码在 Qt 6 中应该是正常工作的——addWidget 不会拦截子 widget 的信号。真正可能出问题的地方是 change_font 槽函数的实现。如果槽函数里用 QFont(font_family) 设置字体，但传入的是中文字体名，在某些平台上 QFont 可能找不到对应的字体。这不是 QToolBar 的问题而是字体匹配的问题。不过这段代码本身的结构是正确的——如果你遇到了信号不触发的情况，优先检查信号槽连接是否成功、ComboBox 是否有足够的项、以及槽函数的逻辑。

### 3.4 工具栏样式与 QSS 定制

QToolBar 的 QSS 定制涉及几个子控件选择器。QToolBar 本身支持 background、border、spacing 等属性。QToolBar::separator 是工具栏中分隔符的样式。QToolButton 是工具栏中按钮的样式。

```css
/* 工具栏背景和边框 */
QToolBar {
    background: #f0f0f0;
    border: none;
    padding: 2px;
    spacing: 3px;  /* action 之间的间距 */
}

/* 分隔符样式 */
QToolBar::separator {
    width: 1px;
    background: #c0c0c0;
    margin: 4px 2px;
}

/* 工具栏按钮的通用样式 */
QToolButton {
    background: transparent;
    border: 1px solid transparent;
    border-radius: 3px;
    padding: 3px;
}

/* 悬停效果 */
QToolButton:hover {
    background: #e0e0e0;
    border: 1px solid #c0c0c0;
}

/* 按下效果 */
QToolButton:pressed {
    background: #d0d0d0;
}

/* 选中（checkable action 的 checked 状态）*/
QToolButton:checked {
    background: #d0e0f0;
    border: 1px solid #80a0c0;
}
```

这里有几个 QSS 定制的坑需要注意。第一，QToolBar 的 min-width / min-height 属性在某些 Qt 版本中不生效，因为 QMainWindow 的布局引擎会覆盖这些值。如果你需要固定工具栏的高度，用 setFixedHeight 或 setMinimumHeight 代码设置比 QSS 更可靠。

第二，QToolButton 的 menu-indicator 子控件控制带菜单的工具按钮的下拉箭头。如果你设置了 QToolButton 的 border，下拉箭头可能会被 border 覆盖或者位置不对。这时候需要单独定制 menu-indicator。

```css
QToolButton::menu-indicator {
    image: url(:/icons/dropdown-arrow.png);
    subcontrol-position: right center;
    subcontrol-origin: padding;
    left: -4px;
}
```

第三，QToolBar 的扩展按钮（overflow 时出现的 `>>` 按钮）也是一个 QToolButton，但它有特殊的 object name 和属性。如果你想单独定制扩展按钮的样式，需要用属性选择器。

```css
/* 溢出扩展按钮样式 */
QToolBar QToolButton[popupMode="1"] {
    /* popupMode == 1 即 QToolButton::MenuButtonPopup */
    background: #e8e8e8;
}
```

如果你要做一个完整的暗色主题工具栏，不要只改 QToolBar 的 background。所有子控件——QToolButton 的各个状态、separator、extension 按钮——都需要配套修改。建议把工具栏相关的 QSS 集中写在一个 QSS 文件的一个区块内，便于统一调整。

## 4. 踩坑预防

第一个坑是 addWidget 的 widget 必须指定 parent 为工具栏或其子控件。addWidget 不会自动设置 widget 的 parent——如果你 new 了一个 QWidget 但没指定 parent，widget 的 parent 会是 nullptr。工具栏虽然会把它显示出来，但在工具栏析构时不会自动 delete 这个 widget，导致内存泄漏。更糟糕的是，如果 widget 的 parent 是另一个已经销毁的对象，工具栏还会持有悬空指针。解决方案是构造 widget 时把工具栏作为 parent 传入，或者调用 addAction 后手动 setParent。

第二个坑是 removeToolBar 之后工具栏消失但没被 delete。这个坑在上一篇也提到了。removeToolBar 从主窗口布局中移除工具栏并调用 hide()，但不会 delete 它。如果你以为 removeToolBar 会自动释放内存，那你就会收获一个不可见但仍然在内存中活着的工具栏及其所有子控件。如果你是真的不需要这个工具栏了，removeToolBar 之后要手动 delete 或者交给 Qt 对象树。如果你只是临时移除稍后还要用，用成员变量保存指针，后续 addToolBar + show 加回来。

第三个坑是 QSS 设置 QToolBar 的背景在 macOS 上不生效。macOS 使用原生工具栏渲染，某些 QSS 属性会被原生渲染覆盖。特别是在使用 macOS 统一标题栏和工具栏（unified title and toolbar）时，QToolBar 的 background 样式可能完全不起作用。解决方案是使用 window()->setUnifiedTitleAndToolbarArea(true) 配合系统原生样式，或者在 macOS 上不使用 QSS 定制工具栏。

第四个坑是 setToolButtonStyle 对 addWidget 添加的自定义控件不生效。setToolButtonStyle 只影响通过 addAction 添加的按钮（内部包装为 QToolButton）。addWidget 添加的自定义 QWidget 的外观完全由你自己控制——工具栏不会对它应用 ToolButtonStyle。所以如果你在工具栏中混合了 action 按钮和自定义 widget，需要注意两者的视觉一致性。

## 5. 练习项目

练习项目：响应式多工具栏编辑器。我们要做一个有多个工具栏的主窗口，窗口大小变化时工具栏能优雅地溢出折叠。

我们要实现的功能是：主窗口有"文件"和"格式"两个工具栏，文件工具栏包含新建、打开、保存、分隔符、撤销、重做六个按钮，格式工具栏包含粗体、斜体、下划线三个 checkable 按钮加一个字体选择 ComboBox。窗口变窄时工具栏自动溢出，溢出时切换为只显示图标模式。格式工具栏不可浮动但可在主窗口内移动，文件工具栏固定在顶部不可移动。完成标准是工具栏溢出时扩展菜单可用（点击溢出按钮可以看到被折叠的 action），格式工具栏可以拖拽到底部区域但不会脱离主窗口，两个工具栏的 QSS 样式统一。

提示几个关键点：用 QMainWindow::addToolBar 分别添加两个工具栏并指定不同区域，用 setFloatable(false) 防止格式工具栏浮动，用 overflow 信号在溢出时切换 ToolButtonIconOnly 模式，addWidget 添加 ComboBox 时指定 toolbar 作为 parent，QSS 用统一的选择器同时覆盖两个工具栏。

## 6. 官方文档参考链接

[Qt 文档 · QToolBar](https://doc.qt.io/qt-6/qtoolbar.html) -- 工具栏类，addWidget/setToolButtonStyle/setMovable/setFloatable/overflow 信号

[Qt 文档 · QToolButton](https://doc.qt.io/qt-6/qtoolbutton.html) -- 工具按钮类，setPopupMode/setToolButtonStyle/setArrowType

[Qt 文档 · QAction](https://doc.qt.io/qt-6/qaction.html) -- 动作类，setCheckable/setIcon/setVisible

[Qt 文档 · QMainWindow](https://doc.qt.io/qt-6/qmainwindow.html) -- 主窗口类，addToolBar/removeToolBar/insertToolBar

[Qt 文档 · The Style Sheet Syntax](https://doc.qt.io/qt-6/stylesheet-syntax.html) -- QSS 语法参考，包含子控件选择器和伪状态

---

到这里我们把 QToolBar 的溢出折叠、定位控制、动态内容和 QSS 定制讲完了。工具栏看起来简单，但溢出机制和 QMainWindow 布局引擎的交互、addWidget 的生命周期管理、以及跨平台样式一致性，都是需要仔细处理的工程细节。下一篇我们继续主窗口周边组件的进阶——QStatusBar 的多区域复杂状态显示。
