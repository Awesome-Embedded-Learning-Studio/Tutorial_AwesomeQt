# 现代Qt开发教程（新手篇）2.1——QPainter 绘图基础

## 1. 前言 / 为什么需要 QPainter

Qt是一个GUI框架，对吧。如果您是从一个标准路径（我是指，对着Console那个默认的控制台程序走过来的朋友），咱们以前学 C++ 的控制台程序，输出文字就 `printf` 搞定了。但到了 GUI 的世界里，想画一个矩形、画一条直线，居然要专门学一整套绘图 API吗？又寸！

后来做项目的时候才发现，QPainter 是 Qt 里最核心的绘图工具（之前撸桌面CCIMXDesktop写控件简直天天见），几乎所有的自定义 UI 都离不开它。你看到的按钮、进度条、图表——底层全都是 QPainter 一笔一笔画出来的。当你需要做一个控件样式不满足需求的自定义界面，或者想画一个数据可视化图表，QPainter 就是你唯一的出路。（有人问QML还有QSS呢？额，底层是这个对不对）

更现实一点说，如果你要做嵌入式设备上的界面，很多现成的控件根本用不了，几乎全靠 QPainter 手绘（QSS也行，但是笔者发现但凡动态一点，QSS是真的不好用，还要花费时间parse字符串，其实不算特别好的方案）。所以这门手艺，早点练比晚点练好。

这篇文章我们一起来搞清楚：怎么在 `paintEvent` 里正确拿到 QPainter、怎么设置画笔和画刷、怎么画各种图形、坐标系的坑在哪里。这些都是 QPainter 的基本功，搞明白了后面学坐标系变换、双缓冲绘图才有基础。

## 2. 环境说明

本篇代码适用于 Qt 6.5+ 版本，CMake 3.26+，C++17 或更高标准。示例代码依赖 QtGui 和 QtWidgets 模块（QPainter 在 QtGui 里，但 paintEvent 需要 QWidget，所以两个都要），可以在任何支持 Qt6 的桌面平台上编译运行。

## 3. 核心概念讲解

### 3.1 paintEvent 中获取 QPainter 的正确姿势

QPainter 不能凭空使用，它必须绑定一个"画布"——在 Qt 里这个画布通常就是 QWidget。你要在一个 Widget 上画东西，唯一正确的入口就是重写 `paintEvent(QPaintEvent *)` 函数。

```cpp
class MyWidget : public QWidget
{
    Q_OBJECT

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QPainter painter(this);  // 以 this（当前 Widget）为画布
        // 接下来所有的绘图操作都通过 painter 来完成
        painter.drawLine(0, 0, 100, 100);
    }
};
```

这里有个非常重要的细节：QPainter 的构造函数传入 `this`，意思是"我要在这个 Widget 上画画"。QPainter 构造完成的那一刻，它就拿到了 Widget 的绘制上下文，可以开始绑定了。

你可能在网上看到过另一种写法：先构造 QPainter，再调 `begin()`，画完再调 `end()`。这种写法完全可行，但对于 `paintEvent` 来说没必要。直接用构造函数传入 `this` 是最简洁也最不容易出错的方式。（begin和end可以更精确的控制，但是大哥，玩意你忘记释放了。。。我只能说自求多福，没必要给自己上风险。）

```cpp
// 不推荐：手动 begin/end，容易忘记
void paintEvent(QPaintEvent *) override
{
    QPainter painter;
    painter.begin(this);
    painter.drawLine(0, 0, 100, 100);
    painter.end();  // 忘了就等着出 bug 吧
}

// 推荐：RAII 风格，构造即开始，析构即结束
void paintEvent(QPaintEvent *) override
{
    QPainter painter(this);
    painter.drawLine(0, 0, 100, 100);
    // 函数结束，painter 析构，自动 end
}
```

还有一个关键点：不要在 `paintEvent` 之外创建 QPainter 绑定到 Widget 上。Qt 的绘图系统是基于事件的，只有在 `paintEvent` 被调用时，系统才准备好了一切绘制资源。在别的地方画画，要么直接崩溃，要么画出来被下一次 paintEvent 覆盖掉。

当你需要主动触发重绘时，不要直接调用 `paintEvent()`，而是调用 `update()`。`update()` 会在下一个事件循环周期统一处理绘制请求，避免频繁重绘带来的性能问题。

```cpp
// 想更新画面？这样触发
void changeColor()
{
    m_color = Qt::red;
    update();  // 安排一次 paintEvent，不要自己调用 paintEvent()！
}
```

