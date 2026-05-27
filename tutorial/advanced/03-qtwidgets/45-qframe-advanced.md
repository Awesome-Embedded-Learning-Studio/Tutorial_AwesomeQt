---
title: "3.45 QFrame 进阶"
description: "入门篇我们用 QFrame 做了分隔线和简单边框容器，掌握了 setFrameShape/setFrameShadow/setLineWidth 以及 QSS border 定制的基本用法。"
---

# 现代Qt开发教程（进阶篇）3.45——QFrame 进阶

## 1. 前言 / 当边框不再只是"一条线"

入门篇我们用 QFrame 做了分隔线和简单边框容器，掌握了 setFrameShape / setFrameShadow / setLineWidth 以及 QSS border 定制的基本用法。对于大多数"画个分隔线"或者"给面板加个边框"的需求，入门篇的内容已经够用了。但一旦你想要更精致的视觉效果——圆角边框、带模糊的投影阴影、半透明的毛玻璃容器——QFrame 的 setFrameShape 就完全不够看了。setFrameShape 提供的几种预定义形状（Box、Panel、HLine、VLine 等）都是硬编码的直角矩形，你不能改变圆角半径，不能加阴影，更不能做半透明。

今天我们把 QFrame 的视觉定制能力推到 QPainter 层面。核心内容是三个方面：重写 paintEvent 用 QPainter 绘制圆角矩形边框和投影阴影、QGraphicsDropShadowEffect 的使用与性能陷阱、以及 QFrame 作为自定义卡片容器的布局实践。

## 2. 环境说明

本篇基于 Qt 6.5+ 版本，CMake 3.26+，C++17 标准。自定义绘制涉及 QPainter（QtGui）、QGraphicsDropShadowEffect（QtWidgets）。链接 Qt6::Widgets 即可。所有行为在 Qt 6.9.1 上验证通过。

## 3. 核心概念讲解

### 3.1 重写 paintEvent 绘制圆角边框与投影阴影

QFrame 的默认 paintEvent 会根据 frameShape 和 frameShadow 调用 QStyle::drawPrimitive 来绘制边框。如果你需要圆角、渐变背景、或者带模糊的阴影效果，就需要重写 paintEvent 用 QPainter 自己画。

绘制圆角边框的核心是 QPainterPath::addRoundedRect。你先构造一个圆角矩形路径，然后用 drawPath 或 fillPath 来绘制。投影阴影通过在背景矩形之前画一个偏移的、半透明的、更大的圆角矩形来模拟。

```cpp
class CardFrame : public QFrame
{
    Q_OBJECT

public:
    explicit CardFrame(QWidget *parent = nullptr)
        : QFrame(parent)
    {
        setAttribute(Qt::WA_StyledBackground);
        // 关闭默认边框绘制
        setFrameShape(QFrame::NoFrame);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        int radius = 12;
        int shadow_offset = 4;
        int shadow_blur = 8;

        QRect shadow_rect = rect().adjusted(
            shadow_offset, shadow_offset,
            -shadow_blur + shadow_offset,
            -shadow_blur + shadow_offset);

        // 绘制投影阴影（多层半透明模拟模糊）
        for (int i = shadow_blur; i > 0; --i) {
            int alpha = 30 - i * 3;
            if (alpha <= 0) continue;
            QColor shadow_color(0, 0, 0, alpha);
            painter.setPen(Qt::NoPen);
            painter.setBrush(shadow_color);
            QRect r = shadow_rect.adjusted(-i, -i, i, i);
            painter.drawRoundedRect(r, radius + i, radius + i);
        }

        // 绘制背景
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 255, 255));
        painter.drawRoundedRect(rect().adjusted(2, 2, -2, -2),
                                radius, radius);

        // 绘制边框
        QPen border_pen(QColor(220, 220, 220), 1);
        painter.setPen(border_pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(rect().adjusted(2, 2, -2, -2),
                                radius, radius);
    }
};
```

这段代码做了三层绘制：先画阴影（多层半透明圆角矩形由外到内叠加模拟模糊效果），再画白色背景填充，最后画灰色边框线。注意绘制顺序——先阴影再背景再边框，后画的覆盖先画的，层次关系才正确。

