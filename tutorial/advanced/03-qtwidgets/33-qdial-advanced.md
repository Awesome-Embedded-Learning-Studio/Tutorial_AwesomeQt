---
title: "3.33 QDial 进阶"
description: "入门篇用 QDial 做了仪表盘面板，进阶篇对着 6.9.1 源码把圆形表盘的机制认清，wrapping 覆写了钳位函数 bound 用 maximum 减 minimum 的差做模数回绕，模数比值域少一个 1，键盘与滚轮从 max 加一步落在 min+1 跳过 min，而鼠标把整圈 360 度铺满这 max 减 min 个单位，扫过正下方时值从 max 连续跨到 min，非 wrapping 的有效角度从 240 度（7 点钟）顺时针经 12 点钟到负 60 度（5 点钟）共 300 度，底部 60 度的点击被钳到最近端，invertedAppearance 在鼠标映射与绘制两侧都生效，鼠标交互没有命中检查，点在方形区域圆外的角落也会跳值，notchSize 以值为单位且必为 singleStep 的整数倍，模型里的整弧长只有真实刻度圆周的一半再叠加整数除法截断，实际槽间距普遍疏于 notchTarget，刻度线由 style 侧的 calcLines 绘制且长刻度按 pageStep 对齐，singleStep 设 0 会连刻度一起消失，键盘左右上下单步、PageUp 与 PageDown 翻页、Home 与 End 到端点，滚轮步数封顶一页，全走同一道 bound，sizeHint 是 100 乘 100 的常数不经 style，配套示例在 examples/advanced/03-qtwidgets/33-qdial-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.33——QDial 进阶

[3.32 QScrollBar 进阶](./32-qscrollbar-advanced.md) 收尾把家族最后一站留给咱们。QDial 把线性的值轴弯成一个圆，滑条与滚动条那套 range、tracking、步进原样继承，重做的活集中在钳位与角度映射上。钳位这头藏着一个模数少 1 的回绕，映射那头藏着一段被钳住的底部空档，咱们对着源码逐个认清，刻度与鼠标交互顺路收掉。

默认值照 32 篇的样子记下。不带参数构造出来，wrapping 关，出处 qdial.cpp 第 32 行，notchTarget 3.7 像素，出处第 35 行，刻度默认不显示，焦点策略 WheelFocus，出处第 52-53 行。范围与步长继承基类，0 到 99，singleStep 1，pageStep 10，invertedControls 保持 false，出处 qabstractslider.cpp 第 178-183 行。正文行号都带出处。Qt 内部实现主要指 qdial.cpp，键盘与滚轮那段在 qabstractslider.cpp，绘制与刻度落在 qcommonstyle.cpp、qfusionstyle.cpp 与 qstylehelper.cpp。笔者把这几个文件对着源稿断言逐条过了一遍，角度数字与刻度计算偏得最远，正文按源码改准。

## wrapping 换掉的是钳位函数，模数比值域少一个 1

QAbstractSliderPrivate 的 bound 是夹逼，出界的值被 qMax 与 qMin 压回端点，出处 qabstractslider_p.h 第 88 行。QDialPrivate 把这个虚函数覆写了，出处 qdial.cpp 第 56-70 行，咱们跟着它走：进函数头一件事是查范围，在范围内就直接返回，出处第 58-59 行，minimum 等于 maximum 时返回 minimum，出处第 61-62 行，出界才轮到取模回绕：

```cpp
// qdial.cpp 第 63-66 行
val = minimum + ((val - minimum) % (maximum - minimum));
if (val < minimum)
    val += maximum - minimum;
return val;
```

wrapping 关着时这个覆写不接管，走基类夹逼，出处第 67-69 行。模数用的是 maximum 减 minimum，值域里却站着比它多一个的值。您从 max 出发按一次右方向键，Key_Right 会被翻译成 SliderSingleStepAdd，出处 qabstractslider.cpp 第 842 行，triggerAction 再调 setSliderPosition，出处第 586 行，setSliderPosition 进门就过 bound，出处第 460 行，max 加一个 singleStep 落在 min+1，min 被整个跳过去。范围 0 到 359 的表盘，359 上按右，值到 1。反方向对称，0 上减一步，负 1 对 359 取模在 C++ 里仍是负 1，补一圈到 358，max 也被跳过。滚轮同走这道 bound，出处第 718 行，从 359 向上滚一格走三步，值直接落在 3。

