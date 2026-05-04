# 现代Qt开发教程（新手篇）3.34——QLabel：文本与图像显示

## 1. 前言 / 你可能低估了 QLabel

QLabel 是 Qt 里最基础也最容易被忽视的控件之一。大部分人的认知停留在"往界面上放一段文字"这个层面——setText 设文字，show 出来，完事。但如果你翻开 Qt 文档认真看一下 QLabel 的接口列表，会发现它能做的事情远比"显示文字"多得多：它可以渲染 HTML 富文本，可以显示 QPixmap 图片，可以播放 QMovie 动画（包括 GIF），可以通过 setAlignment 控制对齐方式，通过 setWordWrap 开启自动换行，通过 setBuddy 把自己关联到一个伙伴控件实现快捷键聚焦，通过 linkActivated 信号捕获用户点击超链接的事件。

说真的，QLabel 在 Qt 的控件体系里扮演的是一个"万能展示器"的角色——它不做输入，只做输出，但输出的内容涵盖了文字、图片、动画、超链接这四种最常见的信息载体。今天我们不讲那些三秒钟就能上手的 setText 基础用法，而是把火力集中在四个实战维度上：显示文本、HTML 富文本、图片和 GIF 动画的能力边界和注意事项，setAlignment 对齐与 setWordWrap 换行的行为细节，setBuddy 关联快捷键到伙伴控件的机制，以及 linkActivated / linkHovered 信号处理超链接交互。搞清楚这些之后，你会发现 QLabel 在实际项目中承担的工作比想象中多得多。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QLabel 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。显示图片用到了 QPixmap（QtGui 模块），播放 GIF 动画用到了 QMovie（也是 QtGui 模块）。因为 Qt6::Widgets 已经自动链接了 QtGui，所以不需要额外添加模块依赖。示例代码中还用到了 QLineEdit、QSpinBox、QVBoxLayout、QHBoxLayout、QGridLayout、QGroupBox 和 QScrollArea 来搭建界面。

## 3. 核心概念讲解

### 3.1 显示文本、HTML 富文本、图片、GIF 动画

QLabel 显示普通文本的方式非常直接——调用 setText(const QString &) 即可。但 setText 做的事情不仅仅是"把字符串画上去"，它会先对传入的字符串做一个内容类型判断：如果字符串包含 HTML 标签（通过 Qt::mightBeRichText 判断），QLabel 会自动切换到富文本渲染模式；否则按纯文本渲染。这个自动检测机制在大部分时候是方便的，但偶尔也会给你挖坑——如果你的纯文本里恰好包含 `<`、`>` 这些字符（比如显示一段 C++ 模板代码 `std::vector<int>`），QLabel 可能会误判为 HTML 标签导致显示异常。遇到这种情况，用 setTextFormat(Qt::PlainText) 强制指定纯文本模式就行了。

```cpp
auto *label = new QLabel;

// 纯文本
label->setText("Hello, Qt!");

// 强制纯文本模式（避免 HTML 误判）
label->setTextFormat(Qt::PlainText);
label->setText("std::vector<int> vec;");

// HTML 富文本
label->setTextFormat(Qt::AutoText);  // 默认值，自动检测
label->setText("<h2>标题</h2>"
               "<p>这是一段 <b>加粗</b> 和 <i>斜体</i> 文字。</p>"
               "<p><a href='https://doc.qt.io'>Qt 文档</a></p>");
```

QLabel 支持的 HTML 子集是 Qt 自己实现的，不是完整的 Web 浏览器级别的 HTML。它支持常见的标签如 `<b>`（加粗）、`<i>`（斜体）、`<u>`（下划线）、`<s>`（删除线）、`<font>`（字体颜色和大小）、`<h1>` 到 `<h6>`（标题）、`<p>`（段落）、`<br>`（换行）、`<a>`（超链接）、`<img>`（图片）、`<table>`（表格）、`<ul>` / `<ol>` / `<li>`（列表）。不支持 `<div>` 的复杂布局、CSS 样式表、JavaScript 等。如果你需要渲染复杂的 HTML 内容，应该用 QTextBrowser 或者 QWebEngineView，而不是 QLabel。

