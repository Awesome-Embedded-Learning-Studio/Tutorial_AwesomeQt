# 现代Qt开发教程（新手篇）2.2——坐标系与 QTransform 变换基础

## 1. 前言 / 为什么需要坐标变换

说实话，刚开始学 QPainter 的时候，我以为只要会画矩形和圆就够了。直到有一天我需要画一个旋转 45 度的指针仪表盘，我盯着屏幕上那根斜着的线，脑子里只有一个想法：难道我要手算每个点的坐标？

你想想看，一个时钟的秒针，每秒钟旋转 6 度。如果纯用手算三角函数去算端点坐标，代码会变成什么样子？每画一个旋转图形就得写一堆 sin/cos，维护起来简直噩梦。更别提还要处理缩放和平移的组合了。

坐标变换就是为了解决这个问题而生的。你可以把坐标变换理解成"移动画布"——你不用重新计算每个点的位置，只需要告诉 QPainter"把坐标原点移到这儿"、"旋转 45 度"、"放大两倍"，然后像什么都没发生一样用原来的坐标画画就行。

更直白地说：坐标变换让你在"局部坐标系"里工作，不用管"全局坐标系"是什么样。你在 (0, 0) 画一个圆，通过变换，这个圆可以出现在屏幕的任何位置、任何角度、任何大小。这就是变换的威力。

这篇文章我们搞清楚三件事：基础的 translate/rotate/scale 怎么用、save/restore 怎么管理状态、viewport 和 window 坐标映射是什么意思。

## 2. 环境说明

本篇代码适用于 Qt 6.5+ 版本，CMake 3.26+，C++17 或更高标准。示例代码依赖 QtGui 和 QtWidgets 模块，和上一篇一样。坐标变换是 QPainter 的一部分，不需要额外的库。

## 3. 核心概念讲解

### 3.1 translate —— 平移坐标原点

`translate(dx, dy)` 的作用是：把坐标原点从当前位置移动 `(dx, dy)` 的距离。移动之后，所有绘图操作的坐标都基于新的原点。

```cpp
void paintEvent(QPaintEvent *) override
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 默认原点在左上角 (0, 0)
    painter.setPen(Qt::black);
    painter.drawRect(0, 0, 50, 50);  // 在左上角画一个 50x50 的矩形

    // 平移原点到 (100, 100)
    painter.translate(100, 100);

    // 现在原点在 (100, 100)，同样的代码画出来的矩形位置不同了
    painter.setPen(Qt::red);
    painter.drawRect(0, 0, 50, 50);  // 这个矩形在 (100, 100) 位置

    // 再次平移
    painter.translate(100, 50);  // 累加！现在原点在 (200, 150)
    painter.setPen(Qt::blue);
    painter.drawRect(0, 0, 50, 50);  // 这个矩形在 (200, 150) 位置
}
```

这里有个非常重要的概念：变换是累加的。每次 translate 都是在当前坐标系的基础上继续偏移，不是从初始位置重新开始。这就像你给人指路："往前走 100 米，再往右走 50 米"，而不是每次都从起点开始算。

translate 最常见的用途是在循环里画重复图形：

```cpp
// 在不同位置画 5 个一模一样的矩形
for (int i = 0; i < 5; ++i) {
    painter.drawRect(0, 0, 40, 40);
    painter.translate(60, 0);  // 每次往右移 60 像素
}
```

不用变换的话，你得手动算每个矩形的 x 坐标。用了变换，代码清爽多了。

### 3.2 rotate —— 旋转坐标系

`rotate(angle)` 的作用是：绕当前原点顺时针旋转 `angle` 度。注意，单位是度，不是弧度(挺好的，不用算pi什么的)

