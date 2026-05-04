# 实战练习 · ToggleSwitch — 把 paintEvent 和动画玩到一起

## 前言：开始上难度了

前面我们做的 StatusLed 用了 paintEvent 自绘，但没有动画——状态切换是瞬间完成的。SearchEdit 用了组合而不是自绘。这篇练习我们要把两者结合起来：用 paintEvent 画一个漂亮的开关，再用 QPropertyAnimation 让滑块在两个状态之间平滑滑动。这是你在 Qt 里做"有质感的自定义控件"的入门关卡——一旦掌握了 paintEvent + QPropertyAnimation 的组合技，进度条、旋钮、仪表盘这些控件都不在话下。

iOS 的 toggle switch 看起来很简单——一个圆角矩形轨道里面有个圆形滑块，点击一下滑块从左滑到右，颜色从灰变绿。但就是这么个小东西，背后涉及的 Qt 知识点不少：自绘圆角矩形、自绘圆形、Q_PROPERTY 属性系统、QPropertyAnimation 动画驱动、鼠标事件处理。所以我们这篇的节奏会比之前稍微紧凑一些，但一步一步来，不会有断层。

---

## 出发前的装备清单

- **QPainter** — 自绘引擎。你应该在 StatusLed 那篇已经用过了。这次我们要画圆角矩形（drawRoundedRect）和圆形（drawEllipse），以及配合抗锯齿。
- **QPropertyAnimation** — Qt 属性动画框架，可以在一段时间内自动插值某个 Q_PROPERTY 属性。比如把一个 double 从 0.0 渐变到 1.0，持续 200ms。
- **Q_PROPERTY** — Qt 的属性系统宏，声明一个可以通过元对象系统访问的属性。QPropertyAnimation 需要通过 Q_PROPERTY 来驱动动画。
- **鼠标事件** — mousePressEvent、mouseReleaseEvent、mouseMoveEvent，用来处理点击和拖拽。

---

## 我们的目标长什么样

```
    关闭状态:                      开启状态:
  ┌──────────────────┐          ┌──────────────────┐
  │                  │          │                  │
  │    ○             │          │             ●    │
  │                  │          │                  │
  └──────────────────┘          └──────────────────┘
     灰色背景                       绿色背景
     滑块在左侧                     滑块在右侧

尺寸约 48×26，轨道是圆角矩形（圆角半径 = 高度/2，所以两端是半圆），
滑块是白色圆形，点击后动画滑动约 200ms。
```

完成标准：点击开关后滑块平滑滑动到另一侧，背景色同步渐变。动画过程中不会卡顿。支持 setChecked()/isChecked()/toggled(bool) 信号。

---

## 第一步 — 类结构与 Q_PROPERTY

### 思考题

QPropertyAnimation 需要一个 Q_PROPERTY 来驱动。我们的动画目标是"滑块从左滑到右"，也就是滑块位置从 0.0 变到 1.0（归一化坐标）。所以我们需要声明一个 double 类型的 Q_PROPERTY，比如叫 `handlePosition`。问题是：为什么不直接用 bool 类型的 `checked` 属性做动画？提示：bool 只有 true 和 false 两个值，没有中间状态，动画插值没有意义。

### 动手写

```cpp
#pragma once

#include <QPropertyAnimation>
#include <QWidget>

class ToggleSwitch : public QWidget
{
    Q_OBJECT
    // TODO: 声明两个 Q_PROPERTY
    //       Q_PROPERTY(bool checked READ isChecked WRITE setChecked NOTIFY toggled)
    //       Q_PROPERTY(double handlePosition READ handlePosition
    //                  WRITE setHandlePosition NOTIFY handlePositionChanged)
    //       第一个给外部 API 用，第二个给动画引擎用。

public:
    explicit ToggleSwitch(QWidget *parent = nullptr);

    bool isChecked() const;
    void setChecked(bool checked);

    QSize sizeHint() const override;
    // 建议返回 QSize(48, 26) 左右的尺寸

    void toggle();

signals:
    void toggled(bool checked);
    void handlePositionChanged(double position);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

    // 可选：拖拽支持
    // void mouseMoveEvent(QMouseEvent *event) override;
    // void mouseReleaseEvent(QMouseEvent *event) override;

private:
    // Q_PROPERTY 的 READ/WRITE 函数
    double handlePosition() const;
    void setHandlePosition(double pos);

    bool m_checked = false;
    double m_handlePosition = 0.0;  // 0.0 = 左边, 1.0 = 右边

    // TODO: 动画对象。每次创建新的？还是复用一个？
    //       建议用成员变量持有一个 QPropertyAnimation。
};
```

关于 Q_PROPERTY 宏的参数：READ 指定读取方法名，WRITE 指定写入方法名，NOTIFY 指定属性变化时发射的信号。QPropertyAnimation 会通过元对象系统找到这些方法来读写属性值。这就是为什么我们必须用 Q_PROPERTY——没有它，动画引擎不知道该怎么操作你的属性。

### 你可能会遇到的坑

