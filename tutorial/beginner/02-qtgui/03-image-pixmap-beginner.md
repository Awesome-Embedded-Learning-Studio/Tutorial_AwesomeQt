# 现代Qt开发教程（新手篇）2.3——QImage、QPixmap、QIcon 图像处理基础

## 1. 前言 / 为什么需要搞清楚这几个类

我第一次在 Qt 里加载一张图片显示到界面上的时候，直接面对了三个长得差不多的类：QImage、QPixmap、QIcon。当时的心情就是——不就是显示一张图吗，为什么搞出这么多类？后来做项目做多了才明白，Qt 把图像处理拆成这几个类是有原因的，它们各自的定位完全不同，用错了地方性能和效果都会出问题。

咱们开门见山吧！QPixmap 是专门为屏幕显示优化的，它在底层直接存储的是显卡能用的像素数据，往屏幕上画的时候速度飞快。QImage 则反过来，它存储的是原始像素数据，你可以用代码一个像素一个像素地读写，做图像处理的时候就靠它。QIcon 不是一张图，而是一组图——它可以根据控件的状态（正常、禁用、选中）和尺寸自动选择最合适的图片来显示。

这篇文章我们一起来弄清楚：QPixmap 和 QImage 的本质区别是什么、怎么从文件和资源系统加载图像、怎么缩放图片保持比例不变形、怎么用 QIcon 给按钮和工具栏配图标。搞明白这些，以后在 Qt 里处理任何图像相关的需求你都不会再纠结该用哪个类了。

## 2. 环境说明

本篇代码适用于 Qt 6.5+ 版本（示例基于 Qt 6.9.1 验证），CMake 3.26+，C++17 标准。示例代码依赖 QtGui 和 QtWidgets 模块，QImage 和 QPixmap 在 QtGui 中，QIcon 也属于 QtGui，但我们的示例用到了 QWidget 和 QPushButton 来展示效果，所以需要同时链接 QtWidgets。所有代码在 Linux、Windows、macOS 上都可以编译运行，不过你需要注意准备几张测试图片——我们会用代码生成一些，也会演示从文件加载的方式。

## 3. 核心概念讲解

### 3.1 QPixmap vs QImage：先搞清楚再动手

这是新手最容易搞混的地方。我们直接从底层存储机制来说：QPixmap 底层存储的是跟显示设备相关的像素数据，你可以理解为它已经是"渲染好了的位图"，直接交给显卡去显示就行，所以 `QPainter::drawPixmap()` 的速度非常快。但正因为它跟显示设备绑定，你没法直接访问里面的像素数据——没有 `pixel()` 函数给你用。

QImage 就不一样了，它存储的是设备无关的原始像素数据，底层就是一个二维数组。你可以用 `pixel(x, y)` 读取任意位置的像素颜色，也可以用 `setPixel(x, y, color)` 逐像素修改，甚至可以用 `scanLine()` 拿到每一行的原始内存指针来做批量操作。代价是它显示到屏幕上时需要先转换成显示设备格式，所以 `drawImage()` 比 `drawPixmap()` 稍慢一些。

```cpp
// QPixmap：显示速度快，不能直接操作像素
QPixmap pixmap(200, 200);
pixmap.fill(Qt::blue);
// pixmap 没有 pixel() / setPixel() 接口

// QImage：可以直接读写像素，显示稍慢
QImage image(200, 200, QImage::Format_RGB32);
image.fill(Qt::red);
// 可以逐像素操作
image.setPixel(100, 100, qRgb(0, 255, 0));  // 在 (100,100) 画一个绿点
QRgb color = image.pixel(100, 100);  // 读取像素颜色
```

那什么时候用哪个呢？规则很简单：如果只是加载一张图片显示到界面上，用 QPixmap；如果需要读取或修改像素数据（比如截图、图像滤镜、生成图片），用 QImage；处理完了要显示的话，用 `QPixmap::fromImage()` 转一下就行：

```cpp
// 先用 QImage 处理像素
QImage image("photo.png");
// 做一些像素操作……
image = image.convertToFormat(QImage::Format_RGB32);

// 处理完了要显示，转成 QPixmap
QPixmap pixmap = QPixmap::fromImage(image);
// 然后在 paintEvent 里用 drawPixmap 画出来
```

还有一个很多人忽略的性能细节：如果你要在 paintEvent 里反复画同一张 QPixmap，最好把它存成类的成员变量，不要每次 paintEvent 都重新加载。QImage 也一样，如果图片不变，加载一次就够了。每帧都从磁盘读文件这种事，就算 SSD 也扛不住。

