---
title: "3.31 QSlider 进阶"
description: "入门篇把 QSlider 当拖拽调值的控件用会了，进阶篇对着 6.9.1 源码把拖拽链路认清，拖拽中 sliderPosition 走在 value 前面，sliderMoved 只在按住时发，tracking 默认开着、valueChanged 拖拽中就发，tracking 关掉后松手由 setSliderDown 补提交，左键点 groove 默认是页步进并自动连发，中键才做绝对跳转，绝对跳转的映射跨度是 groove 减一个手柄宽，两端各留半个手柄的无效区，拖拽保持的是抓取点偏移，刻度跟 groove、handle 一样由 QStyle 画，QSS 管不到，刻度疏密在 tickInterval 设 0 时由 style 按 singleStep 与 pageStep 自挑，invertedAppearance 折进 QStyleOptionSlider 的 upsideDown，视觉与点击映射一起翻，invertedControls 只翻键盘与滚轮、Home 与 End 不翻，默认构造是垂直方向、默认范围 0 到 99，配套示例在 examples/advanced/03-qtwidgets/31-qslider-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.31——QSlider 进阶

[3.30 QDateTimeEdit 进阶](./30-qdatetimeedit-advanced.md) 收尾把咱们送到了 QAbstractSlider 家族，QSlider 从它继承（qslider.h 第 17 行），没有行编辑框也没有节段，手柄在 groove 上的位置本身就是值。它自己的接口比输入类那几位少得多，调值的事全在拖拽链路里，咱们按次序走，从拖拽中哪个值算数认起，跟着是点击怎么变成数值、刻度归谁画、方向反转翻哪一头。笔者把 qslider.cpp 与 qabstractslider.cpp 对着源稿断言逐条过了一遍，按钮分工与 tracking 的默认行为两处偏得最远，正文按源码改准。

咱们顺手记下默认值。不带参数的 QSlider 构造出来是垂直方向（qslider.cpp 第 253-255 行），要水平得显式传 Qt::Horizontal，默认范围 0 到 99，pageStep 10，tracking 默认开（qabstractslider.cpp 第 178-180 行）。正文行号都带出处，示例在 examples/advanced/03-qtwidgets/31-qslider-advanced/，Qt 内部实现指源码树里的 qslider.cpp 与 qabstractslider.cpp，style 侧的实现落在 qcommonstyle.cpp 与 qfusionstyle.cpp。

## 拖拽中 sliderPosition 走在 value 前面，值落地有两条路

QAbstractSlider 的私有数据里存着 value 与 position 两个数，构造初始化表里各占一位（qabstractslider.cpp 第 178-180 行）。咱们拖动手柄或按方向键，动的一直是 position，入口是 setSliderPosition（第 457 行），传入值当场夹进合法范围（第 460 行），其余动作都在这十来行里。

范围本身动的时候，值会被重新夹一遍。setRange 把范围改掉后，拿旧的 value 再跑一次 setValue，源码注释写着 re-bound（qabstractslider.cpp 第 207-218 行）。所以您把上限从 100 收到 50，停在 80 的值会自己落到 50，valueChanged 跟着发一轮，咱们依赖旧值写的逻辑得防着这一下。

sliderMoved 有个安静的细节，它只在滑条被按住时发（第 466-467 行），程序里自己调 setSliderPosition 不会惊动它。您想在拖拽中每帧知道手柄在哪，连 sliderMoved 就够，它转播的就是 position。

头文件里这些信号各占一行（qabstractslider.h 第 87-95 行），valueChanged 管值，sliderPressed 与 sliderReleased 管按下的起止，sliderMoved 转播拖拽过程，rangeChanged 与 actionTriggered 跟在后面。sliderPressed 与 sliderReleased 的发源地是 setSliderDown，状态一翻就发（qabstractslider.cpp 第 433-438 行），按下那一发来自 mousePressEvent，它抓到手柄后调 setSliderDown(true)，出处 qslider.cpp 第 379 行。键盘与滚轮的步进最终也汇进 triggerAction，出处 qabstractslider.cpp 第 888 行，它每跑一次都发 actionTriggered，出处第 607 行，您想旁听所有输入，连 actionTriggered 就够。

