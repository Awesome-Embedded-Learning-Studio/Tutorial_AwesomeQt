---
title: "3.32 QScrollBar 进阶"
description: "入门篇把 QScrollBar 当驱动自定义绘制的数值源用，进阶篇对着 6.9.1 源码把滚动语义认清，手柄长度由 style 按 pageStep 乘可用长度除以 range 加 pageStep 的公式算出并钳到 PM_ScrollBarSliderMin（common 给 9 像素、Fusion 给 26 像素），空范围时手柄撑满且对鼠标装死，把滚动条收起来的是 QAbstractScrollArea 的 AsNeeded 策略，滚轮一格的步数是 wheelScrollLines 默认 3 乘 delta 的一百二十分之一乘 singleStep、单次封顶一页，Ctrl 或 Shift 按整页走，点 groove 翻一页并以 500 毫秒首延 50 毫秒间隔连发，绝对跳转要左右键或中键的 hint 放行（common 默认都不放行，Fusion 放行中键），resize 前存下 value 与 max 的比例再恢复，pageStep 为 0 时手柄被钳到下限且 groove 点击步长为零，QSS 接管几何但公式照跑，min-height 正好落在钳位下限，配套示例在 examples/advanced/03-qtwidgets/32-qscrollbar-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.32——QScrollBar 进阶

[3.31 QSlider 进阶](./31-qslider-advanced.md) 收尾说，双值机制、tracking 与页步进到滚动条这儿原样适用，多出来的滚动语义接着认。分岔就在滚动语义上，滑条的手柄长短只是画出来的样子，滚动条的手柄长短本身携带信息，它告诉您一屏占了整份文档的多少。手柄长短的像素值您设不了，长度由一个公式算出来，range 与 pageStep 的联动也绕着这个公式转。咱们从公式认起，滚轮、点击、resize、QSS 逐个落回源码。

默认值照 31 篇的样子记下。不带参数的构造出来是垂直方向（qscrollbar.cpp 第 315-318 行），默认范围 0 到 99，pageStep 10，singleStep 1，tracking 开（qabstractslider.cpp 第 178-183 行）。与基类有一处分歧值得您记下，QScrollBar 的 invertedControls、也就是操作反转，默认开，出处 qscrollbar.cpp 第 350 行，基类默认关，出处 qabstractslider.cpp 第 182 行，键盘与滚轮的方向在滚动条这边默认按反转语义解读，滚轮一节咱们再对着源码看。focusPolicy 默认 NoFocus（qscrollbar.cpp 第 357 行），想要键盘翻页得自己开。正文行号都带出处，Qt 内部实现指源码树里的 qscrollbar.cpp 与 qabstractslider.cpp，style 侧落在 qcommonstyle.cpp、qfusionstyle.cpp 与 qstylesheetstyle.cpp，滚动区域的部分在 qabstractscrollarea.cpp。笔者把六个文件对着源稿断言逐条过了一遍，隐藏机制与除零两处偏得最远，正文按源码改准。

## 手柄多长不归您定，公式与钳位都在 style 手里

咱们翻出 QCommonStyle::subControlRect，只看 CC_ScrollBar 分支，从可用长度看起。滚动条总长减去两端箭头按钮占的位置，按钮的宽度只在非 transient 的 style 里才扣，出处 qcommonstyle.cpp 第 4104-4108 行，SH_ScrollBar_Transient 默认 false，出处第 5367-5369 行。长度按比例切：

```cpp
// qcommonstyle.cpp 第 4112-4114 行
if (scrollbar->maximum != scrollbar->minimum) {
    uint range = scrollbar->maximum - scrollbar->minimum;
    sliderlen = (qint64(scrollbar->pageStep) * maxlen) / (range + scrollbar->pageStep);
}
```

写成数学式就是 pageStep 除以 range 加 pageStep，再乘可用长度。语义是手柄占 groove 的比例等于一屏占整份文档的比例，100 行的文档显示 20 行，手柄就占五分之一。QScrollBar 的文档把范围与文档长度的关系写成了等式，document length 等于 maximum 减 minimum，再加 pageStep，出处 qscrollbar.cpp 第 112 行，您想让 range 表达“内容超出视口的部分”，就把 max 设成内容尺寸减视口尺寸，20 行视口配 100 行文档，max 是 80，文档被切成整整齐齐五页（第 100-103 行）。文档同时提醒，范围超过十万在现在的屏幕上没法用，手柄细到拖不住（第 117-120 行）。

