---
title: "3.67 QProgressDialog 进阶"
description: "进阶篇把 QProgressDialog 的估时机关对到源码行上：minimumDuration 默认 4000 毫秒（qprogressdialog.cpp 第 31 行），setValue 拿前几步的速度外推全程决定现身与否（第 636-650 行），50 毫秒内不做预测（第 33 行），第一次 setValue 到最小值会重拨计时钟（第 627-631 行），forceShow 定时器兜底（第 811-820 行），内部是一只 QLabel 加一条 QProgressBar 加一枚 QPushButton（第 50-53 行），canceled 在构造时就接进 cancel() 槽（第 80 行），cancel() 藏着一次 reset（第 580-587 行），reset 会把 shownOnce 清零让估时机关重新武装（第 554-572 行），canceled 的出处只有按钮点击、Esc 快捷键与 closeEvent 三条（第 373、375、744-748 行），任务到顶只走 reset 不发 canceled（第 660-661 行），模态下 setValue 替您泵 processEvents（第 621-625 行），不确定模式的动画在 QCommonStyle 一派吃 value（qcommonstyle.cpp 第 1566-1572 行），Fusion 自带 60 帧动画（qfusionstyle.cpp 第 247、1256-1262 行），示例在 examples/advanced/03-qtwidgets/67-qprogressdialog-advanced/"
---

# 现代Qt开发教程（进阶篇）3.67——QProgressDialog 进阶

轮到进度对话框了。QProgressDialog 是配耗时任务用的对话框，笔者把 Qt 6.9.1 的 QProgressDialog 源码，也就是 qprogressdialog.cpp，从头读到尾，发现它有两个看家机关。一个是估时自动出现，任务够快它一声不吭，预计够慢它才现身，minimumDuration 管着现身与否。另一个是取消判定，canceled 信号与 cancel() 槽，在构造那天就被 Qt 接到了一起。网上几处流传的说法笔者逐一拿行号对过，有的对不上，咱们这篇按源码重新讲。

引用文件咱们交代清楚。主战场是 qprogressdialog.cpp 与 qprogressdialog.h（src/widgets/dialogs），不确定模式的动画还要借 qprogressbar.cpp 和两个样式文件对证。入门篇走过的构造函数与 setValue 基本用法本篇不重讲，配套示例是一只定时器驱动的演示窗口，完整玩法放结语。

## 估时自动出现：minimumDuration 拿速度做外推

默认门槛 4000 毫秒，咱们从文件头的一对常量里读到出处（qprogressdialog.cpp 第 31、33 行），defaultShowTime 是 4000ms，旁边站着 minWaitTime 是 50ms，注释写明前者管“预计要花这么久的操作才显示”，后者管“至少等这么久才做预测”。您再看 minimumDuration 的属性文档，它重申了默认 4000 毫秒，设成 0 则一有进度就现身（第 719-720 行）。

咱们跟着 setValue 走一遍（第 613-662 行）。对话框还没现身时，您调 setValue 且给的值等于最小值，比如常见的 setValue(0)，这一步只做一件事，把计时钟与兜底定时器重新拨到零再启动（第 627-631 行），注释写明是兼容 Qt 5.4 以前的行为。所以计时真正起算的位置是您交出最小值的那次调用，不是构造函数。构造时确实也拨过一次钟（第 88-89 行），但只要您按惯例调了 setValue(0)，钟就被重拨，构造到开工之间的初始化耗时不计入。网上有说法称计时从对象创建起算、头一次 setValue 不影响它，笔者拿这两段源码对过，说法不成立。

交出非最小值的进度之后，外推开始（第 632-650 行）。耗时不足 showTime 时，只要过了 50 毫秒的观察期，它就拿已有速度推全程，myprogress 取当前步数与 1 的较大者，remainingSteps 是余下步数，估算时长等于耗时乘以余下步数再除以 myprogress，估算结果不低于 minimumDuration，对话框才现身（第 640-650 行）。咱们代一组实数进去，100 步的任务跑到第 2 步用了 100 毫秒，外推出剩余约 4.9 秒，超过默认 4 秒的闸门，对话框当场现身（第 652-655 行）。也就是说一个预计 5 秒的任务，用户在第 2 步就能看到它，不必干等 4 秒。观察期内的调用一律不预测，头几步的噪声太大，Qt 不拿它做推断。

forceShow 是另一条保险（第 811-820 行）。构造时启动的兜底定时器 forceTimer 到点触发它，对话框还没现身就补一次 show。但它得有活着的事件循环才发得了火，模态紧循环里您不泵事件，定时器等不到执行机会，现身与否全由 setValue 里的外推裁决。setMinimumDuration 也有个细节，它只在进度值仍等于最小值时才重拨 forceTimer（第 727-730 行），任务跑到一半改时限，兜底定时器不吃这一套。