鼠标路径跟键盘不一样，咱们看 valueFromPoint 怎么铺角度：整圈 360 度分给 max 减 min 个单位，出处 qdial.cpp 第 128 行。绘制侧 calcArrow 给出的几何相同，max 和 min 都落在正下方那一点，出处 qcommonstyle.cpp 第 3212-3214 行，右侧一点点是 max，左侧一点点是 min。您扫着指针过正下方，值从 359 连续跨到 0，min 摸得到。同一只 wrapping 表盘，鼠标能停在 0，键盘却停不到，来路就在模数上。Home 与 End 例外，它们直接给端点值，出处 qabstractslider.cpp 第 877-882 行。setWrapping 本身只翻转标志位再重画，不重新钳当前值，出处 qdial.cpp 第 303-310 行。

## 非 wrapping 只有 300 度，底部 60 度的点击被钳到最近端

角度映射的核心就两行，咱们盯着变量 a 看它的来路：

```cpp
// qdial.cpp 第 127-130 行
if (wrapping)
    v = int(0.5 + minv + r * (Q_PI * 3 / 2 - a) / (2 * Q_PI));
else
    v = int(0.5 + minv + r * (Q_PI * 4 / 3 - a) / (Q_PI * 10 / 6));
```

您拿鼠标点对中心做 atan2 得到 a，出处第 111 行，中心就是 width 除 2、height 除 2，出处第 109-110 行。小于负 90 度的角度加一整圈，归进 [−90°, 270°)，出处第 113-114 行。minimum 为负时区间整体抬到 0，算完再压回，出处第 119-123 行，末行处理 invertedAppearance 的翻转，出处第 135 行。

wrapping 那行把 360 度全铺上，非 wrapping 那行给的是 240 度起点、300 度跨度，min 站在 240 度，也就是 7 点钟，max 站在负 60 度，5 点钟，值从 7 点钟顺时针经 12 点钟走到 5 点钟。源稿写的 225 度到负 45 度、270 度范围、90 度死角都不对，公式里 4π/3 是 240 度，10π/6 是 300 度，底部空出来的只有 60 度。咱们再到绘制侧对一遍几何，calcArrow 在位置为 min 时算出 4π/3，为 max 时算出负 π/3，出处 qcommonstyle.cpp 第 3215-3217 行，刻度线的角度同款，出处 qstylehelper.cpp 第 219 行。min 与 max 相等时，箭头固定指 12 点钟，出处第 3210-3211 行。

底部 60 度那一段，点击并不失效。公式照样算出一个 v，右侧那半边算出的 v 超过 max，左侧低于 min，然后进基类的夹逼，出处 qabstractslider_p.h 第 88 行，分别压到 max 与 min。源稿把它当不可点的死角，实际的语义是钳到最近端，您点在 5 点钟下方，指针贴住 max 不动。

## 没有命中检查，方形区域每个像素都活着

mousePressEvent 门口只查三样，空范围、按的不是左键、发生时还按着别的键，出处 qdial.cpp 第 242-247 行。检查过了，直接 setSliderPosition(valueFromPoint(点击点))，第 249 行，连半径都不看。您点在方形区域的角落，离画的圆老远，atan2 照样算出一个角度，指针跳过去。源码注释把这事写明白了，第 250-253 行写着本该像 QSlider 那样做命中测试，只是 QDial 没有子控件可测。所以哪段区域点不动，答案是一个例外都没有，整个方形区域全热。move 与 release 同款直跳，出处第 280-291、262-273 行，release 用 setValue 定案，再把 sliderDown 抬起来，出处第 271-272 行。move 里包了一层 doNotEmit 开关，出处第 288、290 行，笔者翻遍 widgets 目录没有读到它的人，只有 qdial.cpp 这几处碰它，是个没人消费的旧开关。

