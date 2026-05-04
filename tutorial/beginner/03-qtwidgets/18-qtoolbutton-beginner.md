# 现代Qt开发教程（新手篇）3.18——QToolButton：工具栏专用按钮

## 1. 前言 / 为什么 QToolButton 跟 QPushButton 不一样

上一篇文章我们刚把 QPushButton 的默认按钮机制、下拉菜单、图标用法和 QSS 美化全部过了一遍。你可能会想：QPushButton 已经能带菜单了（`setMenu`），也能显示图标了（`setIcon`），那 QToolButton 存在的意义是什么？这个问题特别好，而且答案并不简单——QToolButton 跟 QPushButton 的核心区别不在于"能做什么"，而在于"在什么场景下做什么更方便"。

QPushButton 是一个通用按钮，它在对话框、表单、主窗口中都可以使用，设计目标是"点击触发一个动作"。而 QToolButton 的设计初衷是专门服务于工具栏（QToolBar）的，它在工具栏中的行为有一套自动适配机制——比如根据工具栏的样式自动切换"只显示图标""只显示文字""图标加文字"的显示模式，再比如根据 `popupMode` 决定菜单是延迟弹出还是立即弹出。这些能力 QPushButton 要么不支持，要么需要你手动管理。

我们在实际项目中最常遇到 QToolButton 的场景大概有这几个：在 QMainWindow 的工具栏上放置带下拉菜单的操作按钮（比如"撤销"按钮旁边带一个下拉箭头展示历史操作列表），在工具栏之外需要"按钮主体点击做一件事、箭头点击弹出菜单"这种双区域交互的按钮，以及需要根据工具栏状态动态切换图标/文字显示模式的情况。这篇文章我们就把 QToolButton 的四个核心能力讲透：`setToolButtonStyle` 的显示模式控制、`setPopupMode` 的菜单弹出策略、与 `QAction` 的关联机制，以及在 `QToolBar` 中自动调整样式的底层原理。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QToolButton 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。本篇涉及 QToolBar、QMenu、QAction 也都在 QtWidgets 模块内，不需要额外链接其他模块。QToolButton 在所有桌面平台上的行为一致，视觉外观由 QStyle 和当前主题决定。

## 3. 核心概念讲解

### 3.1 setToolButtonStyle：图标、文字、还是两者都要

`setToolButtonStyle(Qt::ToolButtonStyle)` 控制的是 QToolButton 的显示模式——按钮上显示什么内容。这个枚举有四个值，我们按使用频率从高到低来理解。

`Qt::ToolButtonTextBesideIcon` 是最常见的模式，图标在左、文字在右，水平排列。大多数现代应用的工具栏用的就是这个模式——比如 VS Code 的"运行和调试"按钮，左边一个三角播放图标，右边跟着文字说明。

`Qt::ToolButtonIconOnly` 只显示图标不显示文字，适合工具栏空间紧张的场景。这个模式要求你给按钮设置一个含义明确的图标，否则用户根本不知道按钮是干什么的。记得配上 `setToolTip()`。

`Qt::ToolButtonTextOnly` 只显示文字不显示图标，看起来就像一个普通的 QPushButton。这个模式用得不多，但偶尔在工具栏需要展示"长文字操作"的时候会用到。

`Qt::ToolButtonTextUnderIcon` 图标在上、文字在下，垂直排列。这个模式在老版本的 Microsoft Office 工具栏里特别常见，现代应用用得相对少一些，但在需要突出图标的场景下还是有用的。

```cpp
auto *btn = new QToolButton();
btn->setText("新建");
btn->setIcon(QIcon::fromTheme("document-new"));
btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
```

你可以在运行时随时调用 `setToolButtonStyle()` 来切换显示模式，按钮会立即更新布局——不需要手动触发任何刷新操作。这个特性在"用户可以在设置里切换工具栏风格"这种需求中非常方便。

### 3.2 setPopupMode：菜单的弹出时机

QToolButton 的 `setPopupMode(QToolButton::ToolButtonPopupMode)` 是它跟 QPushButton 在菜单交互上最大的区别。QPushButton 通过 `setMenu()` 关联菜单后，点击按钮就直接弹出菜单——没有中间地带。而 QToolButton 提供了三种弹出模式，让你可以精细控制菜单弹出的时机和方式。

