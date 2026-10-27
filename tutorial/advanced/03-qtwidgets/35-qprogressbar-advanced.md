---
title: "3.35 QProgressBar 进阶"
description: "入门篇用 QProgressBar 做了范围设置、跨线程更新与 busy 模式，进阶篇对着 6.9.1 源码认清它的分工：控件自身一个定时器都没有，paintEvent 只把 QStyleOptionProgressBar 填好交给 drawControl(CE_ProgressBar)，凹槽、色块、文本全由 style 画，text() 是虚函数，initStyleOption 填选项时调用，Fusion 的 label 以进度位置切两块矩形、text 色与 highlightedText 色各裁剪绘制一遍，busy 动画在 Fusion 与 Windows style 里由挂在控件上的 QProgressStyleAnimation 驱动，determinate 绘制时反手 stopAnimation，QCommonStyle 基础实现把 busy 位置算成 (progress - minimum) 对两倍宽取模的三角波、值不动线不动，setValue 在 min 与 max 同为 0 时越界检查整体短路、外部递增永远有效，reset() 把值置为 minimum 减 1 后直接 repaint、不发 valueChanged 也不发无障碍事件，repaintRequired 按端点、%v、百分比变化与像素密度节流重绘，setRange 用 qMax 把 max 抬到 min 之上、传反了塌成单步、totalSteps 为 0 时 %p 给 100，垂直条默认从下往上填、invertedAppearance 翻转，textDirection 映射的 bottomToTop 字段在整个 qtbase 没有任何 style 读取，sizeHint 宽为 max(9, PM_ProgressBarChunkWidth) 乘 7 加字符 0 字宽乘 4、高为字体高加 8，配套示例在 examples/advanced/03-qtwidgets/35-qprogressbar-advanced/"
---

# 现代Qt开发教程（进阶篇）3.35——QProgressBar 进阶

[3.34 QLabel 进阶](./34-qlabel-advanced.md) 收尾把咱们送进显示家族第二站。QLabel 难在同一段文字走哪条管线，QProgressBar 的难处换个方向：控件自己几乎不落笔，凹槽、色块、百分比文字统统外包给 QStyle，自己只把值、范围、方向这些状态记准。咱们对着源码把外包链摸清，busy 动画由谁驱动、reset 为什么悄悄回卷、垂直条的文本到底转不转，这几处源稿说法偏得最远，正文按源码改准。

默认值照 34 篇的样子记下。不带参数构造，minimum 0、maximum 100，value 却是负 1，出处 qprogressbar.cpp 第 47-53 行，对齐左对齐，textVisible 为真，orientation 水平，invertedAppearance 假，textDirection 取 TopToBottom，format 是 %p 加本地化的百分号。正文引用以 qprogressbar.cpp 为主，绘制下沉到 qcommonstyle.cpp 与 qfusionstyle.cpp，busy 动画的推进看 qstyleanimation.cpp 与 qwindowsstyle.cpp，动画事件的接收还查了 qwidget.cpp。笔者把这几个文件对着源稿断言逐条过了一遍，busy 驱动与 textDirection 两处偏得最远。

## paintEvent 只递选项不落笔，文本的归属也在 style

咱们从 paintEvent 看起，整个函数就这么几行：

```cpp
// qprogressbar.cpp 第 376-383 行
void QProgressBar::paintEvent(QPaintEvent *)
{
    QStylePainter paint(this);
    QStyleOptionProgressBar opt;
    initStyleOption(&opt);
    paint.drawControl(QStyle::CE_ProgressBar, opt);
    d_func()->lastPaintedValue = d_func()->value;
}
```

事件参数连形参名都没起，函数体一次也没碰它，您重写时把 event 透传给父类就是了，示例 busy_bar_demo.cpp 第 34 行正是这么写的。QCommonStyle 收到 CE_ProgressBar 后按凹槽、色块、文本的次序委托下去，textVisible 为真才轮到 label 那段，出处 qcommonstyle.cpp 第 1496-1508 行。所以您关掉 textVisible，省的只是 label 一段的绘制，色块照画。

文本的计算与绘制是分家的。text() 是虚函数，声明在 qprogressbar.h 第 43 行，initStyleOption 往选项里填 option->text 时调用它，出处 qprogressbar.cpp 第 103 行，画笔始终握在 style 手里。这给换文字留了一条轻路：重写 text()，内置 label 的对齐、位置、配色机制全跟着您的文字走，子类化一节咱们再展开。