Q_PROPERTY 的 WRITE 方法里**必须**调用 update() 来触发重绘。因为属性值变了并不意味着界面会自动更新——你得告诉 Qt "我这个控件需要重绘了"。另外，NOTIFY 信号必须在你改变属性值之后 emit 出去，不然 QPropertyAnimation 不知道动画进度。

### 检查点

编译确认类声明没有语法错误。Q_PROPERTY 的声明格式很严格，少一个逗号都会报一堆莫名其妙的错误——仔细检查。

---

## 第二步 — 画轨道：圆角矩形

### 思考题

圆角矩形的关键是圆角半径。如果轨道的高度是 26px，圆角半径设成多少能让两端变成完美的半圆？提示：半径 = 高度 / 2。

### 动手写

```cpp
void ToggleSwitch::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 不加这个圆角会有锯齿

    // --- 画轨道 ---
    // TODO: 根据宽度/高度计算轨道的 QRect
    //       轨道占满整个 widget 区域
    //       qreal radius = height() / 2.0;

    // TODO: 选择轨道颜色
    //       checked ? 绿色 : 灰色
    //       你可以用 QColor(76, 175, 80) 作为绿色（Material Design 的绿色）
    //       QColor(189, 189, 189) 作为灰色

    // TODO: 用 painter.setBrush() 设置颜色
    //       用 painter.setPen(Qt::NoPen) 去掉边框线
    //       用 painter.drawRoundedRect(rect, radius, radius) 画圆角矩形
}
```

drawRoundedRect 的后两个参数是 x 方向和 y 方向的圆角半径。当这两个值等于矩形高度的一半时，矩形两端就变成了半圆——正是我们想要的效果。

抗锯齿（Antialiasing）必须开。如果你看到圆角边缘有锯齿状的阶梯，说明忘记设 `setRenderHint(QPainter::Antialiasing)`。对于圆形和圆角矩形，这个设置几乎是强制的。

### 检查点

在 main.cpp 里创建 ToggleSwitch 并 show。你应该能看到一个灰色的圆角矩形。如果只有方形，检查 drawRoundedRect 的半径参数是否正确。

---

## 第三步 — 画滑块：位置由属性驱动

### 思考题

滑块是一个白色圆形。它的圆心 x 坐标由 `m_handlePosition`（0.0 到 1.0）决定。当 position = 0.0 时圆心在最左边，position = 1.0 时在最右边。你需要计算具体的像素坐标。想一想：圆心不能贴着轨道边缘——应该留一些内边距。圆的半径应该比轨道高度的一半小多少？

### 动手写

继续在 paintEvent 里，画完轨道之后画滑块：

```cpp
    // --- 画滑块 ---
    // TODO: 计算滑块半径
    //       qreal handleRadius = height() / 2.0 - padding;
    //       padding 大约 2-3 像素，给滑块和轨道之间留点间隙

    // TODO: 计算滑块圆心的 x 坐标
    //       当 position = 0.0 时在最左边，1.0 时在最右边
    //       左边界 = handleRadius + padding
    //       右边界 = width() - handleRadius - padding
    //       centerX = leftBound + position * (rightBound - leftBound)
    //       这是线性插值的基本公式。

    // TODO: centerY = height() / 2.0

    // TODO: 画白色圆形
    //       painter.setBrush(QColor(255, 255, 255));
    //       painter.drawEllipse(QPointF(centerX, centerY), handleRadius, handleRadius);
```

画滑块的关键就是那个线性插值公式：`leftBound + position * (rightBound - leftBound)`。当 position 从 0.0 变到 1.0 时，centerX 从 leftBound 平滑移动到 rightBound。而 position 的变化是由 QPropertyAnimation 驱动的——它会在 200ms 内把 position 从当前值插值到目标值，每一帧自动调用 setHandlePosition()，我们只需要在 setHandlePosition 里 update() 一下就行。

### 检查点

手动调用 `setHandlePosition(1.0)` 和 `setHandlePosition(0.0)`，看看滑块是否正确移动到右侧和左侧。如果滑块位置不对，检查一下插值公式的计算。

---

## 第四步 — 点击切换：启动动画

### 思考题

QPropertyAnimation 的使用模式是：创建动画对象 → 设目标属性名 → 设起止值 → 设时长 → start()。问题是：我们每次点击都要 new 一个 QPropertyAnimation 吗？还是复用同一个？如果复用，连续快速点击会不会出问题？提示：QPropertyAnimation::start() 会自动停止之前正在运行的同一动画对象。所以复用一个对象是安全的。

### 动手写

```cpp
void ToggleSwitch::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    // 翻转状态
    m_checked = !m_checked;
    emit toggled(m_checked);

    // 创建或复用动画
    // TODO: 如果没有现成的动画对象，创建一个
    //       m_animation = new QPropertyAnimation(this, "handlePosition");
    //       m_animation->setDuration(200);  // 200ms
    //       m_animation->setEasingCurve(QEasingCurve::OutCubic);  // 先快后慢，更自然

    // TODO: 设定起止值
    //       m_animation->setStartValue(m_checked ? 0.0 : 1.0);
    //       m_animation->setEndValue(m_checked ? 1.0 : 0.0);

    // TODO: start() 启动动画
    //       动画引擎会自动调用 setHandlePosition() 更新位置
    //       每次 setHandlePosition 被调用时我们 update() 触发重绘
    //       所以视觉上就是滑块在平滑滑动
}
```