`QToolButton::DelayedPopup` 是默认模式。在这个模式下，用户按住按钮一段时间（大约 600 毫秒，具体时长由 QStyle 决定）后菜单才会弹出。如果用户快速点击并释放，则触发按钮的 `clicked()` 信号而不是弹出菜单。这个模式的设计意图是：按钮的主体点击仍然是"执行默认操作"，长按才是"展示更多选项"。一个典型的例子是浏览器的"后退"按钮——快速点击执行后退操作，长按展示历史页面列表。

`QToolButton::InstantPopup` 模式下，点击按钮立刻弹出菜单，不触发 `clicked()` 信号。这个模式的行为跟 QPushButton 的 `setMenu()` 类似，区别在于 QToolButton 还可以在菜单弹出前执行 `aboutToShow` 信号等逻辑。适合那种"按钮本身没有默认操作，纯粹就是用来弹出菜单"的场景。

`QToolButton::MenuButtonPopup` 是最强大的模式，也是 QPushButton 完全做不到的。在这个模式下，QToolButton 会被分成两个独立的点击区域——按钮主体和右侧的一个小箭头。点击按钮主体触发 `clicked()` 信号（执行默认操作），点击箭头弹出菜单。这个模式让你能同时提供"快速操作"和"更多选项"两个入口，而且两个入口在视觉上是明确分离的。

```cpp
auto *btn = new QToolButton();
btn->setText("保存");
btn->setIcon(QIcon::fromTheme("document-save"));

auto *menu = new QMenu(btn);
menu->addAction("保存", []() { qDebug() << "保存"; });
menu->addAction("另存为...", []() { qDebug() << "另存为"; });
menu->addAction("保存全部", []() { qDebug() << "保存全部"; });
btn->setMenu(menu);

// 关键：MenuButtonPopup 让按钮和箭头分成两个区域
btn->setPopupMode(QToolButton::MenuButtonPopup);
btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
```

你会发现 `MenuButtonPopup` 在实际项目中用得特别多——几乎所有 IDE 和编辑器的工具栏都大量使用这个模式。比如 Qt Creator 的"运行"按钮，主体点击是运行当前项目，箭头弹出菜单让你选择运行配置。

有一点值得注意：`setPopupMode` 只在按钮关联了菜单（`setMenu()` 被调用过）的情况下才有意义。如果你没有给 QToolButton 设置菜单，三个模式的行为完全一样——都是普通的按钮点击。

### 3.3 与 QAction 关联：setDefaultAction()

QToolButton 跟 QPushButton 的另一个重要区别是它可以直接绑定一个 `QAction`。`setDefaultAction(QAction *)` 把一个 action 设置为按钮的默认行为——按钮的文字、图标、tooltip、快捷键、enabled 状态全部从 action 同步过来，并且点击按钮等同于触发这个 action。

```cpp
auto *copyAction = new QAction(QIcon::fromTheme("edit-copy"), "复制");
copyAction->setShortcut(QKeySequence::Copy);
copyAction->setToolTip("复制选中内容 (Ctrl+C)");
connect(copyAction, &QAction::triggered, []() {
    qDebug() << "复制操作被触发";
});

auto *btn = new QToolButton();
btn->setDefaultAction(copyAction);
// 此时 btn 的文字、图标、tooltip 都是从 copyAction 同步过来的
// 点击 btn 等同于触发 copyAction->triggered()
```

这个机制的好处在于"一处定义，到处使用"。你定义一个 action，设置好它的图标、文字、快捷键和触发逻辑，然后把它同时添加到菜单（QMenu::addAction）和工具栏按钮（QToolButton::setDefaultAction）上。用户无论从哪个入口触发，走的都是同一个 action 的逻辑，不需要你手动同步状态。