### 3.2 从文件和资源系统加载图像

最直接的方式就是从文件路径加载。QPixmap 和 QImage 都有接受文件路径的构造函数，用起来非常方便：

```cpp
// 从文件加载
QPixmap pixmap("images/photo.png");
if (pixmap.isNull()) {
    qDebug() << "图片加载失败，检查路径是否正确";
}

QImage image("images/photo.png");
if (image.isNull()) {
    qDebug() << "图片加载失败";
}
```

这里有个坑你可能会踩到：文件路径是相对于程序的工作目录的，不是相对于源代码文件的。这意味着你在 IDE 里运行和双击可执行文件运行，工作目录很可能不一样，图片路径就找不到了。调试的时候建议先 `qDebug() << QDir::currentPath()` 看一下当前工作目录到底在哪里。

说到图片路径的问题，Qt 提供了一个更好的解决方案：Qt Resource System（QRC）。它可以把图片、配置文件等资源直接编译进可执行文件里，这样不管程序在哪里运行，资源路径都是稳定的，不存在找不到文件的问题。

使用 QRC 分三步：首先创建一个 `.qrc` 文件，这是一个 XML 格式的资源描述文件，里面列出你要打包的文件；然后在 CMake 里用 `qt_add_resources` 或者依赖 `CMAKE_AUTORCC` 自动编译；最后在代码里用 `:/` 前缀的路径来访问资源。

```xml
<!-- resources.qrc -->
<RCC>
    <qresource prefix="/images">
        <file>logo.png</file>
        <file>icon_edit.png</file>
        <file>icon_delete.png</file>
    </qresource>
</RCC>
```

```cpp
// 用 :/ 前缀访问 QRC 中的资源
QPixmap logo(":/images/logo.png");
QIcon editIcon(":/images/icon_edit.png");
```

你可能会问：什么时候用 QRC，什么时候用外部文件？简单来说，小图标、logo 这些不会经常变的东西放 QRC 最好，程序一个文件就能跑。但如果是用户数据、大量高清图片、视频这种体积大的东西，就不要往 QRC 里塞了——它们会让可执行文件膨胀到离谱。

### 3.3 QPixmap::scaled() 与缩放

加载了一张图片，但显示区域大小不一定刚好合适，这时候就需要缩放。QPixmap 提供了 `scaled()` 函数，功能非常丰富：

```cpp
QPixmap original("photo.png");

// 指定目标尺寸，忽略比例（可能变形）
QPixmap stretched = original.scaled(200, 100);

// 保持宽高比，缩放到不超过 200x100 的最大尺寸
QPixmap scaled = original.scaled(200, 100, Qt::KeepAspectRatio);

// 保持宽高比，缩放到完全覆盖 200x100 区域（可能裁剪）
QPixmap covered = original.scaled(200, 100, Qt::KeepAspectRatioByExpanding);

// 平滑缩放（质量更好，速度稍慢）
QPixmap smooth = original.scaled(200, 100, Qt::KeepAspectRatio,
                                  Qt::SmoothTransformation);
```

`Qt::KeepAspectRatio` 是你 90% 情况下要用的模式——它保证图片不变形，缩放到目标区域内能放下的最大尺寸。`Qt::KeepAspectRatioByExpanding` 则是反过来，保证整个目标区域被图片填满，多出来的部分被裁掉，做背景图的时候常用。

`Qt::SmoothTransformation` 这个参数值得一提：默认的缩放用的是快速算法（近邻采样），放大后会看到明显的锯齿和马赛克；加上这个参数后会使用双线性滤波，放大后的效果平滑得多，代价是计算量稍大。对于静态图片一次性缩放来说，这点性能损失完全可以忽略，所以建议默认就加上。

在实际开发中，你可能需要让图片跟随窗口大小自适应。一个典型的做法是在 paintEvent 里根据 Widget 当前尺寸动态缩放：

```cpp
void paintEvent(QPaintEvent *) override
{
    QPainter painter(this);
    if (!m_pixmap.isNull()) {
        // 根据 Widget 尺寸缩放图片，保持比例，居中显示
        QPixmap scaled = m_pixmap.scaled(width(), height(),
                                          Qt::KeepAspectRatio,
                                          Qt::SmoothTransformation);
        // 居中偏移
        int x = (width() - scaled.width()) / 2;
        int y = (height() - scaled.height()) / 2;
        painter.drawPixmap(x, y, scaled);
    }
}
```