```cpp
void paintEvent(QPaintEvent *) override
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 先把原点移到窗口中央
    painter.translate(width() / 2.0, height() / 2.0);

    // 画一个十字标记中心点
    painter.setPen(Qt::gray);
    painter.drawLine(-20, 0, 20, 0);
    painter.drawLine(0, -20, 0, 20);

    // 绘制旋转的矩形 —— 每次旋转 30 度
    QPen pen(Qt::blue, 2);
    painter.setPen(pen);
    painter.setBrush(QBrush(QColor(100, 150, 255, 80)));

    for (int i = 0; i < 12; ++i) {
        painter.rotate(30);          // 每次旋转 30 度
        painter.drawRect(50, -10, 80, 20);  // 离原点 50 像素处画矩形
    }
}
```

rotate 有个特别容易踩的坑：旋转中心是当前坐标原点，不是你画的图形的中心。如果你想让一个矩形绕自己的中心旋转，你必须先把原点移到矩形中心，然后再旋转。

```cpp
// 想让一个矩形绕自己的中心旋转 45 度
// 矩形大小 100x60，中心在 (200, 150)

// 方法：先 translate 到矩形中心，再 rotate，再 translate 回去偏移一半宽高
painter.translate(200, 150);     // 把原点移到旋转中心
painter.rotate(45);              // 旋转 45 度
painter.translate(-50, -30);     // 偏移半个宽高，让矩形居中
painter.drawRect(0, 0, 100, 60);
```

记住这个口诀：先移到旋转中心，再旋转，再偏移回去。这个顺序不能乱，因为变换的叠加顺序和数学上的矩阵乘法一样，顺序不同结果完全不同。

### 3.3 scale —— 缩放坐标系

`scale(sx, sy)` 的作用是：对坐标轴进行缩放。`sx` 是 x 轴缩放比例，`sy` 是 y 轴缩放比例。

```cpp
// 放大两倍
painter.scale(2.0, 2.0);
painter.drawRect(10, 10, 50, 50);  // 实际显示为 20, 20, 100, 100

// 水平翻转（x 轴镜像）
painter.scale(-1, 1);

// 垂直翻转（y 轴镜像）
painter.scale(1, -1);
```

scale 有个不太直觉的副作用：它不仅缩放坐标，还缩放画笔宽度和字体大小。如果你 `scale(2, 2)`，原本 1 像素宽的线条会变成 2 像素，16 号字体会变成 32 号。有时候这是你要的效果，有时候不是。

如果你只想缩放图形而不缩放线条，可以在 scale 之前把画笔宽度设成期望宽度除以缩放比例：

```cpp
double s = 2.0;
painter.scale(s, s);
QPen pen(Qt::black, 1.0 / s);  // 这样缩放后线条仍然是 1 物理像素
painter.setPen(pen);
```

### 3.4 save() / restore() —— 保存恢复画笔状态

前面说了变换是累加的，但很多时候你需要"试一试某个变换，然后回到之前的状态"。这时候 `save()` 和 `restore()` 就派上用场了。

`save()` 会把当前 QPainter 的所有状态（画笔、画刷、变换矩阵、裁剪区域等）压入一个内部栈。`restore()` 从栈顶弹出并恢复之前保存的状态。

```cpp
void paintEvent(QPaintEvent *) override
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 初始状态：黑色画笔，原点在左上角
    painter.setPen(Qt::black);
    painter.drawText(10, 20, "原始状态");

    // ---- 第一段变换 ----
    painter.save();  // 保存当前状态
    painter.translate(100, 100);
    painter.rotate(45);
    painter.setPen(Qt::red);
    painter.setBrush(QBrush(QColor(255, 0, 0, 80)));
    painter.drawRect(-30, -30, 60, 60);
    painter.restore();  // 恢复到 save() 时的状态

    // ---- 此时回到初始状态 ----
    // 画笔又变回黑色，原点又回到左上角
    painter.drawText(10, 50, "恢复后仍然是黑色文字");

    // ---- 第二段变换 ----
    painter.save();
    painter.translate(300, 150);
    painter.scale(1.5, 1.5);
    painter.setPen(Qt::blue);
    painter.drawRect(0, 0, 60, 40);
    painter.restore();

    // ---- 又回到初始状态 ----
    painter.drawText(10, 80, "第二次恢复后还是黑色文字");
}
```