关于 QEasingCurve——它是动画的缓动函数，控制值的变化速率。"线性"（Linear）就是匀速，"OutCubic"是先快后慢，"InOutQuad"是两头慢中间快。对于开关这种小动画，OutCubic 或 OutQuad 看起来最自然。你可以多试几种，感受一下区别。

### 你可能会遇到的坑

QPropertyAnimation 的目标对象（this）和属性名（"handlePosition"）必须在动画的整个生命周期内有效。如果你在构造函数里 `new QPropertyAnimation(this, "handlePosition")` 并且 parent 设为 this，那一切正常。但如果你忘记设 parent 或者目标对象提前销毁了，运行时你会收到一个"cannot find property"的警告，动画不工作。

### 检查点

点击开关，滑块应该平滑地从一侧滑到另一侧，约 200ms。再次点击滑回去。背景色应该跟着状态变化。快速连续点击不应该导致崩溃或动画叠加。

---

## 第五步 — setHandlePosition：连接动画和重绘的桥梁

### 动手写

这个方法是 QPropertyAnimation 每帧都会调用的 WRITE 方法：

```cpp
void ToggleSwitch::setHandlePosition(double pos)
{
    // TODO: 如果 pos 和当前值相同就不折腾了
    //       m_handlePosition = pos;
    //       emit handlePositionChanged(pos);
    //       update();  // ← 这行是关键，触发 paintEvent 重绘
}
```

update() → Qt 事件循环在下一帧调用 paintEvent() → paintEvent 读取 m_handlePosition 画滑块 → 动画引擎更新 m_handlePosition → 循环。整个链条就是这样跑起来的。

### 检查点

如果你之前已经实现了前面的步骤，这里应该不需要额外验证——动画能跑就说明 setHandlePosition 在正常工作。

---

## 最终组装

```cpp
#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include "toggle_switch.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // TODO: 创建 QWidget 作为主窗口
    // TODO: QHBoxLayout 放一个 ToggleSwitch + QLabel 显示状态
    // TODO: 连接 toggled 信号，更新 QLabel 文本和 qDebug 输出

    return app.exec();
}
```

---

## 验收标准

点击开关后滑块平滑滑动约 200ms，不是瞬间跳到另一侧。关闭状态是灰色背景+滑块在左，开启状态是绿色背景+滑块在右。setChecked() 可以编程切换状态且带动画。toggled(bool) 信号在状态变化时正确发射。快速连续点击不崩溃、不动画叠加。缩放窗口后开关保持正确比例。

---

## 进阶挑战

实现拖拽切换——按住滑块可以左右拖动，松手后根据滑块位置判断是开还是关。需要处理 mouseMoveEvent 和 mouseReleaseEvent，计算鼠标位置对应的 handlePosition。或者添加禁用状态（setEnabled(false)），禁用时变灰且不可点击——查一下 QWidget::enabledChanged 信号。再或者给滑块加一个阴影效果，用 QPainter::setShadow 或者 QRadialGradient 模拟。

---

## 踩坑预防清单

> **坑 #1：忘记 Q_PROPERTY 宏**
> QPropertyAnimation 通过 Qt 的元对象系统来读写属性。没有 Q_PROPERTY 宏，它找不到 READ/WRITE 方法，运行时会报 "property not found" 警告并且动画不工作。这和 StatusLed 那篇的 Q_OBJECT 坑类似——编译不报错，运行时静默失败。

> **坑 #2：抗锯齿没开**
> 圆形和圆角矩形在不开启 Antialiasing 的情况下会有明显的锯齿，尤其是在低分辨率屏幕上。一行 `setRenderHint(QPainter::Antialiasing)` 就能解决，但忘记加的话视觉效果会很差。

> **坑 #3：动画对象的 parent 管理**
> QPropertyAnimation 的 parent 必须设为 this（ToggleSwitch），否则控件销毁后动画对象变成野指针。如果用成员变量持有动画对象，在析构时它会跟着一起被销毁（Qt 的父子对象管理机制）。

---

## 官方文档参考

- [QPropertyAnimation Class](https://doc.qt.io/qt-6/qpropertyanimation.html) — 属性动画
- [QProperty System](https://doc.qt.io/qt-6/properties.html) — Q_PROPERTY 宏详解
- [QEasingCurve Class](https://doc.qt.io/qt-6/qeasingcurve.html) — 缓动函数
- [QPainter::drawRoundedRect](https://doc.qt.io/qt-6/qpainter.html) — 圆角矩形

到这里就大功告成了。这是目前最难的一篇——你把自绘、属性系统、动画框架、鼠标事件全串起来了。如果你一路做到了这里，说明你已经具备了独立开发中等复杂度自定义控件的能力。下一篇我们换个方向，练一练 QWizard 多页向导。
