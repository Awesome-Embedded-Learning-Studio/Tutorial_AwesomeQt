---
title: "3.35 QProgressBar 进阶"
description: "入门篇我们把 QProgressBar 的 setValue/setRange、跨线程信号槽、setFormat 自定义文字、无限进度模式这四个维度讲透了。进阶篇要把火力集中在那些日常开发中真正让人头疼的边缘场景。"
---

# 现代Qt开发教程（进阶篇）3.35——QProgressBar 进阶

## 1. 前言 / 进度条的边缘场景比你想的多

入门篇我们把 QProgressBar 的 setValue/setRange、跨线程信号槽、setFormat 自定义文字、无限进度模式这四个维度讲透了。正常使用确实没什么大问题了，但在工程实践中你会遇到一些更刁钻的场景：有些 style 下 busy 模式的动画根本不动，垂直方向的进度条宽度窄到看不清文字，小尺寸下百分比和色块重叠在一起根本没法看。

这篇文章我们要把三个进阶话题掰开揉碎：busy 模式（setMinimum(0) + setMaximum(0)）的动画驱动机制以及为什么在某些 style 下它不动的根本原因，垂直进度条的布局行为和自定义文字绘制方案，以及子类化 QProgressBar 重写 paintEvent 实现完全自定义文本的实战套路。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17 标准。QProgressBar 属于 QtWidgets 模块，paintEvent 重写部分涉及 QPainter（QtGui），不需要额外模块依赖。不同平台的 style 差异会在文中特别标注。

## 3. 核心概念讲解

### 3.1 Busy 模式的动画驱动机制

入门篇我们知道 setRange(0, 0) 会让进度条进入无限进度模式，显示一个来回滚动的动画。现在我们要搞清楚这个动画是谁驱动的、怎么驱动的、以及为什么有时候它不动。

QProgressBar 的 busy 动画不是由 QProgressBar 自己驱动的，而是由 QStyle 决定的。当 minimum == maximum == 0 时，QProgressBar::paintEvent 会把绘制委托给 QStyle::drawControl(CE_ProgressBarContents, ...)。具体的 QStyle 实现决定了 busy 动画怎么画——QFusionStyle 会画一个来回移动的色块，QWindowsStyle 和 QMacStyle 各有自己的动画实现方式。

关键问题在于：有些 QStyle 的 busy 动画是靠内部的 QProgressBar 自身定时器驱动的（通过 update() 触发重绘），而有些 style 需要外部不断调用 setValue() 来触发重绘才能让动画动起来。Qt 的官方文档对此有一句不起眼的话："If minimum and maximum both are set to 0, the bar shows a busy indicator instead of a percentage of steps." 但它没有告诉你的是——在某些 style 下，如果 value 一直是 0 不变，setValue(0) 不会触发 valueChanged 信号，也就不会触发重绘，动画就停在那里不动。

```cpp
auto *busy = new QProgressBar;
busy->setRange(0, 0);

// 在某些 style 下需要不断 setValue 才能动
QTimer *animDriver = new QTimer(this);
connect(animDriver, &QTimer::timeout, this, [busy]() {
    // 轮转一个内部计数器来驱动动画
    busy->setValue(busy->value() + 1);
});
animDriver->start(30);  // ~33fps
```

上面这段代码是一个跨 style 保险的做法。用一个 QTimer 以 30ms 的间隔不断调用 setValue 递增，强制触发重绘。虽然在大多数 style 下不需要这么做（Fusion style 会自己动），但在那些需要外部驱动的 style 下，不做这一步就会看到进度条死死地停住不动。

还有一个细节：busy 模式下 setValue 的值本身没有意义——它不会显示百分比，不会触发 setFormat 的 %p% 替换。这个值只是被当作一个"动画相位"来使用，QStyle 内部用它来计算动画块的当前位置。

### 3.2 垂直进度条的布局行为

QProgressBar 默认是水平方向的，通过 setOrientation(Qt::Vertical) 可以切换为垂直方向。入门篇提了一嘴，但没有展开垂直方向下那些让人头疼的布局问题。

