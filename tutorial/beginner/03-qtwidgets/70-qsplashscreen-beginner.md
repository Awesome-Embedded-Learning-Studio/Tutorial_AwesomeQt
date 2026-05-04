# 现代Qt开发教程（新手篇）3.70——QSplashScreen：启动画面

## 1. 前言 / 双击图标然后盯着空白桌面发呆的体验谁都不想要

随便打开一个体量稍微大一点的桌面应用——Visual Studio、Blender、LibreOffice——你会发现它们有一个共同的行为：双击图标后不会直接弹出主窗口，而是先闪出一个带 Logo 的启动画面，上面可能还有一行"正在加载模块...""正在初始化渲染引擎..."之类的状态文字。等所有初始化工作完成后，启动画面消失，主窗口登场。这个启动画面就是 QSplashScreen。

为什么需要它？原因很直接：如果你的应用启动时要加载大量资源（图片、字体、翻译文件、插件），初始化复杂的子系统（数据库连接、网络栈、渲染管线），或者执行耗时的配置解析（读取 JSON/XML 配置文件、扫描插件目录），从用户双击图标到主窗口出现之间可能有几秒甚至十几秒的空白。这段时间里用户什么都看不到，很可能会以为应用卡死了，然后反复双击图标试图启动，结果打开了七八个实例。启动画面的作用就是在这个空白期内给用户一个明确的视觉反馈——"程序正在启动，请稍等"。

QSplashScreen 的 API 非常精简。构造时传入一个 QPixmap 作为背景图片，然后调用 show() 显示出来。在初始化过程中用 showMessage 更新状态文字，告诉用户当前在做什么。等所有准备工作完成后，调用 finish(mainWindow) 关闭启动画面并把主窗口推到前台。整个流程就是这么三步：show、showMessage、finish。

今天我们从四个方面展开。先看基本用法——show 展示 Logo 和 finish 关闭，然后讨论 showMessage 更新加载状态文字的方式，接着研究与耗时初始化操作配合的完整启动序列，最后看看一些高级配置比如消息对齐方式、自动渐隐效果和与 QCoreApplication::processEvents 的配合。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QSplashScreen 在 QtWidgets 模块中，QPixmap 在 QtGui 模块中，链接 Qt6::Widgets 即可（Qt6::Gui 是隐式依赖）。示例代码涉及 QSplashScreen、QPixmap、QMainWindow、QApplication、QThread、QTimer、QLabel、QProgressBar 和 QDebug。

## 3. 核心概念讲解

### 3.1 show / finish：启动画面的基本生命周期

QSplashScreen 的基本使用方式非常简单——在 main 函数中创建 QSplashScreen 实例，在创建主窗口之前调用 show() 显示启动画面，等主窗口初始化完成后调用 finish(mainWindow) 关闭启动画面。

```cpp
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 创建启动画面，传入背景图
    QPixmap pixmap(":/images/splash.png");
    QSplashScreen splash(pixmap);
    splash.show();

    // 在这里执行耗时的初始化操作...
    // 加载配置文件、初始化数据库、扫描插件...

    // 创建并显示主窗口
    QMainWindow mainWindow;
    mainWindow.show();

    // 关闭启动画面
    splash.finish(&mainWindow);

    return app.exec();
}
```

这段代码的执行流程是这样的。QApplication 构造完成后，我们立即创建 QSplashScreen 并调用 show()。这时屏幕上会弹出一个无边框的窗口，显示 splash.png 图片的内容。接下来执行初始化操作。最后创建主窗口并调用 show()，然后调用 splash.finish(&mainWindow)。

finish 方法做了两件事：首先把主窗口提到前台并激活（相当于 raise + activateWindow），然后关闭启动画面。这个顺序确保了用户在启动画面消失的那一刻就能看到已经就绪的主窗口，没有视觉上的闪烁或空档。

如果你不传参数直接调用 splash.finish()（无参版本在 Qt 6 中不存在，必须传一个 QWidget*），或者干脆不用 finish 而是手动调用 splash.close()，效果基本一样——启动画面关闭，主窗口已经在那了。但 finish 做了额外的窗口管理操作（确保主窗口获得焦点），所以在正常流程中推荐用 finish。

关于背景图片有几个要注意的地方。QSplashScreen 本质上是一个无边框的顶层窗口（window flags 是 Qt::Window | Qt::FramelessWindowHint），窗口大小就是 QPixmap 的大小。如果你的图片特别大（比如 1920x1080），启动画面会占满整个屏幕，这在大多数情况下不是你想要的效果——启动画面通常是一个居中的小矩形，比如 400x300 或者 500x200。你需要提前准备好合适尺寸的图片，或者在代码中缩放：

```cpp
QPixmap pixmap(":/images/splash.png");
pixmap = pixmap.scaled(
    500, 300,
    Qt::KeepAspectRatio,
    Qt::SmoothTransformation);
QSplashScreen splash(pixmap);
```