## 内部组装：一块标签、一条进度条、一枚按钮

私有类里的家底，咱们数得出三样（qprogressdialog.cpp 第 50-53 行），一块 QLabel、一条 QProgressBar、一枚 QPushButton，另有强制现身用的 forceTimer。init 里挨个造出来（第 75-76 行，取消按钮由第 86 行 setCancelButtonText 间接创建），标签的对齐方式听样式提示，提示名叫 SH_ProgressDialog_TextLabelAlignment，出处见第 78-79 行。布局不用布局管理器，手工摆位的活儿在私有类的 layout()（第 92-137 行），resizeEvent（第 688 行）每次调它，样式一换就重摆一遍（changeEvent 第 697-706 行），摆位函数里专门有一段循环逐步砍间距（第 110-125 行），注释写明进度对话框要经得起被用户压得很小。

三样都能换。setLabel、setBar、setCancelButton 收您的替换件，adoptChildWidget 负责改挂父对象并重算尺寸（第 446-461 行），文档叮嘱咱们别传栈上的对象，旧的会被 delete（第 321、370、441 行）。setBar 在调试构建里多一道检查，旧进度条已有进度时还硬要换，就会收到 qWarning（第 432-435 行）。取消按钮这边还压着一个细节，setCancelButton 传 nullptr，或者 setCancelButtonText 传空字符串，按钮就被删掉（第 370 行 delete，空串转调在 411-413 行），后果咱们下一节接着讲。

## canceled 的三条出处与构造时接好的一条内部线

canceled 信号在类里有一条内部线，笔者把接线的位置翻了出来。构造时 init 就把 canceled 连到自己的 cancel() 槽（第 80 行），信号文档写明“点取消按钮时发出，默认连着 cancel() 槽”（第 296-301 行）。cancel() 的身体只有几行（第 580-587 行），置起 forceHide，调一次 reset，再把 forceHide 落回去，最后把 cancellationFlag 置真。reset（第 554-572 行）负责在 autoClose 为真时藏对话框、进度条归零、清 cancellationFlag、停 forceTimer，另有一处容易被忽略的收拾，shownOnce 与 setValueCalled，也一并清零（第 561-562 行）。

标志清零之后，估时机关整体重新武装，这是笔者想让您留意的地方。有流传说法称延迟只管头一次现身，整个生命周期一次性的，源码不支持，reset 之后复用同一只对话框，外推照常再跑一轮。cancel() 里 reset 在前、置标志在后，顺序也有讲究，reset 会清 cancellationFlag，所以取消结束后 wasCanceled() 保持为真，直到下一次 reset，属性文档是这么写的（第 574-577 行）。

canceled 的出处咱们数全，一共三条。按钮的 clicked 被转接成 canceled（第 373 行）。Esc 走一枚 QShortcut，键序是 QKeySequence::Cancel，同样发 canceled（第 375 行）。标题栏关闭钮也算，closeEvent 里直接 emit canceled（第 744-748 行），用户点 X 等于点取消。按钮被删时这枚快捷键一起被删（第 377-381 行），所以没有取消按钮的进度对话框，Esc 也不再发 canceled，上一节压着的那个细节，在这里收掉。另有一个字符串式的 open(QObject*, const char*)，把 canceled 接到您的槽上，对话框关闭时自动断开（第 828-835 行）。

有条流传较广的说法笔者要当面驳回，任务跑完到顶时会发 canceled。setValue 的尾部写得明白，进度值等于最大值且 autoReset 为真时只调 reset()，没有 emit（第 660-661 行），reset 的身体里同样没有。完成与取消在信号层面分得开，您的完成逻辑放心挂在任务自己的信号上，不会被 canceled 抢戏。

## 模态循环里的 setValue：它替您泵了一轮事件

类文档把用法分模态与无模态两路（第 190-214 行），构造函数里没有 setModal 的调用，模不模态由您用 exec 或 setWindowModality 决定。模态这一路咱们在循环里干，setValue 推进，wasCanceled 查取消。这里藏着 setValue 的一个副作用，对话框已现身且是模态时，每次 setValue 都替您泵一轮 QCoreApplication::processEvents()（第 621-625 行），processingEvents 标志挡住递归（第 623 行）。属性文档的警告在案，重入风险自己扛，paintEvent 里别用进度对话框（第 605-609 行）。事件被处理掉，界面才能刷新，也让您自己的代码路径可能被半路插进来的事件重入，模态循环里每一步都要经得起这个。无模态这一路的代价是事件循环得一直在跑，文档把话说在了前头（第 206-208 行）。

后台线程的搭档模式，取消方向要靠信号送。咱们给一段骨架。

