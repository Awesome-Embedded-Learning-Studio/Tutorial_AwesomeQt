---
title: "3.13 QFrame 基类进阶"
description: "入门篇把 Shape、Shadow、lineWidth 调了一遍，进阶篇翻开 qframe.cpp 看边框由谁画。paintEvent 把参数打包交给 QStyle，边框宽度折进 contentsMargins，阴影卡片被裁的根子在父控件只认自己的 rect，配套示例在 examples/advanced/03-qtwidgets/13-qframe-base-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.13——QFrame 基类进阶

[入门篇 QFrame 基础](../../beginner/03-qtwidgets/13-qframe-base-beginner.md) 把 Shape、Shadow、lineWidth、HLine 分隔线这些边框属性都调了一遍，拿 QFrame 套个框、画条线，那些知识够用。控件基类部分走到第三篇，咱们看 QLabel、QStackedWidget、QScrollArea 共同的祖先 QFrame，边框的形状、立体感、线宽参数由它统一管着，上面那些控件用的 setFrameShape() 和 setFrameShadow() 也全是从它继承来的。

平时拿现成子类用，边框跟着应用风格走，咱们感觉不到它的存在。一旦继承 QFrame 或其子类做定制绘制，三个场面就会把人问住。咱们重写了 paintEvent，边框不画了。给卡片加 QGraphicsDropShadowEffect，阴影被父控件切掉一半。frameRect 和 contentsRect 谁套着谁，一算子控件的位置就露馅。三个场面各有各的病根，本篇讲明白四件事：边框到底由谁画，frameRect 与 contentsRect 怎么从属，卡片阴影为什么被裁、怎么留边，完全接管边框绘制时要注意什么。

## 边框不是 QFrame 自己画的：paintEvent 只负责打包交货

QFrame::paintEvent 的实现非常短，咱们翻开 qframe.cpp 就能数完，起一个 QStylePainter，调一次 drawFrame()。drawFrame 里构建一个 QStyleOptionFrame，initStyleOption 把 frameShape、lineWidth、midLineWidth 填进去，Shadow 转成 State_Raised 或 State_Sunken 状态位，最后 style()->drawControl(QStyle::CE_ShapedFrame, ...) 把绘制交给当前应用风格。具体落点咱们在 QCommonStyle 里也看得到，StyledPanel 分支转进 drawPrimitive(QStyle::PE_Frame)，Box 和 Panel 分支不经它，直接调 qDrawShadeRect、qDrawShadePanel，Shadow 取 Plain 时这两支都换成 qDrawPlainRect，画的是单色平边框。QFrame 自己不画边框，只负责打包交货，画笔始终在 QStyle 手里。

咱们可能会问，重写自己的 paintEvent，跟基类画边框有什么相干。QFrame 的边框全在基类 paintEvent 里画，咱们不调它，QStyle 就永远收不到那次绘制请求，边框不会出现，什么 Shape 都一样，StyledPanel 不是特例。还有一种顺序也吃亏，咱们先画自己的内容再调基类，内容就会被边框压住，因为 QStyle 拿到的绘制矩形是整个 frameRect，立体边框的线条就落在先画好的内容上面：

```cpp
void MyFrame::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    // 先画自定义内容
    painter.fillRect(rect(), QColor("#F5F5F5"));
    draw_custom_content(&painter);

    // 再让 QFrame 画边框——边框会覆盖在内容上方
    QFrame::paintEvent(event);
}
```

要同时保住边框和自定义内容，做法是让基类 paintEvent 先跑，咱们再画自己的内容，并且收在 contentsRect 之内。contentsRect 是 frameRect 扣掉四周边框宽度之后的区域，顺序反了是边框压内容，画出去是内容压边框，两头都难看。

## frameRect 套着 contentsRect：宽度折进了 contentsMargins

frameRect 是 QFrame 整个控件的矩形，默认等于控件自身的 rect()。contentsRect 是去掉边框之后的内容区域，等于 frameRect 四周各扣一圈边框宽度。咱们拿手算对照一遍：

```cpp
// contentsRect 的等价手动计算
int fw = frameWidth();  // Box + Raised/Sunken 时成立，Panel 恒为 lineWidth（见下表）
QRect content = rect().adjusted(fw, fw, -fw, -fw);
// 这等价于 contentsRect()
```

frameWidth() 的返回值跟着 Shape 与 Shadow 的组合走，咱们把 QCommonStyle 里 SE_ShapedFrameContents 分支（QStyle 计算内容区收进去多少的入口）整理成表：

| Shape 与 Shadow 组合 | frameWidth() |
| -------- | -------- |
| NoFrame（Shadow 任意） | 0 |
| Box + Plain | lineWidth |
| Box + Raised / Sunken | lineWidth * 2 + midLineWidth |
| Panel（Shadow 任意） | lineWidth |
| StyledPanel | 由当前 QStyle 决定 |

