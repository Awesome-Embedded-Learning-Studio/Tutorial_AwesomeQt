---
title: "3.38 QGroupBox 进阶"
description: "入门篇用 setTitle、setCheckable、setFlat 把 QGroupBox 用了起来，进阶篇对着 6.9.1 源码把框拆开：标题旁的勾选框是 style 画的 PE_IndicatorCheckBox、没有一个真的 QCheckBox 对象、命中靠 hitTestComplexControl、空格键走 ButtonPressKeys 主题提示，取消勾选由 _q_setChildrenEnabled 挨个对子控件 setEnabled、分组框自身保持可用、您手动禁用的子控件靠 WA_ForceDisabled 保住、后加的子控件由 childEvent 补禁用、分组框重新启用且未勾选时 changeEvent 再压回去，标题里的 & 记号由 setTitle 抓成助记符、非 checkable 时走 _q_fixFocus 送焦点且偏爱选中的 QRadioButton、checkable 时改走 click 加 setFocus，flat 在 common style 里只画顶部一条线、内容边距由 calculateFrame 一次算定、setFlat 不触发重算、字体或样式变更才重算，标题宽度记在 minimumSizeHint 里、setTitle 每次触发 update 加 updateGeometry，默认标题嵌边框靠裁剪区域减去标题矩形实现、接管 paintEvent 后 QSS 边框不再落地，QSS 的 ::indicator 与 ::title 伪元素可用，配套示例在 examples/advanced/03-qtwidgets/38-qgroupbox-advanced/"
---

# 现代Qt开发教程（进阶篇）3.38——QGroupBox 进阶

[3.37 QCalendarWidget 进阶](./37-qcalendarwidget-advanced.md) 收尾时咱们预告过容器家族，QGroupBox 把一组控件圈进一个带标题的框里。框和标题都不复杂，门道藏在几处不起眼的地方：标题旁的勾选框根本没有对应的对象，取消勾选时禁用是挨个落到子控件头上的，标题里的 & 记号另握着一条送焦点的通路。咱们对着 6.9.1 源码把几处都翻出来，源稿在禁用传播和 flat 边距上偏得最远，正文按源码改准。

咱们把默认状态照源码记一遍。标题为空、标题左对齐、flat 关闭、不可勾选，私有位 checked 却是 true，出处（qgroupbox.cpp 第 176-188 行），尺寸策略用 QSizePolicy::GroupBox 这个专用档，出处第 187-188 行。正文引用以 qgroupbox.cpp 为主，绘制侧补 qcommonstyle.cpp 的行号，样式表侧补 qstylesheetstyle.cpp 的行号。

## 勾选框是画上去的，没有一个真的 QCheckBox 对象

咱们从绘制管线看起。paintEvent 的活儿很薄，建一个 QStylePainter，填一个 QStyleOptionGroupBox，再调 drawComplexControl 交给样式，出处（qgroupbox.cpp 第 278-284 行）。勾选框没有子控件身份，initStyleOption 只是把 SC_GroupBoxCheckBox 加进 subControls，把 State_On 或 State_Off 写进 state，出处第 80-86 行，实际下笔的是 common style 侧的 PE_IndicatorCheckBox，出处（qcommonstyle.cpp 第 3839-3845 行）。所以您在对象树上找不到它，findChild 按类型找 QCheckBox 永远拿回空指针。有个细节值得多看一眼，_q_setChildrenEnabled 上方的注释还在说 “except the qt_groupbox_checkbox”，源码里早就没这个对象了，注释留着旧世界的名字，出处（qgroupbox.cpp 第 618-621 行）。

