---
title: "3.57 QToolBar 进阶"
description: "进阶篇把 QToolBar 的机制读明白:溢出箭头分展开与弹菜单两种模式,判据在 checkUsePopupMenu(qtoolbarlayout.cpp 第 119-125 行),普通 action 由 createItem 现做 QToolButton 包装并汇流 actionTriggered(第 677-721 行),addWidget 走 QWidgetAction 且工具栏接管所有权(qtoolbar.cpp 第 771、782-788 行),widget 显隐必须走 QAction::setVisible(第 776-778 行),removeAction 时包装按钮 hide 加 deleteLater、自定义控件经 releaseWidget 归还(qtoolbarlayout.cpp 第 141-160 行),方向随停靠区域自动翻转(qtoolbararealayout.cpp 第 256-262 行),movable 默认值跟 SH_ToolBar_Movable 走(qtoolbar.cpp 第 63 行),iconSize 与 toolButtonStyle 由主窗口广播、显式设置压得住(qmainwindow.cpp 第 740-745 行、qtoolbar.cpp 第 78-87 行),toggleViewAction 出生自带(qtoolbar.cpp 第 61-64 行),saveState 认 objectName(qtoolbararealayout.cpp 第 1217-1250 行),示例的响应式样式切换在 examples/advanced/03-qtwidgets/57-qtoolbar-advanced/"
---

# 现代Qt开发教程（进阶篇）3.57——QToolBar 进阶

菜单栏管清单，工具栏管手速，主窗口顶上这排按钮把高频操作摆到您一击可达的地方。动手读源码您会发现，QToolBar 本体薄得很，几何排列与溢出的计算一概不在 qtoolbar.cpp 里，活全外包给私有的 QToolBarLayout，连每只按钮都是替 action 临时现做的。咱们顺着这个外包结构把每个机制读一遍。

引用文件咱们交代清楚。主战场是 qtoolbar.cpp 与 qtoolbar.h（qtbase/src/widgets/widgets），溢出细节下探到 qtoolbarlayout.cpp 与 qtoolbarextension.cpp，区域与存档在 qtoolbararealayout.cpp，广播行为到 qmainwindow.cpp 核对。配套示例是一只窄窗时自动切换按钮样式的响应式工具栏（examples/advanced/03-qtwidgets/57-qtoolbar-advanced/），完整玩法放结语。

## 溢出的箭头：装得下就展开，装不下才弹菜单

工具栏宽度不够时右端冒出一只小箭头，不少资料把它笼统说成“折叠进菜单”。您到源码里看，箭头在这里有两种工作方式，切换的开关是 setUsePopupMenu，咱们直接读代码（qtoolbarlayout.cpp 第 96-118 行，摘录）：

```cpp
if (!set) {
    QObject::connect(extension, SIGNAL(clicked(bool)),
                    this, SLOT(setExpanded(bool)), Qt::UniqueConnection);
    extension->setPopupMode(QToolButton::DelayedPopup);
    extension->setMenu(nullptr);
    delete popupMenu;
    popupMenu = nullptr;
} else {
    QObject::disconnect(extension, SIGNAL(clicked(bool)),
                        this, SLOT(setExpanded(bool)));
    extension->setPopupMode(QToolButton::InstantPopup);
    if (!popupMenu) {
        popupMenu = new QMenu(extension);
    }
    extension->setMenu(popupMenu);
}
```

主窗口还装得下时，箭头是一只普通按钮，点击调 setExpanded 把工具栏拉满一行，被藏起来的按钮当场回来。装不下了，或者工具栏正浮动、压根不在 QMainWindow 里，箭头切换成 InstantPopup，点开是一只 QMenu。判据咱们读 checkUsePopupMenu（第 119-125 行），它拿工具栏展开后垂直于取向的那一维，与主窗口同一维比大小，装不下就走菜单，浮动与无主两种情况同样直接走菜单。

具体谁被裁掉，度量发生在 layoutActions（第 402-547 行）。第一行就放不下时标记 ranOutOfSpace，排不进的项进 hideWidgets 列表逐个藏起。菜单模式下，被藏项的 action 会被 addAction 进 popupMenu（第 521-523 行），但 addWidget 塞进来的自定义控件进不了菜单，被裁掉的若全是这类，箭头干脆禁用（第 531-539 行），按钮还留在那儿，提醒您有东西被收起来了。

箭头本体是 QToolBarExtension，出厂名定为 qt_toolbar_ext_button，出处 qtoolbarextension.cpp 第 18 行。图标随工具栏取向换横竖两只标准图标（第 25-33 行）。您想用 QSS 单独打扮它，认这个 ID 选择器就够，别用 popupMode 属性选择器，展开与菜单两种模式的 popupMode 并不相同。

