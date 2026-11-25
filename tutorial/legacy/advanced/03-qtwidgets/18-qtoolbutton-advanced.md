---
title: "3.18 QToolButton 进阶"
description: "入门篇会调 setToolButtonStyle、setPopupMode、setDefaultAction，进阶篇看一次点击在 QToolButton 上能走上哪几条路。箭头压过图标的绘制归属、三种弹出模式的信号时序、工具栏里 autoRaise 的来历与 action 对外观的接管，配套示例在 examples/advanced/03-qtwidgets/18-qtoolbutton-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.18——QToolButton 进阶

[入门篇 QToolButton 基础](../../beginner/03-qtwidgets/18-qtoolbutton-beginner.md) 把 setToolButtonStyle 的显示模式、setPopupMode 的三种弹出策略、setDefaultAction 的绑定连同 QToolBar 里的样式自适应过了一遍，往工具栏上摆几个按钮够用。[3.17 QPushButton 进阶](./17-qpushbutton-advanced.md) 结尾留了个话头，点主体执行默认操作、点箭头挑更多选项，出路在 QToolButton 身上。按钮五连走到第二个，咱们把一个现象摆在眼前。同一个 QToolButton 挂着同一个菜单，快速点一下，clicked 响了，动作照常执行。换成按住一秒再松手，菜单弹了出来，clicked 一声不吭。QPushButton 挂菜单时 clicked 从头沉默到尾，QToolButton 把点击与弹菜单的分界做成了三档可调，分岔点就在按住多久，本篇把三档模式连同它们牵动的信号时序一起理清。

笔者把三个容易把人问住的场面摆出来。方向按钮设了 setIcon，图标从头到尾不露面。您在 pressed 里发起网络请求，用户长按想弹菜单时，请求早已经出去了。给按钮 setText 改了个字样，action 状态一变，字样被拉回原样。三个场面各有一套根因，本篇讲三件事：箭头与图标在绘制层的归属，三种弹出模式下信号的触发时序，工具栏里 autoRaise 的来历与 action 对外观的接管。

## 箭头压过图标：setArrowType 与 setIcon 只能活一个

`setArrowType(Qt::DownArrow)` 让按钮画一个标准方向箭头，上、下、左、右四选一，做滚动区两端的调节按钮、绘图区的平移指示都靠它。它有个不写在使用说明里的脾气：跟 setIcon 谁也容不下谁。咱们要是两个都设，按钮上只有箭头，图标一次都不露面。

根因在绘制层。ArrowType 不是 Qt::NoArrow 时，QToolButton 的 initStyleOption 填样式选项时，会往里面挂一个 Arrow 特性。QStyle 沿着绘制按钮标签的路径（CE_ToolButtonLabel）走，先查这个特性，挂了就画箭头，走的是 PE_IndicatorArrowUp、PE_IndicatorArrowDown 这些图元，图标的 pixmap 分支根本轮不上。qcommonstyle.cpp 的判断旁边留着一句注释原话“Arrow type always overrules and is always shown”，翻译过来是箭头总是压过一切并且始终显示。咱们想让箭头按钮换成自己的图标，改 setIcon 没用，得把 `setArrowType()` 设回 Qt::NoArrow。

箭头按钮的 sizeHint 由 QStyle 按箭头尺寸计算，比带文字的按钮小一截。您把它放进布局，占的地方可能比预期的还小，需要 `setFixedSize()` 或 `setMinimumSize()` 把尺寸稳住，配套示例的四个方向按钮就是 `setFixedSize(32, 32)` 固定的。

“图标加下拉箭头”的组合（比如刷新图标旁边带个向下的小箭头）咱们另有出路，不必跟 setArrowType 较劲。更常见的一条是 MenuButtonPopup 模式，它自带的箭头指示由 QStyle 单独绘制，跟图标互不干扰，下方代码里的方式二走的就是这条路，后文还会展开。剩下两条路，一条是自己合成一张带箭头的 QIcon，另一条是用 QSS 的 sub-control 补画指示器。

```cpp
// 方式一：纯箭头按钮
auto *scrollDown = new QToolButton();
scrollDown->setArrowType(Qt::DownArrow);
scrollDown->setAutoRaise(true);  // 扁平化外观，更像滚动箭头

// 方式二：自定义图标 + 下拉箭头（不用 setArrowType）
auto *exportBtn = new QToolButton();
exportBtn->setIcon(QIcon(":/icons/export.png"));
exportBtn->setMenu(exportMenu);
exportBtn->setPopupMode(QToolButton::MenuButtonPopup);
```

