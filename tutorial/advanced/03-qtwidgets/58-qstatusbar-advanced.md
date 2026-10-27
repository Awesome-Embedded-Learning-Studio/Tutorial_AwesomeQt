---
title: "3.58 QStatusBar 进阶"
description: "进阶篇把 QStatusBar 的内部机制读明白：临时消息只是私有类里一枚 QString（qstatusbar.cpp 第 50 行），showMessage 惰性建定时器直连 clearMessage（第 494-499 行），文本相同早退、只重置定时器不发 messageChanged（第 504-505 行），非永久控件由 hideOrShow 逐只 hide 并清显隐标记、循环遇第一枚永久项即 break（第 558-584 行），消息文本由 paintEvent 直接 drawText 画进 messageRect（第 627-630 行），messageRect 右界取第一枚可见永久件左缘减 2、可见 sizegrip 也收窄、RTL 镜像（第 91-118 行），addWidget 永远插在最后一枚非永久件之后（第 62-71、229-234 行），addPermanentWidget 押列表末尾（第 294-299 行），insertWidget 遇在场消息当场藏新件（第 270-271 行），reformat 整删整建布局、非永久件与 stretch 与永久件依次入列（第 417-474 行），removeWidget 只藏不删、重加须 addWidget 加 show（第 351-365 行），ChildRemoved 分支自清列表（第 670-676 行），高度由 strut 决定、逐项取 qSmartMinSize 与 maximumHeight 的较小值（第 442-469 行），resizeEvent 只转基类、重活在 LayoutRequest 分支（第 636-668 行），sizegrip 在最大化或全屏时自动隐藏（qsizegrip.cpp 第 451-464 行），菜单悬停提示经 QMainWindow 的 StatusTip 处理直通 showMessage（qmainwindow.cpp 第 1270-1276 行），示例四枚永久件与模拟任务在 examples/advanced/03-qtwidgets/58-qstatusbar-advanced/"
---

# 现代Qt开发教程（进阶篇）3.58——QStatusBar 进阶

窗口三件套——菜单栏、工具栏、状态栏——走到最后一件。55 篇咱们读 QMainWindow 时见过 setStatusBar，旧栏被藏起来排队销毁，那一篇把状态栏当成一只普通部件对待。本篇咱们进 QStatusBar 自己的内部。笔者把 qstatusbar.cpp 从头读到尾，统共 686 行，公开接口背后只有一个私有类撑着，家当少得意外，咱们挨样清点。

引用文件交代清楚。咱们把主战场定在 qstatusbar.cpp 与 qstatusbar.h（qtbase/src/widgets/widgets），sizegrip 的显隐逻辑下探到 qsizegrip.cpp，菜单悬停的转发链路到 qmainwindow.cpp 里核对。配套示例是 examples/advanced/03-qtwidgets/58-qstatusbar-advanced/ 的多区域状态栏，完整玩法放结语。

## 家当只有三样：一张列表、一个字符串、一只定时器

QStatusBarPrivate 在文件开头（第 30-88 行），成员咱们数得过来。items 是一张 QList，每枚 SBItem 装着控件指针、stretch 因子和一枚类别（第 42-47 行），类别只有 Normal 与 Permanent 两个值。tempItem 是一枚 QString，装的正是临时消息（第 50 行）。timer 惰性创建，resizer 指着右下角的 QSizeGrip，出处第 53-57 行。

您在类文档里读到的三类状态指示（第 130-141 行），落进实现只剩两个户口：Temporary 在 items 里没有席位，它就是 tempItem 这枚字符串本身，Normal 与 Permanent 才是列表住户。咱们往下看这枚字符串怎么变成屏幕上的字。

## showMessage：控件被藏起来，文字直接画上去

showMessage 全文在第 490-509 行，咱们把主干摘出来：

```cpp
// 定时器惰性创建，同文本早退（qstatusbar.cpp 第 490-509 行，摘录）
void QStatusBar::showMessage(const QString &message, int timeout)
{
    Q_D(QStatusBar);

    if (timeout > 0) {
        if (!d->timer) {
            d->timer = new QTimer(this);
            connect(d->timer, &QTimer::timeout, this, &QStatusBar::clearMessage);
        }
        d->timer->start(timeout);
    } else if (d->timer) {
        delete d->timer;
        d->timer = nullptr;
    }
    if (d->tempItem == message)
        return;
    d->tempItem = message;

    hideOrShow();
}
```

timeout 大于 0 时定时器头一回才建，timeout 信号直连 clearMessage，start(timeout) 把旧倒计时作废重走，出处第 494-499 行。timeout 传 0 就删定时器，消息常驻，等 clearMessage 或下一条消息来接班。这里有个细节值得停一停：您发了一条与当前文本一模一样的消息，函数在第 504-505 行早退，messageChanged 不发，hideOrShow 也不调，但定时器已经重启。倒计时重新走，界面一个字不动，靠 messageChanged 统计消息条数的代码在这里会漏。

