---
title: "2.1 QPainter 进阶：双缓冲、合成模式、抗锯齿"
description: "入门篇我们用 QPainter 画了各种基本图形——矩形、椭圆、线条、弧形，调了颜色和画刷。说实话，那些知识拿来画个简单的仪表盘或者做个颜色选择器完全够用。"
---

# 现代Qt开发教程（进阶篇）2.1——QPainter 进阶：双缓冲、合成模式、抗锯齿

## 1. 前言 / 当画笔不再是「画个矩形填个色」

入门篇我们用 QPainter 画了各种基本图形——矩形、椭圆、线条、弧形，调了颜色和画刷。说实话，那些知识拿来画个简单的仪表盘或者做个颜色选择器完全够用。但真到了产品级别，事情就开始不对劲了：自定义控件拖动窗口大小的时候疯狂闪烁，设计小姐姐甩过来一张叠加效果图问「你能做出 Photoshop 里正片叠底那种混合效果吗」，曲线锯齿感太强被用户吐槽，4K 屏幕上线条粗细完全不对……每一个问题背后都是 QPainter 的进阶特性在等着我们。

这篇我们一起来把 QPainter 的三个进阶主题拆干净：双缓冲消除闪烁、CompositionMode 实现图层混合、RenderHint 控制渲染质量。搞完这些，你画出来的东西就能真正达到产品级。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17 标准。所有内容涉及 QtGui 和 QtWidgets 模块，需要链接 `Qt6::Gui` 和 `Qt6::Widgets`。示例程序是桌面 GUI 应用，需要显示环境（X11/Wayland/Windows/macOS）。Qt 6 中 QPainter 的 API 与 Qt 5 基本一致，但底层的渲染后端（QRasterPaintEngine）在 Qt 6 中有一些性能优化。

## 3. 核心概念讲解

### 3.1 双缓冲——消除控件闪烁的经典方案

控件闪烁的根因是这样的：当你在 `paintEvent` 里直接在屏幕上绘制时，每一条绘制指令都会立即刷新屏幕。如果你先画了一个白色背景，然后画了一个红色矩形，用户在极短的时间内会看到「白屏 → 白屏加矩形」两帧。当 paintEvent 被频繁触发（比如窗口 resize 时），这种逐指令刷新就会产生明显的闪烁。

双缓冲的思路很简单：先在一个离屏画布（QPixmap）上完成所有绘制，然后一次性把画布内容复制到屏幕上。这样用户永远只看到完整的最终结果，不存在中间帧。

```cpp
class PaintWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PaintWidget(QWidget* parent = nullptr)
        : QWidget(parent)
    {
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        if (m_buffer.isNull()) {
            return;
        }
        // 双缓冲：直接将离屏画布 blit 到屏幕
        QPainter screenPainter(this);
        screenPainter.drawPixmap(0, 0, m_buffer);
    }

    void resizeEvent(QResizeEvent* event) override
    {
        // 窗口大小变化时重建画布
        m_buffer = QPixmap(event->size());
        m_buffer.fill(Qt::white);
        renderToBuffer();  // 在离屏画布上绘制
        update();          // 触发 paintEvent
    }

private:
    void renderToBuffer()
    {
        QPainter painter(&m_buffer);
        // 在这里完成所有绘制...
    }

    QPixmap m_buffer;
};
```

关键点在于：`QPainter painter(&m_buffer)` 在 QPixmap 上绘制，而不是直接在 QWidget 上绘制。paintEvent 里只有一次 drawPixmap 调用，这一步是非常快的 blit 操作。

现在有一道调试题。如果上面的代码中 `renderToBuffer()` 执行需要 100ms（绘制内容非常复杂），窗口 resize 时会不会卡顿？答案是会的——双缓冲解决的是闪烁问题，不是性能问题。100ms 的绘制仍然需要 100ms，只是用户不会看到中间过程。要解决卡顿，需要把复杂绘制放到后台线程（使用 QImage 而不是 QPixmap，因为 QPixmap 依赖 GUI 线程）。

### 3.2 CompositionMode——图层混合的核心

QPainter 的合成模式决定了新绘制的像素如何与已有像素混合。默认模式是 `CompositionMode_SourceOver`：新像素直接覆盖在旧像素上方，按 alpha 值混合。但 QPainter 提供了几十种合成模式，对应 Photoshop 中的图层混合效果。

最常用的几种合成模式：

`CompositionMode_Multiply`（正片叠底）将两个颜色的 RGB 分量分别相乘再除以 255。结果颜色总是比原来的两个颜色都暗。适合创建阴影和暗调效果。

`CompositionMode_Screen`（滤色）与 Multiply 相反，结果颜色总是比原来的两个颜色都亮。适合创建高光和发光效果。

`CompositionMode_Overlay`（叠加）结合了 Multiply 和 Screen——暗区域更暗，亮区域更亮。适合增强对比度。

```cpp
// 设置合成模式后绘制
painter.setCompositionMode(QPainter::CompositionMode_Multiply);
painter.setBrush(QColor(255, 0, 0, 180));  // 半透明红色
painter.drawEllipse(100, 100, 100, 100);

painter.setBrush(QColor(0, 0, 255, 180));  // 半透明蓝色
painter.drawEllipse(130, 130, 100, 100);    // 重叠区域会呈现 Multiply 混合色
```