StyledPanel 不吃线性公式，宽度由当前风格报出来，换一个 QStyle 数值就可能变。所以自己手算很容易在 StyledPanel 或者换风格之后算错，笔者的习惯是直接调 contentsRect()，不跟公式较劲。

机制里还有一层值得看。qframe.cpp 里 updateFrameWidth 算完四边宽度，setFrameRect 转头就把它们写进了 QWidget 的 setContentsMargins，布局管理器随后拿到的 contentsRect 已经扣掉了边框。所以在 QFrame 里用布局放子控件，咱们只管把控件塞进布局，子控件自动落在边框内侧。手动 setGeometry 没有这层保护，子控件和边框重叠的错位都是这么来的。

### 一道思考题：inner 落在哪、多大

咱们看一段配置代码，您先自己算一遍：

```cpp
auto *frame = new QFrame();
frame->setFrameShape(QFrame::Box);
frame->setFrameShadow(QFrame::Raised);
frame->setLineWidth(3);
frame->setMidLineWidth(1);
frame->setFixedSize(200, 100);

auto *inner = new QWidget(frame);
inner->setGeometry(frame->contentsRect());
```

一步步推。Box 配 Raised，每侧边框带是 lineWidth 加 midLineWidth 再加 lineWidth，3 + 1 + 3 = 7，frameWidth() 报 7。contentsRect 因此是 (7, 7, 186, 86)，200 减两侧共 14 得 186，100 减 14 得 86，inner 拿到的正是它。您留意 contentsRect 返回的 x 和 y 是边框的偏移量，不从 (0, 0) 开始，要是图省事写 setGeometry(0, 0, contentsRect().width(), contentsRect().height())，inner 的左上角就和边框叠上了。要么整块用 contentsRect()，要么交给布局管理器自动约束。

## 阴影卡片：效果画在控件 rect 外面，裁剪只认父控件的 rect

现代 UI 里卡片很流行，带圆角和阴影的容器面板，内容放在里面。QFrame 配合 QGraphicsDropShadowEffect 是常见方案，咱们把典型配置摆出来：

```cpp
auto *card = new QFrame(parent);
card->setFrameShape(QFrame::StyledPanel);
card->setStyleSheet("QFrame { background: white; border-radius: 8px; }");

auto *shadow = new QGraphicsDropShadowEffect(card);
shadow->setBlurRadius(20);
shadow->setOffset(0, 4);
shadow->setColor(QColor(0, 0, 0, 60));
card->setGraphicsEffect(shadow);
// 如果父控件的布局没有给 card 留至少 24px 的 margin，阴影会被裁掉
```

QGraphicsDropShadowEffect 在控件周围绘制一幅模糊的阴影图像，绘制区域会超出控件自身的 rect。咱们拿 200x100 的 QFrame 加偏移 0、模糊半径 20 的阴影算，实际绘制区域约 240x140。问题出在裁剪上，QWidget 默认把子控件的绘制裁剪在自身 rect 之内，父控件的布局要是没给卡片留够边距，阴影的边缘就会被硬切出一道笔直的边。解决方向是给父控件的布局留足 margin，至少等于 blurRadius 加 offset 的绝对值。用 QScrollArea 当父容器时还得再加一手，给视口调 setViewportMargins()，因为 QScrollArea 的视口裁剪独立于布局，光给布局设 margin 不够。

还有性能一面要您留意。QGraphicsDropShadowEffect 在控件每次重绘时都要做一次模糊运算，界面上几十个带阴影的卡片一起滚动或做动画，可能掉帧。大量重复的卡片场景，更高效的做法是在 paintEvent 里用 QPainter 手绘阴影。想从 CSS 抄近路、给 QSS 写个 box-shadow 是走不通的，Qt 的样式表引擎从不解析这个属性，写上去只换来运行时一条 Unknown property 的告警，阴影不会出现。阴影可走的始终是 QGraphicsDropShadowEffect 和手绘这两条路，正是本篇摆在一起讲的。

