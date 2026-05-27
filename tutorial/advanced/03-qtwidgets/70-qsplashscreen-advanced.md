---
title: "3.70 QSplashScreen 进阶"
description: "入门篇我们学会了 QSplashScreen 的基本用法：设置一张 pixmap、showMessage 显示加载状态、主窗口 show 后 close。进阶篇要解决的是工程中真正头疼的问题——启动画面怎么优雅地消失，加载太快时闪一下就没了怎么办，以及怎么在 splash 上画一个靠谱的进度条。"
---

# 现代Qt开发教程（进阶篇）3.70——QSplashScreen 进阶

## 1. 前言 / 启动画面不是 show 一下就完事的

入门篇我们学会了 QSplashScreen 的基本用法：设置一张 pixmap、showMessage 显示加载状态、主窗口 show 后 close。说实话，做到这一步已经能用了——但如果你的应用启动加载比较快，splash 画面可能闪一下就消失了，用户体验比没有 splash 还差。反过来，如果你的应用启动特别慢，用户盯着一张静态图片什么反馈都没有，会以为程序卡死了。

这篇文章要解决的是工程中真正头疼的三个问题：第一，怎么让 splash 画面在关闭时有一个渐变消隐动画，而不是突兀地消失；第二，怎么保证 splash 至少显示一个最小时间（比如 2 秒），避免快速启动时画面闪烁；第三，怎么在 splash 的 pixmap 上叠加一个自绘的进度条，让用户知道加载进展到哪了。这三个问题搞清楚了，QSplashScreen 就从"能用"变成"好用"。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17 标准。QSplashScreen 属于 QtWidgets 模块，渐变动画需要用到 Qt 的 QPropertyAnimation（QtCore 模块）。计时功能使用 QElapsedTimer（QtCore 模块），不需要额外依赖。

## 3. 核心概念讲解

### 3.1 showMessage 与 pixmap 的绘制层级

入门篇我们用 showMessage 在 splash 上显示文字。现在要搞清楚的是这个文字是怎么画上去的——因为理解了绘制层级，后面自绘进度条才能找对位置。

QSplashScreen 的绘制机制是这样的：它内部持有一个 QPixmap 作为背景图，每次 paintEvent 时先绘制这个 pixmap，然后在 pixmap 上方绘制 showMessage 设置的消息文字。文字的颜色和位置由 setMessageRect 和消息文本决定。也就是说，pixmap 是底层，message 是上层。

这个层级关系意味着：如果你在 pixmap 本身画了东西（比如进度条），然后调用 showMessage，文字会覆盖在进度条上方。反过来，如果你想在文字上方再画东西，就需要在 paintEvent 里自己处理绘制顺序——因为 showMessage 的绘制发生在 QSplashScreen::paintEvent 的默认实现中。

```cpp
class MySplash : public QSplashScreen {
    // ...
protected:
    void paintEvent(QPaintEvent* event) override
    {
        // 先调用父类实现——它画 pixmap + message
        QSplashScreen::paintEvent(event);
        // 再画进度条，覆盖在 message 之上
        QPainter painter(this);
        drawProgressBar(&painter);
    }
};
```

这里我们要做的就是在自定义 QSplashScreen 子类中重写 paintEvent，先调用父类的 paintEvent（完成 pixmap 和 message 的绘制），再用 QPainter 画我们自己的内容。这样自定义内容就叠在 pixmap 和 message 之上。如果你希望 message 在最顶层（盖住进度条），就反过来：先画进度条，再调用父类 paintEvent。绘制顺序完全由你控制。

### 3.2 QPropertyAnimation 实现渐变消隐

默认情况下 QSplashScreen::close() 是瞬间消失的。要让它在关闭时有一个淡出效果，我们需要用 QPropertyAnimation 对窗口透明度（windowOpacity）做动画。思路很直接：在主窗口准备好之后，不直接 close splash，而是启动一个 300-500ms 的 opacity 动画，从 1.0 渐变到 0.0，动画结束后再 close。

```cpp
// 主窗口准备就绪后，开始渐变消隐
auto *opacity_anim = new QPropertyAnimation(splash, "windowOpacity");
opacity_anim->setDuration(400);      // 400ms 淡出
opacity_anim->setStartValue(1.0);
opacity_anim->setEndValue(0.0);
opacity_anim->setEasingCurve(QEasingCurve::OutQuad);

QObject::connect(opacity_anim, &QAbstractAnimation::finished, splash, &QWidget::close);
opacity_anim->start(QAbstractAnimation::DeleteWhenStopped);
```

这里有一个细节：QPropertyAnimation 对 windowOpacity 做动画时，窗口并不是真的"变透明"——它通过平台窗口系统的合成器（compositor）来改变整个窗口的 alpha 值。在 Windows 和 macOS 上效果很好，因为原生支持窗口级透明度。但在某些 Linux 窗口管理器上（特别是没有合成器的环境），windowOpacity 动画可能没有效果，窗口会直接消失。如果需要跨平台一致的淡出效果，可以考虑用 QGraphicsOpacityEffect 配合 QPropertyAnimation 作用于 graphicsOpacity 属性。