如果你没有现成的图片资源，也可以用代码绘制一个简单的启动画面——先创建一个指定大小的 QPixmap，用 QPainter 在上面画背景色、文字、Logo，然后传给 QSplashScreen：

```cpp
QPixmap pixmap(500, 300);
pixmap.fill(QColor("#2D2D30"));

QPainter painter(&pixmap);
painter.setPen(Qt::white);
painter.setFont(QFont("Arial", 24, QFont::Bold));
painter.drawText(
    pixmap.rect(),
    Qt::AlignCenter,
    "My Application\n正在启动...");
```

### 3.2 showMessage：更新加载状态文字

如果在启动过程中只显示一张静态图片，用户看到的是一个一动不动的启动画面，跟没看到状态更新一样——用户还是会觉得程序卡住了。showMessage 的作用就是在启动画面上叠加一行状态文字，告诉用户"现在在干什么"。

```cpp
splash.show();
splash.showMessage(
    "正在加载配置文件...",
    Qt::AlignBottom | Qt::AlignHCenter,
    Qt::white);

// ... 加载配置文件 ...

splash.showMessage(
    "正在初始化数据库...",
    Qt::AlignBottom | Qt::AlignHCenter,
    Qt::white);

// ... 初始化数据库 ...

splash.showMessage(
    "正在加载插件...",
    Qt::AlignBottom | Qt::AlignHCenter,
    Qt::white);

// ... 扫描并加载插件 ...
```

showMessage 有三个参数：message 是要显示的文字，alignment 是文字的对齐方式，color 是文字颜色。alignment 默认是 Qt::AlignLeft | Qt::AlignBottom，你可以改成 Qt::AlignBottom | Qt::AlignHCenter 让文字水平居中显示在底部。每次调用 showMessage 会替换之前的状态文字——所以启动画面上始终只有一行文字在更新。

这里有一个非常重要的问题需要理解：showMessage 本身不会让事件循环处理积压的事件。如果你的初始化代码是在主线程中执行的同步操作（比如直接在 main 函数中调用 initDatabase()），主线程被阻塞在初始化代码中，事件循环没有机会运行——启动画面上的文字更新和绘制事件都排在事件队列里等待处理，但没人处理它们。

解决这个问题的标准做法是在每次 showMessage 之后调用 QCoreApplication::processEvents()。processEvents 会强迫事件循环处理一次队列中的所有待处理事件——包括 QSplashScreen 的绘制事件。这样一来，每次更新状态文字后用户都能立即看到变化：

```cpp
splash.showMessage("正在加载配置文件...");
QCoreApplication::processEvents();

loadConfig();

splash.showMessage("正在初始化数据库...");
QCoreApplication::processEvents();

initDatabase();

splash.showMessage("正在加载插件...");
QCoreApplication::processEvents();

loadPlugins();
```

每次 showMessage 之后都跟一行 processEvents() 看起来有点啰嗦，但这是必须的。如果你不调用 processEvents，启动画面可能从头到尾都显示空白——因为在整个初始化过程中主线程没有机会处理任何绘制事件。等到初始化完成、app.exec() 启动事件循环的时候，启动画面已经被 finish 关掉了，用户什么都没看到。

你也可以通过 setMessageRect 来控制状态文字的显示区域。默认情况下 QSplashScreen 把文字放在画面的底部区域，如果你的启动画面底部有重要的视觉元素不想被文字遮挡，可以用 setMessageRect 指定一个自定义的文字区域：

```cpp
// 在启动画面中间偏下的区域显示状态文字
splash.setMessageRect(QRect(50, 200, 400, 50));
```

### 3.3 与耗时初始化操作配合：完整的启动序列

在实际项目中，启动画面的使用场景通常是这样的：应用启动时需要执行一系列初始化步骤，每个步骤耗时不同，启动画面在整个过程中持续显示并更新进度。当所有步骤完成后，主窗口就绪，启动画面关闭。

我们来写一个比较完整的启动序列示例。为了让演示效果更直观，我们用 QThread::msleep 来模拟各个初始化步骤的耗时——在实际项目中这些步骤可能是加载配置文件、建立数据库连接、扫描插件目录、加载翻译文件、初始化渲染引擎等等：