在 Qt 5 的时代，QToolBar::addWidget() 返回的是 QWidget，你需要手动创建 QToolButton 再把 action 设上去。但在 Qt 6 中，`QToolBar::addAction()` 可以直接把 action 添加为工具栏按钮——内部自动创建一个 QToolButton 并调用 `setDefaultAction()`。所以如果你只是在工具栏上添加按钮，用 `QToolBar::addAction()` 就够了，不需要手动创建 QToolButton。手动创建 QToolButton 并 `setDefaultAction()` 主要用在需要自定义按钮样式或弹出模式的场景。

```cpp
// 方式一：QToolBar 直接添加 action（最常用）
auto *toolbar = addToolBar("编辑");
toolbar->addAction(copyAction);
toolbar->addAction(cutAction);
toolbar->addAction(pasteAction);

// 方式二：手动创建 QToolButton，适用于需要自定义 popupMode 的场景
auto *undoBtn = new QToolButton();
undoBtn->setDefaultAction(undoAction);
undoBtn->setPopupMode(QToolButton::MenuButtonPopup);
undoBtn->setMenu(undoHistoryMenu);
toolbar->addWidget(undoBtn);
```

`setDefaultAction()` 之后，按钮的文字和图标由 action 决定。如果你之后再调用 `setText()` 或 `setIcon()`，这些手动设置会被下一次 action 状态同步覆盖掉。所以如果你绑定了 action，就不要手动设置按钮的文字和图标——一切都通过 action 来管理。

### 3.4 在 QToolBar 中自动调整样式的机制

QToolButton 在 QToolBar 中有一个其他容器中没有的特殊行为：它会自动跟随工具栏的 `toolButtonStyle` 属性来决定自己的显示模式。

`QToolBar::setToolButtonStyle(Qt::ToolButtonStyle)` 设置工具栏的全局样式。当你调用这个方法后，工具栏内所有的 QToolButton 都会自动使用这个样式——无论它们自己有没有调用过 `setToolButtonStyle()`。这个"工具栏全局样式覆盖按钮本地样式"的机制是 Qt 的 QStyle 系统实现的，具体来说是在 `QToolButton::paintEvent()` 中，QStyle 会检查按钮是否在工具栏内，如果在就使用工具栏的样式而不是按钮自身的样式。

```cpp
auto *toolbar = addToolBar("主工具栏");
toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
// 工具栏内的所有 QToolButton 都会显示"图标 + 文字"

toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
// 切换后所有按钮立即变成"只显示图标"
```

这个机制带来一个你可能踩的坑：如果你手动给 QToolBar 中的 QToolButton 调用了 `setToolButtonStyle()`，你设置的值会被工具栏的全局样式覆盖掉——你改了但看不到效果。如果你确实需要某个按钮在工具栏中使用不同的样式，你需要子类化 QToolButton 并重写 `paintEvent()`，或者干脆不把它放在 QToolBar 里。

还有一个相关的方法：`QToolBar::setMovable(bool)`。虽然它不直接影响按钮样式，但它控制了用户是否可以拖拽工具栏到窗口的其他位置。当工具栏被拖拽到不同的区域（顶部、底部、左侧、右侧）时，主窗口可能会根据空间情况自动调整 `toolButtonStyle`——比如拖到侧边栏时切换为 `ToolButtonTextUnderIcon`。这个行为取决于 QMainWindow 的 `unifiedTitleAndToolBarOnMac` 等属性和当前的 QStyle 实现，不同平台上可能有差异。

另外，`QToolBar::setIconSize(QSize)` 也有类似的"全局覆盖"效果——工具栏内的所有 QToolButton 的图标尺寸会被统一设置为工具栏的 iconSize，按钮自身的 `setIconSize()` 会被忽略。

## 4. 踩坑预防

第一个坑是 `MenuButtonPopup` 模式下按钮的外观跟想象中不一样。`MenuButtonPopup` 在某些 QStyle 下（特别是 Fusion 风格）可能不会显示一个明显的分隔箭头，导致用户不知道按钮旁边还有一个可点击的箭头区域。如果你发现箭头不够明显，可以通过 QSS 来自定义箭头的样式。

第二个坑是 `setDefaultAction()` 之后手动设置的文字和图标被覆盖。前面已经说过了，绑定 action 后按钮的一切外观属性都由 action 驱动。如果你需要动态改变按钮文字，去改 action 的 `setText()` 而不是按钮的。