配套示例顶部的工具栏摆着上、下、左、右四个方向按钮，点击后状态栏报出方向，日志区同步记一笔。咱们可以当场做个实验：给某个方向按钮补一行 setIcon，跑起来图标不露面，箭头照画。再把 setArrowType 那行改成 Qt::NoArrow，图标才出现。绘制的归属，两轮运行就看明白了。

## 按下、按住、点箭头：三种弹出模式各走各的路

17 篇讲过 QPushButton 挂菜单的机制，按下触发的内部槽调 `menu->popup()` 异步弹出菜单，菜单抢走鼠标释放事件，clicked 凑不齐按下后原地释放的条件，从此沉默。QToolButton 的三种弹出模式，差别就在按下之后走哪条路。咱们把 DelayedPopup 的时序从头顺一遍。

按下鼠标，QAbstractButton::mousePressEvent 让按钮进入按下状态，pressed 信号当场发出。QToolButton 在构造时就把 pressed 连到了一个私有槽上，槽里先查按钮有没有菜单，查到了且模式是 DelayedPopup，就启动一个内部定时器，时长取自 QStyle 的 SH_ToolButton_PopupDelay 样式提示，CommonStyle 默认 600 毫秒。到点之前松手，released 信号触发另一个私有槽把定时器停掉，clicked 随后照常发出，用户得到一次普通点击。定时器先到点，按钮把菜单以同步模态的 exec 循环弹出，与 QPushButton 那边的异步 popup 是两条路。菜单一弹就接管了鼠标，释放事件归它管，按钮的 clicked 不会再来。Qt 文档举的原型咱们都熟，浏览器工具栏的后退按钮，快点点它就后退一页，按住它就弹出历史列表。

这套时序里有一处要留神。定时器是 pressed 之后才启动的，pressed 自己在按下那一瞬就发出去了，用户最后是快按还是长按，它一概不管。您要是把不可逆的操作（发起网络请求、删一份文件）连在 pressed 上，用户长按想弹菜单，操作已经在按下那一下执行了，收不回来。刷新按钮外观、预取数据一类的状态准备放 pressed 没问题，动作本身，放 clicked 或 action 的 triggered 里才安全。

InstantPopup 把延迟去掉。按下那一瞬 pressed 照常发出，同一个私有槽发现模式是 InstantPopup，不等定时器，当场调 `showMenu()` 把菜单弹出来，同步模态循环就此接管，clicked 不会再有。菜单每次将弹未弹时，QMenu 会发 aboutToShow 信号，咱们想动态更新菜单内容（最近打开的文件、当前可用的设备），挂它最稳，它不依赖按钮侧的信号时序，17 篇里笔者建议过同一个入口。

```cpp
connect(menu, &QMenu::aboutToShow, this, [this]() {
    // 每次弹出菜单前动态更新选项
    menu->clear();
    for (const auto& item : getRecentItems()) {
        menu->addAction(item.name, this, [item]() { openItem(item); });
    }
});
```

MenuButtonPopup 把一个按钮划成两个区。咱们看箭头区，它单独绘制、有自己的按下反馈，视觉上与主体分得开。区域边界由 QStyle 的 subControlRect 给出，落在 SC_ToolButtonMenu 这个子控件上。点击落在箭头区，mousePressEvent 里当场调 `showMenu()` 弹出菜单，然后提前返回，pressed 与 clicked 一个都不发。QToolButton 连 hitButton 都覆写过，菜单关闭后的释放事件也凑不出 clicked。点击落在主体区，一切照普通按钮走，pressed、released、clicked 依次到来。

三种模式的信号归宿，咱们列一张表：

| 弹出模式 | pressed | 快速点击 | 按住不放 |
| -------- | -------- | -------- | -------- |
| DelayedPopup | 按下即发 | clicked 正常触发 | 约 600 毫秒后弹菜单，clicked 不触发 |
| InstantPopup | 按下即发，先于弹菜单 | 菜单立刻弹出，clicked 不触发 | 菜单已在屏上 |
| MenuButtonPopup | 点主体发，点箭头不发 | 点主体 clicked 正常，点箭头弹菜单 | 主体按住不弹菜单 |

### 一道调试题：DelayedPopup 的菜单迟迟不弹