```cpp
// 备选方案：QGraphicsOpacityEffect（跨平台一致性更好）
auto *effect = new QGraphicsOpacityEffect(splash);
effect->setOpacity(1.0);
splash->setGraphicsEffect(effect);

auto *opacity_anim = new QPropertyAnimation(effect, "opacity");
opacity_anim->setDuration(400);
opacity_anim->setStartValue(1.0);
opacity_anim->setEndValue(0.0);
```

不过 QGraphicsOpacityEffect 也有代价——它会在每一帧对整个 widget 做一次 offscreen 渲染再合成，对于 splash 这种只显示几秒的窗口来说性能影响可以忽略。但要注意：如果在 splash 上同时使用了 setGraphicsEffect 和自绘进度条，绘制顺序可能受影响，需要实际测试。

### 3.3 最小显示时长保证

这是工程中最常见的需求：应用启动特别快（比如 200ms 就加载完了），但 splash 闪一下就没了，用户甚至来不及看到上面的 logo。我们需要一个"最小显示时长"的保证——即使加载很快，splash 也至少显示 N 秒。

实现方式是用 QElapsedTimer 记录 splash 显示的时刻，在主窗口加载完成后检查"已经过了多久"。如果还没到最小时长，就用一个 QTimer 延迟剩余时间后再触发渐变消隐。

```cpp
QElapsedTimer timer;
timer.start();

// ... 主窗口加载过程 ...

const qint64 kMinSplashMs = 2000;  // 最少显示 2 秒
qint64 elapsed = timer.elapsed();

auto finish_splash = [=]() {
    // 开始渐变消隐动画（3.2 的代码）
    startFadeOutAnimation(splash, main_window);
};

if (elapsed < kMinSplashMs) {
    QTimer::singleShot(static_cast<int>(kMinSplashMs - elapsed), finish_splash);
} else {
    finish_splash();
}
```

这个方案的关键是 finish_splash 是一个延迟执行的回调——要么立即调用（已经超过最小时长），要么通过 QTimer::singleShot 延迟调用（还差一点时间）。注意不要用 QThread::msleep 去等待，那会阻塞事件循环，导致 splash 的消息和动画都无法更新，得不偿失。

在 main 函数中，典型的调用流程是这样的：先 show splash 并启动 QElapsedTimer，然后调用 QApplication::processEvents 让 splash 有机会渲染出来，接着执行主窗口的初始化（加载配置、连接数据库、初始化资源等），最后调用 finish_splash 检查最小时长并触发消隐动画。整个过程中可以穿插调用 showMessage 更新加载状态。

### 3.4 进度条集成：在 splash pixmap 上自绘

光有文字提示还不够直观——用户更希望看到一个进度条。QSplashScreen 没有内置进度条，但我们可以通过 3.1 讲的绘制层级机制，在 paintEvent 中自绘一个。

思路是这样的：我们维护一个 0.0 到 1.0 的进度值（m_progress），每次主窗口初始化过程中完成一个阶段就更新这个值并调用 repaint() 触发重绘。paintEvent 中根据 m_progress 在 splash 底部画一个半透明的进度条。

```cpp
class MySplash : public QSplashScreen {
public:
    void set_progress(float progress)
    {
        m_progress = progress;
        repaint();  // 立即重绘
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        QSplashScreen::paintEvent(event);  // 先画 pixmap + message

        QPainter painter(this);
        int bar_height = 6;
        int bar_y = height() - bar_height - 10;  // 底部留 10px 边距
        int bar_width = static_cast<int>((width() - 20) * m_progress);

        // 背景轨道
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 255, 255, 60));
        painter.drawRoundedRect(10, bar_y, width() - 20, bar_height, 3, 3);

        // 进度填充
        painter.setBrush(QColor(0, 200, 120, 200));
        painter.drawRoundedRect(10, bar_y, bar_width, bar_height, 3, 3);
    }

private:
    float m_progress = 0.0f;
};
```

这里有个关键点：repaint() 而不是 update()。原因是 update() 会把重绘请求放入事件队列，而在主窗口初始化的密集计算过程中，事件队列可能来不及处理——结果就是进度条跳帧，从 20% 直接跳到 80%。repaint() 是同步重绘，立刻触发 paintEvent，代价是略占 CPU，但进度条更新更及时。在 splash 这种短暂场景下完全可以接受。

进度的设置方式取决于你的初始化流程。如果你能预知各个阶段的耗时比例，可以手动设置 set_progress(0.3)、set_progress(0.6) 等。如果初始化过程是异步的（比如等待网络响应），可以在每个异步回调中更新进度。不管哪种方式，记得在设置进度后调用 QApplication::processEvents()，让 splash 有机会处理重绘事件。

## 4. 踩坑预防