### 3.2 QPen（线条）与 QBrush（填充）与 QColor

画图之前，你得先搞清楚两个概念：画笔（QPen）和画刷（QBrush）。画笔决定线条的样子——颜色、宽度、线型（实线、虚线、点线）。画刷决定填充的样子——纯色、渐变色、图案填充。一句话区分：画笔管边框，画刷管内部。

```cpp
QPainter painter(this);

// 设置画笔：红色、3 像素宽、实线
QPen pen(Qt::red, 3, Qt::SolidLine);
painter.setPen(pen);

// 设置画刷：蓝色填充
QBrush brush(Qt::blue);
painter.setBrush(brush);

// 画一个有边框有填充的矩形
painter.drawRect(10, 10, 200, 100);
```

QColor 是颜色的基础类。你可以用预定义的颜色常量（`Qt::red`、`Qt::blue`），也可以用 RGB/RGBA 值创建任意颜色。

```cpp
// 各种创建 QColor 的方式
QColor color1(Qt::red);                    // 预定义颜色
QColor color2(255, 128, 0);                // RGB
QColor color3(255, 128, 0, 200);           // RGBA（带透明度）
QColor color4("#FF8000");                   // 十六进制字符串
QColor color5("rgba(255, 128, 0, 200)");   // CSS 风格字符串

// 判断颜色是否有效（构造失败时 isValid() 返回 false）
QColor bad("not_a_color");
if (!bad.isValid()) {
    qDebug() << "这个颜色不合法";
}
```

QPen 有几个常用属性你需要知道：

```cpp
QPen pen;
pen.setColor(Qt::darkBlue);        // 颜色
pen.setWidth(2);                   // 宽度（像素）
pen.setStyle(Qt::DashLine);        // 线型：实线、虚线、点线等
pen.setCapStyle(Qt::RoundCap);     // 线段端点样式：平头、圆头、方头
pen.setJoinStyle(Qt::RoundJoin);   // 拐角样式：尖角、圆角、斜角
```

QBrush 也有几种填充模式：

```cpp
// 纯色填充
QBrush solidBrush(Qt::blue);

// 渐变填充（线性渐变）
QLinearGradient gradient(0, 0, 200, 0);  // 从左到右
gradient.setColorAt(0.0, Qt::white);     // 起始颜色
gradient.setColorAt(1.0, Qt::blue);      // 结束颜色
QBrush gradientBrush(gradient);

// 图案填充
QBrush patternBrush(Qt::DiagCrossPattern);  // 交叉斜线图案
```

说实话，日常开发中 80% 的情况你只需要纯色画笔和纯色画刷。渐变和图案属于锦上添花，用到的时候查文档就行。

### 3.3 绘制基本图形

QPainter 提供了一整套绘图函数，名字都很好记：

```cpp
QPainter painter(this);

// ---- 直线 ----
painter.drawLine(10, 10, 200, 200);          // 两点画线
painter.drawLine(QPoint(10, 10), QPoint(200, 200));  // QPoint 版本

// ---- 矩形 ----
painter.drawRect(50, 50, 150, 100);          // x, y, width, height
painter.drawRect(QRect(50, 50, 150, 100));   // QRect 版本

// ---- 填充矩形（无边框） ----
painter.fillRect(50, 200, 150, 100, Qt::green);

// ---- 椭圆 ----
painter.drawEllipse(100, 100, 120, 80);      // 外接矩形定义椭圆
// 特殊情况：画正圆，宽高相等
painter.drawEllipse(300, 100, 80, 80);

// ---- 圆角矩形 ----
painter.drawRoundedRect(50, 350, 200, 100, 15, 15);  // 最后两个是圆角半径

// ---- 多边形 ----
QPolygon polygon;
polygon << QPoint(200, 400) << QPoint(250, 350)
        << QPoint(300, 400) << QPoint(275, 450)
        << QPoint(225, 450);
painter.drawPolygon(polygon);

// ---- 文字 ----
painter.setFont(QFont("Arial", 16, QFont::Bold));
painter.drawText(50, 500, "Hello QPainter!");
```

这里有个容易搞混的地方：`drawRect(x, y, w, h)` 中的 `x, y` 是矩形左上角的坐标，`w, h` 是宽和高——不是右下角坐标。初学者经常把 `w, h` 当成第二个点的坐标，画出来的东西位置完全不对。

`drawEllipse` 同理，参数不是圆心坐标和半径，而是外接矩形的左上角和宽高。想以 `(cx, cy)` 为圆心画半径为 `r` 的圆，要这样写：