值的落地有两条路。tracking 开着时（默认就是），setSliderPosition 末尾直接 triggerAction(SliderMove)（第 468-469 行），而 triggerAction 的收尾一行是 setValue(d->position)（第 609 行），所以拖拽中 valueChanged 一步一步都在发，value 与 position 始终咬合。源稿出过一道调试题，说拖拽中预览不动、松手才跳，咱们把前提找出来，其实是有人显式 setTracking(false)，默认行为下不会这样。

tracking 关掉后才走第二条路。咱们看松手那一端，mouseReleaseEvent 调 setSliderDown(false)，出处 qslider.cpp 第 414 行，它发现 position 与 value 不等，就补一次 triggerAction(SliderMove)，出处 qabstractslider.cpp 第 440-441 行。

```cpp
// qabstractslider.cpp setSliderDown 的补提交（第 440-441 行）
if (!down && d->position != d->value)
    triggerAction(SliderMove);
```

tracking 开着，您在 sliderMoved 的槽里读 value()，拿到的东西与信号参数同值。tracking 关了才分出两截，拖拽中 value 停在上一次的旧值，松手才追上 position，双值机制的差别，全看 tracking 开不开。源稿叮嘱别在 sliderMoved 槽里拿 value() 当手柄当前位置，这个提醒只在 tracking 关闭时成立，照着默认行为记反而会记错。

## 左键的默认差事是页步进，绝对跳转轮到中键

源稿把 “点击 groove，手柄中心跳到点击处” 当成点击的默认行为，咱们读 mousePressEvent（qslider.cpp 第 324 行起）就会看到按钮是分工的，哪个键干什么由 style hint 决定。绝对跳转键的 hint 叫 SH_Slider_AbsoluteSetButtons，默认给 Qt::MiddleButton，出处 qcommonstyle.cpp 第 5075-5077 行。页步进键的 hint 叫 SH_Slider_PageSetButtons，默认 Qt::LeftButton，出处第 5078-5080 行。笔者核对到这两行才对上，左键点 groove 走的是页步进分支，控件把点击处换算成 pressValue 与当前值比大小，选 SliderPageStepAdd 或 Sub，还 setRepeatAction 让按住连发，出处 qslider.cpp 第 348-365 行。中键按下才走绝对跳转，手柄中心一步到点击处，出处第 336-345 行。

```cpp
// qslider.cpp 绝对跳转分支（第 338-344 行，节选）
const QPoint center = sliderRect.center() - sliderRect.topLeft();
setSliderPosition(d->pixelPosToRangeValue(d->pick(ev->position().toPoint() - center)));
triggerAction(SliderMove);
```

门口还有一道守卫值得您记下。minimum 与 maximum 相等，或者事件发生时别的键还按着，mousePressEvent 一进门就 ignore 返回，出处 qslider.cpp 第 327-330 行。范围退化成单点的滑条对鼠标装死，您别当它失灵，那是源码里写好的短路。

两处共用同一个换算函数 pixelPosToRangeValue，出处第 65 行。咱们拿水平方向算，可用跨度是 groove 左端到 groove 右端减一个手柄宽，出处第 74-77 行，换算交给 QStyle::sliderValueFromPosition 做线性映射，出处第 82-84 行，实现的入口在 qstyle.cpp 第 2268 行，两端自带夹取。绝对跳转在传坐标前减掉半个手柄（qslider.cpp 第 338-343 行，注释写明用 center 减 topLeft），手柄中心因此对齐点击点，也正因此中心最远只能到 groove 两端各留半个手柄宽的位置。源稿说的半个手柄无效区是真的，只是它归在中键名下。

拖拽阶段维持的是您的抓取点。按下时 clickOffset 记下按点相对手柄左上角的偏移（第 377 行），mouseMoveEvent 每次拿鼠标坐标减掉这个偏移再换算（第 394-395 行），您按在手柄边缘，拖起来就还是边缘跟手，手柄不会跳到鼠标中心。

想改交互曲线或限制范围，动手的位置在 mouseMoveEvent 里 setSliderPosition 之前，把换算结果夹好再交出去。您要是反过来，在 sliderMoved 或 valueChanged 的槽里 setValue，就会跟鼠标打架。mouseMoveEvent 每一步都按鼠标重算 position，tracking 又把 value 拉回 position（第 609 行），而 setValue 刚把 value 与 position 一起写过（第 505-511 行），下一步就被覆盖，滑块在拖拽中反复跳回。拖拽期间别用 setValue 改值，源稿警告的正是这层互相覆盖。