save/restore 可以嵌套使用，最多嵌套大约 32 层（具体取决于实现）。日常开发中 3-5 层嵌套已经很罕见了。养成一个好习惯：每次做变换之前先 save()，画完后立刻 restore()。这样你的代码不会因为变换的累加效应而变得难以理解。

### 3.5 视口（viewport）与窗口（window）坐标映射

这一节讲的是 QPainter 的"窗口-视口"映射机制，它听起来很学术，但实际用途很简单：让你用自己定义的坐标系来画图，而不是被迫使用像素坐标。viewport（视口）是 Widget 的物理矩形区域，单位是像素；window（窗口）是你自定义的逻辑矩形区域，单位是你自己定义的。两者之间建立映射关系后，QPainter 会自动把你的逻辑坐标转换为物理坐标。

```cpp
void paintEvent(QPaintEvent *) override
{
    QPainter painter(this);

    // 设置视口：使用整个 Widget 区域（物理像素）
    painter.setViewport(0, 0, width(), height());

    // 设置窗口：逻辑坐标范围 (-100, -100) 到 (100, 100)
    // 这样原点 (0, 0) 就在窗口正中央了！
    painter.setWindow(-100, -100, 200, 200);

    // 现在坐标系变成了：
    // x 范围 [-100, 100]，y 范围 [-100, 100]
    // (0, 0) 在窗口正中央
    // y 轴仍然是向下的！

    // 画一个以中心为原点的坐标系
    painter.setPen(Qt::black);
    painter.drawLine(-100, 0, 100, 0);   // x 轴
    painter.drawLine(0, -100, 0, 100);   // y 轴

    // 在第一象限画一个矩形（注意 y 轴向下）
    painter.setBrush(QBrush(QColor(100, 200, 100, 150)));
    painter.drawRect(10, -50, 40, 40);
}
```

window/viewport 最经典的用法是画数学图形。比如你想画一个函数图像，x 范围 [-pi, pi]，y 范围 [-1, 1]。你不用手动把数学坐标换算成像素坐标，直接设 window 就行：

```cpp
// 数学坐标系：x 从 -3.14 到 3.14，y 从 -1.5 到 1.5
painter.setWindow(-314, -150, 628, 300);

// 画 sin(x) 曲线
QPolygonF curve;
for (int i = -314; i <= 314; ++i) {
    double x = i / 100.0;
    double y = std::sin(x);
    curve << QPointF(i * 100, static_cast<int>(-y * 100));  // y 取反是因为屏幕 y 轴向下
}
painter.drawPolyline(curve);
```

说实话，window/viewport 这个功能在一般的应用开发中用得不算多，大部分时候 translate/rotate/scale 就够用了。但如果你要画地图、图表、数学图形这种需要特定坐标系的场景，这个功能能省你大量的坐标换算代码。

### 3.6 局部坐标系与全局坐标系

理解了变换之后，你需要建立两个概念：全局坐标系是 Widget 的原始坐标系，原点在左上角，单位是像素；局部坐标系是经过 translate/rotate/scale 变换后的坐标系。当你在局部坐标系里画 `(0, 0)` 的时候，这个点在全局坐标系里的位置取决于你做了什么变换。你可以用 `QPainter::transform()` 获取当前的变换矩阵，用 `QPainter::worldTransform().map(QPointF(x, y))` 把局部坐标转换为全局坐标。

```cpp
painter.translate(100, 50);
painter.rotate(30);

// 局部坐标系里的 (0, 0)，在全局坐标系里是哪？
QPointF globalPos = painter.worldTransform().map(QPointF(0, 0));
qDebug() << "全局坐标:" << globalPos;  // 大约 (100, 50)

// 局部坐标系里的 (50, 0)，经过 translate + rotate 后的全局坐标
QPointF globalPos2 = painter.worldTransform().map(QPointF(50, 0));
qDebug() << "全局坐标:" << globalPos2;
```

