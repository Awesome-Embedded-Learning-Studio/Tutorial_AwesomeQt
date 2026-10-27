---
title: "3.44 QScrollArea 进阶"
description: "入门篇用 QScrollArea 装下放不下的控件，进阶篇把本体对着 6.9.1 读穿：内容部件 re-parent 到 viewport 而非本体且旧部件被 delete、布局须在 setWidget 前建好否则内容不显示、widgetResizable 走 updateScrollBars 同一个函数的两条路而 resize 被布局最小最大值夹住、heightForWidth 内容触发滚动条宽度收窄循环防止滚动条反复出现消失、滚动联动链是 valueChanged 到 _q_vslide 到 scrollContentsBy 再到 updateWidgetPosition、滚轮事件被 sendEvent 整个转发给滚动条且每格步数等于 wheelScrollLines 乘 delta 除以 120 乘 singleStep 单次封顶 pageStep 小数余量跨事件累积、方向键与翻页键转发 triggerAction 而 Tab 经 focusNextPrevChild 滚动跟进、ensureWidgetVisible 开头验出身再取 ImCursorRectangle 的 microFocus 且目标比视口大时对齐中心、alignment 只在该轴内容小于视口时发言、sizeHint 封顶 36 行字高乘 24 行字高且 AlwaysOn 滚动条正向补条宽反向扣视口、QSS 背景规则经 embeddedWidget 映射落到 viewport 上、平滑滚动动画须停掉在跑的动画再启新的，配套示例在 examples/advanced/03-qtwidgets/44-qscrollarea-advanced/"
---

# 现代Qt开发教程（进阶篇）3.44——QScrollArea 进阶

[3.43 QToolBox 进阶](./43-qtoolbox-advanced.md) 收尾说，抽屉里的页面们各自住在一个去边框的 QScrollArea 里，43 篇只是路过打了个照面，本篇把它本体读穿。入门篇您用 setWidget 装下放不下的控件，出了滚动条能拉就算完成任务。进阶篇咱们换一双更细的眼睛：内容部件到底住在哪里，widgetResizable 的自适应是怎么算出来的，滚动条与内容谁牵着谁走，还有 ensureWidgetVisible 那手精确滚动。源稿把重心压在平滑滚动动画上，机制层几乎没碰，本篇按源码重排，改准的地方随文给出行号。

本篇引用以 qscrollarea.cpp 为主，咱们把视口与键盘滚轮的转发补到 qabstractscrollarea.cpp，滚轮的步数量化补 qabstractslider.cpp，QSS 的落点补 qstylesheetstyle.cpp。配套示例的完整玩法放在结语，正文引用文件名加行号。

## 内容部件不住在本体里，它住在 viewport 的屋檐下

一棵对象树数下来，滚动区里其实叠了三层，咱们从外往里数。最外层是 QScrollArea 本体，一个带框的 QFrame。中间是 viewport，QAbstractScrollAreaPrivate 的 init 里 new 出来的一个普通 QWidget，对象名 qt_scrollarea_viewport，背景角色 Base，自动填充背景（qabstractscrollarea.cpp 第 249-253 行）。最里层才是您的页面。setWidget 干的就是把页面搬进 viewport 的屋檐。

```cpp
// setWidget 的搬家动作（qscrollarea.cpp 第 237-250 行）
delete d->widget;
d->hbar->setValue(0);
d->vbar->setValue(0);
if (widget->parentWidget() != d->viewport)
    widget->setParent(d->viewport);
if (!widget->testAttribute(Qt::WA_Resized))
    widget->resize(widget->sizeHint());
```

搬家的次序咱们逐行看。旧的页面部件直接 delete（第 237 行），想换掉页面部件又不想删旧的，要走 takeWidget，它把部件的父对象清空后交还给您（第 260-268 行）。两条滚动条的值归零（第 239-240 行），新部件 re-parent 到 viewport（第 241-242 行），没被手动改过尺寸的话，就按 sizeHint 摆好初始大小（第 243-244 行）。搬完还有两笔小动作，autoFillBackground 被强制置真（第 246 行），部件装上事件过滤器（第 247 行），后面 resize 时靠它刷新滚动范围。

这里有一处文档里用粗体叮嘱过的限制，咱们把它织进搬家的叙述。布局必须在 setWidget 之前建好，之后再往部件上安布局，内容怎么 show 都不显示（函数文档第 223-227 行）。根因是 re-parent 与 resize 都发生在 setWidget 那一刻，布局晚到就错过了部件定尺寸的窗口。另外构造函数把 viewport 的背景角色从 Base 改成了 NoRole（第 107 行），同时给两条滚动条设了 20 像素的 singleStep（第 108-109 行），背景与步长两件事后面都会回来看。

## widgetResizable 的自适应：同一个函数里的两条路

开关本身很薄，咱们翻实现，setWidgetResizable 只改一个标志、更新几何、重跑 updateScrollBars（第 352-358 行），真活全在 updateScrollBars 里。它开口就取两把尺子，qSmartMinSize 与 qSmartMaxSize，从部件的布局与尺寸策略里算出允许的最小与最大（第 153-154 行）。