这里有一个性能考量：多层阴影循环绘制意味着每次 paintEvent 要画 shadow_blur 个圆角矩形。如果 shadow_blur 设得太大（比如 20），在一个包含多个 CardFrame 的界面上，重绘开销会明显增加。如果你的界面有频繁重绘的场景（比如 resize 动画、滚动），建议把阴影绘制缓存到 QPixmap 中——第一次绘制时创建 QPixmap 并绘制阴影，后续 paintEvent 直接 drawPixmap。

### 3.2 QGraphicsDropShadowEffect 的使用与性能陷阱

Qt 提供了 QGraphicsDropShadowEffect 作为一种更简单的方式来给 QWidget 添加阴影——不需要自己画，设置属性就行。

```cpp
auto *shadow = new QGraphicsDropShadowEffect;
shadow->setOffset(4, 4);
shadow->setBlurRadius(12);
shadow->setColor(QColor(0, 0, 0, 60));
cardFrame->setGraphicsEffect(shadow);
```

三行代码就能得到一个看起来不错的阴影效果。但 QGraphicsDropShadowEffect 有一个严重的性能问题：它会在每次 paint 时把整个 widget 渲染到一个临时 QPixmap，对 QPixmap 做高斯模糊，再把模糊后的图像叠加到 widget 下面作为阴影。这个过程涉及一次完整的 widget 渲染 + 一次图像模糊运算。对于简单的小控件这不算什么，但如果你给一个包含大量子控件的大面板加了 QGraphicsDropShadowEffect，或者给一个在 QScrollArea 中频繁重绘的控件加了阴影——每次滚动都会触发模糊运算，帧率会肉眼可见地下降。

更糟糕的是 QGraphicsDropShadowEffect 会破坏 QWidget 的裁剪优化。正常情况下 Qt 只重绘脏区域（dirty region），但 effect 需要完整的 widget 渲染结果来生成阴影，所以它会强制重绘整个 widget——即使只有一小块区域变了。

实际工程中的建议是：对于静态的、不频繁重绘的控件（比如工具栏背景、固定位置的面板），QGraphicsDropShadowEffect 是 OK 的。对于频繁重绘的控件（滚动列表中的项目、动画中的控件），用 paintEvent 手动画阴影或者用缓存的 QPixmap 阴影图。

### 3.3 QFrame 作为卡片容器的布局实践

现代 UI 中"卡片"布局非常常见——每个卡片是一个独立的视觉单元，有圆角边框、轻微阴影、内部内容。QFrame 天然适合做卡片容器——它可以有布局管理子控件，可以自定义绘制外观，可以作为 QWidget 的子类参与对象树管理。

用 QFrame 做卡片容器时的布局实践有几个要点。第一是内容区域的内边距——你不能让子控件紧贴着圆角边框，否则圆角区域的内容会被裁切。解决方法是在 QFrame 的内部布局中设置较大的 contentsMargins。

```cpp
auto *card = new CardFrame;
auto *layout = new QVBoxLayout(card);
layout->setContentsMargins(16, 16, 16, 16);  // 内边距
layout->setSpacing(8);

auto *title = new QLabel("项目名称");
title->setStyleSheet("font-size: 16px; font-weight: bold;");
layout->addWidget(title);

auto *desc = new QLabel("项目描述文字");
desc->setWordWrap(true);
layout->addWidget(desc);
```

第二是 QFrame 的 sizePolicy。卡片容器通常是 QSizePolicy::Preferred —— 它有理想大小但愿意被布局系统伸缩。如果你把卡片放在 QFlowLayout（Qt 没有内置的，需要自己实现或者用 QGridLayout 动态调整）中，需要确保卡片的 sizeHint 返回一个合理的值。

第三是 setAttribute(Qt::WA_StyledBackground, true)。这个属性告诉 Qt 这个 widget 的背景由 QSS 负责绘制。如果你重写了 paintEvent 完全自己画背景和边框，这个属性其实可以不设——但如果你同时在 QSS 中设了 background-color（作为 fallback），设上这个属性更安全。