第一个坑是 windowOpacity 动画在 Linux 上可能无效。QPropertyAnimation 对 windowOpacity 做动画依赖平台窗口合成器的支持，没有合成器的 X11 环境（比如某些最小化的 i3wm 配置）下，动画完全没有效果，窗口直接消失。解决方案是用 QGraphicsOpacityEffect 替代 windowOpacity 属性动画，graphicsOpacity 的淡出由 Qt 自己在 offscreen buffer 中处理，不依赖平台合成器。代价是每帧多一次 offscreen 渲染，但 splash 场景下影响可以忽略。

第二个坑是 splash 上同时使用 setGraphicsEffect 和子类 paintEvent 自绘时的绘制冲突。setGraphicsEffect 会改变 widget 的绘制管线——默认情况下 Qt 会先把 widget 渲染到一张 offscreen pixmap 上，再对这张 pixmap 应用效果。如果你的 paintEvent 里有自定义绘制，这些绘制会被包含在 offscreen pixmap 中，然后整体被 opacity effect 作用。这通常是正确的行为，但如果你在 paintEvent 中做了 setClipping 或者复杂的坐标变换，可能会出现意外的裁剪。解决方案是在 opacity 动画期间避免复杂的 paintEvent 逻辑，或者干脆不用 setGraphicsEffect，改用 windowOpacity 并接受 Linux 上可能没有动画的代价。

第三个坑是 repaint() 在某些情况下可能导致重入问题。如果在 splash 的 paintEvent 中又触发了 repaint()（比如通过信号槽间接触发），就会陷入无限递归。这听起来不太可能发生，但如果你在 set_progress 中调用了某些会间接触发 paintEvent 的操作（比如调整 geometry、修改 stylesheet），就可能踩进去。解决方案是确保 set_progress 只修改数据（m_progress）并调用 repaint()，不做其他可能触发重绘的操作。

第四个坑是 QPixmap 在高 DPI 屏幕上的缩放问题。如果你用 QPixmap(":/splash.png") 加载了一张固定尺寸的图片，在 200% 缩放的屏幕上会被拉伸变糊。解决方案是使用 QPixmap 的 devicePixelRatio 感知版本——要么提供 @2x 后缀的图片资源让 Qt 自动选择，要么在代码中手动设置 devicePixelRatio：

```cpp
QPixmap pixmap(":/splash.png");
qreal dpr = qApp->devicePixelRatio();
pixmap.setDevicePixelRatio(dpr);
```

不过更推荐的做法是直接提供高分辨率图片（比如 2x 大小），让 QSplashScreen 自动缩放，这样在任何 DPI 下都能保持清晰。

## 5. 练习项目

练习项目：带进度条和渐变消隐的启动画面。我们要实现一个自定义 QSplashScreen 子类，加载一张 splash 图片（可以用 QPixmap 自己画一个纯色背景加文字作为替代），在底部绘制一个圆角进度条。主窗口的初始化过程模拟为 5 个阶段（每个阶段用 QTimer::singleShot 延迟 400ms），每完成一个阶段进度条前进一步。全部加载完成后，保证 splash 至少显示了 2 秒，然后以 500ms 的 opacity 动画渐变消隐，最后显示主窗口。

完成标准是：进度条随阶段平滑前进，不跳帧不闪烁；splash 至少显示 2 秒（即使模拟加载更快）；消隐动画流畅无卡顿；主窗口在 splash 完全消失后可见。提示几个关键点：用 QElapsedTimer 记录 splash 显示时间，finish 回调中检查是否达到最小时长；进度条在 paintEvent 中绘制，repaint() 触发即时更新；opacity 动画结束后再 show 主窗口和 close splash。

## 6. 官方文档参考链接

[Qt 文档 · QSplashScreen](https://doc.qt.io/qt-6/qsplashscreen.html) -- 启动画面控件，包含 showMessage、setPixmap、finish 方法说明

[Qt 文档 · QPropertyAnimation](https://doc.qt.io/qt-6/qpropertyanimation.html) -- 属性动画类，用于实现 windowOpacity 或 graphicsOpacity 的渐变动画

[Qt 文档 · QElapsedTimer](https://doc.qt.io/qt-6/qelapsedtimer.html) -- 高精度计时器，用于实现最小显示时长保证

[Qt 文档 · QGraphicsOpacityEffect](https://doc.qt.io/qt-6/qgraphicsopacityeffect.html) -- 图形透明度效果，替代 windowOpacity 的跨平台淡出方案

---

到这里，QSplashScreen 的进阶内容就过了一遍。绘制层级搞清楚了——pixmap 在底层，message 在中层，paintEvent 自绘在最上层，调用父类 paintEvent 的位置决定了自定义内容的叠放关系。渐变消隐的两种实现各有取舍：windowOpacity 简单但依赖平台合成器，QGraphicsOpacityEffect 更可靠但有 offscreen 渲染开销。最小显示时长用 QElapsedTimer + QTimer::singleShot 实现最干净，千万别用 sleep 阻塞事件循环。自绘进度条的关键是用 repaint() 而不是 update() 保证即时更新，同时在 paintEvent 里只做绘制不做其他可能触发重绘的操作。把这些点都注意到，你的 splash 画面就能从"闪一下"变成"体面的启动体验"了。