```cpp
// 伪代码：worker 在后台线程发进度，主线程更新对话框
connect(worker, &Worker::progressChanged, dlg, &QProgressDialog::setValue);
    // 跨线程连接自动排队投递，setValue 落在主线程执行
connect(dlg, &QProgressDialog::canceled, worker, [worker] { worker->requestCancel(); });
    // 取消意图送进 worker，内部落在原子标志上
```

两个方向各有一条边界。后台线程不能调 wasCanceled()，它是控件成员函数，控件只归主线程碰，取消意图用 canceled 信号加原子标志送过去。让线程停下也别指望 QThread::quit()，它只请事件循环退场，正在跑的那一步不打断，worker 得在每个循环节点自己查标志收工。进度方向正好相反，信号从后台线程发出，跨线程连接自动排队投递，这套机制咱们在进阶篇第 09 模块多线程篇铺过，这里挂链不重讲。

## 收尾开关：到顶只归零，不发信号

咱们把收尾的两个开关看一眼。autoReset 与 autoClose，默认都是真（qprogressdialog.cpp 第 63-64 行）。到顶时 reset() 被调，autoClose 为真 reset 就顺手 hide（第 557 行），进度值归零，一套收尾全自动。示例把两个开关都关掉手动收尾（progress_dialog_demo.cpp 第 65-66 行），这是异步场景的稳妥做法，收尾时机攥在您自己手里。类文档另有一条提醒，用 setMaximum 或 setRange 把新最大值设成当前值，对话框不会因此关闭（第 184-188 行），因为收尾判定长在 setValue 里，范围改完不补一次 setValue，判定不会再跑。

## 不确定模式：动画由谁推，样式分两派

setRange(0, 0) 进不确定模式，QProgressBar 的文档写明，两头都为零时显示忙指示（qprogressbar.cpp 第 169 行）。动画由谁推，咱们查到两派样式两种答案。走 QCommonStyle 画法的，忙指示是一根四像素宽的竖线，横坐标直接拿 progress 值取模算出来（qcommonstyle.cpp 第 1566-1572 行），值不动线就不动，而 QProgressBar 自己没有动画定时器，这是笔者全文翻过确认的。Fusion 另起炉灶，样式私有对象里挂了一只 QProgressStyleAnimation，帧率 60（qfusionstyle.cpp 第 247 行），步进自己走（第 1256-1262 行），不用外人推。

所以在 CommonStyle 一派里驱动动画得靠改值，而 QProgressDialog 的 setValue 有去重，同值直接返回（qprogressdialog.cpp 第 616-617 行）。好在忙模式下 QProgressBar 放行越界值，两头为零时越界检查被短路（qprogressbar.cpp 第 285-288 行），咱们递增着喂 setValue(value() + 1) 就能一路穿过去重，线一步步挪。有个位置要避开，setValue(0) 走的是拨钟分支，到不了重绘（第 627-631 行），驱动序列从 1 起数就好。示例没演示忙模式，您想试就把 setRange(0, 0) 换上，拿定时器递增着喂。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，行为咱们在 6.9.1 源码核对过。

[Qt 文档 · QProgressDialog](https://doc.qt.io/qt-6/qprogressdialog.html) -- 进度对话框类，minimumDuration 与 canceled 的属性定义处

[Qt 文档 · QProgressBar](https://doc.qt.io/qt-6/qprogressbar.html) -- 进度条控件，不确定模式的载体

[Qt 文档 · QThread](https://doc.qt.io/qt-6/qthread.html) -- 后台线程，异步搭档模式的另一头

---

标准对话框的连载走到后半程，下一站 [3.68 QErrorMessage 进阶](./68-qerrormessage-advanced.md)，错误消息的抑制机制与 QSettings 持久化在那边等咱们。

想动手的您把示例跑起来（examples/advanced/03-qtwidgets/67-qprogressdialog-advanced/，可执行文件 67-qprogressdialog-advanced）。窗口里一枚 Start Task，点下去任务以 50 毫秒一步走 100 步，示例把 minimumDuration 设成 1000，见 progress_dialog_demo.cpp 第 64 行。您留神看，对话框不会等满一秒，走到第二步左右就现身了，因为 100 毫秒跑两步外推出剩余约 4.9 秒，远超闸门，估时机关抢在兜底定时器前头拍了板。任务跑完对话框不自己关，autoClose 被示例关掉了（第 65 行），收尾由 finishTask 里的手动 reset 加 delete 完成，见第 107-110 行。半路按 Esc 或者点标题栏关闭钮，窗口顶部的状态标签会记下 Task cancelled at N%，两条路都汇进 canceled，正是咱们数过的三条出处里的两条。

完成标准照旧。minimumDuration 的默认值与外推公式，您说得出。canceled 的三条出处与构造时接好的内部线，您数得清。任务到顶发不发 canceled，您答得准。不确定模式的动画在两派样式里各靠谁推，您讲得明。