Fusion 的 label 还有个源稿没提的巧劲。它以进度指示器位置把 label 矩形切成两块，文字画两遍、各裁剪各的，未填充一侧用 text 色、填充一侧用 highlightedText 色，出处 qfusionstyle.cpp 第 1297-1329 行。深色块压浅背景时的可读性难题，内置文本早就用双色解掉了，您自绘 overlay 时才需要自己操心对比度。

对齐还牵出一处 style 分岔。QCommonStyle 家族里，非居中对齐会把 label 挪到条右侧的独立区域，条身缩窄，预留宽度取当前文本与 “100%” 里较宽的那个再加 6，出处 qcommonstyle.cpp 第 2594-2607 行。Fusion 不认这套，subElementRect 对凹槽、色块、label 三个元素一律返回整个控件矩形，出处 qfusionstyle.cpp 第 3469-3472 行，所以 Fusion 下文本始终压在条中央，alignment 改了也不挪窝。笔者拿离屏小程序对 Fusion 量过凹槽、色块、label 三个矩形，默认 AlignLeft 下都占满整个控件。

textVisible 也不是承诺。属性文档写明它可能被 style 无视，Mac 平台样式从不画文本，出处 qprogressbar.cpp 第 338 行。枚举文档里“文本是否绘制取决于 style”的注脚还列着 CleanLooks、Plastique 这些 Qt4 时代的名字，出处第 185-187 行，现役主力只剩 Fusion，老注脚您别当现状读。

## busy 动画：Fusion 自己挂动画对象，CommonStyle 系等您喂值

setRange(0, 0) 进 busy，入门篇讲过用法，还说动画由内部定时器驱动，这句话得改口。进阶篇要回答的是动画由谁驱动，头一件要讲清的：QProgressBar 自身一个定时器都没有，笔者把 qprogressbar.cpp 从头翻到尾，QTimer、QBasicTimer 一个都找不到。动画要么由 style 挂在控件上的动画对象推，要么压根没人推。

Fusion 走自驱，咱们顺着头一次绘制看。busy 态下样式检查控件上有没有挂 QProgressStyleAnimation，没有就起一个，出处 qfusionstyle.cpp 第 1257-1264 行，斜线纹路的位置取 animationStep 对 22 取模，出处第 1259 行，步数由动画运行时间除以帧间隔算出，出处 qstyleanimation.cpp 第 103-106 行，速度参数取 60，出处 qfusionstyle.cpp 第 247 行。动画对象到点给控件发 StyleAnimationUpdate 事件，出处 qstyleanimation.cpp 第 69-76 行，QWidget 只在可见且窗口没最小化时接受并 update，出处 qwidget.cpp 第 9317-9321 行，事件被拒动画自己停，出处 qstyleanimation.cpp 第 74-75 行。画到 determinate 态时样式反手 stopAnimation，把动画对象撤下来，出处 qfusionstyle.cpp 第 1248-1251 行。Windows style 的 busy 同款自驱，出处 qwindowsstyle.cpp 第 1608-1612 行。这套动画对象住在私有头文件 qstyleanimation_p.h 里，应用代码别去碰它。

QCommonStyle 的基础实现是另一个世界。busy 位置 x 取 progress 减 minimum 后对两倍宽度取模，超过一个宽度就折返，一根 4 像素宽的高亮竖线来回扫，出处 qcommonstyle.cpp 第 1565-1572 行。位置纯由 progress 值算出，与时钟无关，值不动，线就不动。您在某些自绘或第三方 style 下看到 busy 死住，根因就是这段画法只认值、不读时间。

外部 QTimer 喂值的写法为什么能救场，咱们到 setValue 的早退条件里找答案：值相同就返回，越界且范围不是双零也返回，出处 qprogressbar.cpp 第 285-288 行。min 与 max 同为 0 时，条件里的 (d->maximum != 0 || d->minimum != 0) 整体为假，越界检查被短路，任何值都收下。示例的 animDriver 每 30 毫秒 setValue(value() + 1) 一直往上加也没事，底气就在这，出处 busy_bar_demo.cpp 第 94-98 行。busy 态下 text() 返回空串，出处 qprogressbar.cpp 第 432 行，%p、%v 一律不替换，递增的值在 CommonStyle 系里只当动画相位使。

## reset 悄悄回卷，setValue 带着节流