```cpp
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("MyApp");
    app.setOrganizationName("MyOrg");

    // 创建启动画面（使用代码绘制的简易画面）
    QPixmap pixmap(500, 300);
    pixmap.fill(QColor("#1E1E2E"));

    QPainter painter(&pixmap);
    painter.setPen(QColor("#CDD6F4"));
    painter.setFont(
        QFont("Sans", 20, QFont::Bold));
    painter.drawText(
        QRect(0, 80, 500, 40),
        Qt::AlignCenter,
        "My Application");
    painter.setFont(QFont("Sans", 10));
    painter.setPen(QColor("#A6ADC8"));
    painter.drawText(
        QRect(0, 130, 500, 30),
        Qt::AlignCenter,
        "Version 1.0.0");
    painter.end();

    QSplashScreen splash(pixmap);
    splash.show();

    // 确保启动画面立即绘制
    QCoreApplication::processEvents();

    // === 初始化步骤 1: 加载配置 ===
    splash.showMessage(
        "正在加载配置文件...",
        Qt::AlignBottom | Qt::AlignHCenter,
        Qt::white);
    QCoreApplication::processEvents();
    QThread::msleep(800);  // 模拟耗时

    // === 初始化步骤 2: 数据库 ===
    splash.showMessage(
        "正在初始化数据库连接...",
        Qt::AlignBottom | Qt::AlignHCenter,
        Qt::white);
    QCoreApplication::processEvents();
    QThread::msleep(600);

    // === 初始化步骤 3: 加载翻译 ===
    splash.showMessage(
        "正在加载翻译资源...",
        Qt::AlignBottom | Qt::AlignHCenter,
        Qt::white);
    QCoreApplication::processEvents();
    QThread::msleep(400);

    // === 初始化步骤 4: 扫描插件 ===
    splash.showMessage(
        "正在扫描插件目录...",
        Qt::AlignBottom | Qt::AlignHCenter,
        Qt::white);
    QCoreApplication::processEvents();
    QThread::msleep(500);

    // === 初始化步骤 5: 初始化渲染 ===
    splash.showMessage(
        "正在初始化渲染引擎...",
        Qt::AlignBottom | Qt::AlignHCenter,
        Qt::white);
    QCoreApplication::processEvents();
    QThread::msleep(700);

    // === 创建主窗口 ===
    splash.showMessage(
        "正在准备主窗口...",
        Qt::AlignBottom | Qt::AlignHCenter,
        Qt::white);
    QCoreApplication::processEvents();

    QMainWindow mainWindow;
    mainWindow.setWindowTitle("My Application");
    mainWindow.resize(800, 600);
    mainWindow.show();

    splash.finish(&mainWindow);

    return app.exec();
}
```

这个启动序列的模式很清晰：showMessage + processEvents 成对出现，中间夹着实际的初始化操作。processEvents 确保每次状态更新都能立即反映到屏幕上。这个模式在 Qt 社区是广泛使用的标准做法，几乎所有的 QSplashScreen 教程和示例都是这么写的。

如果你觉得在 main 函数里写一长串初始化步骤太杂乱，可以把初始化逻辑封装到一个单独的函数或者类中，让这个函数在每个步骤开始前通过回调或者信号通知启动画面更新状态。示例代码中我们就是这么做的——用一个 InitStep 结构体把步骤名称和耗时绑定在一起，然后循环执行。

还有一个细节值得一提：QSplashScreen 在关闭时可以选择性地显示一个渐隐效果。你可以通过 close() 代替 finish()，然后自己实现一个 QPropertyAnimation 来做淡出动画。不过大部分应用不需要这么花哨——启动画面消失的那一刻主窗口已经就位了，用户的注意力已经转移到主窗口上，启动画面的消失方式（瞬间关闭还是渐隐）用户基本不会注意到。

### 3.4 setMessageRect 与进程事件机制的深入理解

前面我们提到了 setMessageRect 和 processEvents，这里再深入讨论一下它们的机制。

setMessageRect 的参数是一个 QRect，指定了状态文字在启动画面上的显示区域。这个区域必须是 QPixmap 范围内的有效矩形。QSplashScreen 在内部用 QLabel 来显示状态文字，setMessageRect 实际上就是在设置这个 QLabel 的 geometry。默认的文字区域是启动画面底部的一个水平条带。如果你的启动画面图片底部有装饰性元素或者渐变效果，文字可能会和这些元素重叠导致可读性差——这时候用 setMessageRect 把文字挪到一个干净的显示区域就很有用。

processEvents 的作用不仅是让 QSplashScreen 的文字更新生效。在主线程执行同步耗时操作期间，Windows 等系统可能会认为你的应用"没有响应"——因为你的应用没有处理窗口系统发送的消息（比如 WM_PAINT、WM_TIMER）。processEvents 让主线程临时从你的初始化代码中"抽身"出来处理一下这些系统消息，避免操作系统把你的启动画面标记为"未响应"（在 Windows 上表现为标题栏显示"（未响应）"，鼠标变成转圈的光标）。

processEvents 有两个重载版本。无参版本 processEvents() 处理所有待处理的事件。带参数的版本 processEvents(QEventLoop::ProcessEventsFlags flags, int maxTime) 可以限制处理的事件类型和最大处理时间。如果你担心 processEvents 调用时处理了不该处理的事件（比如用户在启动过程中点击了某个按钮触发了一个还没准备好的操作），可以用 flags 参数排除某些事件类型：