子类化时算角度的中心顾虑，咱们可以放下。valueFromPoint 用的是 width/2、height/2，出处第 109-110 行，画刻度的 calcLines 用 width/2 加 0.5，出处 qstylehelper.cpp 第 202-203 行，画箭头的 calcArrow 用 width/2，再加 rect.left()，出处 qcommonstyle.cpp 第 3219-3220 行，三处同一个圆心。盘面本来就往里收 r/6 给刻度留位，common 与 Fusion 两侧都无条件这么做，出处 qcommonstyle.cpp 第 3709-3713 行，notchesVisible 只管刻度线画不画，刻度线画到外缘半径 r，出处 qstylehelper.cpp 第 223-229 行，圆心不动。源稿“开启刻度后中心点偏移”的说法按源码不成立。

## notchSize 数的是值，QDial 算个数，style 画线

绘制这分工，QDial 一笔不画，paintEvent 整体交给 CC_Dial，出处 qdial.cpp 第 227-233 行，刻度线由 style 侧的 QStyleHelper::calcLines 生成，common 直接画，出处 qcommonstyle.cpp 第 3717-3720 行，Fusion 委托 QStyleHelper::drawDial，出处 qfusionstyle.cpp 第 2723-2726 行与 qstylehelper.cpp 第 253-260 行。QDial 自己算的是 notchSize，单位是值，咱们记下它必为 singleStep 的正整数倍，属性注释在 qdial.cpp 第 336-344 行。链条在 notchSize() 里：

```cpp
// qdial.cpp 第 352-359 行
int l = qRound(r * (d->wrapping ? 6.0 : 5.0) * Q_PI / 6.0);
if (d->maximum > d->minimum + d->pageStep)
    l = qRound(l * d->pageStep / double(d->maximum - d->minimum));
l = qMax(l * d->singleStep / (d->pageStep ? d->pageStep : 1), 1);
l = qMax(qRound(d->target / l), 1);
```

整弧长按 wrapping 与否取 πr 或 5πr/6，折成每 pageStep 的弧长，再折成每 singleStep 的弧长，数一数 notchTarget 里装得下几个单步弧长，第 361 行返回 singleStep 乘上这个数。您回头看官方注释，它说尽量画“好看”的刻度数、画不下就隔行跳，出处 qdial.cpp 第 171-175 行，落到代码就是 notchSize() 里那段倍数链。

notchTarget 是像素目标，实际槽间距普遍疏于它。模型的整弧长只有真实刻度圆周的一半，刻度线画在半径 r 的外缘，出处 qstylehelper.cpp 第 223-229 行，wrapping 的真圆周是 2πr、非 wrapping 的真弧长是 5πr/3，模型给的却是 πr 与 5πr/6，出处 qdial.cpp 第 352 行，第 357 行的整数除法还会把单步弧长截短，两个方向都把槽间距往外推。咱们拿示例参数代一遍，旋钮 180 像素，r 是 90，范围 0 到 99，target 10，链条算出 notchSize 5，真实弧上每单位合 5πr/3 除以 99，约 4.76 像素，槽间距约 23.8 像素，是 target 的两倍多。您要精确槽距，调 target 只能粗调，细活得 QProxyStyle 或自己画。

刻度线本身还有一层规则，咱们接着看。calcLines 里，i 为 0，或者 tickInterval 乘 i 对 pageStep 取余为零，这些位置画长刻度，其余短一半，出处 qstylehelper.cpp 第 216、222 行。刻度数量按 (max + ns − 1 − min)/ns 向上取整，出处第 207 行，范围超过 1000 时只按 min 到 min+1000 的一段布点，出处第 210-213 行。

tickInterval 由 initStyleOption 填成 notchSize()，出处 qdial.cpp 第 103 行，这里藏着一条连锁，singleStep 设 0 会让 notchSize 返回 0，出处第 361 行，calcLines 见 tickInterval 为 0 直接返回空，出处 qstylehelper.cpp 第 205-206 行，键盘与滚轮的步进本来也全废了，表盘上连刻度一起消失。initStyleOption 还有个反直觉的填法，notchesVisible 关着时 tickPosition 填 TicksAbove，开着反而填 NoTicks，出处 qdial.cpp 第 97-102 行，传话靠 subControls 里的 SC_DialTickmarks，出处第 97-98 行，您读 dial 的 option 时，别照 tickPosition 的字面意思走。