第一个问题是宽度。水平进度条的默认 sizeHint 高度大约 20px 左右，看起来很自然。但垂直进度条的默认 sizeHint 宽度也是差不多这个值——而进度条上的文字需要足够的水平空间来显示。如果你用 setFormat("%p%") 显示百分比，"100%" 这个字符串在默认宽度下很可能被截断或者和色块重叠。

```cpp
auto *vbar = new QProgressBar;
vbar->setOrientation(Qt::Vertical);
vbar->setRange(0, 100);
vbar->setValue(75);

// 默认宽度太窄，文字显示不全
vbar->setFixedWidth(40);  // 加宽到 40px 文字才看得清
```

第二个问题是文字方向。垂直进度条的文字默认是旋转 90 度绘制的——文字沿着进度条的方向竖着排列。这在某些 style 下可读性很差，特别是中文字符旋转后几乎无法辨认。如果你需要更好的可读性，一种方案是关闭文本显示（setTextVisible(false)），然后在进度条旁边放一个独立的 QLabel 来显示当前值。

```cpp
auto *vbar = new QProgressBar;
vbar->setOrientation(Qt::Vertical);
vbar->setTextVisible(false);  // 关闭内置文字

auto *valueLabel = new QLabel;
connect(vbar, &QProgressBar::valueChanged, this,
        [valueLabel](int val) {
    valueLabel->setText(QString("%1%").arg(val));
});
```

第三个问题是 setInvertedAppearance。默认情况下垂直进度条从下往上填充，setInvertedAppearance(true) 可以让它从上往下填充。但 invertedAppearance 同时会影响文本的旋转方向——有些 style 在翻转外观后文字会变成倒的。这个行为是 style-dependent 的，没有统一的规范。

### 3.3 子类化重写 paintEvent 自定义文本

当你发现 setFormat 和 setAlignment 都无法满足你的显示需求时——比如你需要在进度条上方显示"正在下载第 3/10 个文件"这样的多段文字，或者你希望文字的字体大小和颜色随进度值动态变化——就需要子类化 QProgressBar 重写 paintEvent。

核心思路是：先调用父类的 paintEvent 让 QStyle 画好进度条的基础外观（背景、填充色块），然后在上面叠加你自己的文字绘制。

```cpp
class CustomProgressBar : public QProgressBar
{
    Q_OBJECT

protected:
    void paintEvent(QPaintEvent*) override
    {
        // 先让父类画好进度条外观
        QProgressBar::paintEvent(nullptr);

        // 在上面叠加自定义文字
        QPainter painter(this);
        painter.setPen(Qt::white);
        painter.setFont(font());

        // 获取填充区域的宽度来定位文字
        QRect rect = this->rect();
        QString text = QString("第 %1 / %2 步").arg(value()).arg(maximum());
        painter.drawText(rect, Qt::AlignCenter, text);
    }
};
```

这里有一个要点：调用 QProgressBar::paintEvent(nullptr) 时传 nullptr 是安全的，因为 QProgressBar 的 paintEvent 内部不使用 QPaintEvent 参数——它直接通过 QWidget::rect() 获取绘制区域。不过更规范的做法是把事件指针透传下去。

另一个需要注意的问题是文字颜色。如果进度条的填充色块和背景色对比度差异很大（比如深蓝色填充配浅灰色背景），居中的文字会同时横跨两个颜色区域——一半在深色上、一半在浅色上。无论你用什么文字颜色，总有一半看不清。解决方案有两种：要么只在填充区域内部画文字（通过计算填充区域的 QRect），要么给文字加一个半透明的背景衬底。

```cpp
void paintEvent(QPaintEvent* event) override
{
    QProgressBar::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 计算填充区域
    double ratio = static_cast<double>(value()) / maximum();
    int fillWidth = static_cast<int>(width() * ratio);
    QRect fillRect(0, 0, fillWidth, height());

    // 只在填充区域画白色文字
    painter.setPen(Qt::white);
    painter.setClipRect(fillRect);
    painter.drawText(rect(), Qt::AlignCenter, customText());
}
```