点击判定靠命中测试，不走任何按钮信号。鼠标按下时 mousePressEvent 拿 hitTestComplexControl 算出按在哪个子控件上，按中 SC_GroupBoxCheckBox 或 SC_GroupBoxLabel 都算压下，出处（qgroupbox.cpp 第 672-676 行），抬起时 release 侧再测一次，两回都落在标题区域才触发 click，出处第 716-721 行。hover 同样只认 checkable 分组框的标题与勾选框，出处第 313 行。键盘也能触发同样的切换，您在分组框有焦点时按下空格，KeyPress 记下 pressedControl，KeyRelease 才完成切换，键位来自平台的 ButtonPressKeys 主题提示，出处第 329-349、727-738 行。

QSS 的定制入口也在这里。样式表把 SC_GroupBoxLabel 映射成 ::title，把 SC_GroupBoxCheckBox 映射成 ::indicator，出处（qstylesheetstyle.cpp 第 266-267 行），勾选框最终仍走 PE_IndicatorCheckBox 绘制，出处第 3259-3265 行，您写 QGroupBox::indicator 的宽度规则就能改勾选框尺寸，写 ::title 的规则能管标题文字。

标题文字的颜色另有一个样式提示口子。调色板没给 WindowText 设画刷时，initStyleOption 会去问样式提示 SH_GroupBox_TextLabelColor，把结果存进 option.textColor，出处（qgroupbox.cpp 第 88-91 行）。您自绘时想跟平台风格保持一致，读这个字段比硬编码颜色稳妥。

## 取消勾选是挨个禁用子控件，您手动禁用的会保住

setChecked 的路径咱们全程跟一遍。状态翻过来后，_q_setChildrenEnabled 遍历 children()，对每个子控件直接调 setEnabled(false)，出处（qgroupbox.cpp 第 601-616、622-639 行）。分组框自己不禁用，它得保持可用，不然下一次勾选就没人接了。您手动 setEnabled(false) 过的子控件另有保护，禁用方向上已经禁用的控件被跳过，启用方向上带着 WA_ForceDisabled 标记的也被跳过，出处第 629-635 行，勾选轮回几轮，您压住的那几个都不会被拉起来。

后加的子控件也有人管。childEvent 在控件加入或抛光时看一眼分组框状态，未勾选就把新来的禁用，出处（qgroupbox.cpp 第 357-380 行）。还有一处联动容易被漏看，整个分组框被外部禁用再启用后，changeEvent 的 EnabledChange 分支会在未勾选的状态下把子控件再压回禁用，出处第 645-650 行。嵌套的场景咱们照源码推一遍：外层取消勾选时，内层分组框作为普通子控件被 setEnabled(false)，内层的 checked 位原样保留，外层恢复后内层接着用之前的勾选状态，想让外层连带清掉内层，得您自己在 toggled 槽里写。

打开 checkable 这个动作本身也有副作用。setCheckable(true) 会顺带 setChecked(true)，保证子控件开箱可用，同时把焦点策略设成 StrongFocus，关掉 checkable 时退回 NoFocus，出处（qgroupbox.cpp 第 521-548 行）。您后面想用 focusPolicy 微调键盘行为，记得这里被动过手脚。isChecked 的实现也顺手看一眼，checkable 为假时它永远返回假，私有位 checked 再真也不算数，出处第 557-561 行。

子控件的内容它一概不碰。禁用传播只管 enabled 一层，输入框里的文字、下拉框的当前项都留在原地，取消勾选再勾回来，现场原样恢复。要不要在取消时清空内容是业务决定，咱们建议保留，用户关掉一组选项多半是暂时不用，内容没了才让人恼火，真要清就在 toggled 槽里自己动手。

焦点不用担心。源稿说禁用后焦点留在原处、光标还在闪，笔者在 QWidget 这一层查了实证：禁用一个持有焦点的控件时，setEnabled_helper 会把焦点交给焦点链上的下一个可停靠目标，交不出去才 clearFocus，出处（qwidget.cpp 第 3407-3410 行）。挨个禁用的路径正好挨个走完这套交接，toggled 槽里再补 setFocus 属于多余的保险。

## 标题里的 & 记号把焦点递给选中的单选按钮