实际开发中，最常见的需求是"把全局坐标转换成局部坐标"——比如鼠标点击事件给你的是全局坐标，但你想知道它对应到画面的哪个位置。这时候需要用逆变换：

```cpp
void mousePressEvent(QMouseEvent *event) override
{
    QPainter painter(this);
    // ... 设置各种变换 ...
    // 把鼠标的窗口坐标转换到局部坐标
    QPointF localPos = painter.worldTransform().inverted().map(event->position());
    qDebug() << "局部坐标:" << localPos;
}
```

到这里你可以停下来想一想：translate、rotate、scale 分别改变了坐标系的什么？如果你想让一个矩形绕自身的中心旋转，代码的执行顺序应该是什么样的？搞清楚这些问题，后面写复杂绘图代码就不会手忙脚乱了。

## 4. 踩坑预防

坐标变换这块的坑还真不少，我们逐个来说。

第一个坑，也是最经典的：rotate 的旋转中心不是图形中心。很多人写了 `painter.rotate(45)` 再画矩形，发现矩形飞到屏幕外面去了，以为没画出来。实际上 rotate 是绕当前坐标原点旋转的，不是绕你画的图形的中心。如果你想绕图形自身中心旋转，必须先 translate 到图形中心，再 rotate，再偏移回去半个宽高。这个顺序不能乱——先做什么后做什么直接影响结果，因为变换叠加的顺序和矩阵乘法一样，顺序不同结果完全不同：

```cpp
// 错误：直接 rotate，图形绕原点旋转飞走了
painter.rotate(45);
painter.drawRect(100, 100, 80, 60);  // 不是绕矩形中心旋转！是绕原点旋转

// 正确：先 translate 到旋转中心，再 rotate，再偏移回去
painter.translate(140, 130);  // 移到矩形中心 (100+40, 100+30)
painter.rotate(45);
painter.translate(-40, -30);  // 偏移半个宽高
painter.drawRect(0, 0, 80, 60);
```

第二个坑是忘记 save/restore 导致变换累加。这个坑特别隐蔽，因为它不会立刻报错，而是让你的图形位置和角度变得完全不可预测。比如你在循环里不断 translate 和 rotate，但没有用 save/restore 重置状态，每循环一次，偏移量和旋转角度就累加一次。到最后你完全不知道原点跑哪去了。解决方法很简单：每次做变换前 save()，画完后 restore()，养成习惯就好：

```cpp
// 错误：循环里不断累加
for (int i = 0; i < 10; ++i) {
    painter.translate(50, 0);   // 每次累加 50 像素！
    painter.rotate(15);         // 每次累加 15 度！
    painter.drawRect(0, 0, 30, 30);
}
// 循环结束后，原点已经偏移了 500 像素，旋转了 150 度

// 正确：每次循环内 save/restore
for (int i = 0; i < 10; ++i) {
    painter.save();
    painter.translate(50 + i * 60, 100);
    painter.rotate(i * 15);
    painter.drawRect(-15, -15, 30, 30);
    painter.restore();
}
```

第三个坑是 scale 会影响画笔宽度和字体大小。`scale(3, 3)` 之后你会发现线条粗得离谱、文字大得吓人，整个画面一团糊。这是因为 scale 缩放的是整个坐标系，线宽和字号也跟着缩放了。如果你只想缩放图形本身而保持线宽不变，需要手动补偿：把画笔宽度设成期望值除以缩放比例，字体大小同理：

```cpp
double s = 3.0;
painter.scale(s, s);
QPen pen(Qt::black, 1.0 / s);  // 缩放后仍然是 1 像素
painter.setPen(pen);
QFont font("Arial", 16.0 / s);  // 缩放后仍然是 16 号
painter.setFont(font);
```