显示图片是 QLabel 的另一个核心能力。通过 setPixmap(const QPixmap &) 可以把一个 QPixmap 对象设置到 QLabel 上，QLabel 会自动调整自己的尺寸来适配图片——前提是你没有手动设置 fixedWidth / fixedHeight 之类的约束。如果你希望 QLabel 的尺寸固定而图片自适应，可以结合 setScaledContents(true) 让图片缩放到 QLabel 的尺寸，但这样会导致图片变形。更推荐的做法是用 QPixmap::scaled() 在设值之前就把图片缩放到合适的尺寸，保持宽高比。

```cpp
auto *imageLabel = new QLabel;

// 方式 1：直接设置原始图片
QPixmap pixmap(":/images/photo.png");
imageLabel->setPixmap(pixmap);

// 方式 2：缩放图片到固定尺寸，保持宽高比
QPixmap scaled = pixmap.scaled(200, 200,
    Qt::KeepAspectRatio, Qt::SmoothTransformation);
imageLabel->setPixmap(scaled);
imageLabel->setFixedSize(200, 200);
imageLabel->setAlignment(Qt::AlignCenter);

// 方式 3：开启 scaledContents（可能导致变形）
imageLabel->setScaledContents(true);
imageLabel->setFixedSize(200, 200);
imageLabel->setPixmap(pixmap);  // 图片会被拉伸到 200x200
```

这里有一个实战中很常见的坑：如果你把 QLabel 放在布局里（比如 QVBoxLayout），QLabel 会因为图片的存在而撑大整个布局。解决方法有两种——要么给 QLabel 设一个固定尺寸（setFixedSize），要么给布局设一个 sizeConstraint（比如 QLayout::SetFixedSize）。选择哪种取决于你希望窗口是固定大小还是可调整的。

播放 GIF 动画需要借助 QMovie 类。QMovie 不是 QLabel 的内置功能，而是一个独立的动画播放器，它通过 setFramePixmap 信号逐帧把图片推送给 QLabel。使用方式是先创建 QMovie 对象并指定 GIF 文件路径，然后把 QMovie 设置到 QLabel 上，最后调用 start() 开始播放。

```cpp
auto *gifLabel = new QLabel;

auto *movie = new QMovie(gifLabel);
movie->setFileName(":/animations/loading.gif");
movie->setSpeed(100);  // 播放速度百分比，100 = 原速

gifLabel->setMovie(movie);
movie->start();  // 开始播放

// 暂停和恢复
movie->stop();
movie->start();

// 跳转到特定帧
movie->jumpToFrame(5);
```

QMovie 支持 GIF 和 MNG 格式（如果你编译 Qt 时开启了 MNG 支持）。它提供了 stateChanged、frameChanged、error 等信号，可以用来监控播放状态。QMovie 的 setSpeed(int) 可以调节播放速度——100 是原速，50 是半速，200 是两倍速。setCacheMode(QMovie::CacheAll) 可以把所有帧缓存到内存中，适合需要反复播放的小动画。

有一点需要注意：QMovie 的生命周期管理。QMovie 的 parent 通常设为对应的 QLabel，这样 QLabel 销毁时 QMovie 也会跟着销毁。但如果你需要在多个 QLabel 之间共享同一个动画，就要注意不要让 QMovie 被提前 delete。另外，在调用 setMovie 之后如果再调用 setPixmap 或者 setText，QLabel 会停止显示动画——因为 setPixmap / setText 会覆盖 movie 的显示。反过来也一样，调用 setMovie 后之前的 pixmap 和 text 就不再显示了。

### 3.2 setAlignment 对齐与 setWordWrap 换行