笔者把消息露出界面的链路追完了，分两步。第一步 hideOrShow（第 558-584 行）遍历 items，走到第一枚永久项就 break（第 563-565 行），这就是“永久件永不被打断”的实现级保证，藏的动作压根轮不到它们。非永久件被 hide，同时清掉 WA_WState_ExplicitShowHide 标记（第 566-568 行），标记清了，消息过场的 show 分支（第 569-571 行）才把它们放回来。随后 emit messageChanged（第 574 行），update 只刷 messageRect 一块（第 583 行）。第二步在 paintEvent（第 605-631 行）：给整条栏画 PE_PanelStatusBar 底子，给可见件逐个画 PE_FrameStatusBarItem 小框（第 613-626 行），消息在场时，drawText 把 tempItem 画进 messageRect，出处第 627-630 行。所以入门篇说临时消息会“覆盖” addWidget 的控件，控件确实会被藏起来，入门篇也自注了这半截，本篇补上的是另外半截：腾出来的地方由 drawText 画上文字，没有谁盖在谁上面。

messageRect 的边界在私有类里算（第 91-118 行）。左起 6 像素，右界取第一枚可见永久件左缘减 2，取不到就退到宽度减 12，可见的 sizegrip 也会把右界往左推，RTL 布局整面镜像。currentMessage 把 tempItem 原样奉还（第 536-540 行），咱们拿它配合 messageChanged 就能跟踪消息的完整生命周期。

还有一条白送的通道顺路看。菜单或工具栏动作悬停时的状态提示，QMainWindow::event 收到 QEvent::StatusTip 直接转给 statusBar()->showMessage(tip)，不带超时（qmainwindow.cpp 第 1270-1276 行），QApplication 沿父链上传事件，直到有人接住为止，出处 qapplication.cpp 第 2994-3002 行。您给 QAction 的 setStatusTip 填了文字，状态栏就自动亮，一行连接代码都不用写。

## 两区就是一张列表：addWidget 插队，永久件押尾

addWidget（第 229-234 行）一行转手，落点交给 indexToLastNonPermanentWidget() 决定，返回值加一就是插入位。这个私有函数从列表尾部往前找第一枚非永久件（第 62-71 行），所以您每次 addWidget，新件都落在已有非永久件之后、全部永久件之前，与添加时机无关。addPermanentWidget 反着来，插到 items.size()，永远押尾（第 294-299 行）。

咱们再看 insertWidget（第 255-278 行），里面还有一处贴心：临时消息在场时，新进的 Normal 件当场 hide（第 270-271 行），免得它闪一下又被藏。索引越界时，qWarning 提示一句，回落到合法位置，出处第 264-267 行。

分区落进几何要靠 reformat（第 417-474 行），它每次都把旧布局整只 delete 再重建。有 sizegrip 时外层是 QHBoxLayout 套一只 QVBoxLayout（第 425-429 行），没有就单只 QVBoxLayout（第 433 行）。内层再起一只水平布局，间距 6（第 440 行），咱们看入列的顺序：

```cpp
// 非永久件入列，弹性占位，永久件随后（qstatusbar.cpp 第 444-461 行，摘录）
    for (i = 0; i < d->items.size(); ++i) {
        const auto &item = d->items.at(i);
        if (item.isPermanent())
            break;
        l->addWidget(item.widget, item.stretch);
        int itemH = qMin(qSmartMinSize(item.widget).height(), item.widget->maximumHeight());
        maxH = qMax(maxH, itemH);
    }

    l->addStretch(0);
```

非永久件依次入列，中间 addStretch(0) 弹性占位（第 454 行），永久件排在 stretch 之后（第 456-461 行），sizegrip 挂在外层行布局最右，底对齐（第 463-467 行）。您在屏幕上看到的左边普通件、中间弹性空档、右边永久件加把手，几何来源就是这几行。

嵌入什么控件它都不挑，您塞 QLabel、QProgressBar、QCheckBox，是 QWidget 子类就收。示例把四枚永久件摆进右区：坐标标签（status_bar_demo.cpp 第 54 行）、建好即 hide 的进度条（第 57-62 行）、编辑模式复选框（第 65-66 行）、模式标签（第 69-71 行），复选框一勾，模式标签跟着换字（第 73-75 行）。

## removeWidget：只藏不删，回来要 addWidget 加 show

removeWidget（第 351-365 行）用 removeIf 把项摘出列表，widget->hide()，然后 reformat()。它不 delete，父子关系原样保留，控件您随时可以收回自己处置。查无此件时，debug 构建里补一句 qDebug，出处第 361-364 行。

