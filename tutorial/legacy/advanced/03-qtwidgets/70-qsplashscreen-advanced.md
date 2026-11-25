---
title: "3.70 QSplashScreen 进阶"
description: "进阶篇把 QSplashScreen 的实现细节对到源码行上：构造函数固定注入 Qt::SplashScreen 加 FramelessWindowHint（qsplashscreen.cpp 第 100 行），setPixmap 按图的 devicePixelRatio 定窗口逻辑尺寸并挪到屏幕中心（第 250-262 行），绘制不走 paintEvent 而是重写的 event() 拦下 Paint 交给私有类画图再调虚函数 drawContents（第 319-347 行），repaint 重写里多泵一轮 processEvents（第 142-146 行），showMessage 默认左对齐黑字（qsplashscreen.h 第 32-33 行），落笔矩形四边各让五像素（第 290 行），富文本走 QTextDocument（第 291-313 行），点一下画面 mousePressEvent 直接 hide（第 132-135 行），finish 轮询等主窗口映射完成再 close（第 206-244 行），渐变消隐与最小展示时长咱们自己搭，示例在 examples/advanced/03-qtwidgets/70-qsplashscreen-advanced/"
---

# 现代Qt开发教程（进阶篇）3.70——QSplashScreen 进阶

启动屏是“主窗口出现前的第一面”。咱们想想 QApplication 的时间线：构造、建主窗口、exec()，把画面交给用户是第三步，那主窗口露面之前的几秒钟，用户看到的是桌面。QSplashScreen 就是来填这段空档的，数据库连接、插件加载、大资源初始化，这些活儿干多久，它就占着画面多久。笔者把 Qt 6.9.1 的 QSplashScreen 源码（qsplashscreen.cpp）从头读到尾，满打满算三百来行，发现它的几处要紧细节全藏在重写里：绘制不走 paintEvent，repaint 多泵一轮事件，连点一下消失都是 mousePressEvent 里的一句 hide。网上流传的几处讲法，咱们这篇拿行号逐一对照，有对得上的，也有要当面驳回的。

引用文件咱们交代清楚。主战场是 qsplashscreen.cpp 与 qsplashscreen.h（src/widgets/widgets），paintEvent 的分发对照，要借 qwidget.cpp 一小段，渐变消隐的平台条件也写在 qwidget.cpp 的属性文档里。官方示例片段，咱们借自 Qt 源码树自带的 snippets 目录。入门篇走过的构造加 show 加 close 流程本篇不重讲，配套示例带渐变消隐与最小展示时长，完整玩法放结语。

## 构造函数把旗标与居中都办妥了：咱们只管递图

构造函数只有一行正事（qsplashscreen.cpp 第 99-103 行），递给 QWidget 的旗标，是 Qt::SplashScreen 加 Qt::FramelessWindowHint，再叠上咱们传入的 f（第 100 行）。也就是说无框加无边框装饰这套外观，是构造那天就定下的，f 只能往上叠。文档给 f 的建议只有一条值得您考虑，Qt::WindowStaysOnTopHint（第 96-97 行），让启动屏压住桌面上全部窗口。类文档同时给 X11 留了备用方案，有些窗口管理器不理会置顶旗标，替代做法是起一只定时器周期性调 raise()（第 57-59 行），拿轮询模拟置顶。

多屏的场合有第二个构造函数，5.15 起提供，咱们多屏部署时用得上。它的头一个参数收 QScreen，私有类的 setScreen 把它指到副屏（第 105-120 行）。官方片段给的写法，咱们照着看，从 screens() 里取第二块屏再构造，见片段 main.cpp 第 28-32 行。

图递进去之后，setPixmap 一个人做了四件事，咱们挨个数（第 250-262 行）。存图（第 253 行），按 pixmap.hasAlpha() 决定给不给窗口开 WA_TranslucentBackground（第 254 行），把窗口 resize 到图的 deviceIndependentSize（第 256-257 行），再把窗口挪到屏幕几何中心（第 259 行）。构造函数的注释还写明，这一步顺手 repaint（第 102 行）。

高分屏的讲究就藏在 deviceIndependentSize 里，笔者把它翻出来给您看。这个尺寸是物理像素除以图自带的 devicePixelRatio，咱们递一张两倍分辨率的图并在图上设 dpr 为 2.0，窗口的逻辑尺寸与设计稿一比一，绘制时又开着 SmoothPixmapTransform（第 323 行，下一节讲它的出处），画面就是脆的。反过来递一张不带 dpr 的固定尺寸图，200% 缩放的屏幕上窗口按物理尺寸摆，画面被拉伸发糊。有流传说法称 QSplashScreen 会自动缩放大图，笔者在 setPixmap 的身体里没找到任何缩放路径，它只按图的 dpr 定窗口尺寸，图本身多大就画多大，缩放的准备得咱们自己做。

## 上屏的路径：绘制由 event() 的 Paint 分支接管