算出的长度要过两道闸。第一道是下限 PM_ScrollBarSliderMin，算出来比它小就抬到它（第 4116-4118 行），数值各 style 自己定，QCommonStyle 给 dpiScaled(9)（第 4536-4538 行），Fusion 给 26（qfusionstyle.cpp 第 2759-2761 行）。第二道与第一道写在同一个 if 里，range 超过 INT_MAX 的一半也直接抬到最小值（第 4117 行），笔者核到这一行才看清钳位有两个并列条件，那是防乘法溢出的保险，与视觉无关。上限另有一道，长度不许超过可用长度（第 4119-4120 行）。maximum 与 minimum 相等时根本不进公式，手柄直接取满可用长度（第 4121-4122 行），空范围的滚动条画出来是一根实心槽。sizeHint 也拿这批 style 的量来算，顺着滚动方向，它要两个 extent 加一个最小手柄的长度，出处 qscrollbar.cpp 第 418-426 行，保证滚动条至少装得下最小手柄。

您嫌手柄太小想放大，直接设像素没有入口。可行的路有两条。调大 pageStep，手柄按比例变长，代价是点 groove 翻页的步长跟着变大。或者 QProxyStyle 重写 pixelMetric，把 PM_ScrollBarSliderMin 抬高，只动下限不动比例，比例仍由 pageStep 与 range 说了算。

## resize 前存下比例再恢复，value 的钳位发生在两处

内容尺寸或视口尺寸一变，range 就得重配。您若只调 setRange 不管 value，值会被悄悄改写，钳位有两处。setValue 一进门就把传入值夹进范围，出处 qabstractslider.cpp 第 496-501 行，setRange 改完范围后，拿旧 value 重跑一遍 setValue，注释写着 re-bound（第 207-218 行）。窗口变宽把 max 从 2000 压到 1500，停在 1800 的值自己落到 1500，valueChanged 跟着发一轮。内容缩到不超过视口时 max 归零，值跟着归零，画面跳回顶部。resize 后位置的“跳”，来路都在钳位上。

咱们把比例存下来再恢复：

```cpp
double ratio = double(bar->value()) / qMax(1, bar->maximum())
bar->setRange(0, qMax(0, contentHeight - viewportHeight))
bar->setValue(int(ratio * bar->maximum()))
```

这样您看到的内容位置前后大体一致，不会突然跳到底或跳回顶。动态内容同理，列表项增删后忘了重配 range，滚到底剩空白只是表象，更隐蔽的是手柄比例与真实内容脱节，您拖动手柄时“拖了多少”与“内容滚了多少”对不上，因为手柄长度还是拿旧的 pageStep 与 range 算的，出处 qcommonstyle.cpp 第 4114 行。

配步长有两处源码级的事实，咱们摆出来。setSteps 对两个步长都取绝对值再存，出处 qabstractslider.cpp 第 222-228 行，负数存不进去。pageStep 设 0 也走不到除零，maximum 不等于 minimum 时分母是 range 加 0，必然大于零，出处 qcommonstyle.cpp 第 4112-4114 行，真实后果是分子为 0，手柄长度算出 0，被钳到最小值，出处第 4117-4118 行，同时点 groove 的翻页步长也是 0，出处 qabstractslider.cpp 第 591-595 行，整根滚动条失去交互。pageStep 至少给 1，正常场景给一屏的量。

## 滚轮一格走多远，三个数相乘再封顶

您滚一格滚轮，事件最终汇进 scrollByDelta，出处 qabstractslider.cpp 第 669 行起。普通滚动的步数由三个数相乘，QApplication::wheelScrollLines()，默认 3，出处 qplatformtheme.cpp 第 626-627 行，乘上 delta 除以 120 的比例，再乘 singleStep，出处 qabstractslider.cpp 第 686-690 行。singleStep 是 5 的滚动条，一格实际走 15，一格并不等于一个 singleStep。小数零头也不丢，除不尽的部分攒在 offset_accumulated 里等下一个事件（第 695 行），触控板轻拂能积累出一次滚动。单次滚动有封顶，步数夹在正负 pageStep 之间（第 698 行），一格滚不出一页多。按住 Ctrl 或 Shift 再滚，步数直接按整页算（第 678-681 行）。