想把它加回去有个机关。removeWidget 里的 hide 给控件打上了显式隐藏标记，insertWidget 结尾的复活检查 isExplicitlyHidden 会把 show 拦下（第 274-275 行），所以文档叮嘱重新入列要 addWidget 加 show 两句一起调。另一条路也通：您直接 delete 掉子控件，event() 的 ChildRemoved 分支会把列表里的残项清干净（第 670-676 行），不留野项。

频繁显隐的场景咱们换工具。add 一次，之后全靠 show 与 hide 切换，布局自动收缩。示例的进度条就是这么干的：建好时 hide（第 61 行），任务启动 show（第 96 行），收尾再 hide（第 111 行），从头到尾没碰过 removeWidget。

## 高度由 strut 说了算，reformat 整删整建

咱们逐行看 reformat 里算高度那段。maxH 起步于字体行高 fontMetrics().height()（第 442 行），随后每枚件取 qSmartMinSize 高度与控件 maximumHeight 的较小值来比较（第 450-451、459-460 行），sizegrip 的 sizeHint 高度也来比一场（第 464 行），末尾 addStrut(maxH) 把这个高度定成内层布局的最低高，出处第 469 行。状态栏莫名偏高时，赢家常常是 sizegrip，把它关掉或者给子控件设 maximumHeight，strut 就降下来了。

窗口尺寸变化走哪条路？resizeEvent 只有转给基类的一行（第 636-639 行），布局归 QLayout 管。重活在 event() 的 LayoutRequest 分支（第 650-668 行）：重算一遍 maxH，与 savedStrut 不同才重建布局，相同就只 update 一把。您 addWidget 之后高度没变的场景，走的正是省事的短路。

## sizegrip：建房归状态栏，显隐归它自己

咱们从构造函数看起。setSizeGripEnabled(true) 是默认动作，注释里写明它带出一次 reformat，出处第 198-203 行。开它时 new 一只 QSizeGrip 挂给自己，藏起来，installEventFilter，再立一个 showSizeGrip 待办标志（第 393-397 行）。关它时直接 delete（第 399-401 行）。isSizeGripEnabled 只问 resizer 指针在不在（第 376-384 行），不管它此刻可见与否。

显隐的裁判权在 QSizeGrip 自己手里。状态栏这边的 tryToShowSizeGrip 经 invokeMethod 调它的 _q_showIfNotHidden 槽，两处出处分别在 qstatusbar.cpp 第 74-84 行与 qsizegrip.cpp 第 73-88 行，这个槽多问一句：顶层窗口最大化或全屏就不出场。showEvent 也会再试一次（第 589-596 行）。窗口运行中切换状态时，QSizeGrip::eventFilter 过滤顶层的 WindowStateChange，最大化或全屏把自己藏起来，恢复普通状态再现身，出处 qsizegrip.cpp 第 451-464 行。笔者特意去 qsizegrip.cpp 核对了整段链路，看过之后可以放心：咱们不用在最大化切换的代码里伺候它，Qt 替咱们看着。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，行为咱们在 6.9.1 源码核对过。

[Qt 文档 · QStatusBar](https://doc.qt.io/qt-6/qstatusbar.html) -- 状态栏类，showMessage 与两区添加接口的定义处

[Qt 文档 · QSizeGrip](https://doc.qt.io/qt-6/qsizegrip.html) -- 右下角调整把手，最大化与全屏下的显隐规则

[Qt 文档 · QMainWindow](https://doc.qt.io/qt-6/qmainwindow.html) -- 主窗口类，statusBar 懒创建与 StatusTip 的转发处

[Qt 文档 · QProgressBar](https://doc.qt.io/qt-6/qprogressbar.html) -- 进度条控件，状态栏里最常见的嵌入件

---

三件套读完，下一站轮到停靠部件。[3.59 QDockWidget 进阶](./59-qdockwidget-advanced.md) 咱们去读多文档布局的持久化，正好接上 55 篇 saveState 埋下的伏笔。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/58-qstatusbar-advanced/，可执行文件 58-qstatusbar-advanced）。在中央区域移动鼠标，右区坐标标签跟着走（status_bar_demo.cpp 第 86-90 行）。点 Start Task，进度条现身推进，期间 “Task running...” 常驻左区（无超时，第 118 行），任务收尾 500 毫秒后进度条退场，“Task completed!” 亮 3 秒（第 110-113 行），四枚永久件全程纹丝不动。再把第 54 行的 addPermanentWidget 改成 addWidget 编译一遍，随便触发一条临时消息，坐标标签当场消失，消息清掉又回来，这就是 Normal 件的待遇。

完成标准照旧。临时消息上屏分哪两步，您指得出。hideOrShow 的循环遇到谁就 break，您说得出。removeWidget 之后想加回控件要补哪一句，您讲得明。状态栏偏高时 strut 里谁在撑，您数得清。