reset() 把值置为 minimum 减 1，minimum 恰好是 INT_MIN 时直接置 INT_MIN 防下溢，然后 repaint，出处 qprogressbar.cpp 第 225-233 行。整段没有 emit valueChanged，也没有无障碍更新。您拿 valueChanged 驱动的联动界面，在 reset 时全都不会跟着回零，这个静默回卷排查时容易漏。

回卷值 minimum 减 1 落在范围之外，text() 对 value 小于 minimum 的情况一律返回空串，出处第 432-433 行，属性文档把这个状态叫 reset 态，出处第 421-427 行。刚构造出来的进度条也在这个态里，初始 value 是负 1，落在 minimum 0 之下，所以您 new 出来的条默认不显示 0%，文本是空白，出处第 48 行。

style 对这个越界值早有防备。Fusion 画色块起手就拿 qMax(progress, minimum) 兜住，行内注释写明这是给 QProgressBar 的老缺陷打的补丁（原文 `workaround for bug in QProgressBar`），出处 qfusionstyle.cpp 第 1202 行。私有类里还留着个 bound 帮手函数，做着从 minimum 减 1 到 maximum 的钳位，笔者在 qprogressbar.cpp 里找不到它的调用者，出处 qprogressbar.cpp 第 43 行。钳位的责任从控件挪去了 style。

setValue 的另一面是节流。值真的变了才发 valueChanged，出处第 289-290 行，repaint 与否交给 repaintRequired 裁决，出处第 297-298 行，判据咱们挨个看：值与上次画的不同才有下文，到达 minimum 或 maximum 端点必重绘，文本可见且格式带 %v 必重绘，%p 要百分比变化至少一个点才重绘，都不满足时按凹槽像素密度算，值差换算超过一像素才重绘，出处第 108-142 行。paintEvent 末尾把 lastPaintedValue 记下，出处第 382 行。您高频 setValue 时进度条并不每拍都重画，节流在替您省帧。

## setRange 的边界：传反了塌成单步，双零才是 busy

setRange 内部拿 qMax(minimum, maximum) 当新的 maximum，出处 qprogressbar.cpp 第 320-331 行。您把最小值写得比最大值还大，不会有任何警告，max 被抬平到 min，范围塌成单步。min 与 max 相等且非零时，text() 里 totalSteps 为 0，%p 直接给 100，行内注释写明是为了避开除零，出处第 444-449 行。

当前值落在新范围之外就触发 reset，出处第 327-328 行，判定特意给 minimum 减 1 留了余量，回卷态不至于改一次范围又被重置一遍。您设负方向的范围也完全合法，totalSteps 用 qint64 装，出处第 436 行，setRange(-100, -50) 照常工作。setMinimum 会把 maximum 顶到不低于新 min，setMaximum 把 minimum 压到不高于新 max，出处第 244-247 与 265-268 行，两个单向闸门保证范围永远不倒挂。

## 垂直、翻转与 textDirection：一个没人读的开关

setOrientation 换掉方向标志，sizePolicy 没被您动过就顺手转置，接着重算 layout item margins，update 与 updateGeometry 收尾，出处 qprogressbar.cpp 第 468-481 行。min 与 max 不跟着换，方向只是画法的事。填充方向上，垂直条默认从下往上长，invertedAppearance 翻成从上往下。CommonStyle 家族把水平 RTL 布局与垂直都算 reverse，inverted 再取反，出处 qcommonstyle.cpp 第 1561-1563 行，色块从哪端起画由它定，出处第 1604-1605 行。Fusion 给垂直条上正负 90 度的旋转变换，inverted 决定转角方向，出处 qfusionstyle.cpp 第 1184-1193 行。咱们把两段变换的坐标各自推了一遍，两边对得上，默认都从底端长起。

文本方向是源稿偏得最远的一处。源稿说垂直条的文字默认旋转 90 度竖排。textDirection 确实映射进了选项的 bottomToTop 字段，出处 qprogressbar.cpp 第 105 行，可笔者把整个 qtbase 搜了一遍，除选项构造函数的默认置假外只有赋值那一行，更没有任何 style 读取它。Fusion 画 label 的整段代码没有旋转变换，垂直与水平共用同一段 drawText，出处 qfusionstyle.cpp 第 1294-1329 行。QCommonStyle 更干脆，见垂直直接跳过 label，出处 qcommonstyle.cpp 第 1518 行。旋转 90 度、倒排文字这些说法全出自枚举文档，出处 qprogressbar.cpp 第 182-187 行，而文档里列的 CleanLooks、Plastique，全是 Qt4 时代的老 style。现役 style 下您别指望 textDirection 起作用，垂直条要显示数值，外置一只 QLabel 接 valueChanged 最稳。