setTitle 每次都重新抓助记符。标题变了就释放旧的快捷号，再拿 QKeySequence::mnemonic 解析标题抓新号，出处（qgroupbox.cpp 第 191-209 行）。触发时 event() 的 Shortcut 分支分两路，不可勾选的分组框走 _q_fixFocus 送焦点，可勾选的直接 click 切换再给自己 setFocus，出处第 291-302 行。同样一个 Alt 加字母，勾选与否行为完全两样，您做表单时得想清楚标题给谁用。

_q_fixFocus 的挑法，笔者把代码读了一遍：它沿着 nextInFocusChain 把分组框的后代扫一圈，只收带 TabFocus 且可见的控件，选中状态的 QRadioButton 排在最前，出处第 390-419 行。设计意图贴合典型用法：一组互斥的单选按钮住在分组框里，Alt 加标题字母直接落到您选中的那一项上。分组框自己拿到焦点时，focusInEvent 在 NoFocus 策略下也走同一套挑选，出处第 437-445 行。

标题对齐只有水平方向几档，setAlignment 改完触发重绘加几何重算，值经 initStyleOption 的 textAlignment 传给样式，出处第 260-266、72 行。想让标题靠右或居中就在这里设，垂直位置归样式管，您插不上手。

## flat 只画顶部一条线，内容边距却停在旧状态

flat 的外观变化发生在绘制原语那一层。PE_FrameGroupBox 看到 Flat 标记就只画顶部一条阴影线，非 flat 画整圈阴影矩形，出处（qcommonstyle.cpp 第 410-424 行）。子控件矩形同样看这个标记，flat 时 frameWidth 取 0、标题左右缩进取 0，非 flat 时标题上下边距写死为 8、frameWidth 取 PM_DefaultFrameWidth，出处第 4391-4403 行。flat 瘦身之后剩的就是顶部一条线，标题为空时线还在，分组框在视觉上只剩一根头发，您调试布局时别把它当成不存在的容器。这一段说的是 common 一系样式的行为，Fusion 把 PE_FrameGroupBox 整个换成边框贴图，不认 Flat 标记，Linux 默认就是 Fusion，flat 切过去画的是整圈圆角边框，您以本机样式实测为准。

边距的更新有个反直觉的地方。内容边距由 calculateFrame 拿 SC_GroupBoxContents 算出来，一次写进 setContentsMargins，出处（qgroupbox.cpp 第 424-433 行），可 setFlat 只做 updateGeometry 加 update，不回头重算边距，出处第 494-502 行。运行时把 flat 切来切去，子控件区域还停留在算边距那一刻的形态，要等字体或样式变更触发 changeEvent 才重算，出处第 651-657 行。您真需要运行时切 flat，就自己调一把 setContentsMargins 把两种形态对齐，别指望它自动跟。

## 标题宽度记在 minimumSizeHint 里

QGroupBox 没有覆写 sizeHint，覆写的是 minimumSizeHint。标题文字宽度加一个空格宽度做底，checkable 再叠上指示器宽度与间距，高度取文字与指示器的较大者，出处（qgroupbox.cpp 第 451-469 行）。切换 checkable 同样牵动几何，两条路径都调了 updateGeometry，出处第 533、539 行，勾选框加进标题后最小尺寸跟着长。setTitle 本身也不轻，文字一变就 update 加 updateGeometry，布局系统跟着把脏标记往上冒，出处第 203-204 行，calculateFrame 也被顺带重跑，内容边距跟着标题走，出处第 201 行。您拿标题当状态栏用，一秒一改 “已启用 3/5” 式的计数，每秒都是一轮几何重算，紧凑布局里能感觉到。省事的办法是给分组框一个够宽的 setMinimumWidth，标题在宽度里怎么变，布局都懒得重排。

## 接管 paintEvent 就接管了全部视觉