方向上有一串取反，咱们理一理。水平滚动条的 delta 会被取一次反，出处 qscrollbar.cpp 第 489 行，基类在水平方向也有同款取反，出处 qabstractslider.cpp 第 674-675 行，invertedControls 再决定翻不翻，出处第 714-715 行，滚动条默认 true，出处 qscrollbar.cpp 第 350 行，开篇说的分歧在这儿兑现。还有个小便利，垂直滚轮也能滚水平滚动条，前提是没有同时的水平滚动分量，出处 qscrollbar.cpp 第 479-483 行，那是为了触控板上两个方向一起滚时不打架。

singleStep 的量级咱们按内容挑。设 1 的话，几千像素的内容每格滚 3 像素，太慢，常见做法是给视口高度的二十分之一到十分之一，行高固定的列表给一个行高。设 0 更干脆，步数恒为 0，scrollByDelta 走零步分支直接返回（第 690、701-711 行），滚轮整个失效，源稿那句“滚轮无效”的准确出处就是这段。

## 点 groove 翻一页还自动连发，绝对跳转要 style 放行

咱们从 mousePressEvent 的门口看起，一进门过三道检查，空范围、事件发生时还按着别的键、按下的不是左键，出处 qscrollbar.cpp 第 537-542 行。中键想被受理，得看 SH_ScrollBar_MiddleClickAbsolutePosition 给不给，出处第 531-532、539 行。检查全过，才拿 hitTestComplexControl 问 style，点中了哪个子控件，出处第 544 行。

您点中手柄两侧的空白区，activateControl 把 AddPage 或 SubPage 翻译成 SliderPageStepAdd 或 Sub，出处第 227-232 行，triggerAction 拿 pageStep 加减值，出处 qabstractslider.cpp 第 591-595 行。按住不放还有连发，动作被挂上 repeatAction（qscrollbar.cpp 第 249-251 行），首延 500 毫秒，之后每 50 毫秒一发（第 569、580-581 行）。

“点哪跳哪”的绝对跳转不是默认行为。mousePressEvent 里确实有这个分支（第 558-568 行），但要 hint 放行：左键要 SH_ScrollBar_LeftClickAbsolutePosition，QCommonStyle 默认 false（qcommonstyle.cpp 第 5447 行，落在第 5458 行返回 false 的大组里），中键要 SH_ScrollBar_MiddleClickAbsolutePosition，common 同样 false（第 5424 行同组）。笔者读到 Fusion 的 styleHint 才发现例外，它对中键这个 hint 返回 1（qfusionstyle.cpp 第 3389、3398 行），Fusion 桌面上中键点空白就是直接跳。对照 [3.31 QSlider 进阶](./31-qslider-advanced.md)，滑条把绝对跳转固定给了中键、页步进固定给了左键，出处 qcommonstyle.cpp 第 5075-5080 行，滚动条这边把两个都交给 style 表决，默认全关。

拖手柄维持的是抓取点。按下时 clickOffset 记下按点相对手柄左上角的偏移（第 553-556 行），move 时鼠标坐标减掉这个偏移再换算（第 620-622 行），您按在边缘拖起来就还是边缘跟手。拖远了还有回弹，PM_MaximumDragDistance 大于等于 0 时，鼠标离控件超出这个距离，位置弹回 snapBackPosition（第 623-629 行）。换算函数是 pixelPosToRangeValue，跨度取 groove 长度减一个手柄长（第 676-686 行），映射交给 QStyle::sliderValueFromPosition（第 688-689 行），与 31 篇滑条那套同构。

## 空范围它不自己躲，收起它的是 QAbstractScrollArea

源稿有个说法要改，它说 setRange(0, 0) 之后部分 style 会把滚动条整个隐藏。咱们把源码翻遍，QScrollBar 没有隐藏自己的路径，空范围时它对鼠标装死（mousePressEvent 门口短路，qscrollbar.cpp 第 537-542 行），画出来是撑满的手柄（qcommonstyle.cpp 第 4121-4122 行），控件仍在原地。收起滚动条的是 QAbstractScrollArea，布局时按策略判断，ScrollBarAsNeeded 下的条件是 minimum 小于 maximum，出处 qabstractscrollarea.cpp 第 302-311 行，不满足就 setVisible(false) 把滚动条容器收掉，出处第 407-408 行。

所以“滚动条一消失布局抖一下”的场景，动手的是滚动区域的策略，style 只是照常画。您独立使用 QScrollBar 自己搭布局，它不会消失，除非您自己 setVisible。想按内容量控制显隐，听 rangeChanged 比监听 resize 稳，范围一空就按自己的布局策略处理，别等控件替您决定。