```cpp
int cx = 200, cy = 200, r = 50;
painter.drawEllipse(cx - r, cy - r, 2 * r, 2 * r);
```

`drawText` 的坐标 `(x, y)` 是文字基线的左端位置，不是文字矩形的左上角。这意味着如果你把 y 设成 0，文字的大半部分会画到 Widget 上方看不见。初学者很容易在这里栽跟头。

### 3.4 坐标系原点与单位

QPainter 的坐标系默认是这样的：原点 (0, 0) 在 Widget 的左上角，x 轴向右递增，y 轴向下递增——这跟数学里的笛卡尔坐标系不一样，很多新手在这里会很别扭。这意味着 `painter.drawLine(0, 0, 100, 0)` 是从左上角往右画一条水平线。

关于单位：QPainter 默认使用的是逻辑坐标，单位是像素。但这里有个坑——在高 DPI 屏幕上，一个逻辑像素可能对应 2 个甚至 3 个物理像素。Qt6 默认开启了高 DPI 缩放，所以你设置的 100 像素宽度，在 2x 缩放的屏幕上实际会占据 200 个物理像素。

```cpp
// 获取当前 Widget 的实际像素尺寸
int logicalWidth = width();    // 逻辑尺寸
int logicalHeight = height();

// 如果需要获取物理像素尺寸（高 DPI）
qreal dpr = devicePixelRatioF();  // 设备像素比，2x 屏幕返回 2.0
int physicalWidth = logicalWidth * dpr;
```

日常绘图你不需要关心物理像素，Qt 会自动处理缩放。但如果你要做像素级的精确操作（比如截图或者图像处理），这个区别就会变得很重要。

还有一个常见的困惑：`width()` 和 `rect().width()` 返回的值一样吗？大多数情况下是一样的，都是 Widget 的逻辑宽度。但 `contentsRect()` 返回的是去除边距后的可用区域——如果你给 Widget 设了 stylesheet 的 padding 或者设置了 margin，两者就不一样了。

到这里你可以停下来想一想：QPen 和 QBrush 各自负责什么？如果你要画一个红色边框、蓝色填充的圆角矩形，代码逻辑应该是怎样的？理解了这两个概念的区别，后面使用起来就不会搞混了。

很好，概念讲完了，我们来看看实际开发中最容易踩的坑。

## 4. 踩坑预防

第一个坑，也是最常见的一个：在 paintEvent 外面创建 QPainter。我看到过不少人在按钮点击的槽函数里直接 `QPainter painter(this)`，想着"我要在这里画个东西"。这几乎一定会出问题。Qt 的绘图系统是基于事件的，系统只在 `paintEvent` 被调用时才会准备好绘制所需的资源。在别的地方创建 QPainter 绑定到 Widget，要么直接崩溃，要么画出来的东西瞬间被下一次 paintEvent 覆盖。正确的做法是设置一个标志位，然后调 `update()` 让 Qt 在合适的时机帮你重绘：

```cpp
// 错误：在 paintEvent 外面画画
void MyWidget::onButtonClicked()
{
    QPainter painter(this);  // 危险！不在 paintEvent 里
    painter.drawRect(0, 0, 50, 50);
}

// 正确：通过标志位 + update() 触发重绘
void MyWidget::onButtonClicked()
{
    m_drawRect = true;
    update();  // 触发 paintEvent
}

void MyWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if (m_drawRect) {
        painter.drawRect(0, 0, 50, 50);
    }
}
```

第二个坑跟第一个坑是配套的：直接调用 `paintEvent()`。有人知道不该在外面创建 QPainter，但又想立即刷新画面，就干脆直接 `paintEvent(nullptr)`。这也不行。直接调用 paintEvent 会跳过 Qt 的绘制流程管理，可能导致绘制不完整、闪烁、或者和系统的绘制事件冲突。想重绘就老老实实 `update()`，让它自己安排时机。

第三个坑是 drawText 的坐标问题。`drawText(x, y, text)` 里的 y 是文字基线位置，不是文字矩形的顶部。所以如果你写 `drawText(0, 0, "Hello")`，文字大部分内容会跑到 Widget 上方被裁剪掉，看起来就像没画出来一样。解决这个问题有两种方式：要么用 `QFontMetrics` 算出 ascent 然后加上去，要么用 QRect 版本的 drawText 配合对齐标志，后者更推荐：

