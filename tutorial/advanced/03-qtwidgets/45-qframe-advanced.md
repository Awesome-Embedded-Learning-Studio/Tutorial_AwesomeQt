---
title: "3.45 QFrame 进阶"
description: "入门篇用 QFrame 画分隔线和包边框容器，进阶篇把本体对着 6.9.1 读穿：frameStyle 是一个整型按位装进 shape 与 shadow 两个位段而 setFrameStyle 会顺手替换尺寸策略、frameWidth 只读且由样式经 SE_ShapedFrameContents 按形状与阴影算出（Box 系立体时等于 lineWidth 乘 2 加 midLineWidth、StyledPanel 问 PM_DefaultFrameWidth、Panel 恒等于 lineWidth）、绘制链 paintEvent 到 drawFrame 到 CE_ShapedFrame 四行交接而 StyledPanel 分支转手 PE_Frame、frameRect 与 contentsMargins 互为镜像经 setContentsMargins 存储并同步 SE_FrameLayoutItem 布局边距、QSS border 一旦接管边框绘制挪到 PE_Widget 且 frameWidth 改随 border 宽度、sizeHint 对 HLine 与 VLine 给出 3 像素厚的固定建议、QLabel 与 QLCDNumber 复用 drawFrame 而 QAbstractScrollArea 构造自带 StyledPanel 配 Sunken、示例 ShadowFrame 撤掉默认边框用四遍固定低透明度叠层的圆角矩形手绘投影并预留内容边距，配套示例在 examples/advanced/03-qtwidgets/45-qframe-advanced/"
---

# 现代Qt开发教程（进阶篇）3.45——QFrame 进阶

[3.44 QScrollArea 进阶](./44-qscrollarea-advanced.md) 结尾把话递了过来：滚动区的本体是个带框 QFrame，带框控件们的共同基类，本篇把它读穿。入门篇您用 setFrameShape 与 setFrameShadow 画过分隔线和边框容器，调出想要的样子就算完成。进阶篇咱们换一双更细的眼睛：frameStyle 那个整型里装了什么，frameWidth 由谁定，绘制链从 paintEvent 走到哪一步，QSS 一来边框换谁画。源稿把重心压在手绘圆角与投影上，机制层几乎没碰，本篇按源码重排，改准的地方随文给出行号。

本篇引用以 qframe.cpp 与 qframe.h 为主，咱们把形状分派补到 qcommonstyle.cpp，样式表接管补到 qstylesheetstyle.cpp，阴影效果的取材补到 qgraphicseffect.cpp。配套示例的完整玩法放在结语，正文引用文件名加行号。

## frameStyle 是一个整型装两样东西，setFrameStyle 顺手换了尺寸策略

默认值咱们从头数。QFramePrivate 构造时 frameStyle 拿 NoFrame 配 Plain，lineWidth 是 1，midLineWidth 是 0（qframe.cpp 第 18-27 行）。Shape 枚举占低四位，NoFrame 的 0 到 StyledPanel 的 6，Shadow 占 0x10 到 0x30 的档位，Shadow_Mask 与 Shape_Mask 两个掩码，就是为按位取出它们准备的（qframe.h 第 38-58 行）。

setFrameShape 与 setFrameShadow 是同一个入口的两个半边，您设哪一个，它都保住另一半再重组。setFrameShape 拿旧 frameStyle 与 Shadow_Mask 按位与，或上新形状（qframe.cpp 第 263-267 行），setFrameShadow 反着来（第 282-286 行）。setFrameStyle 是它们的全量版，文档写明，参数由 shape 与 shadow 按位或而来，出处第 288-305 行。

setFrameStyle 里还藏着一笔容易被忽略的动作，笔者读到这儿停了一下。

```cpp
// setFrameStyle 开头的隐藏动作（qframe.cpp 第 309-323 行）
if (!testAttribute(Qt::WA_WState_OwnSizePolicy)) {
    QSizePolicy sp;

    switch (style & Shape_Mask) {
    case HLine:
        sp = QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed, QSizePolicy::Line);
        break;
    case VLine:
        sp = QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum, QSizePolicy::Line);
        break;
    default:
        sp = QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred, QSizePolicy::Frame);
    }
    setSizePolicy(sp);
    setAttribute(Qt::WA_WState_OwnSizePolicy, false);
}
```

判断标志 WA_WState_OwnSizePolicy，由您显式调 QWidget::setSizePolicy 时置位，出处 qwidget.cpp 第 10396-10399 行。也就是说，只要策略不是您亲手设的，setFrameStyle 就按形状替您设一份：分隔线自动拿 Line 型，其余形状拿 Frame 型。末尾又把标志清回假（第 323 行），于是您不设策略，之后每次调 setFrameStyle 都可能重设。分隔线塞进 QVBoxLayout 天然压成 3 像素高，也与 sizeHint 呼应：HLine 给 QSize(-1, 3)，VLine 给 QSize(3, -1)，负一那一维交给布局去拉伸，其余形状回落到 QWidget::sizeHint，出处第 464-478 行。