还有个流传的说法要纠正。QToolBar 没有 overflow 信号，qtoolbar.h 第 90-98 行的信号表笔者逐行数过，一共八只，管动作的 actionTriggered，管属性变化的 movableChanged、allowedAreasChanged、orientationChanged，管样式的 iconSizeChanged、toolButtonStyleChanged，再加上管浮动的 topLevelChanged，以及管显隐的 visibilityChanged。想要“窄了就变紧凑”的效果，示例的做法是自己监听窗口尺寸，toolbar_demo.cpp 第 105-120 行按 500px 阈值把所有工具栏切成 IconOnly，不依赖任何溢出信号。

## 每个 action 都有一只现做的 QToolButton

addAction 的重载在 qtoolbar.h 第 56-61 行，一行 using QWidget::addAction 带上基类的，QToolBar 自己再补四只，纯文字、图标加文字、带接收者与槽指针的两种。您添加的 action 不会凭空显示，createItem 会替它现做一只 QToolButton，出处 qtoolbarlayout.cpp 第 677-721 行，咱们读包装普通 action 的分支（摘录）：

```cpp
QToolButton *button = new QToolButton(tb);
button->setAutoRaise(true);
button->setFocusPolicy(Qt::NoFocus);
button->setIconSize(tb->iconSize());
button->setToolButtonStyle(tb->toolButtonStyle());
QObject::connect(tb, SIGNAL(iconSizeChanged(QSize)),
                 button, SLOT(setIconSize(QSize)));
QObject::connect(tb, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
                 button, SLOT(setToolButtonStyle(Qt::ToolButtonStyle)));
button->setDefaultAction(action);
QObject::connect(button, SIGNAL(triggered(QAction*)), tb, SIGNAL(actionTriggered(QAction*)));
```

按钮 autoRaise、无焦点，起步样式直接取工具栏当前的 iconSize 与 toolButtonStyle，往后工具栏样式一变，每只按钮凭两条信号连接跟着换（第 705-708 行）。按钮的 triggered 统一汇流到 QToolBar::actionTriggered（第 710 行），您想统一监听整条工具栏的点击，只连 actionTriggered 就够。要拿到某只 action 背后的按钮，widgetForAction（qtoolbar.h 第 80 行）直接给指针。分隔符走另一条路，造的是 QToolBarSeparator，取向跟工具栏同步（第 692-697 行）。

## addWidget 交的是 QWidgetAction，所有权各有其主

addWidget（qtoolbar.cpp 第 782-788 行）替您 new 一只 QWidgetAction 挂给工具栏，setDefaultWidget 之后 addAction，widget 的接管，由 QWidgetAction::requestWidget 落实。类文档写得直白，工具栏接管 widget 的所有权（第 771 行）。所以“必须手动传 parent 否则泄漏”这个担心可以放下，不传也不会漏。

显隐要留神。文档 note（第 776-778 行）要求显隐走 QAction::setVisible，您直接对 widget 调 hide 或 show 是不生效的，工具栏按 action 的可见性管理槽位。样式也管不到它，toolButtonStyle 只作用于自家包装的按钮，addWidget 进来的控件外观全由您负责（第 773-774 行）。

撤下时的处置，咱们读 takeAt（qtoolbarlayout.cpp 第 141-160 行，摘录）：

```cpp
QWidgetAction *widgetAction = qobject_cast<QWidgetAction*>(item->action);
if (widgetAction != nullptr && item->customWidget) {
    widgetAction->releaseWidget(item->widget());
} else {
    // destroy the QToolButton/QToolBarSeparator
    item->widget()->hide();
    item->widget()->deleteLater();
}
```

您自带的控件经 releaseWidget 归还，qwidgetaction.cpp 第 166-181 行里 defaultWidget 被 hide 加 setParent(nullptr)，父级清空，后续的生老病死回到您手里。工具栏自造的包装按钮与分隔符则被 hide，随后由 deleteLater 排队销毁，代码里就落在您看到的这两行上。另有一条纪律，同一只 QWidgetAction 不许同时进两处，actionEvent 的 ActionAdded 分支带着断言，原文是 “widgets cannot be inserted multiple times”，出处 qtoolbar.cpp 第 853-857 行。自动创建的那批 action 还会被重定父给工具栏，出处 qtoolbar.cpp 第 858-861 行。

## 方向随区域翻转，拖拽与浮动由两个属性分管

工具栏拖到左右两区会自己竖过来，这不用您操心，落位的时候 QToolBarAreaLayoutInfo::insertToolBar 就按区域调 setOrientation，出处 qtoolbararealayout.cpp 第 256-262 行。反过来手动调 setOrientation，文档警告主窗口管着时别用（qtoolbar.cpp 第 625-633 行），搬家请走 addToolBar 或 insertToolBar。

movable 与 floatable 各管一段，咱们分开看。movable 的默认值来自风格提示，init 里按 SH_ToolBar_Movable 取（第 63 行），同一份代码在不同平台上开箱即拖或不拖，都可能是对的。floatable 默认 true，拖出主窗口后 isFloating() 的实现就是 isWindow()（第 589-592 行），部件成了顶层窗口，topLevelChanged 信号同时发出。setAllowedAreas（第 609-614 行）收参数时会与 Qt::ToolBarArea_Mask 相与，越界的位被静默裁掉，默认四个区全开。

