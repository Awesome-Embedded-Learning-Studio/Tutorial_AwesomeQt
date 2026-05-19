---
title: "3.13 QFrame 基类进阶"
description: "入门篇我们把 Shape、Shadow、lineWidth 这些表面属性过了一遍。进阶篇我们要钻到 QFrame 的绘制管线里去，搞清楚边框到底是谁画的、自定义绘制时怎么和 QFrame 的边框和平共处、以及 QGraphicsDropShadowEffect 那些让人抓狂的裁剪陷阱。"
---

# 现代Qt开发教程（进阶篇）3.13——QFrame 基类进阶

## 1. 前言 / 为什么 QFrame 还值得再讲一遍

入门篇我们把 QFrame 的 Shape、Shadow、lineWidth 这些 API 用了一遍，知道了 Box 和 Panel 看起来有什么区别，也知道了 HLine 能当分隔线用。说实话，如果你只是拿 QFrame 套个框、画条线，那些知识确实够用。但当你开始做自定义控件——尤其是继承 QFrame 或其子类做定制绘制时——事情就会变得微妙起来：你重写了 paintEvent，结果 QFrame 的边框不画了；你加了个 QGraphicsDropShadowEffect 想做卡片阴影，阴影被父控件裁掉了一半；你搞不明白 frameRect 和 contentsRect 到底谁包含谁。这些问题入门篇没有展开，而这篇进阶就是来解决它们的。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17 标准。内容依赖 QtWidgets 模块和 QtGui 中的 QPainter/QGraphicsEffect。涉及 QStyle::drawPrimitive 的部分需要包含 `<QStyle>` 和 `<QStyleOptionFrame>` 头文件。所有示例在桌面平台上行为一致。

## 3. 核心概念讲解

### 3.1 QFrame 的绘制管线——paintEvent 里到底发生了什么

QFrame 的 paintEvent 并不是什么黑魔法，它做的事情非常明确：构建一个 QStyleOptionFrame，把自己的 frameShape、frameShadow、lineWidth、midLineWidth 全部填进去，然后调用 `QStyle::drawPrimitive(QStyle::PE_Frame, ...)` 让当前的应用风格来画边框。这就意味着，QFrame 的边框并不是"QFrame 自己画的"，而是"QFrame 把参数打包交给 QStyle，QStyle 画的"。

理解这一点非常重要，因为当你重写 paintEvent 时，如果你没有调用基类的 paintEvent，QFrame 的边框就不会被绘制——QStyle 永远不会收到那个 drawPrimitive 调用。反过来，如果你在 paintEvent 里先画了自己的内容再调基类，你的内容就会被边框盖住，因为 QStyle 在绘制 Panel/Raised 这种带立体感的边框时，它的绘制区域会覆盖 frameRect 的全部。

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

如果你需要在 QFrame 上同时保留边框和自定义内容，正确的做法是在基类 paintEvent 之后画你的内容，但要限制在 contentsRect 内——contentsRect 是 frameRect 减去边框宽度之后的区域，你的自定义绘制不应该超出这个范围。

### 3.2 frameRect 与 contentsRect 的关系——别再搞混了

这两个 rect 是 QFrame 坐标系里最容易搞混的概念。frameRect 是 QFrame 整个控件的矩形区域，它等于控件自身的 rect()。contentsRect 是去掉边框之后的内容区域——frameRect 减去四周的边框宽度。

```cpp
// contentsRect 的等价手动计算
int fw = frameWidth();  // lineWidth * 2 + midLineWidth（对于 Box/Panel）
QRect content = rect().adjusted(fw, fw, -fw, -fw);
// 这等价于 contentsRect()
```

frameWidth() 的返回值取决于 frameShape 和 Shadow 的组合。对于 Box + Raised，frameWidth 等于 lineWidth * 2 + midLineWidth；对于 StyledPanel，frameWidth 由当前 QStyle 决定，不一定是简单的线性计算。这就是为什么你应该用 contentsRect() 而不是自己算——自己算很容易在 StyledPanel 或者换了 QStyle 之后算错。

现在有一道思考题给大家。看下面这段代码：

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

inner 控件会被放在什么位置，它的实际大小是多少？frameWidth 在这里是 3 + 1 + 3 = 7（每侧 lineWidth + midLineWidth + lineWidth），所以 contentsRect 是 (7, 7, 186, 86)。inner 的 setGeometry 拿到的是 (7, 7, 186, 86)。这里要注意 contentsRect 返回的 x 和 y 就是边框的偏移量，不是从 (0,0) 开始的。如果你直接用 `inner->setGeometry(0, 0, frame->contentsRect().width(), frame->contentsRect().height())`，inner 的左上角会和边框重叠。正确的做法是用 `inner->setGeometry(frame->contentsRect())` 或者用布局管理器让它自动约束在 contentsRect 内。

### 3.3 QGraphicsDropShadowEffect 与 QFrame 卡片容器

在现代 UI 设计中，"卡片"布局非常流行——一个带圆角和阴影的容器面板，里面放内容。QFrame 配合 QGraphicsDropShadowEffect 是实现卡片的常见方案。但这里有一个经典的坑：阴影会被父控件的裁剪区域切掉。

QGraphicsDropShadowEffect 通过在控件周围绘制一个模糊的阴影图像来工作。阴影的绘制区域会超出控件自身的 rect——比如一个 200x100 的 QFrame 加了偏移量为 0、模糊半径为 20 的阴影，实际绘制区域大约是 240x140。问题在于，QWidget 默认会裁剪子控件的绘制区域到自身的 rect 内。如果 QFrame 的父控件没有足够的边距来容纳阴影，阴影的边缘就会被硬切掉，看起来就像被刀砍了一样。

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