setAlignment(Qt::Alignment) 控制 QLabel 中内容的对齐方式。它的参数是 Qt::AlignmentFlag 的组合，常用的值有 Qt::AlignLeft（左对齐）、Qt::AlignRight（右对齐）、Qt::AlignHCenter（水平居中）、Qt::AlignTop（顶部对齐）、Qt::AlignBottom（底部对齐）、Qt::AlignVCenter（垂直居中）、Qt::AlignCenter（水平+垂直同时居中）。

```cpp
auto *label = new QLabel;
label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
label->setText("左对齐 + 垂直居中");
```

这里需要区分一个容易混淆的点：setAlignment 控制的是内容在 QLabel 的矩形区域内的对齐方式，而不是 QLabel 自身在布局中的位置。QLabel 在布局中的位置由布局管理器决定——比如 QHBoxLayout 默认把所有控件放在垂直居中的位置。如果你想让 QLabel 在布局中靠左或者靠右，需要在布局层面操作（addStretch、alignment 属性等），而不是在 QLabel 的 setAlignment 里操作。

还有一个实际细节：当 QLabel 的文本很短而 QLabel 本身很宽时，setAlignment 的效果最明显——你可以看到文本是靠左、靠右还是居中。当文本很长需要换行时，对齐方式会影响每一行的对齐——比如 Qt::AlignRight 会让每一行都右对齐，而不是最后一行右对齐前面的行左对齐。

setWordWrap(bool) 控制是否开启自动换行。默认情况下 wordWrap 是 false，QLabel 会把所有文本画在一行里——即使文本超出了 QLabel 的可见宽度，它也不会折行，而是直接被裁剪掉。设为 true 后，QLabel 会在单词边界处自动折行，让文本始终在 QLabel 的宽度范围内显示。

```cpp
auto *label = new QLabel;
label->setWordWrap(true);
label->setFixedWidth(300);
label->setText("这是一段很长的文本，用来演示 QLabel 的自动换行功能。"
               "当 wordWrap 开启后，QLabel 会在单词边界处自动折行，"
               "保证文本始终在控件的可见范围内显示。");
```

wordWrap 的换行规则是基于单词边界的——它会在空格、标点符号等位置折行，不会在一个单词的中间断开。对于中文这种没有明确单词边界的文字，Qt 会按字符进行换行，所以不存在中文被截断的问题。这也是为什么 setWordWrap(true) + setFixedWidth 是处理长文本 QLabel 的标准组合——固定宽度决定了一行能放多少文字，wordWrap 确保超出部分自动折行。

有一点要注意：如果 QLabel 同时设置了 wordWrap 为 true 和一个很小的 fixedHeight，折行后的文本可能超出 QLabel 的高度限制，导致下面的内容被裁剪。这时候要么不限制高度让 QLabel 自动扩展，要么把 QLabel 放进 QScrollArea 里让用户滚动查看。

### 3.3 setBuddy 关联快捷键

setBuddy(QWidget *) 是 QLabel 一个比较冷门但非常实用的功能。它的作用是把一个 QLabel 和另一个控件"绑定"成伙伴关系，当用户按下 QLabel 文本中以 `&` 前缀标记的快捷键时，键盘焦点会自动跳到伙伴控件上。

```cpp
auto *nameLabel = new QLabel("姓名(&N):");
auto *nameEdit = new QLineEdit;
nameLabel->setBuddy(nameEdit);

auto *ageLabel = new QLabel("年龄(&A):");
auto *ageSpin = new QSpinBox;
ageLabel->setBuddy(ageSpin);
```

在这个例子中，QLabel 的文本 "姓名(&N):" 中的 `&N` 不会显示为 "&N"，而是显示为带下划线的 "N"。当用户按下 Alt+N 时，焦点会从当前控件跳到 nameLabel 的伙伴控件 nameEdit 上。同理，Alt+A 会跳到 ageSpin。