resizable 为假时，判断条件是最大视口装得下部件的当前尺寸就不出滚动条（第 180-182 行），部件保持自己的大小，谁也不动它。resizable 为真时走另一条路，一句 resize 把部件直接改到 p.expandedTo(min).boundedTo(max)（第 184-185 行）。源稿说视口尺寸会传给部件当参考、部件的 sizePolicy 决定接不接受，机制对不上。updateScrollBars 不咨询部件，它直接 resize，起约束作用的是那两把尺子，布局算出来的最小值撑着底，最大值封着顶。您设了 resizable 内容还是窄窄一条，去查容器布局算出的最小宽度，从那里入手比查 sizePolicy 直接。

范围算完统一登记到滚动条上，咱们看摘录。

```cpp
// 滚动范围与页步（qscrollarea.cpp 第 188-191 行）
hbar->setRange(0, v.width() - p.width());
hbar->setPageStep(p.width());
vbar->setRange(0, v.height() - p.height());
vbar->setPageStep(p.height());
```

范围的上限就是内容减视口，pageStep 直接取视口边长。heightForWidth 的内容另有一段收窄循环，源码注释写明了动机：算出的高度需要垂直滚动条时，就压着宽度重算高度，免得滚动条翻来覆去地出现又消失（第 157-177 行）。手柄长度随 range 与 pageStep 怎么变化，咱们在 3.32 篇读过公式，这里不重讲。

## 滚动条牵着内容走：一条以 setValue 为源头的短链

部件的定位正身是 updateWidgetPosition，咱们看它怎么摆。滚动值取负、按部件尺寸摆出 scrolled 矩形，再按 alignment 摆出 aligned 矩形，每个坐标轴各挑一个（第 135-143 行）。挑法后面 alignment 一节细说。

联动链短得出乎笔者意料。滚动条的 valueChanged，连到私有槽 _q_vslide（qabstractscrollarea.cpp 第 268 行），槽里算出位移增量，交给虚函数 scrollContentsBy（第 1372-1378 行），QScrollArea 的实现只做一件事，updateWidgetPosition（qscrollarea.cpp 第 323-329 行）。所以程序化滚动的正规姿势从来是改滚动条的 value，文档把直接调 scrollContentsBy 判为误用。链条的源头只有一个，手柄、键盘、滚轮全汇到 setValue 那一点上。

滚轮的路径值得咱们单独走一遍。视口上滚轮来了，wheelEvent 按两个方向分量谁大，把整个事件经 sendEvent 转发，交给对应的滚动条（qabstractscrollarea.cpp 第 1170-1176 行），源稿说默认实现直接设置 value，不准确，转发才是实情。

QScrollBar::wheelEvent 收下后，咱们跟着进量化环节。它调 scrollByDelta（qscrollbar.cpp 第 475-491 行），步数在 qabstractslider.cpp 里量化，delta 除以 120 得格数，乘 wheelScrollLines 再乘 singleStep，单次封顶 pageStep，凑不满一整格的小数余量留着，跟下一个事件累积（第 676、687-698 行）。触控板上连续的小 delta 就靠余量累积走通，平台默认已备好这套机制，自己写滚动逻辑时别把它算重了。

键盘的转发咱们在同一片区域找。Up 与 Down 各触发一次 singleStep 的加减，翻页键走 pageStep（qabstractscrollarea.cpp 第 1199-1221 行）。Tab 更体贴，focusNextPrevChild 把焦点交给下一个部件后，顺手调用 ensureWidgetVisible，把滚动跟上去（qscrollarea.cpp 第 400-408 行），表单里按 Tab 永远看得见焦点落点，靠的就是这一句。

## ensureWidgetVisible：滚到刚好够见，大目标对中

ensureVisible 吃内容坐标系里的一个点，四个方向的判断各自独立，已经可见就一个值都不动，默认留白 50 像素（第 416-433 行）。ensureWidgetVisible 是它的部件版，笔者把实现翻开后看到三道工序。

咱们逐道看。头一道验出身，目标部件若不是 widget() 的后代，直接返回（第 450-451 行）。二道取目标矩形，部件支持输入法时改取 ImCursorRectangle 的光标矩形，文本类部件会滚到光标处，普通部件取整个自身（第 453-458 行）。三道比较，可见矩形就是内容部件当前位置取负拼上视口尺寸（第 459 行），目标已经躺在里面就早退（第 461-462 行），否则分轴滚动。

```cpp
// 目标比视口大时的对中（qscrollarea.cpp 第 473-478 行）
if (focusRect.height() > visibleRect.height())
    d->vbar->setValue(focusRect.center().y() - d->viewport->height() / 2);
else if (focusRect.bottom() > visibleRect.bottom())
    d->vbar->setValue(focusRect.bottom() - d->viewport->height() + 1);
else if (focusRect.top() < visibleRect.top())
    d->vbar->setValue(focusRect.top());
```

源稿说目标比视口大时尽量让左上角可见，笔者对着源码核过，恰好相反，装不下时对齐的是中心（第 466-467、473-474 行），装得下时才把边缘对齐边缘。嵌套滚动区只滚外层的观感也从第一道工序来，isAncestorOf 检查只认 widget() 一脉，内层滚动控件里的目标根本进不了门。