不过这里有一个性能隐患：`paintEvent` 每次窗口大小改变都会被调用，每次都做一次缩放计算。如果图片很大，这个开销不小。更好的做法是在 resizeEvent 里做一次缩放，把结果缓存起来：

```cpp
void resizeEvent(QResizeEvent *) override
{
    if (!m_original.isNull()) {
        // 在 resize 时缓存缩放结果
        m_scaled = m_original.scaled(width(), height(),
                                      Qt::KeepAspectRatio,
                                      Qt::SmoothTransformation);
    }
    update();  // 触发重绘
}

void paintEvent(QPaintEvent *) override
{
    QPainter painter(this);
    if (!m_scaled.isNull()) {
        int x = (width() - m_scaled.width()) / 2;
        int y = (height() - m_scaled.height()) / 2;
        painter.drawPixmap(x, y, m_scaled);
    }
}
```

这样 paintEvent 里只做一次 drawPixmap，没有任何计算，重绘速度极快。

### 3.4 QIcon 多尺寸与状态图标

QIcon 和前面两个类不太一样，它不是"一张图"，而是一组图的容器。你可以给同一个 QIcon 添加不同尺寸、不同状态、不同模式下的图片，Qt 在使用的时候会自动挑选最合适的那一张来显示。

```cpp
QIcon icon;
icon.addFile(":/icons/icon_16.png", QSize(16, 16));   // 小尺寸（菜单、工具栏）
icon.addFile(":/icons/icon_32.png", QSize(32, 32));   // 中尺寸（按钮）
icon.addFile(":/icons/icon_64.png", QSize(64, 64));   // 大尺寸（对话框）

// 不同状态可以配不同的图
icon.addFile(":/icons/icon_32_disabled.png", QSize(32, 32),
              QIcon::Disabled);  // 禁用状态
icon.addFile(":/icons/icon_32_active.png", QSize(32, 32),
              QIcon::Active);    // 激活/按下状态
```

如果你只提供了一张图，Qt 会自动帮你缩放到需要的尺寸，效果也能用，但肯定不如手动准备的各尺寸图片清晰。所以对于经常出现在界面上的图标，特别是工具栏图标，建议至少提供 16x16 和 32x32 两个尺寸。

QIcon 最常见的用途就是给 QPushButton、QAction、QToolBar 配图标：

```cpp
// 给按钮设置图标
QPushButton *btn = new QPushButton;
btn->setIcon(QIcon(":/icons/open.png"));
btn->setIconSize(QSize(24, 24));  // 指定显示尺寸
btn->setText("打开文件");

// 给 QAction 设置图标（菜单和工具栏）
QAction *openAction = new QAction(QIcon(":/icons/open.png"), "打开", this);
```

你可能会好奇：iconSize 设多大合适？这个取决于你的使用场景。工具栏图标通常是 22x22 或 24x24，菜单图标通常是 16x16，对话框里的大图标可能是 48x48 或 64x64。如果你不确定，可以参考你所处平台的 Human Interface Guidelines——macOS、Windows、GNOME 都有各自的推荐值。

还有一个实用技巧：QIcon 可以直接从 QPixmap 构造，这样你可以用代码动态生成图标，不一定非要有图片文件：

```cpp
// 用代码画一个简单的彩色圆点图标
QPixmap dot(24, 24);
dot.fill(Qt::transparent);  // 透明背景
QPainter p(&dot);
p.setRenderHint(QPainter::Antialiasing);
p.setBrush(Qt::red);
p.setPen(Qt::NoPen);
p.drawEllipse(2, 2, 20, 20);
QIcon dotIcon(dot);

// 用在按钮上
QPushButton *statusBtn = new QPushButton;
statusBtn->setIcon(dotIcon);
```

到这里你可以停下来想一想：如果你要实现一个图片浏览器，让用户打开一张图片并自适应窗口大小显示，同时有一个缩放到 1:1 原始尺寸的按钮，你会怎么组织 QPixmap 和 QImage 的使用？窗口缩放的时候应该在哪里做缩放计算？想清楚这个，说明你真的理解了这几个类各自该在什么时候出场。

## 4. 踩坑预防

第一个坑是路径问题，刚才已经提过了：用相对路径加载图片，程序换个地方运行就找不到了。尤其在 Windows 上，双击 exe 和在 IDE 里运行的工作目录经常不一样。如果你不想用 QRC，至少用 `QCoreApplication::applicationDirPath()` 拼一个绝对路径出来，或者让用户通过文件对话框选择图片。