咱们再看一段代码，用户怎么按，菜单都不出来，您先自己找找原因：

```cpp
auto *btn = new QToolButton();
btn->setPopupMode(QToolButton::DelayedPopup);
auto *menu = new QMenu(btn);
menu->addAction("选项");
// 没有调用 btn->setMenu(menu)

connect(btn, &QToolButton::clicked, []() {
    qDebug() << "clicked";
});
```

咱们把毛病定位到 menu 从没挂到按钮身上。QToolButton 判断自己有没有菜单，查三处：setMenu 挂进来的菜单，defaultAction 自带的菜单，按钮上除了 defaultAction 之外还挂着的其他 action。三处都查不到，那个私有槽在启动定时器之前就返回了，长按定时器压根没启动过，按钮的行为跟普通按钮一模一样，clicked 正常触发。menu 对象倒是创建出来了，父对象还是 btn，只是从头到尾没人用它，白占一份资源。修法就是把 `btn->setMenu(menu)` 补上，setMenu 与 setPopupMode 谁先谁后都行，两个属性各管各的，运行时才合到一起起作用。第三处还解释了一个少见的行为，按钮上挂了额外 action 却没设菜单时，长按会把这些 action 弹成一个临时菜单。

配套示例把三种模式摆成了可以动手的对比，咱们挨个点过去。中央区域一排三个按钮，DelayedPopup 那个同时连着 pressed 与 clicked 两条日志探针。咱们快速点它，两条都落，pressed 在前。再按住一秒，pressed 那条落完菜单弹出，clicked 那条不会来。InstantPopup 那个点下去，菜单立刻弹出，日志里落一条 pressed。按下那一瞬它就发出了，驱动弹菜单的正是这一下，正文前面推的时序当场看得见。clicked 探针从头到尾不落，它的标注写着“不应出现”，动静与标注对得上。顶部工具栏的 History 按钮是 DelayedPopup 的实用版。咱们快速点它，撤销计数在日志里往上走。按住它，最近十条操作记录弹出来，菜单内容由 aboutToShow 在每次弹出前重建，咱们一次撤销都没做过时，看到的是一个禁用的占位项。旁边的 Export 按钮是 MenuButtonPopup 的实用版，点主体记一笔默认导出，点箭头弹出 PDF、PNG、SVG 三种格式，两类日志一眼分得开。

## 进了工具栏就自动扁平：autoRaise 的来历与 action 的接管

QToolButton 一放进 QToolBar，边框和背景就没了，悬停时才浮一层轻高亮，工具栏按钮的标准长相。这件事不是 QToolBar 动的手，QToolButton 在 changeEvent 里处理父控件变化事件，发现新父控件是 QToolBar，就把 autoRaise 顶成 true，写在 qtoolbutton.cpp 里短短几行。工具栏代建的按钮（addAction 路径）出生时还另有一行显式的 setAutoRaise(true)，写在 qtoolbarlayout.cpp 里。您用 addWidget 把自己 new 的按钮放进去，父控件一换，同样自动变 true，两条路走到的是同一个结果。这是工具栏的视觉惯例，绝大多数情况下不必去改。

[3.12 QAbstractButton 基类进阶](./12-qabstractbutton-base-advanced.md) 讲 QStyleOptionButton 的 features 字段时提过：QPushButton 的扁平用 features 里的 Flat 标记，QToolButton 的扁平用 QStyleOptionToolButton 的 autoRaise。工具栏外手动 `setAutoRaise(true)` 也合法，只是它向 QStyle 声明的是“按工具栏风格画我”，画成什么样各家样式说了算：Fusion 悬停时铺一层浅灰背景，Windows 样式悬停时给一点蓝色高亮，macOS 样式下可能什么变化都看不出。想跨样式统一观感，咱们得自己补 QSS。

setDefaultAction 与 setMenu 可以同时挂在同一个按钮上，分工清楚。action 管文字、图标、enabled、checked 这些外观与状态，点击主体时，按钮自己的 clicked 会照常到来，action 的 triggered 也一并发出。menu 管弹出，箭头区域的点击交给它。绑定的 action 自己带着菜单时，setDefaultAction 还会顺手把 popupMode 切到 MenuButtonPopup，qtoolbutton.cpp 里写着，连 Qt 源码注释都承认这是工具栏时代留下的默认行为。这个切换有一个前置：函数进门先记下按钮当下有没有菜单（源码里的 hadMenu 变量），按钮此前已经挂了菜单，模式就不动。您要是先 setMenu 给按钮挂了自己的菜单，再 setDefaultAction 绑一个带菜单的 action，popupMode 不会被强切，弹出的还是您挂上的那份菜单。所以“按钮执行默认操作、箭头挑更多选项”的组合，咱们用一个带菜单的 action 就配齐了。