这个机制在表单界面中非常常用——它让用户可以通过键盘快速在不同输入框之间跳转，而不需要用 Tab 键一个一个地找。很多桌面应用的"设置"对话框就是这么做的：每个 QLabel 都通过 setBuddy 关联到旁边的输入控件，Alt+字母就能直接跳过去。

需要注意的一点是：`&` 在 QLabel 中的转义。如果你想显示一个真正的 `&` 字符（不是快捷键标记），需要写 `&&`。比如 QLabel 的文本设为 "Tom && Jerry"，显示出来是 "Tom & Jerry"，没有快捷键。

```cpp
// 显示 "Tom & Jerry"，没有快捷键
auto *label = new QLabel("Tom && Jerry");

// 显示 "打开文件(&O)"，O 是快捷键，Alt+O 跳到伙伴控件
auto *openLabel = new QLabel("打开文件(&O):");
auto *openEdit = new QLineEdit;
openLabel->setBuddy(openEdit);
```

还有一个细节：setBuddy 只对包含 `&` 前缀快捷键标记的 QLabel 有效。如果 QLabel 的文本中没有 `&X` 这样的标记，setBuddy 调用了也不会有效果。而且一个 QLabel 只能关联一个 buddy 控件——如果你多次调用 setBuddy，后面的调用会覆盖前面的。

### 3.4 linkActivated 信号处理超链接点击

当 QLabel 显示包含 `<a href="...">` 标签的 HTML 内容时，这些超链接默认是不可点击的——它们只是蓝色带下划线的文字，鼠标点上去没有任何反应。要让超链接变得可交互，需要做两件事：开启 openExternalLinks 或者连接 linkActivated 信号。

```cpp
auto *label = new QLabel;
label->setTextFormat(Qt::RichText);
label->setTextInteractionFlags(Qt::TextBrowserInteraction);
label->setText("<a href='https://doc.qt.io'>Qt 官方文档</a>");

// 方式 1：直接打开外部浏览器（最简单）
label->setOpenExternalLinks(true);

// 方式 2：连接信号，自己处理点击事件
connect(label, &QLabel::linkActivated, this,
        [](const QString &url) {
    qDebug() << "用户点击了链接:" << url;
    // 可以在这里做自定义处理：在应用内打开、记录日志等
});
```

方式 1 适合"点击链接直接打开浏览器"的场景，setOpenExternalLinks(true) 后用户点击链接会自动调用系统的默认浏览器打开 URL。方式 2 适合需要自定义行为的场景——比如在应用内的 QTextBrowser 中打开链接、记录用户点击行为、弹出一个确认对话框再决定是否打开等。

还有一个 linkHovered(const QString &) 信号，当鼠标悬停在超链接上时触发，参数是链接的 URL。这个信号可以用来实现状态栏提示——鼠标悬停时在状态栏显示目标 URL，鼠标移开后清空。

```cpp
connect(label, &QLabel::linkHovered, this,
        [statusBar](const QString &url) {
    if (url.isEmpty()) {
        statusBar->setText("就绪");
    } else {
        statusBar->setText("链接目标: " + url);
    }
});
```

有一点需要特别注意：setTextInteractionFlags 是让 QLabel 的文本区域变得可交互的前提。如果你不设置这个属性（默认值是 Qt::NoTextInteraction），即使你连接了 linkActivated 信号也不会触发——因为 QLabel 根本不接收鼠标事件。常用的设置值是 Qt::TextBrowserInteraction，它包含了 Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard，既允许鼠标选择文本，也允许鼠标和键盘访问链接。

另外一个容易踩的坑是 setTextFormat。如果你设了 Qt::PlainText，那么即使文本中包含 `<a href>` 标签也不会被解析为超链接。确保 setTextFormat 是 Qt::AutoText（默认值）或者 Qt::RichText 才能正确渲染 HTML 超链接。

## 4. 踩坑预防

第一个坑是 HTML 误判。QLabel 默认使用 Qt::AutoText 格式，如果你的纯文本中恰好包含 `<` 和 `>` 字符，可能会被误判为 HTML 导致渲染异常。解决办法是 setTextFormat(Qt::PlainText)。