第二个坑是图片加载失败但不报错。QPixmap 和 QImage 的构造函数接受一个不存在的文件路径时，不会抛异常，也不会输出警告——它们只是默默创建一个 null 对象。你只有在调 `isNull()` 的时候才会发现图片没加载成功。所以每次加载图片之后一定要检查 `isNull()`，别画了半天不知道为什么画面上什么都没有。

```cpp
QPixmap pixmap("nonexistent.png");
// 不检查直接画？画面空白，你排查半天找不到原因
painter.drawPixmap(0, 0, pixmap);

// 正确做法：加载后检查
QPixmap pixmap("nonexistent.png");
if (pixmap.isNull()) {
    qDebug() << "图片加载失败！";
    // 画一个占位符或者错误提示
    painter.setPen(Qt::red);
    painter.drawText(rect(), Qt::AlignCenter, "图片加载失败");
    return;
}
```

第三个坑是在 paintEvent 里做重量级的图片操作。有些人习惯在 paintEvent 里从文件加载图片、做缩放、做格式转换——这些操作每帧都重复执行，窗口稍微拖动一下就触发好几次 paintEvent，程序直接卡成幻灯片。图片加载和预处理都应该在初始化阶段或者数据变化时做一次，paintEvent 里只做最轻量的绘制操作。

第四个坑是 QPixmap 的线程安全问题。QPixmap 是跟 GUI 线程绑定的，你不能在非 GUI 线程里创建或操作 QPixmap——调用就直接崩溃。如果你需要在后台线程里做图像处理，请用 QImage，它是线程安全的（每个线程操作自己的 QImage 对象互不影响），处理完了再到 GUI 线程转成 QPixmap 显示。

## 5. 练习项目

我们来做一个实战练习：实现一个简易图片查看器。程序打开后显示一个窗口，窗口中央有一张图片自适应显示（保持比例居中），窗口上方有一排工具按钮——"打开文件"按钮让用户选择一张图片加载，"适应窗口"按钮切换自适应显示模式，"原始尺寸"按钮以 1:1 比例显示图片（图片超出窗口时需要滚动条提示），还有一个标签显示当前图片的文件名和尺寸信息。

完成标准是：继承 QWidget 实现自定义的图片显示控件，在 paintEvent 中用 drawPixmap 绘制图片；支持拖拽窗口改变大小，图片自适应跟随缩放（在 resizeEvent 中缓存缩放结果）；通过 QPushButton + QFileDialog 让用户选择图片文件加载；用 QLabel 显示图片信息（文件名、原始尺寸、缩放比例）；加载失败时显示错误提示而不是空白画面。几个提示：用 `QPixmap::fromImage()` 把 QImage 转成 QPixmap 用于显示；图片信息可以用 `QPixmap::size()` 和 `QFileInfo::fileName()` 获取；自适应缩放用 `Qt::KeepAspectRatio` + `Qt::SmoothTransformation`。

## 6. 官方文档参考链接

[Qt 文档 · QImage Class](https://doc.qt.io/qt-6/qimage.html) -- QImage 的完整 API，像素读写、格式转换、图像处理相关接口

[Qt 文档 · QPixmap Class](https://doc.qt.io/qt-6/qpixmap.html) -- QPixmap 的完整 API，加载、缩放、转换相关接口

[Qt 文档 · QIcon Class](https://doc.qt.io/qt-6/qicon.html) -- QIcon 多尺寸多状态图标的完整文档

[Qt 文档 · The Qt Resource System](https://doc.qt.io/qt-6/resources.html) -- QRC 资源系统的详细说明，怎么创建和使用资源文件

[Qt 文档 · QPixmap::fromImage](https://doc.qt.io/qt-6/qpixmap.html#fromImage) -- QImage 到 QPixmap 的转换函数，包含格式转换相关的参数说明

---

到这里，QImage、QPixmap、QIcon 三个类各自的角色你应该清楚了：QPixmap 负责高效显示、QImage 负责像素级操作、QIcon 负责多尺寸多状态的图标管理。记住最核心的一条——只显示用 QPixmap，要操作像素用 QImage，最后别忘了在 paintEvent 之外做好预处理，别把重量级操作塞进每帧的重绘里。下一篇文章我们来搞定 QFont 和文本渲染，让文字在 Qt 里也能排版得漂漂亮亮。