## 键盘滚轮全走基类同一条路，表盘上照样回绕

键盘的动作表全在基类，咱们对着按键念：左右上下给 singleStep，PageUp 与 PageDown 给 pageStep，Home 与 End 给端点，出处 qabstractslider.cpp 第 785-889 行，Home 在第 877-879 行，End 在第 880-882 行。QDial 的类注释把同一张表复述了一遍，出处 qdial.cpp 第 180-187 行。这些动作都经 setSliderPosition 过 bound，出处 qabstractslider.cpp 第 460 行，wrapping 表盘上您按住方向键能一圈圈转。滚轮同路，步数按 wheelScrollLines 乘 singleStep 算，单次封顶一页，出处第 698 行，类注释给的简写是取它与 pageStep 里较小的那个，出处 qdial.cpp 第 189-193 行。

尺寸这块 QDial 很省事，sizeHint 与 minimumSizeHint 都是常数，100 乘 100 与 50 乘 50，出处 qdial.cpp 第 416-419、425-428 行，不问 style 要任何量。这与 32 篇滚动条找 style 要 extent 的路子不同，您布局时给个方形就行，拖成矩形它也只认短边画圆，出处 qcommonstyle.cpp 第 3709 行，多出来的部分是空白。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，QDial 在 QtWidgets，链接 Qt6::Widgets 即可。

[Qt 文档 · QDial](https://doc.qt.io/qt-6/qdial.html) -- 旋钮控件，wrapping、notchTarget、notchSize 属性定义

[Qt 文档 · QAbstractSlider](https://doc.qt.io/qt-6/qabstractslider.html) -- 抽象基类，range、两档步长与 triggerAction

[Qt 文档 · QStyle](https://doc.qt.io/qt-6/qstyle.html) -- CC_Dial 与 SC_DialTickmarks 的子控件体系

[Qt 文档 · QStyleOptionSlider](https://doc.qt.io/qt-6/qstyleoptionslider.html) -- notchTarget 与 dialWrapping 的传递载体

[Qt 文档 · QProxyStyle](https://doc.qt.io/qt-6/qproxystyle.html) -- 样式代理，改刻度画法的入口

---

QAbstractSlider 家族到这儿收完，下一程咱们离开滑动条家族去显示控件，QLabel 那篇讲 setTextFormat 的解析模式、setBuddy 的快捷键注册与文本交互权限，入口在 [3.34 QLabel 进阶](./34-qlabel-advanced.md)。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/33-qdial-advanced/，可执行文件 33-qdial-advanced）。窗口左边一只 180 像素的旋钮，范围 0 到 99，notchTarget 10，出处 nonlinear_dial.cpp 第 43-48 行，右边面板报内部线性值、对数输出与累计旋转增量。对数映射在 linearToLog，把 0 到 99 映成 1 到 1000，公式是 10 的 dialValue 除 99 乘 3 次方，出处第 120-131 行，音量旋钮低段精细高段粗，靠的就是对数曲线。穿越点修正在 handleValueChanged，阈值取 (max − min + 1)/2，这里就是 50，出处第 150-160 行，wrapping 开着时值从 99 跨到 0，差值负 99 被修成加 1，累计增量不乱跳。面板上的按钮切换 wrapping，出处第 170-186 行，切换后累计值清零，出处第 184-185 行。

实验补两步。您开着 wrapping 把旋钮拧到 99，按一下右方向键，面板的内部线性值显示 1，min 被模数跳了过去，来路在 qdial.cpp 第 63-66 行。再点旋钮方框左下角的空白，圆外老远的位置，指针照样跳，那是没有命中检查的直跳，出处第 249 行。

完成标准照旧。非 wrapping 的有效角度与底部空档各多少度、公式在哪两行，您报得出。wrapping 下键盘跨过 max 落在哪、鼠标扫过正下方落在哪，您分得开。实际槽间距疏于 notchTarget 的来路在源码哪几行，您指得出来。