默认那套 “标题嵌在边框线上” 的效果，咱们看它怎么实现。drawComplexControl 画框之前把标题矩形（勾选时连勾选框一起）从绘制区域里减掉，再带着裁剪区画 PE_FrameGroupBox，线遇到标题就断，出处（qcommonstyle.cpp 第 3790-3818 行）。示例里换成拿窗口背景色的小矩形压在边框线上，是同一视觉的另一条实现路，两条都走得通。

咱们看示例的骨架（checkable_group_box.cpp 第 60-98 行）：

```cpp
QStyleOptionGroupBox option;
initStyleOption(&option);

const int titleHeight = fontMetrics().height();
QRect frameRect = rect();
frameRect.setTop(frameRect.top() + titleHeight / 2);
painter.drawRoundedRect(frameRect, kCornerRadius, kCornerRadius);

QRect titleBoundingRect = fontMetrics().boundingRect(title());
titleBoundingRect.adjust(-kTitlePadding, -kTitleVerticalPadding,
                         kTitlePadding, kTitleVerticalPadding);
painter.fillRect(titleBoundingRect, palette().window().color());
painter.drawText(titleBoundingRect, Qt::AlignCenter, title());
```

initStyleOption 把标题、对齐、勾选状态、hover 全部填好，出处（qgroupbox.cpp 第 62-95 行），示例里边框颜色跟着 isChecked 分两态，读的就是 option.state 里的 State_On。接管之后 QSS 的 border 一类规则不再落地，那些规则靠 QStyleSheetStyle 在 drawComplexControl 里生效，您把绘制全揽下来，样式表就没口子可进了。微调边框颜色这类活能走 QSS 就走 QSS，想要圆角、渐变、标题背景条一类样式表表达不了的效果，再动手重写。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，QGroupBox 在 QtWidgets，绘制侧的 QStyleOptionGroupBox 与 QStyle 同在 QtWidgets，链接 Qt6::Widgets 即可。

[Qt 文档 · QGroupBox](https://doc.qt.io/qt-6/qgroupbox.html) -- 分组框控件，checkable、flat、title、alignment 属性与信号说明

[Qt 文档 · QStyleOptionGroupBox](https://doc.qt.io/qt-6/qstyleoptiongroupbox.html) -- 绘制选项结构体，自定义 paintEvent 时的数据源

[Qt 文档 · QStyle](https://doc.qt.io/qt-6/qstyle.html) -- 样式基类，CC_GroupBox、SC_GroupBoxCheckBox 与 PE_IndicatorCheckBox 的定义处

[Qt 文档 · QWidget::enabled](https://doc.qt.io/qt-6/qwidget.html#enabled-prop) -- 启用属性，禁用与焦点交接的机制说明

[Qt 文档 · QKeySequence](https://doc.qt.io/qt-6/qkeysequence.html) -- 键序列，mnemonic 对标题助记符的解析

---

分组框到这就拆完了，下一程咱们去翻页签，QTabWidget 把一组组控件叠进同一块地盘，每层只露一个标题，入口在 [3.39 QTabWidget 进阶](./39-qtabwidget-advanced.md)。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/38-qgroupbox-advanced/，可执行文件 38-qgroupbox-advanced）。构造里标题设成 “代理设置”，setCheckable 一开，勾选态与 StrongFocus 策略跟着进来，出处（checkable_group_box.cpp 第 44-53 行）。paintEvent 里圆角框配背景条压线，边框和标题颜色都分勾选两态，出处第 60-98 行。取消勾选时输入内容原样保留，窗口里另有一个分组框外的对照控件，取消前后它都不受影响，出处（main.cpp 第 20-46 行）。

完成标准照旧。勾选框为什么 findChild 找不到、它由谁画在哪一步，您指得出行号。取消勾选时您手动禁用的子控件会不会被拉起来、依据是哪个标记，您答得上来。标题里写了 & 之后，Alt 加字母在可勾选与不可勾选两种分组框上各做什么，您不跑程序就能推。运行时切 flat 为什么内容边距不动，您说得出来。