## alignment 只在内容更小时发言，sizeHint 有封顶

updateWidgetPosition 每个轴上二选一，部件小于视口用 aligned 矩形的对齐坐标，大于视口用滚动坐标（第 139-142 行）。所以 alignment 只管“内容装得下时靠哪边”，内容溢出的轴上对齐设置无效，滚动说了算，您拿规则一推就知道行为。默认 Qt::AlignLeft 配 AlignTop，内容贴在左上角，setAlignment 改完立即重摆（第 500-506 行）。

尺寸建议这边有出人意料的天花板，咱们接着看。有部件时，sizeHint 取部件的 sizeHint 或当前尺寸，再被 36 行字高乘 24 行字高封顶（第 363-381 行），一个 3000 像素的长图进来，滚动区的建议尺寸也就巴掌大，窗口布局不会被内容拖走。滚动条策略是 AlwaysOn 时，建议宽度还主动补上滚动条的条宽（第 376-379 行），maximumViewportSize 反着做同一件事，视口尺寸里扣掉常驻滚动条（qabstractscrollarea.cpp 第 535-546 行）。一边补条宽一边扣视口，空间分配互相抵平，守恒的来路就在一补一扣之间，滑块侧的机制 3.32 与 3.33 篇都铺垫过，本篇正好接上。

## QSS 的背景落点在 viewport，写规则前认清收件人

样式表层维护着一张收件人映射。embeddedWidget 拿到 QAbstractScrollArea，返回的就是它的 viewport（qstylesheetstyle.cpp 第 2514-2516 行），调色板相关的规则随后落到它身上。您写 QScrollArea 加 background 的选择器，颜料刷在 viewport 上，本体那圈边框另走 QFrame 的绘制路径。配合构造时背景角色被改成 NoRole 那一笔（qscrollarea.cpp 第 107 行），默认外观里视口不抢背景，内容部件自己带着置真的 autoFillBackground，盖在上面。想给滚动区换底色，规则写给 QScrollArea 即可命中视口，想动边框就得另写，两个盒子分开对待才不白写。

配套示例的动画部分不碰 QSS，纯走属性动画，动画与默认滚动的冲突怎么处理，咱们放到结语一起看。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，全部行为在 6.9.1 源码核对。QScrollArea 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。动画部分涉及 QtCore 的 QPropertyAnimation。

[Qt 文档 · QScrollArea](https://doc.qt.io/qt-6/qscrollarea.html) -- setWidget 与 ensureWidgetVisible 的定义处

[Qt 文档 · QAbstractScrollArea](https://doc.qt.io/qt-6/qabstractscrollarea.html) -- viewport 与滚动条策略的所属基类

[Qt 文档 · QScrollBar](https://doc.qt.io/qt-6/qscrollbar.html) -- 滚动条本体，value 是联动链的源头

[Qt 文档 · QPropertyAnimation](https://doc.qt.io/qt-6/qpropertyanimation.html) -- 动画化滚动条 value 的工具

[Qt 文档 · QWheelEvent](https://doc.qt.io/qt-6/qwheelevent.html) -- 滚轮事件，angleDelta 与 phase 的定义处

---

视口的裁剪、部件的搬家、滚动的短链与两处尺寸计算都对上了号，43 篇递过来的那句话在这里收梢。QScrollArea 的头顶是 QAbstractScrollArea，再往上是 QFrame，带框控件们的共同基类，下一篇把它读穿，[3.45 QFrame 进阶](./45-qframe-advanced.md)，边框的样式与阴影怎么画，咱们接着看。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/44-qscrollarea-advanced/，可执行文件 44-qscrollarea-advanced）。窗口里是 15 段各 120 像素的彩色长图，底部三个按钮驱动平滑滚动（main.cpp 第 82-84、92-98 行）。SmoothScrollArea 一构造，就把 widgetResizable 设好，出处是 smooth_scroll_area.cpp 第 17 行。动画前必将在跑的旧动画停掉（第 40-42 行），时长按距离伸缩，压在 200 到 800 毫秒之间（第 45 行），缓动曲线取 OutQuad（第 61 行）。中间那个按钮的目标值计算用到了 viewport 的高度（main.cpp 第 106-112 行），正好把本篇的视口概念落在实处。您滚一格滚轮感受一格步进。示例在 configureScrollbar 里把 singleStep 调成了 30，出处 smooth_scroll_area.cpp 第 91 行，滚一格走 wheelScrollLines 默认的 3 乘 30 共 90 像素，比构造默认的 20 爽快。再点按钮看动画接管，两条路径汇到同一个 value 上的体感会非常清楚。

完成标准照旧。内容部件的父对象是谁，您说得出口。resizable 为真时部件尺寸被哪两把尺子夹住，您指得出算尺子的那两行。滚轮事件从视口到 setValue 中间经过谁的手，您数得清。目标比视口大时 ensureWidgetVisible 对齐哪里，alignment 在什么条件下才发言，您能分开答。