配套示例把两条路都做出来了，文件在 examples/advanced/03-qtwidgets/13-qframe-base-advanced/，可执行文件 13-qframe-base-advanced。窗口里摆着两排卡片，上排卡片 A、B 走手绘，下排卡片 C 走特效。两排用的都是 ShadowCard，示例里继承 QFrame 的自定义卡片类，一个类提供 Manual 与 DropEffect 两种模式。Manual 模式走手绘，构造函数设 NoFrame，paintEvent 里从贴着卡片的那圈矩形起，一层层向外铺 18 层半透明圆角矩形，层层叠上来之后越靠近卡片越暗，向外渐渐变淡，叠出柔和阴影，卡片本体和内容都收在预留的内边距里，阴影整体落在控件自己的 rect 内，父布局一点边都不用留。DropEffect 模式走特效，StyledPanel 配 blurRadius 20、offset (0, 6)，main.cpp 给卡片 C 所在的行留了 30 像素的左 margin，盖过 20 加 6 的外溢量。咱们做两个实验。把 main.cpp 里 effectRow（下排那一行）的 setContentsMargins(30, 10, 30, 10) 改成 (0, 10, 0, 10) 重新编译，卡片 C 左侧的阴影贴着窗口边缘被切齐，改回去又完整了。再把 manualRow（上排那一行）的 margin 也改成 0，卡片 A、B 的阴影纹丝不动，手绘的阴影不出自己的 rect，天生不怕裁。两条路的取舍，跑一遍就摸清了。

## 完全接管边框：NoFrame 给自绘让路

要做渐变边框、虚线边框、带图标的边框，Shape 体系给不了，咱们可以自己构建 QStyleOptionFrame 调 drawPrimitive，也可以干脆全部自己画：

```cpp
void GradientFrame::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 自己画一个渐变边框，跳过 QFrame 默认的边框绘制
    QLinearGradient gradient(0, 0, width(), height());
    gradient.setColorAt(0.0, QColor("#667eea"));
    gradient.setColorAt(1.0, QColor("#764ba2"));

    int fw = 3;  // 自定义边框宽度
    QPen pen(QBrush(gradient), fw);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rect().adjusted(fw/2, fw/2, -fw/2, -fw/2), 8, 8);
}
```

这么写就把 QFrame 的 Shape/Shadow 系统整个绕过去了，setFrameShape、setFrameShadow 都不必再调，边框的视觉完全由 paintEvent 决定。选自绘的话，建议把 frameShape 设成 NoFrame，QFrame 基类的 paintEvent 不会画任何东西，不会和自定义绘制冲突，然后重写 paintEvent 只画自己的内容。此时不调用 QFrame::paintEvent 也不碍事，NoFrame 之下它本来就没有边框可画，要保边框时调的才是它，前文那条先基类后内容的顺序说的就是这一层。至于 QWidget::paintEvent，咱们翻开 qwidget.cpp 就能看到，默认实现是空的，调不调都不影响样式表背景。背景走的是另一条通道，样式表给控件挂上 WA_StyledBackground，绘制在 paint 事件分发之前就由样式完成了，跟这一次调用不相干。

自绘这条线还能走得更远。圆角边框的 QPainterPath 画法、把阴影缓存进 QPixmap 省掉重复绘制、卡片容器的布局细节，加上成套的组合示例，咱们都放在 [3.45 QFrame 进阶](./45-qframe-advanced.md)，本篇守住基类机制，样式细节那边接着展开。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，QFrame 在 QtWidgets 模块，QPainter 与 QGraphicsEffect 来自 QtGui，QStyle::drawPrimitive 相关代码需包含 <QStyle> 和 <QStyleOptionFrame> 头文件，示例在桌面平台上行为一致。

[Qt 文档 · QFrame](https://doc.qt.io/qt-6/qframe.html) -- QFrame 可视框架基类，frameRect/contentsRect/frameWidth 接口

[Qt 文档 · QGraphicsDropShadowEffect](https://doc.qt.io/qt-6/qgraphicsdropshadoweffect.html) -- 阴影特效类，blurRadius/offset/color 属性

[Qt 文档 · QStyle::drawPrimitive](https://doc.qt.io/qt-6/qstyle.html#drawPrimitive) -- QStyle 原始绘制接口，PE_Frame 元素

[Qt 文档 · QStyleOptionFrame](https://doc.qt.io/qt-6/qstyleoptionframe.html) -- 帧绘制参数包，lineWidth/midLineWidth/features

---

QFrame 的边框咱们看到底了：绘制权在 QStyle 手里，边框宽度折在 contentsMargins 里，阴影外溢要靠父容器腾地方。想再动手的，请您把配套示例（examples/advanced/03-qtwidgets/13-qframe-base-advanced/，可执行文件 13-qframe-base-advanced）完整玩一轮。上排卡片 A、B 是手绘阴影，下排卡片 C 是特效阴影，上下两排对比着放，质感差别一眼可见。再把 shadow_card.cpp 顶部的 kShadowBlur 从 18 改到 40 重新编译，手绘阴影扩散得更宽，内容区跟着往里收，阴影宽度和内容留白的联动就看明白了，看完记得改回去。下一篇 [3.14 QAbstractScrollArea 基类进阶](./14-qabstractscrollarea-base-advanced.md) 讲 QScrollArea、QTextEdit、QAbstractItemView 的共同基类，从两个滚动区的联动同步讲起，再到视口坐标与内容坐标的换算、滚动条策略切换时视口宽度的变化。