```cpp
// 方法 1：手动计算基线位置
QFontMetrics fm(painter.font());
int textY = fm.ascent();  // 基线到顶部的距离
painter.drawText(0, textY, "Hello");

// 方法 2（推荐）：用 QRect 指定绘制区域 + 对齐标志
painter.drawText(rect(), Qt::AlignTop | Qt::AlignLeft, "Hello");
```

第四个坑是画笔和画刷搞混。想画红色边框的矩形，结果只设了 QBrush 没设 QPen，画出来的矩形内部是红色、边框却是默认的黑色 1 像素线。这不是 bug，是因为 QPen 和 QBrush 是完全独立的——QPen 管线条和边框，QBrush 管内部填充。你需要分别设置才能得到想要的效果：

```cpp
QPen pen(Qt::red, 2);        // 红色边框，2px 宽
QBrush brush(Qt::lightGray); // 浅灰填充
painter.setPen(pen);
painter.setBrush(brush);
painter.drawRect(10, 10, 100, 80);
```

接下来做一个代码填空练习：补全下面的代码，实现一个画红框蓝底矩形的自定义 Widget：

```cpp
class RectWidget : public ________  // 1. 应该继承什么？
{
    Q_OBJECT

protected:
    void paintEvent(________) override  // 2. 参数类型是什么？
    {
        ________ painter(this);  // 3. 创建 QPainter

        QPen pen(________, 2);  // 4. 红色画笔
        painter.________(pen);  // 5. 设置画笔

        QBrush brush(________);  // 6. 蓝色画刷
        painter.________(brush);  // 7. 设置画刷

        painter.drawRect(________, 50, 200, 100);  // 8. 从 (20, 50) 开始画
    }
};
```

再来一道调试挑战：这段代码有什么问题？为什么画面上什么都看不到？

```cpp
class MyWidget : public QWidget
{
    Q_OBJECT

public:
    void drawSomething()
    {
        QPainter painter(this);
        painter.setPen(Qt::red);
        painter.drawLine(0, 0, 300, 300);
    }
};
```

提示：想想 QPainter 和 paintEvent 的关系。

## 5. 练习项目

我们来做一个实战练习：创建一个自定义 Widget，接收一组数据（比如 `{85, 60, 95, 40, 75}`），用 QPainter 画出一个简易柱状图。每根柱子高度按数据值的比例计算，柱子底部有标签，顶部显示数值。

完成标准是：自定义一个 `BarChartWidget` 继承自 QWidget，重写 `paintEvent` 根据数据数组画柱状图；柱子之间有间距，颜色不同（可以用渐变）；底部显示标签（"A"、"B"、"C"...），顶部显示数值；提供一个 `setData(const QList<int> &data)` 方法，调用后自动刷新；窗口大小改变时柱状图自适应缩放（利用 `width()` 和 `height()`）。

几个提示：在 paintEvent 里用 `width()` 和 `height()` 获取当前 Widget 尺寸，据此计算柱子宽度和最大高度；留出底部和顶部的边距给标签和数值，用 `QFontMetrics` 计算文字尺寸；画柱子用 `drawRect`，填颜色用 `QLinearGradient` 做渐变效果会更专业；`setData()` 里存好数据后调 `update()` 触发重绘。

## 6. 官方文档参考链接

[Qt 文档 · QPainter Class](https://doc.qt.io/qt-6/qpainter.html) -- QPainter 的完整 API 参考，包含所有绘图函数和坐标变换方法

[Qt 文档 · QPen Class](https://doc.qt.io/qt-6/qpen.html) -- 画笔类的详细文档，涵盖线型、端点样式、连接样式等所有属性

[Qt 文档 · QBrush Class](https://doc.qt.io/qt-6/qbrush.html) -- 画刷类的详细文档，包含纯色、渐变、纹理等填充模式

[Qt 文档 · QColor Class](https://doc.qt.io/qt-6/qcolor.html) -- 颜色类的完整文档，RGB/HSV/CMYK 等颜色空间支持

[Qt 文档 · Paint System Overview](https://doc.qt.io/qt-6/paintsystem.html) -- Qt 绘图系统的总览文档，帮助理解 QPainter 在整个绘图架构中的位置

---

到这里，QPainter 的基本功你算是入门了。记住核心要点：只在 paintEvent 里创建 QPainter、QPen 管线条 QBrush 管填充、坐标系 y 轴朝下。下一篇文章我们会进入坐标变换的世界——学会 translate、rotate、scale 之后，你就能画出各种旋转、缩放的复杂图形了。