```cpp
// 只处理绘制事件和定时器事件，忽略用户输入
QCoreApplication::processEvents(
    QEventLoop::ExcludeUserInputEvents);
```

这在某些场景下是有意义的——启动过程中你只希望启动画面能正常刷新，但不希望用户在启动完成前和界面交互（因为此时很多功能还没初始化完成）。排除用户输入事件可以防止用户在启动过程中误操作。

## 4. 踩坑预防

第一个坑是忘记调用 processEvents。这是新手使用 QSplashScreen 时最常见的错误——创建了启动画面，调用 show()，然后在主线程中执行一堆初始化操作，但全程没有调用 processEvents。结果启动画面从头到尾都是一片空白或者只有背景图片没有状态文字，直到主窗口出现后启动画面才被 finish 关掉。用户看到的效果和没有启动画面完全一样。记住一个规则：每次 showMessage 之后必须跟一行 processEvents()。

第二个坑是在启动画面的 messageRect 区域没有留足空间。如果你的启动画面图片是 500x200 的，底部 50 像素是纯色背景用于显示状态文字，那没问题。但如果底部有复杂的图案，文字可能看不清楚。解决方法是用 setMessageRect 把文字挪到画面上的一块干净区域，或者在图片上预留一个半透明的深色条带作为文字背景。

第三个坑是启动画面在多显示器环境下的定位。QSplashScreen 默认显示在屏幕中央（QScreen::availableGeometry 的中心）。在多显示器环境下，"屏幕中央"通常是主显示器的中心。如果你希望启动画面出现在鼠标所在的显示器上，需要手动计算位置并调用 splash.move()。

第四个坑是 QSplashScreen 不支持直接显示动画 GIF。如果你想要一个带动画的启动画面（比如旋转的加载图标），不能直接用 QSplashScreen——它只支持静态 QPixmap。替代方案是用 QMovie + QLabel 自己实现一个无边框窗口来播放 GIF 动画，或者用 QPropertyAnimation 做简单的动画效果。不过对于大多数桌面应用来说，静态图片 + 状态文字已经足够了。

第五个坑是 QPixmap 在 QApplication 构造之前不可用。QPixmap 依赖 GUI 相关的系统资源（在 X11 上需要 Display 连接，在 Windows 上需要 GDI），而这些资源是在 QApplication 构造时初始化的。如果你在 QApplication 之前创建 QPixmap，程序会崩溃。确保 QSplashScreen 和 QPixmap 的创建都在 QApplication 构造之后。

## 5. 练习项目

我们来做一个综合练习：创建一个完整的启动流程，包含 QSplashScreen 和 QMainWindow。启动画面使用代码绘制——500x300 像素，深灰色背景，顶部居中显示应用名称"MyApp"（大号白色粗体），中间显示版本号"v1.0.0"，底部区域留作状态文字显示区。

启动过程分五步执行，每步使用 QThread::msleep 模拟不同耗时：加载配置（800ms）、初始化日志（400ms）、连接数据库（1000ms）、扫描插件（600ms）、准备界面（300ms）。每步开始前调用 showMessage 更新状态文字，然后调用 processEvents 确保画面刷新。所有步骤完成后创建 QMainWindow——中央放一个 QLabel 显示"应用已就绪"，状态栏显示"启动完成，共耗时 X.X 秒"。QMainWindow 显示后调用 splash.finish() 关闭启动画面。

提示：使用 QElapsedTimer 在启动开始时 start()，结束时 elapsed() 获取毫秒数，除以 1000.0 得到秒数。

## 6. 官方文档参考链接

[Qt 文档 -- QSplashScreen](https://doc.qt.io/qt-6/qsplashscreen.html) -- 启动画面类

[Qt 文档 -- QSplashScreen::showMessage](https://doc.qt.io/qt-6/qsplashscreen.html#showMessage) -- 显示状态消息

[Qt 文档 -- QSplashScreen::finish](https://doc.qt.io/qt-6/qsplashscreen.html#finish) -- 关闭启动画面

[Qt 文档 -- QCoreApplication::processEvents](https://doc.qt.io/qt-6/qcoreapplication.html#processEvents) -- 事件处理方法

[Qt 文档 -- QPixmap](https://doc.qt.io/qt-6/qpixmap.html) -- 像素图类

---

到这里，QSplashScreen 的核心用法就全部讲完了。show + finish 构成启动画面的基本生命周期，showMessage 在画面上叠加状态文字给用户反馈，processEvents 确保在同步初始化过程中画面能正常刷新，setMessageRect 控制文字的显示位置。这套机制虽然简单，但解决了桌面应用启动体验中一个很实际的问题——让用户在等待的几秒钟里不再对着空白屏幕发呆。