绑了 action，按钮的外观就归 action 管，这里有一处不动声色的覆盖。action 挂上后，它的任何属性变动都以 ActionChanged 事件送到按钮的 actionEvent，处理时对 defaultAction 重跑一遍 setDefaultAction，按钮的 text、icon、tooltip，连同 enabled，全按 action 的当前值重新灌一遍。您要是 `btn->setText("另存为")` 手动改了字样，action 那边任何一次变动（setEnabled、setChecked 都算）都把字样拉回 action 的 text。想改外观，改 action，别碰按钮自己的 setter：

```cpp
auto *action = new QAction("保存");
action->setIcon(QIcon::fromTheme("document-save"));

auto *btn = new QToolButton();
btn->setDefaultAction(action);
btn->setText("另存为");  // 这行会被 action 的 text 同步覆盖

// 正确做法：通过 action 修改
action->setText("另存为");
```

还有一处跟 QToolBar 相关的范围问题值得说清，它要把入门篇 3.4 的说法收窄一步。入门篇讲过，工具栏设好 toolButtonStyle 后栏内按钮自动跟随，切换后所有按钮立即变样。这话对 addAction 代建的按钮成立，addWidget 放进去的自有按钮不在跟随之列。`setToolButtonStyle()` 与 `setIconSize()` 看着像管全工具栏，官方文档写得直白：样式只给以 action 形式添加的按钮，addWidget 放进去的 QToolButton，拿不到这个 button style。源码对得上，工具栏代建按钮时顺手把 toolButtonStyleChanged、iconSizeChanged 两个信号连到按钮上，addWidget 的自有按钮没被连上那两个信号。配套示例正好摆着现成的对照：Toggle Style 按钮是 addAction 代建的，点击在 IconOnly 与 TextBesideIcon 之间切换，变样的只有它自己。History 与 Export 是 addWidget 放进去的，切换对它们不生效。四个方向按钮没 text 也没 icon，从始至终只有箭头。您要让自己的按钮跟随工具栏风格，要么改用 addAction 交给工具栏代建，要么自己监听 toolButtonStyleChanged 与 iconSizeChanged，手动做同步。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17。QToolButton 属于 QtWidgets 模块，内容涉及 QToolBar、QMenu、QAction 与 QStyle 的协作，示例可在任何支持 Qt 6 的桌面平台上编译运行。

[Qt 文档 · QToolButton](https://doc.qt.io/qt-6/qtoolbutton.html) -- 工具栏按钮，包含 ArrowType 和 PopupMode 的详细说明

[Qt 文档 · QToolBar](https://doc.qt.io/qt-6/qtoolbar.html) -- 工具栏，setToolButtonStyle / setIconSize 的作用范围（只管 addAction 代建的按钮）

[Qt 文档 · QAction](https://doc.qt.io/qt-6/qaction.html) -- 动作类，与 QToolButton 的绑定机制

[Qt 文档 · QMenu](https://doc.qt.io/qt-6/qmenu.html) -- 菜单控件，aboutToShow 信号

[Qt 文档 · QStyle](https://doc.qt.io/qt-6/qstyle.html) -- 样式系统，SH_ToolButton_PopupDelay 等 style hint

---

17 篇留下的弹出模式话头到这里有了着落。想再动手的，请您把配套示例（examples/advanced/03-qtwidgets/18-qtoolbutton-advanced/，可执行文件 18-qtoolbutton-advanced）完整玩一轮。DelayedPopup 按钮快按与长按各来一次，两条日志探针的先后对着正文看。InstantPopup 按钮点一下，把 pressed 探针的动静记下来。Toggle Style 切换前后扫一遍整个工具栏，谁变了样谁没变，两个信号的连接范围肉眼可辨。给某个方向按钮补一行 setIcon 再跑，箭头与图标谁上场当场就能看到。下一篇 [3.19 QRadioButton 进阶](./19-qradiobutton-advanced.md) 讲按钮五连的第三个 QRadioButton，QButtonGroup 的 exclusive 边界与 idClicked 的信号选择，互斥这摊事比入门篇看到的要深。