咱们跟着一次重绘走一遍。QSplashScreen 重写了 event()（第 331-347 行），QEvent::Paint 被它拦下，转手交给私有类的 handlePaintEvent（第 336-337 行），QEvent::Show 也拦下，触发 waitForWidgetMapped，等自己真的映射到屏幕（第 339-340 行）。handlePaintEvent（第 319-328 行）开一只 QPainter，renderHints 里设了 SmoothPixmapTransform（第 323 行），画两笔：图不为空就画在原点（第 325-326 行），再调虚函数 drawContents（第 327 行）。层级就此定下，pixmap 在底，drawContents 画的内容在上，showMessage 的字也从这里画上去，所以它永远盖在图上。

旧稿的方案在这里出了错，咱们得把它纠正过来。那个方案是重写 paintEvent 并调用父类实现，理由是父类实现画图加字，实际上 QSplashScreen 根本没有重写 paintEvent，子类里调 QSplashScreen::paintEvent 落到 QWidget 的空实现上，一笔不画。重写 paintEvent 出不出得来效果？碰巧出得来，因为 event() 拦完 Paint 仍把事件交回 QWidget::event，那里照常分发给 paintEvent（qwidget.cpp 第 9144-9149 行），子类的重写排在 handlePaintEvent 之后落笔，叠在上层纯属顺序使然。正路是类文档指明的方案，子类化并重写 drawContents（qsplashscreen.cpp 第 85 行），绘制入口收拢在一个虚函数里，第三节的进度条咱们就挂在它身上。

repaint 的重写是另一处要紧改动（第 142-146 行）。QWidget::repaint 之后跟着一句 QCoreApplication::processEvents()，文档写明差别就在这一句，没有事件循环在场，更新也能上屏。showMessage 内部调的正是重写过的 repaint（第 177 行），所以 exec() 起来之前的那段加载代码里，咱们随手 showMessage，画面立刻换字，不必自己处理事件。官方片段在 splash.show() 之后补一句 app.processEvents()，见片段 main.cpp 第 15-16 行，伺候的是点击与首轮显示，消息刷新不指望它。

说到点击，mousePressEvent 的实现干净得很，一句 hide()（第 132-135 行），用户点一下启动屏就把它收走。类文档叮嘱鼠标处理要生效，启动期间得周期性调 QApplication::processEvents()（第 71-73 行），道理同前，没人处理事件，点击永远排不上队。repaint 里泵事件的另一面您也要留神，被处理的事件里若有回头再碰启动屏的，重入的口子比普通控件大，所以咱们在 drawContents 里只画图，不做任何会再触发重绘的动作。

## showMessage：落笔的矩形四边各让五像素

签名咱们看头文件（qsplashscreen.h 第 32-33 行），alignment 默认 Qt::AlignLeft，color 默认 Qt::black，私有类构造里 currAlign 也初始化成 AlignLeft，出处 qsplashscreen.cpp 第 276 行。showMessage 把三样存进私有成员（第 173-175 行），发出 messageChanged（第 176 行），再 repaint（第 177 行）。clearMessage 清空后同样发信号（第 199-204 行），文档写明此时的参数是空串。message() 从 5.2 起能把当前文字读回来（第 180-192 行）。

字画在哪，咱们进 drawContents 看（第 286-317 行）。画笔蘸的是 currColor（第 289 行），落笔的矩形是 rect().adjusted(5, 5, -5, -5)（第 290 行），四边各让出五像素，这是文字的活动范围。有流传讲法称有个 setMessageRect 函数能划文字区，笔者在 Qt 6.9.1 的源码树里 grep 过，这个函数不存在，那是别的工具包的接口，落点只由对齐旗标加这五像素边距决定。

富文本还有一条专用路径。Qt::mightBeRichText 判定消息像 HTML 时（第 291 行），改走 QTextDocument 路线，setHtml、设文本宽度、用 QTextCursor 把对齐写进段落格式（第 292-304 行），AlignBottom 与 AlignVCenter 再挪矩形的顶边（第 306-309 行），doc.drawContents 收尾（第 312 行）。纯文本则一句 drawText(r, currAlign, currStatus) 画完（第 315 行）。咱们给消息加个颜色标签就能换字体换行，不用自绘。

自绘就重写 drawContents。pixmap() 返回的图不带 showMessage 画的字（第 264-271 行），文档说自绘可以拿它当底（第 82-85 行），但它按值返回，递到您手里的是隐式共享的副本，直接在副本上动笔，改动不会回到启动屏身上。挂一只进度条，咱们还是进 drawContents。

```cpp
// 伪代码：在 drawContents 里叠加进度条，铺在官方绘制之上
class ProgressSplash : public QSplashScreen {
public:
    using QSplashScreen::QSplashScreen;
    void setProgress(float p) { m_progress = p; repaint(); }
protected:
    void drawContents(QPainter* painter) override
    {
        QSplashScreen::drawContents(painter);  // 官方的字照画，进度条铺在字的上层
        const int barY = height() - 16;
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(255, 255, 255, 60));
        painter->drawRoundedRect(10, barY, width() - 20, 6, 3, 3);  // 轨道
        painter->setBrush(QColor(0, 200, 120, 200));
        painter->drawRoundedRect(10, barY, int((width() - 20) * m_progress), 6, 3, 3);
    }
private:
    float m_progress = 0.0f;
};
```