## 刻度归 QStyle 画，QSS 到这里没了手

QSlider::paintEvent 自己一笔都不画。它构造 QStyleOptionSlider，subControls 摆上 groove 与 handle，tickPosition 不是 NoTicks 才把 SC_SliderTickmarks 加进去，整包交给 QStylePainter 的 drawComplexControl，出处 qslider.cpp 第 282-293 行。刻度线长什么样、画多密，全是 style 的决定，QSS 能动 groove 与 handle 的外观，刻度线与标注文字没有入口，您试到这儿就该换路子了。

刻度的疏密也归 style 挑。tickInterval 是值的间隔，单位不是像素，setTickInterval 的文档写得分明（qslider.cpp 第 487-496 行，实现第 498 行）。您设 0，Fusion 就自己挑，拿 singleStep 试，间隔换算成像素不足 3 再换 pageStep，两样都不行给 1（qfusionstyle.cpp 第 2631-2642 行）。所以刻度忽密忽疏时，除了查 tickInterval，咱们也看看两档步长改没改。

刻度还占布局。sizeHint 给滑条算厚度时，在 PM_SliderThickness 之上，TicksAbove 或 TicksBelow 每开一处加 5 像素，出处 qslider.cpp 第 424-435 行，setTickPosition 落定后还会 updateGeometry，出处第 472-478 行。所以刻度一开，滑条在布局里就变厚，您要是看到它挤到邻居，别当成样式串了，那就是刻度占的厚度。

想插手有两条路。一条是 QProxyStyle，重写 drawComplexControl，在调基类的前后画自己的东西，您只拦关心的步骤，其余交给平台 style，原生外观保得住。另一条是子类化 QSlider 重写 paintEvent，让基类把 groove、handle 与系统刻度画完，再拿 QPainter 补标注，示例选了重写 paintEvent 的路子，出处 custom_tick_slider.cpp 第 69-103 行，基类绘制在第 72 行，标注文字画在第 101 行，横向位置按 groove 矩形的比例算，出处第 92 行。

initStyleOption 始终把控件的 tickPosition 填进 option，出处 qslider.cpp 第 111 行，没设过刻度的滑条，这个字段就是 NoTicks。您在 QProxyStyle 里拿 opt.tickPosition != NoTicks 当画标注的条件，控件又忘了 setTickPosition，您的绘制就永远不会跑，界面上什么都看不出来。要让标注独立于系统刻度存在，判断条件换成自定义属性，或学示例只看自己的 m_tickRatios，出处 custom_tick_slider.h 第 63 行。

## 反转是一对开关，一个管画与映射，一个管键盘滚轮

invertedAppearance 与 invertedControls 都是 QAbstractSlider 的属性，声明在 qabstractslider.h 第 29-30 行，名字像一对，管的却是两摊事，咱们分开看。

invertedAppearance 进的是绘制与换算共用的字段。initStyleOption 把它折进 QStyleOptionSlider::upsideDown，水平方向还要与 RightToLeft 异或一次（qslider.cpp 第 113-115 行），紧接着 direction 被强制设成 LeftToRight，注释写明用 upsideDown 顶替方向字段（第 116 行）。笔者读到这行注释停了一下，方向与反转在 option 里合并成了一个开关。upsideDown 既给 style 画 groove 高亮与手柄用，出处 qfusionstyle.cpp 第 2579-2586 行，又给数值换算用，出处 qslider.cpp 第 82-84 行，QStyle::sliderValueFromPosition 拿它决定坐标两端谁映射 min 谁映射 max，出处 qstyle.cpp 第 2268-2272 行。源稿说它只影响视觉，说小了，点击映射跟着视觉一起翻，翻得倒齐，两边不会各说各话。垂直滑条默认最大值在上，开反转后最大值到下。

invertedControls 只翻操作。键盘的左右、上下与翻页全按它选方向，出处 qabstractslider.cpp 第 822、842、856、869、872-875 行，滚轮方向也由它取反，出处第 715 行。Home 与 End 不参与，无条件去 SliderToMinimum 与 SliderToMaximum（第 878、881 行），您把方向键反转了，Home 还是奔最小值那头。

滚轮值得咱们单看一段。scrollByDelta 会把不足一步的零头攒起来，滚下一个事件时接着算（qabstractslider.cpp 第 695 行），按住 Ctrl 或 Shift 滚，步数直接按一整页算（第 678-680 行），水平方向的滑条还要把 delta 取一次反（第 675 行），invertedControls 的取反发生在步数算完之后，出处第 715 行。