## QSS 换皮不换骨架，min-height 落在钳位下限上

QScrollBar::paintEvent 一笔都不自己画，构造 option、填上 SC_All，交给 drawComplexControl，出处 qscrollbar.cpp 第 504-519 行。QSS 的接管比您想的深，QStyleSheetStyle 对 groove 与手柄，另有一套 subControlRect 分支，groove 矩形按 QSS 规则算（qstylesheetstyle.cpp 第 6025-6035 行），手柄长度在这个 groove 上重新跑一遍同款公式与钳位，出处第 6036-6048 行。皮换了，骨架没换，pageStep 与 range 该怎么联动还怎么联动。

手柄的 min-height 有正式的落点。QSS 路径下 PM_ScrollBarSliderMin 直接取值，取的是 handle 规则的 minimumSize（第 5168-5175 行），您写 min-height 就是在给钳位下限赋值。几行示意，每块只放一条声明，真实样式表里照常补分隔符：

```css
QScrollBar:vertical { width: 12px }
QScrollBar::handle:vertical { min-height: 30px }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px }
```

您想动公式本身，QSS 够不着，得用 QProxyStyle 重写 subControlRect，或改 pixelMetric。子控件的名单您记下来够用：groove 是槽，handle 是手柄，add-line 与 sub-line 是两端按钮，add-page 与 sub-page 是手柄两侧的空白，QSS 里按名分别定制。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，QScrollBar 与 QAbstractSlider 都在 QtWidgets，链接 Qt6::Widgets 即可。

[Qt 文档 · QScrollBar](https://doc.qt.io/qt-6/qscrollbar.html) -- 滚动条控件，document length 等式与子控件的说明在这页

[Qt 文档 · QAbstractSlider](https://doc.qt.io/qt-6/qabstractslider.html) -- 抽象基类，range、两档步长与 triggerAction 的定义

[Qt 文档 · QScrollArea](https://doc.qt.io/qt-6/qscrollarea.html) -- 滚动区域容器，ScrollBarAsNeeded 策略与滚动条显隐的归属

[Qt 文档 · QStyle](https://doc.qt.io/qt-6/qstyle.html) -- subControlRect 与 pixelMetric 的正式说明

[Qt 文档 · QProxyStyle](https://doc.qt.io/qt-6/qproxystyle.html) -- 样式代理，改 PM_ScrollBarSliderMin 的入口

---

滚动条认完，QAbstractSlider 家族还剩最后一站，[3.33 QDial 进阶](./33-qdial-advanced.md) 的圆形表盘等着咱们给整个家族收尾。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/32-qscrollbar-advanced/，可执行文件 32-qscrollbar-advanced）。窗口分三块，四只输入框调参数（slider_size_demo.cpp 第 58-114 行），中间一根固定 300 像素高的竖滚动条（第 129-131 行），底下公式区实时代入计算（第 172-271 行），默认 max 100、pageStep 20、singleStep 5，出处第 25、28、31 行。

实验走三步。第一步把 max 拉到 5000，公式区按 pageStep 乘 300、再除以 range 加 pageStep，算出 1 像素（第 220 行），窗口里的手柄却留着一小截，那是 style 的最小值在兜底，Fusion 是 26 像素（qfusionstyle.cpp 第 2759-2761 行），common 是 9（qcommonstyle.cpp 第 4536-4538 行），公式区“小于 20 像素”的警告线（第 236-239 行）只是演示阈值，真实下限以您桌面的 style 为准。第二步把 pageStep 调到 400，range 还是 100，手柄占到 groove 的八成，警告提示“手柄几乎撑满、滚动空间极小”（第 245-251 行），触发条件是 pageStep 不小于 range，这就是内容比视口大不了多少时的样子。第三步把 pageStep 归回 20，singleStep 调到 50，警告说方向键步进比点 groove 还大（第 253-258 行），然后您在滚动条上滚一格滚轮，步数 3 乘 50 算出 150，被 pageStep 封顶截回 20，封顶的出处就是 qabstractslider.cpp 第 698 行的 qBound，一格滚不出一页。

完成标准照旧。手柄长短由谁定、钳位在源码哪两行，您说得清。pageStep 设 0 与 singleStep 设 0 各废掉哪条交互，您能分开报。resize 跳动的两个来路与比例恢复的写法，您拿得出来。