应用面咱们顺带认全。QAbstractScrollArea 的私有 init 给本体设了 setFrameStyle，样式是 StyledPanel 配 Sunken，出处 qabstractscrollarea.cpp 第 274 行，44 篇那圈凹陷边框的出处就在这儿。QLabel 与 QLCDNumber 重写了 paintEvent 又要复用边框绘制，调的是受保护的 drawFrame（qlabel.cpp 第 969 行、qlcdnumber.cpp 第 665 行），qframe.cpp 第 492 行的注释写明函数供它们两位使用。QStackedWidget、QSplitter、QToolBox 也都从 QFrame 派生（各自头文件），一份边框机制全家族共用。

## frameWidth 只读，由样式按形状与阴影算给您

frameWidth 在 qframe.h 里是只读属性（第 24 行），咱们设不进值，setFrameStyle、setLineWidth、setMidLineWidth，收尾都调 updateFrameWidth（qframe.cpp 第 342-349、366-373 行）。算的过程走 updateStyledFrameWidths：拿当前选项问样式的 SE_ShapedFrameContents，内容矩形与外框的差值是四边各自的边框宽，frameWidth 取四边最大（第 385-398 行）。

分派表的正文在 QCommonStyle，咱们按形状数（qcommonstyle.cpp 第 2985-3027 行）。NoFrame 恒为 0。Box、HLine、VLine 一组，Plain 影子下等于 lineWidth，Raised 或 Sunken 下等于 lineWidth 乘 2 加 midLineWidth，立体边框要画明暗两圈。StyledPanel 特殊，转手问 SE_FrameContents，那里取像素度量 PM_DefaultFrameWidth（第 2966-2971 行），Fusion 风格给 1（qfusionstyle.cpp 第 2850-2851 行）。WinPanel 恒为 2。Panel 不分影子，恒等于 lineWidth（第 3016-3024 行）。

算的时机也值得记一笔。StyleChange、ParentChange 与 Polish 三类事件，都会重跑 updateFrameWidth（qframe.cpp 第 504-526 行），部件换风格、换父对象、完成初始化时边框宽都会重算，您在构造函数里读到的 frameWidth 未必是定稿。

## 绘制链四行交接，StyledPanel 半路转手 PE_Frame

QFrame 的 paintEvent 只有两行正文，咱们跟着走（qframe.cpp 第 483-487 行）。QStylePainter 起笔，drawFrame 收尾。drawFrame 也是薄壳，initStyleOption 填好选项，再交给 drawControl，控件类型是 CE_ShapedFrame（第 494-499 行）。源稿说默认绘制直接调 drawPrimitive 画边框，笔者对着源码核过，改准为：这一步走 drawControl，PE_Frame 只在后面 StyledPanel 分支出现。

选项里带了什么，咱们看 initStyleOption（第 195-227 行）。Box、HLine、VLine、StyledPanel、Panel 五种形状原样透传 lineWidth 与 midLineWidth，其余形状只给 frameWidth（第 203-221 行），Sunken 与 Raised 则被翻译成 State_Sunken 与 State_Raised，成为两个风格状态位（第 223-226 行）。

CE_ShapedFrame 的分派表，咱们逐格看（qcommonstyle.cpp 第 2354-2423 行）。Box 由 qDrawShadeRect 负责，Plain 时换 qDrawPlainRect，单色描线（第 2371-2377 行）。Panel 走 qDrawShadePanel，见第 2387-2393 行。HLine 与 VLine 算出两端点画一条线，立体时 qDrawShadeLine（第 2401-2420 行）。StyledPanel 是唯一不自己画的，转手 drawPrimitive(PE_Frame)，边框长相完全交给当前风格（第 2378-2386 行），44 篇说 QScrollArea 边框随平台变脸，根子在这步转手。

分派表也向咱们解释了源稿那句“预定义形状都是直角”。表里每一格用的都是矩形描线函数，没有一个带圆角参数，想要圆角边框，机制层帮不上忙，只能重写 paintEvent 自己画，示例正是这么做的。

## frameRect 与 contentsMargins 互为镜像，geometry 管外壳

frameRect 名字像个存储属性，实现却是现场算的。frameRect 从 contentsRect 出发，四边各向外扩对应的边框宽（qframe.cpp 第 446-452 行）。setFrameRect 反着写，咱们看摘录。