第四个坑是 setWindow 的参数理解错误。`setWindow(x, y, w, h)` 的后两个参数是宽度和高度，不是右下角坐标——这和 `drawRect` 的参数约定是一样的，但还是有很多人搞混。比如你想让坐标范围是 [-50, 50]，正确的写法是 `setWindow(-50, -50, 100, 100)`，因为宽度是 100。如果你写成 `setWindow(-50, -50, 50, 50)`，范围就变成了 [-50, 0]，画出来的图形位置完全不对或者只显示一部分。

接下来做一个代码填空练习：补全下面的代码，在窗口中央画一个旋转 30 度的矩形：

```cpp
void paintEvent(QPaintEvent *) override
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::________);  // 1. 抗锯齿提示

    // 先保存状态
    painter.________();  // 2. 保存当前状态

    // 移动原点到窗口中央
    painter.________(width() / 2.0, height() / 2.0);  // 3. 平移

    // 旋转 30 度
    painter.________(30);  // 4. 旋转

    // 偏移半个矩形大小，使矩形居中
    painter.translate(________, -25);  // 5. 矩形宽 100，偏移量是多少？

    painter.setPen(QPen(Qt::red, 2));
    painter.setBrush(QBrush(QColor(255, 100, 100, 150)));
    painter.drawRect(0, 0, 100, 50);

    // 恢复状态
    painter.________();  // 6. 恢复之前保存的状态
}
```

再来一道调试挑战：这段代码想画一个绕自身中心旋转的矩形，但运行后发现矩形飞到了窗口外面。问题出在哪里？

```cpp
void paintEvent(QPaintEvent *) override
{
    QPainter painter(this);
    painter.rotate(45);
    painter.translate(200, 150);
    painter.drawRect(-40, -30, 80, 60);
}
```

提示：想想变换的执行顺序和累加效果。

## 5. 练习项目

我们来做一个实战练习：创建一个模拟时钟 Widget，显示当前时间的时、分、秒针。秒针每秒更新一次，使用 QTimer 驱动。

完成标准是：自定义 `AnalogClockWidget` 继承 QWidget；使用 translate 把原点移到窗口中央；用 rotate 旋转不同角度来画时针、分针、秒针；用 QTimer 每秒触发 `update()` 刷新画面；画表盘包括圆形外框和 12 个刻度标记；时针短粗、分针中等、秒针细长，颜色区分明显；save/restore 正确使用，每次画指针前 save，画完后 restore。

几个提示：用 `QTime::currentTime()` 获取当前时间；秒针角度等于秒数乘以 6（每秒 6 度）；分针角度等于分钟加秒除以 60 再乘以 6；时针角度等于小时对 12 取模加分钟除以 60 再乘以 30；画指针时先 translate 到中心，再 rotate 对应角度，再画一条从原点向上的线（注意 y 轴向下，所以向上是负方向）。

## 6. 官方文档参考链接

[Qt 文档 · QPainter Coordinate System](https://doc.qt.io/qt-6/coordsys.html) -- Qt 坐标系统的完整说明，涵盖逻辑坐标、物理坐标、变换矩阵

[Qt 文档 · QTransform Class](https://doc.qt.io/qt-6/qtransform.html) -- 变换矩阵类的详细文档，支持平移、旋转、缩放、剪切等仿射变换

[Qt 文档 · QPainter::save/restore](https://doc.qt.io/qt-6/qpainter.html#save) -- 状态栈的保存与恢复机制说明

[Qt 文档 · Analog Clock Example](https://doc.qt.io/qt-6/qtwidgets-widgets-analogclock-example.html) -- Qt 官方的模拟时钟示例，非常好的坐标变换学习参考

---

到这里，坐标变换的基础你应该掌握了。核心就三件事：translate 移原点、rotate 绕原点转、scale 缩放坐标——加上 save/restore 管理状态。变换的顺序很重要，先做什么后做什么会直接影响结果。下一篇文章我们换一个话题，聊聊 Qt 里的图像处理：QImage、QPixmap 和 QIcon。