第二个坑是 QPixmap 的生命周期和内存。QLabel::setPixmap 做的是浅拷贝——QLabel 内部持有 QPixmap 的引用。如果你在栈上创建 QPixmap 然后传给 QLabel，函数返回后 QPixmap 被销毁，QLabel 上的图片会消失。确保 QPixmap 的生命周期至少和 QLabel 一样长，或者用 setPixmap(pixmap.copy()) 做深拷贝。

第三个坑是 QMovie 和 setPixmap / setText 的互斥。调用 setMovie 后，再调用 setPixmap 或 setText 会覆盖动画显示。反过来也一样。QLabel 同时只能处于一种显示模式：纯文本、富文本、图片、动画——四选一，后设的覆盖先设的。

第四个坑是 wordWrap 配合固定高度导致文本被裁剪。如果文本很长而 QLabel 的高度不够，开启 wordWrap 后超出的内容会被截断。要么不限制高度，要么放进 QScrollArea。

第五个坑是 setBuddy 只在有 `&` 前缀时有效。如果你调了 setBuddy 但 QLabel 的文本里没有 `&X` 标记，快捷键不会生效。另外 `&` 在 Qt 中是转义字符，显示真正的 `&` 要写 `&&`。

第六个坑是 linkActivated 需要配合 setTextInteractionFlags。默认的 Qt::NoTextInteraction 不接收鼠标事件，超链接点了没反应。需要至少设为 Qt::TextBrowserInteraction。

## 5. 练习项目

我们来做一个综合练习：创建一个"用户信息卡片"窗口，覆盖 QLabel 的核心用法。窗口顶部用 QLabel 显示一张用户头像（用 QPixmap 加载一张本地图片，缩放到 80x80 圆形显示）。头像下方是一个 QLabel 显示用户名（大号粗体），再下方是一个 QLabel 显示个人简介（开启 wordWrap，固定宽度 300px，自动换行）。简介中包含一个超链接指向用户的个人网站。窗口中部是一个表单区域，每组用 QLabel + QLineEdit 配合 setBuddy 实现快捷键跳转（Alt+N 跳到姓名输入框，Alt+E 跳到邮箱输入框）。窗口底部用一个 QLabel 播放一个 GIF 加载动画（模拟数据加载状态），加一个按钮控制动画的播放和暂停。

提示：头像的圆形裁剪可以通过 QPixmap 的 mask 或者 QPainter 的 clipPath 实现；setBuddy 的快捷键在表单布局中最自然；GIF 动画用 QMovie 配合 QLabel::setMovie。

## 6. 官方文档参考链接

[Qt 文档 -- QLabel](https://doc.qt.io/qt-6/qlabel.html) -- 文本与图像显示控件

[Qt 文档 -- QMovie](https://doc.qt.io/qt-6/qmovie.html) -- GIF / MNG 动画播放器

[Qt 文档 -- QPixmap](https://doc.qt.io/qt-6/qpixmap.html) -- 离屏图像表示

---

到这里，QLabel 的四个核心维度就全部讲完了。文本和 HTML 富文本的显示让 QLabel 既能渲染简单的字符串，也能渲染带有加粗、斜体、超链接的富文本内容，但要注意 Qt 的 HTML 子集是有限的，不要把它当浏览器用。setAlignment 和 setWordWrap 这对组合解决了"文本在控件内的位置"和"长文本如何折行"两个最基本的排版问题。setBuddy 把 QLabel 变成了表单中的快捷键导航工具，Alt+字母直接跳到对应的输入框。linkActivated 和 linkHovered 信号则让 QLabel 上的超链接变得可交互——要么直接打开浏览器，要么自定义处理逻辑。QLabel 看起来只是个"放文字的地方"，但把文本、图片、动画、超链接、对齐、换行、伙伴控件这些功能组合起来，它在实际项目中承担的职责远比想象中丰富。