setProgress 里调 repaint 而非 update，讲究咱们也落在源码上。重写过的 repaint 自带一轮 processEvents（第 142-146 行），进度一推进，重绘当场发生，排队中的事件一并被处理，刷新不依赖事件循环转得多欢。旧稿对比两者时讲的是队列与同步的老话，没提这一句，笔者替您补上。

## finish 等主窗口就位：消隐动画与最短时长咱们自己搭

退场有两个层次，咱们分头看。保守的一层是 finish(QWidget* mainWin)（第 236-244 行），主窗口还没有原生窗口句柄就 createWinId（第 239-240 行），然后交给 waitForWidgetMapped 轮询，等它真的可见，最后 close() 自己（第 243 行）。waitForWidgetMapped（第 206-229 行）的等法是泵事件、送投递事件、睡十毫秒，一圈圈转到 windowHandle 可见为止，默认一千毫秒超时。这一手防的是启动屏撤得比主窗口上屏还早，中间露出一瞬空白桌面。官方片段的收尾，咱们也见过，正是主窗口 show() 之后一句 splash.finish(&window)，见片段 main.cpp 第 21-24 行。

体面的一层是渐变消隐，Qt 没有内置，咱们拿 QPropertyAnimation 对 windowOpacity 做动画，从 1.0 淡到 0.0。windowOpacity 是 QWidget 的属性，取值从 1.0 到 0.0（qwidget.cpp 第 11602-11607 行），平台条件您部署前要心里有数，属性文档写明它覆盖 Embedded Linux、macOS、Windows 与支持 Composite 扩展的 X11（第 11610-11611 行），X11 上还得有合成管理器在跑，窗口管理器也要认 _NET_WM_WINDOW_OPACITY 原子，条件写在第 11613-11616 行。精简的窗口管理器配齐不了这些，动画就指望不上，这是您上线前值得试一遍的事。

最短展示时长用 QElapsedTimer 保证。启动屏现身那一刻开始计时，加载收尾时核对差值，不够就 QTimer::singleShot 补足再起淡出。示例的补差算法咱们直接看（splash_demo.cpp 第 60-92 行）：四条加载消息按 600 毫秒一条排队，总时长按五个间隔算出 3000 毫秒，恰好等于 kMinimumSplashMs（splash_demo.h 第 55 行），差值为零，淡出从第 3000 毫秒起跑，时长 800 毫秒（splash_demo.cpp 第 103 行）。等待千万别用 sleep 凑，阻塞了事件循环，消息换不了字，动画也走不动，singleShot 让等待落在事件循环里，画面一直活着。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，行为咱们在 6.9.1 源码核对过。

[Qt 文档 · QSplashScreen](https://doc.qt.io/qt-6/qsplashscreen.html) -- 启动画面类，showMessage、drawContents 与 finish 的定义处

[Qt 文档 · QPropertyAnimation](https://doc.qt.io/qt-6/qpropertyanimation.html) -- 属性动画，windowOpacity 淡出的引擎

[Qt 文档 · QElapsedTimer](https://doc.qt.io/qt-6/qelapsedtimer.html) -- 单调时钟，最短展示时长的度量尺

---

启动屏这篇给窗口杂项收个尾，下一站 [3.71 QMdiArea 进阶](./71-qmdiarea-advanced.md)，多文档界面把一批子窗口收进一只容器里排布，画面从启动屏的单窗口换成多窗口，咱们换个方向继续。

想动手的您把示例跑起来（examples/advanced/03-qtwidgets/70-qsplashscreen-advanced/，可执行文件 70-qsplashscreen-advanced）。主窗口已经显示出来，启动屏随后盖上来居中显示，画面是代码现画的 500×300 渐变底加标题（splash_demo.cpp 第 119-147 行），底部中间每隔 600 毫秒换一条白色加载消息（第 68-84 行）。满三秒起 800 毫秒的淡出，画面渐隐收干净，主窗口被 activateWindow 请回前台，重新拿到焦点（第 108-113 行）。您手快点一下启动屏，它当场隐藏，这正是 mousePressEvent 里那句 hide() 的直接后果，后面的淡出照常走完，只是藏在隐藏状态下看不见，收尾的 close 和 deleteLater 照常执行。窗口里的 Replay Splash Screen 按钮能把整套流程再放一遍，适合反复观察淡出的节奏。

完成标准照旧。绘制走 event() 不走 paintEvent，您说得清。showMessage 落笔的矩形四边各让五像素，默认左对齐黑字，您数得出。finish 等的是主窗口映射完成，您答得准。repaint 多泵的那一轮 processEvents 意味着什么，您讲得明。