解决方案有两个方向。第一，给父控件的布局留足够的 margin——至少等于 blurRadius + abs(offset)。第二，如果你使用 QScrollArea 作为父容器，需要给 QScrollArea 的 viewport 设置 `setViewportMargins()` 而不是给布局设 margin，因为 QScrollArea 的视口裁剪是独立于布局的。另外要注意，QGraphicsDropShadowEffect 对性能有一定影响——它在每次控件需要重绘时都要做一次高斯模糊运算。如果你的界面上有几十个带阴影的卡片，滚动或动画时可能会出现掉帧。对于大量重复的卡片场景，更高效的做法是用 QPainter 在 paintEvent 里手动绘制阴影，或者直接用 QSS 的 box-shadow（如果你的 Qt 版本支持）。

### 3.4 QStyle::drawPrimitive 自定义帧渲染

前面说了 QFrame 的 paintEvent 本质上是调用 `QStyle::drawPrimitive`。如果你想做完全自定义的边框效果——比如渐变边框、虚线边框、带图标的边框——你可以自己构建 QStyleOptionFrame 然后调用 drawPrimitive，或者干脆跳过 drawPrimitive 自己画。

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

这种做法完全绕过了 QFrame 的 Shape/Shadow 系统——你不再需要 setFrameShape、setFrameShadow 这些 API，因为边框的视觉完全由你的 paintEvent 决定。如果你走这条路，建议把 frameShape 设为 NoFrame，这样 QFrame 的基类 paintEvent 不会画任何东西，不会和你的自定义绘制冲突。然后重写 paintEvent 只画你自己的内容。你也可以不调用 QFrame::paintEvent，但一定要调用 QWidget::paintEvent 以确保样式表的背景等属性被正确处理。

## 4. 踩坑预防

第一个坑是 QGraphicsDropShadowEffect 的阴影被父控件裁剪。现象是阴影的某一侧或某几侧突然消失，像被刀切了一样整齐。原因就是 QWidget 默认的裁剪行为：子控件的绘制不会超出父控件的 rect。解决方案是确保父控件的布局给带阴影的 QFrame 留了足够的 margin——至少 blurRadius 加上 offset 的绝对值。如果你用的是 QScrollArea，还需要额外调用 `scrollArea->setViewportMargins(...)` 给视口留边距。

第二个坑是 StyledPanel 配合自定义 paintEvent 时边框消失。StyledPanel 的绘制完全依赖 QStyle——如果你在 paintEvent 里没有调用 QFrame::paintEvent（或者没有自己构建 QStyleOptionFrame 并调用 drawPrimitive），StyledPanel 就不会画边框。而 Box 和 Panel 的边框也依赖 QStyle::drawPrimitive，不是硬编码的绘制逻辑。所以只要你重写了 paintEvent 并且没调基类，不管用什么 Shape，边框都不会出现。解决方案是：要么在 paintEvent 末尾调用 QFrame::paintEvent 让基类画边框，要么自己接管所有绘制并设 frameShape 为 NoFrame。

第三个坑是 frameRect 与 contentsRect 混淆导致子控件布局错位。frameRect 等于控件自身的 rect()，contentsRect 是去掉边框后的区域。如果你手动 setGeometry 子控件时用了 frameRect 而不是 contentsRect，子控件会和边框重叠。如果你用了布局管理器，这个问题不会出现——布局管理器会自动把子控件约束在 contentsRect 内。所以最简单的建议是：QFrame 里放子控件时尽量用布局管理器，不要手动 setGeometry。

## 5. 练习项目

练习项目：可自定义边框风格的卡片容器控件。我们要实现一个 CardWidget 类，继承 QFrame，支持三种边框模式——"系统默认"使用 StyledPanel 让 QStyle 画边框、"渐变边框"在 paintEvent 中用 QLinearGradient 自己画、"阴影卡片"使用 QGraphicsDropShadowEffect 配合 NoFrame。CardWidget 有一个公共方法 setCardMode(CardMode) 切换三种模式。窗口上方放三个按钮切换模式，下方放一个 CardWidget 展示效果，CardWidget 内部放一些示例内容（标题 QLabel + 一段说明文字 + 一个按钮）。

完成标准是三种模式切换正确且视觉无误，渐变边框圆角平滑无锯齿，阴影卡片不被裁剪，切换模式时无闪烁。提示几个关键点：切换到自定义绘制模式时设 frameShape 为 NoFrame 并重写 paintEvent；阴影模式需要给外层布局留足 margin；模式切换后调用 update() 触发重绘。

## 6. 官方文档参考链接

[Qt 文档 · QFrame](https://doc.qt.io/qt-6/qframe.html) -- QFrame 可视框架基类，frameRect/contentsRect/frameWidth 接口

[Qt 文档 · QGraphicsDropShadowEffect](https://doc.qt.io/qt-6/qgraphicsdropshadoweffect.html) -- 阴影特效类，blurRadius/offset/color 属性

[Qt 文档 · QStyle::drawPrimitive](https://doc.qt.io/qt-6/qstyle.html#drawPrimitive) -- QStyle 原始绘制接口，PE_Frame 元素

[Qt 文档 · QStyleOptionFrame](https://doc.qt.io/qt-6/qstyleoptionframe.html) -- 帧绘制参数包，lineWidth/midLineWidth/features

---

到这里，QFrame 进阶的内容就过了一遍。绘制管线的本质是 QStyle::drawPrimitive，frameRect 和 contentsRect 的关系搞清楚了就不会再布局错位，QGraphicsDropShadowEffect 的裁剪陷阱知道了就不会再被切阴影搞得抓狂，自定义帧渲染给了你完全掌控边框视觉的能力。QFrame 虽然是个看起来很简单的基类，但当它跟自定义绘制和图形特效结合时，门道不少。