第四是 clipChildren 的考虑。圆角边框意味着四个角是弧形的，但子控件不受圆角裁切——它们会延伸到圆角区域之外。如果你的卡片有纯色背景，这个问题看不出来（背景挡住了溢出）。但如果卡片有半透明背景或者边框线很细，子控件在圆角区域的溢出就会很明显。解决方法是在 paintEvent 中用 QPainter::setClipPath 设置圆角裁切路径，或者给内容 widget 也设一个带圆角的 QSS clip。

## 4. 踩坑预防

第一个坑是 QGraphicsDropShadowEffect 导致滚动卡顿。QGraphicsDropShadowEffect 在每次 paint 时都要做一次 widget 全渲染 + 高斯模糊。如果你的卡片在 QScrollArea 中，每次滚动都会触发重绘，模糊运算的开销会累积到可感知的卡顿。后果是滚动帧率从 60fps 掉到 30fps 甚至更低。解决方案是对滚动中的卡片用 paintEvent 手动画阴影（或者用缓存的 QPixmap），只在静态面板上用 QGraphicsDropShadowEffect。

第二个坑是圆角区域子控件溢出。QFrame 的圆角是通过 QPainter 画的——它只影响 QFrame 自身的绘制，不影响子控件的绘制区域。子控件仍然按照矩形区域渲染，在圆角处会超出边框线。后果是卡片看起来像有圆角，但文字或控件在四个角溢出了圆角线。解决方案是在 paintEvent 开头设置 clipPath，或者把内容放在一个带 stylesheet border-radius 的内部 QWidget 里。

第三个坑是手动阴影绘制的性能随 blur 半径线性增长。循环画多层半透明矩形模拟模糊的方案，blur 半径越大画的层数越多。shadow_blur = 8 要画 8 层，shadow_blur = 20 要画 20 层。后果是包含很多卡片的界面 resize 时明显卡顿。解决方案是限制 blur 半径在 8-12 之间，或者把阴影预渲染到 QPixmap 缓存。

## 5. 练习项目

练习项目：卡片式仪表盘面板。我们要做一个由多个自定义卡片组成的仪表盘界面。

完成标准是：主窗口使用 QGridLayout 放置 6 个 CardFrame 卡片（2 行 3 列）。每个卡片有 12px 圆角、轻微投影阴影（手绘，不用 QGraphicsDropShadowEffect）、16px 内边距。6 张卡片分别是："CPU 使用率"（显示一个 QProgressBar）、"内存"（显示百分比 QLabel）、"磁盘"（显示 QProgressBar）、"网络"（显示 QLabel 文字）、"进程"（显示 QListWidget 列表 5 项）、"日志"（显示 QTextEdit 5 行日志）。卡片之间有 12px 间距。窗口深色背景，卡片浅色背景，对比度明显。

提示几个关键点：QGridLayout 的 setSpacing 控制卡片间距；CardFrame 的 paintEvent 中先画阴影再画背景再画边框；把阴影预渲染到 QPixmap 缓存提升重绘性能——在 resizeEvent 中重建缓存。

## 6. 官方文档参考链接

[Qt 文档 · QFrame](https://doc.qt.io/qt-6/qframe.html) -- 基础边框容器，包含 setFrameShape/setFrameShadow 等接口

[Qt 文档 · QPainterPath](https://doc.qt.io/qt-6/qpainterpath.html) -- 绘图路径，addRoundedRect 用于绘制圆角矩形

[Qt 文档 · QGraphicsDropShadowEffect](https://doc.qt.io/qt-6/qgraphicsdropshadoweffect.html) -- 阴影效果，setBlurRadius/setOffset/setColor 控制阴影外观

[Qt 文档 · QPainter](https://doc.qt.io/qt-6/qpainter.html) -- 绘图引擎，setRenderHint(Antialiasing) 开启抗锯齿

---

到这里，QFrame 的进阶内容就拆完了。自定义绘制圆角边框和投影阴影的路径很清晰：重写 paintEvent，用 QPainterPath::addRoundedRect 画圆角，多层半透明矩形模拟模糊阴影。QGraphicsDropShadowEffect 方便但有性能陷阱，滚动场景别用它。QFrame 做卡片容器时注意内容内边距和圆角裁切。把这些搞透后，你就能做出精致的自定义面板——而不只是"画条分隔线"的基本用法。