removeToolBar 的行为您记一手，qmainwindow.cpp 第 800-806 行，摘出布局加 hide，不销毁。重新 addToolBar 的路径笔者翻过，qmainwindowlayout.cpp 第 1541-1560 行没有任何 show 调用，藏着的工具栏加回来还是藏着的，得您亲手 show。

## iconSize 与 toolButtonStyle：主窗口广播，显式设置压得住

同一窗口多条工具栏要风格一致，QMainWindow 靠广播。addToolBar 收编工具栏时，把主窗口当前的 iconSize 与 toolButtonStyle 推过去，同时连接 iconSizeChanged 与 toolButtonStyleChanged，往后您在主窗口上一改，条条工具栏跟着换（qmainwindow.cpp 第 740-745 行）。

工具栏这头留了口子。QToolBarPrivate::_q_updateIconSize 收广播前查 explicitIconSize，您显式设过的就不被覆盖（qtoolbar.cpp 第 78-87 行）。setIconSize 传无效尺寸时，它会向上找主窗口的 iconSize，再找不到，就取 PM_ToolBarIconSize 像素值，出处第 686-711 行。运行中换风格也没问题，没显式设过的工具栏，会在 StyleChange 事件里重取像素值，出处 changeEvent 第 904-914 行。

示例走的是直设路线。setupUI 用 addToolBar 加进两条工具栏，出处 toolbar_demo.cpp 第 46、84 行。第 48 行显式设 TextBesideIcon，窗口一变窄，updateToolbarStyle 用 findChildren 挨条切样式（第 118-120 行），每只包装按钮再凭 createItem 里的信号连接跟着换。状态栏同时报出当前宽度与样式名（第 126-127 行），您拖动窗口就能看见阈值切换的瞬间。

## toggleViewAction 出生自带，saveState 仍然认 objectName

每条工具栏出生就带一只显隐开关。init 里 new QAction 挂给自己，可勾选，triggered 直连内部的 _q_toggleView，出处 qtoolbar.cpp 第 61-64 行。它的文字跟着 windowTitle 走（第 901-903 行），工具栏的 Show 与 Hide 事件里同步勾选态，顺带发一次 visibilityChanged，出处第 1016-1026 行。55 篇讲过的 createPopupMenu 右键菜单收编的就是它，咱们不重讲。

存档这边咱们接着看，QMainWindow::saveState 的工具栏部分，交给 QToolBarAreaLayout::saveState 完成，出处 qtoolbararealayout.cpp 第 1217-1250 行。它按行序列化，每只工具栏写进 objectName 加一个字节，字节里的位分别记显隐与取向。objectName 为空时存档现场 qWarning（第 1240-1243 行），55 篇叮嘱过的命名纪律在这里同样生效。搬家也会影响溢出，ParentChange 事件会触发 checkUsePopupMenu 重估，出处 qtoolbar.cpp 第 1027-1029 行，箭头按新环境换工作方式。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，行为咱们在 6.9.1 源码核对过。

[Qt 文档 · QToolBar](https://doc.qt.io/qt-6/qtoolbar.html) -- 工具栏类，addWidget 与 movable、floatable 属性的定义处

[Qt 文档 · QToolButton](https://doc.qt.io/qt-6/qtoolbutton.html) -- 工具按钮，popupMode 与 autoRaise

[Qt 文档 · QWidgetAction](https://doc.qt.io/qt-6/qwidgetaction.html) -- 把自定义控件包装成 action 的基类，requestWidget 与 releaseWidget

[Qt 文档 · QMainWindow](https://doc.qt.io/qt-6/qmainwindow.html) -- addToolBar 与 removeToolBar、saveState 的宿主

[Qt 文档 · The Style Sheet Syntax](https://doc.qt.io/qt-6/stylesheet-syntax.html) -- 子控件与 ID 选择器，扩展按钮定制的语法来源

---

工具栏讲完，主窗口上常驻的还有底部的状态栏。下一站 [3.58 QStatusBar 进阶](./58-qstatusbar-advanced.md)，临时消息与常驻部件的分栏方式在那边等咱们。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/57-qtoolbar-advanced/，可执行文件 57-qtoolbar-advanced）。把窗口拖到 500px 以下，按钮全部变成纯图标，状态栏里的样式名跟着换，拖宽又回来。打开 toolbar_demo.cpp 第 95 行的注释重新编译，View Toolbar 挪到下一行，两条工具栏分行显示。再往 setupUI 里多加几个 action，把窗口压到三百像素上下，右端会冒出小箭头，点击它工具栏整条拉满宽度，这就是展开模式的样子。

完成标准照旧。箭头在什么条件下切换工作方式，您讲得清。addWidget 塞进的控件显隐走哪条路，所有权归谁，您指得出。removeToolBar 之后工具栏处于什么状态，重新加回来缺哪一步，您说得准。