```cpp
// setFrameRect 落到 contentsMargins 上（qframe.cpp 第 454-460 行）
QRect cr = r.isValid() ? r : rect();
cr.adjust(d->leftFrameWidth, d->topFrameWidth, -d->rightFrameWidth, -d->bottomFrameWidth);
setContentsMargins(cr.left(), cr.top(), rect().right() - cr.right(), rect().bottom() - cr.bottom());
```

您给的矩形无效时回落整个部件（文档第 433-444 行），有效时四边内缩边框宽，差值写进 setContentsMargins。所以 frameRect 与 contentsMargins 是同一份数据的两个视图，改谁另一个跟着变，QSS 下它还被标成 DESIGNABLE false（qframe.h 第 25 行），设计器都不让您碰。与 geometry 的分工一句话摆平：geometry 与 resize 管部件外壳的位置大小，frameRect 只管边框画在壳内哪个矩形。布局那边还有联动，updateFrameWidth 收尾调 setLayoutItemMargins(SE_FrameLayoutItem)（第 33-42、411 行），边框宽变化会同步成布局项的边距，子内容自动避开边框。

## QSS 一接管，绘制与宽度一起换主

样式表层给绘制链做了岔路口。QSS 规则带原生边框（hasNativeBorder，即规则没写 border 属性、留给底风格画）时，CE_ShapedFrame 把矩形裁成 borderRect，之后仍交给底风格（qstylesheetstyle.cpp 第 4528-4535 行）。您一旦写了 border 属性，分支直接跳过，源码注释写明边框已在 PE_Widget 里画过（第 4536 行）。

frameWidth 同步换主。SE_ShapedFrameContents 遇到 box 或非原生边框，返回 rule.contentsRect（第 6200-6205 行），PM_DefaultFrameWidth 遇到非原生边框，直接返回 border 宽（第 5052-5054 行），您写了 3 像素的 border，frameWidth 就是 3，回不到 lineWidth 那套算法。所以 QSS border 与 frameStyle 的关系是接管，别当两层妆叠着，写 QSS 定制边框时 setFrameStyle 一套可以整个放掉。44 篇那句“本体那圈边框另走 QFrame 的绘制路径”，链路到这儿闭合。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，全部行为在 6.9.1 源码核对。QFrame 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。手绘部分会用到 QtGui 的 QPainter，以及 QPainterPath。

[Qt 文档 · QFrame](https://doc.qt.io/qt-6/qframe.html) -- frameStyle 与 frameWidth 属性的定义处

[Qt 文档 · QStyle](https://doc.qt.io/qt-6/qstyle.html) -- CE_ShapedFrame 与 SE_ShapedFrameContents 的所属类

[Qt 文档 · QPainterPath](https://doc.qt.io/qt-6/qpainterpath.html) -- addRoundedRect 画圆角路径

[Qt 文档 · QGraphicsDropShadowEffect](https://doc.qt.io/qt-6/qgraphicsdropshadoweffect.html) -- 另一条阴影路线，代价随重绘频率走

[Qt 文档 · QPainter](https://doc.qt.io/qt-6/qpainter.html) -- setRenderHint(Antialiasing) 开抗锯齿

---

位段、宽度来源、绘制链、镜像与接管都对上了号，44 篇递过来的话在这里收梢。QFrame 往下撑着 QLabel 与 QLCDNumber 这类小件，往上撑着滚动区这类大件，本系列也该去看内容更重的部件了，[3.46 QListWidget 进阶](./46-qlistwidget-advanced.md)，列表部件怎么把条目管起来，咱们接着看。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/45-qframe-advanced/，可执行文件 45-qframe-advanced）。窗口是 2 行 3 列六张卡片，各拿不同的圆角、模糊与偏移参数对着看（main.cpp 第 68-122 行）。ShadowFrame 构造里两步关门，setFrameShape(NoFrame) 撤掉默认边框，WA_StyledBackground 显式置假，出处 shadow_frame.cpp 第 22-23 行，随后 updateContentMargins 预留阴影边距，出处第 24 行。paintEvent 的投影是四遍叠加（第 136-155 行），每遍把圆角矩形向外扩一截，透明度每遍固定取 alpha 的四分之一，靠四遍叠层渐隐，软边就出来了。内容矩形与布局边距共用一个边距公式，模糊半径加两个方向偏移的绝对值再加 2（第 108-109、116-117 行），子控件因此不压进阴影。另一条路 QGraphicsDropShadowEffect，生效绘制时，它把源部件渲染成一张 QPixmap，再交给滤镜，出处 qgraphicseffect.cpp 第 1014-1034 行，静态面板用它省事，滚动里频繁重绘的卡片学示例手绘更划算。

完成标准照旧。frameStyle 里两个位段各占哪几位，您数得出。Box 配 Sunken 时 frameWidth 等于什么，您算得出。paintEvent 到 PE_Frame 中间经过谁的手，您数得清。QSS border 写上之后，绘制与宽度各自换到谁手里，您说得出口。