合成模式在自定义绘图控件中非常有用——比如需要叠加半透明指示层、创建颜色混合效果、实现遮罩等。但要注意性能：非 SourceOver 的合成模式通常比 SourceOver 慢，因为需要逐像素计算混合公式。在高频绘制场景下要慎用。

### 3.3 RenderHint——抗锯齿与渲染质量控制

QPainter 的渲染提示（RenderHint）控制绘制质量。最常用的是三个：

`QPainter::Antialiasing` 对图形边缘做抗锯齿处理。开启后圆形、曲线、斜线的边缘会变得平滑，但绘制速度略有下降。对于 UI 控件的自定义绘制，强烈建议始终开启。

`QPainter::TextAntialiasing` 对文本做抗锯齿。默认在大多数平台上是开启的。如果你需要像素级的文本渲染（比如终端模拟器），可以关闭它。

`QPainter::SmoothPixmapTransform` 在缩放 QPixmap 时使用双线性插值而不是最近邻采样。如果你需要把一张小图片放大显示，开启这个选项可以让放大后的图片更平滑。

```cpp
QPainter painter(this);
painter.setRenderHint(QPainter::Antialiasing);         // 图形抗锯齿
painter.setRenderHint(QPainter::TextAntialiasing);      // 文本抗锯齿
painter.setRenderHint(QPainter::SmoothPixmapTransform);  // 图片平滑缩放
```

### 3.4 QPainterPath——复杂路径与渐变填充

QPainterPath 是构建复杂几何路径的工具。你可以用 lineTo、cubicTo、arcTo 等方法逐步构建路径，然后用 QPainter 一次性绘制。QPainterPath 还支持渐变填充——把 QGradient 设置为 brush，fillPath 会自动按路径区域填充渐变色。

```cpp
QPainterPath path;
path.moveTo(50, 50);
path.cubicTo(100, 30, 150, 80, 100, 120);
path.cubicTo(50, 100, 20, 70, 50, 50);

QLinearGradient gradient(50, 50, 150, 120);
gradient.setColorAt(0.0, Qt::red);
gradient.setColorAt(1.0, Qt::blue);

painter.setBrush(gradient);
painter.setPen(QPen(Qt::darkGray, 1));
painter.drawPath(path);
```

QPainterPath 的另一个重要用途是做命中检测——`QPainterPath::contains(QPointF)` 可以判断一个点是否在路径内部。这在自定义控件的交互逻辑中非常有用（比如判断鼠标点击是否在某个不规则区域内）。

## 4. 踩坑预防

第一个坑是 QPixmap 在非 GUI 线程中创建导致崩溃。QPixmap 依赖底层平台特定的图形资源（X11 Pixmap、Windows HBITMAP），这些资源只能在 GUI 线程中访问。如果你在子线程中创建了 QPixmap 然后传给 GUI 线程使用，可能在工作线程创建时就崩溃。后果是程序在后台任务中直接段错误，栈回溯指向 QPixmap 构造函数。解决方案是在后台线程中使用 QImage（纯内存操作，不依赖平台资源），绘制完成后在 GUI 线程中将 QImage 转换为 QPixmap。

第二个坑是 setRenderHint 在绘制过程中频繁切换。每次 setRenderHint 都会影响 QPainter 的内部状态机，频繁切换会导致渲染管线反复重置。后果是绘制性能下降，特别是在复杂控件的 paintEvent 中。解决方案是在 paintEvent 开头统一设置所有 RenderHint，绘制过程中不要切换。如果确实需要不同质量级别的区域（比如文字区域抗锯齿，像素艺术区域不抗锯齿），用 save/restore 隔离。

第三个坑是 CompositionMode 设置后忘记恢复。合成模式是 QPainter 状态的一部分，设置后会影响后续所有绘制操作。如果你在某处设置了 Multiply 模式，后面的所有绘制（包括文字和背景）都会以 Multiply 模式混合，可能得到完全不符合预期的颜色。后果是整个控件的颜色看起来「脏」或不正确。解决方案是使用 save/restore 包裹合成模式切换，确保恢复为默认的 SourceOver。

## 5. 练习项目

练习项目：合成模式调色板。实现一个交互式控件，展示所有 CompositionMode 的效果。

具体要求是：左侧绘制一个固定的彩色矩形阵列作为底图，右侧通过下拉框选择不同的合成模式，在底图上方叠加一个半透明的渐变圆形，实时显示混合结果。完成标准是至少展示 8 种合成模式、切换时实时重绘、控件在窗口 resize 时自动适配。

提示几个关键点：用 QVector<QPair<QString, QPainter::CompositionMode>> 存储模式列表，切换模式时 renderToBuffer + update，resizeEvent 时重建画布。

## 6. 官方文档参考链接

[Qt 文档 · QPainter](https://doc.qt.io/qt-6/qpainter.html) -- QPainter 类完整参考

[Qt 文档 · QPainter::CompositionMode](https://doc.qt.io/qt-6/qpainter.html#CompositionMode-enum) -- 合成模式枚举

[Qt 文档 · QPainterPath](https://doc.qt.io/qt-6/qpainterpath.html) -- 绘制路径类参考

[Qt 文档 · QGradient](https://doc.qt.io/qt-6/qgradient.html) -- 渐变基类参考

---

到这里，QPainter 的进阶知识就拆完了。双缓冲消除闪烁、合成模式实现图层混合、渲染提示控制质量——掌握这些之后，自定义绘制的输出质量就能达到产品级。下一篇我们来看坐标变换进阶。