只开 invertedAppearance 不开 invertedControls，画面反了、按键没反。您按右方向键，值在涨，手柄却朝画面的左边走，因为最大值那一端已经换到左边了，两头打架。真要反转就两个一起设，视觉与操作同步掉头。刻度不用操心，TicksAbove 与 TicksBelow 的方位是纯视觉，Fusion 直接读 tickPosition 不交换，出处 qfusionstyle.cpp 第 2538-2539 行，刻度线的值位置会跟着 upsideDown 翻转对齐手柄，出处第 2654 行，标 0 的线始终贴在最小值那一头。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，QSlider 与 QAbstractSlider 都在 QtWidgets，链接 Qt6::Widgets 即可。

[Qt 文档 · QSlider](https://doc.qt.io/qt-6/qslider.html) -- 滑动条控件，TickPosition 枚举与刻度接口在这页

[Qt 文档 · QAbstractSlider](https://doc.qt.io/qt-6/qabstractslider.html) -- 抽象基类，value、sliderPosition、tracking 与两个反转属性的定义

[Qt 文档 · QStyleOptionSlider](https://doc.qt.io/qt-6/qstyleoptionslider.html) -- 传给 style 的状态包，upsideDown、sliderPosition 与 subControls 字段

[Qt 文档 · QProxyStyle](https://doc.qt.io/qt-6/qproxystyle.html) -- 样式代理，拦截 drawComplexControl 的入口

[Qt 文档 · QStyle](https://doc.qt.io/qt-6/qstyle.html) -- drawComplexControl 与 sliderValueFromPosition 的正式说明

---

滑条认完，下一站 [3.32 QScrollBar 进阶](./32-qscrollbar-advanced.md)。滚动条与 QSlider 同一个基类，今天认的双值机制、tracking 与页步进原样适用，多出来的滚动语义咱们去那边接着认。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/31-qslider-advanced/，可执行文件 31-qslider-advanced），窗口里一枚 CustomTickSlider 加两块标签（main.cpp 第 29、39-40 行），范围 0 到 100、初始值 50（第 30-31 行），两块标签分别挂在 sliderMoved 与 valueChanged 上（第 47-54 行），groove 下方画着 0% 到 100% 的五档标注，出处 custom_tick_slider.cpp 第 21、92-101 行。

实验咱们换中键走三步。示例的鼠标事件只拦左键，左键的按下与拖动都被换成了自己的直接映射，中键的按下与拖动原样交给基类（custom_tick_slider.cpp 第 42-52、54-63 行），咱们想看 QSlider 本来的拖拽链路，中键就是这枚滑条上唯一的入口。第一步您用中键按住手柄正中慢慢拖，两块标签就同步逐帧动了起来，posLabel 靠的是 sliderMoved，它转播的就是 position（qabstractslider.cpp 第 466-467 行），tracking 咱们没动、默认开着，所以每一步都补一次 setValue（第 468-469 与 609 行），valLabel 的 valueChanged 也跟着每帧发。第二步您在 main.cpp 第 31 行后面添一句 slider->setTracking(false)，换中键慢慢地拖一遍，posLabel 照旧一帧一帧地走，valLabel 却停在了旧值上，拖拽中的 valueChanged 一次都不发，等您松手才跳一次，补提交的出处就是 setSliderDown 的第 440-441 行。第三步您把手柄归回 50，左键在 groove 宽度九成的地方点一下松开，您再换中键点同一个点。左键一按下去就跳到了 90，页步进没了，连发也没了，因为左键的默认差事已经被示例换掉了。中键点的还是同一个地方，落点会抬到 92 上下，窗口几何不同会差个几分，中键比左键高出的部分不变，原因在映射的跨度上，基类算的是 groove 减一个手柄宽（qslider.cpp 第 74-77 行），换算前还减了半个手柄对齐中心（第 338-343 行），同一像素在更短的跨度里占的比例更大，值就抬了上去。您看同一个像素点出两个值，两套映射当场就分开了。

完成标准咱们照旧，tracking 开与关两条落地路的触发时机您说得清，左键与中键在同一枚滑条上各走哪套映射，您能分开报出两边的值，反转只开一半时手柄会朝哪边走您心里有数。