尺寸提示的来路也记下。宽取 max(9, PM_ProgressBarChunkWidth) 乘 7 加字符 0 的字宽乘 4，高取字体高加 8，垂直方向整体转置，末了过 style 的 sizeFromContents，出处 qprogressbar.cpp 第 388-399 行，chunk 宽默认 9，出处 qcommonstyle.cpp 第 4709-4710 行。垂直条的默认宽就是字体高加 8，和水平条的高度同源，横排竖排的文字都塞不下，窄是结构性的，要看得清就得您自己 setFixedWidth。

## 换文字：重写 text() 比重写 paintEvent 轻

轻路是重写 text()，虚函数声明在 qprogressbar.h 第 43 行。您返回什么，内置 label 就显示什么，对齐、位置、Fusion 的双色机制全不用您管。%p、%v、%m 的替换发生在默认 text() 里，出处 qprogressbar.cpp 第 441-453 行，重写之后这些占位符就跟您无关了。

重路是重写 paintEvent。头一步把 event 透传给父类画好底，再起 QPainter 叠自己的内容，画家算法后画的压住前面画的，顺序反了文字会被底色整个盖掉。父类的形参没有名字也没人用，传 nullptr 不会出事，出处 qprogressbar.cpp 第 376 行，透传 event 仍是更体面的写法。示例的 OverlayTextBar 就是范本：算出填充宽度，clipRect 把白字限制在填充区内，出处 busy_bar_demo.cpp 第 31-58 行，ratio 在 range 不大于 0 时取 0，出处第 44-48 行，单步与 busy 状态下不至于除零。对比度只有自绘路径需要您自己操心，内置文本 Fusion 已经用双色两遍解掉了。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，QProgressBar 在 QtWidgets，自绘用的 QPainter 在 QtGui，链接 Qt6::Widgets 即可。

[Qt 文档 · QProgressBar](https://doc.qt.io/qt-6/qprogressbar.html) -- 进度条控件，text、format、invertedAppearance 等属性说明

[Qt 文档 · QStyle](https://doc.qt.io/qt-6/qstyle.html) -- 样式基类，CE_ProgressBar 系列绘制元素与 subElementRect

[Qt 文档 · QStyleOptionProgressBar](https://doc.qt.io/qt-6/qstyleoptionprogressbar.html) -- 进度条绘制选项，progress、text、bottomToTop 字段

[Qt 文档 · QPainter](https://doc.qt.io/qt-6/qpainter.html) -- 绘图引擎，clipRect 裁剪与自绘叠加

[Qt 文档 · QAbstractAnimation](https://doc.qt.io/qt-6/qabstractanimation.html) -- 动画基类，样式动画 QStyleAnimation 借它的时钟驱动

---

显示家族第二站到这收住，下一程还在原地等咱们，QLCDNumber 把数值显示换成数码管字形，入口在 [3.36 QLCDNumber 进阶](./36-qlcdnumber-advanced.md)。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/35-qprogressbar-advanced/，可执行文件 35-qprogressbar-advanced）。第一区 busy 模式：setRange(0, 0) 加 setTextVisible(false)，出处 busy_bar_demo.cpp 第 88-90 行，QTimer 每 30 毫秒递增喂值，出处第 94-98 行，开始与停止按钮只控制定时器，出处第 100-109 行，提示文字也点明了停掉定时器后某些 style 会停，出处第 111-113 行。第二区 OverlayTextBar：setRange(0, 10)、固定高 32，下一步按钮推进值并换 overlay 文字，重置按钮归零，出处第 137-161 行。

实验补两步。您在 Fusion 下点第一区的停止，动画照转，样式挂的动画对象还在推，停掉的只是喂值定时器，来路在 qfusionstyle.cpp 第 1257-1264 行。再把第二区推到 10 后点重置，画面回零，此时您把那行 setValue(0) 换成 reset() 再跑一遍，画面一样，但接一行 qDebug 打 valueChanged 就看得出差异：reset 整段没有 emit，出处 qprogressbar.cpp 第 225-233 行。

完成标准照旧。busy 动画在 Fusion 与 CommonStyle 系下各靠什么推进，您说得出对应的源码位置。reset 之后 value 落在哪、valueChanged 发不发，您不用翻文档就能答。textDirection 为什么不起作用、那个没人读的字段叫什么，您指得出来。setRange(100, 0) 之后范围塌成什么、%p 显示多少，您算得出来。