第三个坑是在 QToolBar 中手动设置 `setToolButtonStyle()` 不生效。这也是前面提到的——工具栏的全局样式会覆盖按钮的本地样式。如果你真的需要某个按钮的样式跟其他按钮不同，考虑用 `QWidget::setStyleSheet()` 给那个按钮单独设置 QSS 来绕过这个限制，或者把它放在工具栏外面。

第四个坑是 `DelayedPopup` 的延迟时间不可直接配置。延迟时间由 `QStyle::SH_ToolButton_PopupDelay` 这个 style hint 决定，默认大约 600 毫秒。如果你觉得太长或太短，可以通过 `QApplication::setStyle()` 切换 style 或者子类化 QStyle 来修改这个值——但说实话，大多数情况下默认的延迟时间已经够用了。

第五个坑是 QToolButton 在 QToolBar 之外使用时不会自动调整大小。在工具栏中，QToolButton 会根据 `iconSize` 和 `toolButtonStyle` 自动计算合适的大小。但在普通 QWidget 布局中，QToolButton 的尺寸计算跟 QPushButton 差不多，需要你手动设置 `setFixedSize()` 或者依赖布局来管理大小。如果你在工具栏外使用 QToolButton 并且设置了 `ToolButtonIconOnly` 模式，按钮可能会变得特别小——记得用 `setMinimumSize()` 或 `setFixedSize()` 来约束。

## 5. 练习项目

我们来做一个综合练习：创建一个 QMainWindow，顶部放一个 QToolBar，工具栏里放多个 QToolButton 展示不同模式。窗口中央放一个 QTextEdit 作为文档编辑区。工具栏的按钮分为三组：第一组是"文件操作"按钮，使用 `MenuButtonPopup` 模式，按钮主体点击弹出"保存成功"提示（通过 `setDefaultAction`），箭头弹出包含"保存""另存为""保存全部"的菜单；第二组是"编辑操作"按钮，使用 `DelayedPopup` 模式，包含"撤销""重做"两个按钮，长按弹出操作历史列表，快速点击直接执行撤销/重做并在 QTextEdit 中记录日志；第三组是纯菜单按钮，使用 `InstantPopup` 模式，弹出"插入"菜单（插入时间、插入日期、插入模板文字）。窗口底部放一个 QPushButton 用于切换工具栏的 `toolButtonStyle`，在 IconOnly 和 TextBesideIcon 之间来回切换，让你直观感受工具栏自动调整样式的效果。

几个提示：`MenuButtonPopup` 需要同时调用 `setMenu()` 和 `setDefaultAction()`；`DelayedPopup` 的快速点击会触发 `clicked()` 信号；切换工具栏样式用 `toolbar->setToolButtonStyle()`；QTextEdit 的 `append()` 方法可以在末尾追加一行文字。

## 6. 官方文档参考链接

[Qt 文档 · QToolButton](https://doc.qt.io/qt-6/qtoolbutton.html) -- 工具栏按钮

[Qt 文档 · QToolBar](https://doc.qt.io/qt-6/qtoolbar.html) -- 工具栏

[Qt 文档 · QAction](https://doc.qt.io/qt-6/qaction.html) -- 动作类

[Qt 文档 · QMenu](https://doc.qt.io/qt-6/qmenu.html) -- 菜单控件

[Qt 文档 · QAbstractButton](https://doc.qt.io/qt-6/qabstractbutton.html) -- 按钮基类

---

到这里，QToolButton 的四个核心维度我们就全部梳理完了。`setToolButtonStyle` 让你控制按钮上显示什么内容，`setPopupMode` 提供了 QPushButton 做不到的"按钮与菜单分离交互"能力，`setDefaultAction()` 把按钮的整个生命周期跟 QAction 绑定在一起实现了"一处定义到处使用"，而在 QToolBar 中的自动样式适配机制则让你不需要手动管理工具栏中每个按钮的显示模式。QToolButton 看起来像 QPushButton 的一个变体，但它在工具栏场景下的设计考量远比 QPushButton 深入，用好它能让你的工具栏交互体验上一个台阶。