现在有一道调试题给大家。下面这段自定义进度条的代码有什么问题？

```cpp
void paintEvent(QPaintEvent*) override
{
    QPainter painter(this);
    painter.drawText(rect(), Qt::AlignCenter, "50%");
    QProgressBar::paintEvent(nullptr);
}
```

问题出在绘制顺序上。QPainter 先画了文字，然后父类 paintEvent 又把进度条的背景和色块画上去——文字被完全覆盖了。正确的顺序必须是先画父类的外观，再叠加文字。Qt 的绘制是画家算法，后画的覆盖先画的。

## 4. 踩坑预防

第一个坑是 busy 模式在某些 style 下不动画。根本原因是有些 QStyle 的 busy 动画需要外部不断触发重绘，而 setRange(0, 0) 后 value 不变就不会触发重绘。解决方案是用一个 QTimer 定期调用 setValue(value() + 1)，以大约 30ms 间隔强制刷新。这是跨 style 兼容性最保险的做法。

第二个坑是垂直进度条宽度太窄导致文字重叠。垂直进度条的默认 sizeHint 宽度只有约 20px，而进度文字需要足够空间。如果你使用 setTextVisible(true)，一定要同时 setFixedWidth 给出足够的宽度（建议至少 40px）；或者干脆关闭内置文字，在进度条旁边放独立的 QLabel 来显示数值。

第三个坑是小尺寸进度条文字与色块重叠。当进度条的高度很小（比如 12px 以下）且 setTextVisible(true) 时，文字和填充色块会挤在一起，在部分 style 下完全不可读。解决方案是在小尺寸时关闭文字显示（setTextVisible(false)），用独立的 QLabel 或者 tooltip 来显示进度值。如果一定要在进度条上显示文字，建议进度条高度至少 20px。

## 5. 练习项目

练习项目：多任务并行下载面板。我们要实现一个面板，界面左侧是任务列表，右侧是每个下载任务的进度条。每个任务的进度条是一个自定义的 CustomProgressBar：上方显示 "正在下载: filename.zip"，中间是进度色块，色块内部显示百分比文字（白色），背景区域不显示文字。支持水平模式和垂直模式切换——切换到垂直模式时文字改为在进度条旁边的 QLabel 显示。

完成标准是：水平模式下文字在色块内部清晰可读，垂直模式下宽度合理、文字不重叠，busy 模式动画在 Fusion 和 Windows style 下都能正常滚动。提示几个关键点：重写 paintEvent 时先调用父类再叠加文字，用 clipRect 限制文字只在填充区域内绘制，垂直模式的宽度用 setFixedWidth 控制，busy 模式的动画用 QTimer 驱动。

## 6. 官方文档参考链接

[Qt 文档 · QProgressBar](https://doc.qt.io/qt-6/qprogressbar.html) -- 进度条控件，包含 orientation、invertedAppearance 等属性说明

[Qt 文档 · QStyle](https://doc.qt.io/qt-6/qstyle.html) -- Qt 样式系统基类，理解 busy 动画的 style 层实现

[Qt 文档 · QPainter](https://doc.qt.io/qt-6/qpainter.html) -- 绘图引擎，重写 paintEvent 时绑定使用

---

到这里，QProgressBar 的进阶内容就过了一遍。busy 模式的动画不是 QProgressBar 自己驱动的，而是委托给 QStyle，所以在某些 style 下需要外部 QTimer 来强制刷新。垂直进度条的默认宽度对文字显示极度不友好，要么加宽要么把文字外置到 QLabel。子类化重写 paintEvent 是实现完全自定义文本的终极方案，但要注意绘制顺序——先画父类外观再叠加文字，画家算法后画的覆盖先画的。把这些搞清楚，进度条在任何场景下都能按你的意思来渲染了。
