# 现代Qt开发教程（新手篇）3.44——QScrollArea：滚动区域容器

## 1. 前言 / 那个在你塞了太多控件时默默出现的救场控件

我们写桌面应用时经常会遇到一个尴尬的情况：你精心设计了一个表单页面，二十几个输入框加上分组标题、说明文字、按钮，纵向一铺开直接超出了窗口的可视范围。在小屏幕笔记本上更是惨不忍睹——下半截控件完全被截断，连个滚动条都没有，用户根本不知道下面还有内容。解决这个问题的办法非常直接：把内容放进一个可以滚动的容器里。当内容的实际尺寸超出容器可见区域时，自动出现滚动条，用户通过滚动来访问全部内容。

Qt 的 QScrollArea 就是这个功能的标准实现。它是一个容器控件——你往里面放一个内容控件（可以是单个 QLabel、一个装满子控件的 QWidget、或者任何自定义控件），当内容控件的尺寸大于 QScrollArea 的可见区域时，QScrollArea 会自动提供水平和垂直滚动条。你不需要手动管理滚动条的出现和消失——QScrollArea 会根据内容大小自动判断是否需要显示滚动条。

今天的内容分四个部分：setWidget 设置被滚动的内容控件，setWidgetResizable 让内容自适应 QScrollArea 的宽度，动态添加内容后自动滚动到底部的实现方式，以及通过 QSS 自定义滚动条的外观样式。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QScrollArea 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。示例代码中用到了 QScrollArea、QLabel、QPushButton、QLineEdit、QVBoxLayout、QHBoxLayout、QScrollBar 和 QTimer。

## 3. 核心概念讲解

### 3.1 setWidget 设置被滚动的内容控件

QScrollArea 的核心方法是 setWidget(QWidget *widget)——它把一个 QWidget 设为 QScrollArea 的内容控件。QScrollArea 会在内容控件外围套一层滚动视口（viewport），当内容控件的大小超过视口大小时，自动显示滚动条。

```cpp
auto *scrollArea = new QScrollArea;

// 创建一个很"长"的内容控件
auto *content = new QWidget;
auto *contentLayout = new QVBoxLayout(content);
for (int i = 0; i < 50; ++i) {
    contentLayout->addWidget(new QLabel(
        QString("第 %1 行内容").arg(i + 1)));
}

scrollArea->setWidget(content);
```

setWidget 有一点需要特别注意：QScrollArea 会接管内容控件的所有权——它会把自己设为内容控件的 parent。这意味着你不需要手动 delete 传给 setWidget 的控件。如果你之后又调了一次 setWidget(newWidget)，之前那个控件会被 QScrollArea 自动 delete。这是 Qt 的父子对象树在起作用——QScrollArea 析构时会自动销毁它管理的 widget()。

widget() 方法返回当前设置的内容控件指针。如果你还没有调用 setWidget，widget() 返回 nullptr。

QScrollArea 继承自 QAbstractScrollArea，后者提供了水平和垂直滚动条的底层管理。你可以通过 setHorizontalScrollBarPolicy 和 setVerticalScrollBarPolicy 来控制滚动条的显示策略。三个可选值是 Qt::ScrollBarAsNeeded（内容超出时自动显示，默认）、Qt::ScrollBarAlwaysOff（始终隐藏）和 Qt::ScrollBarAlwaysOn（始终显示）。

```cpp
// 始终显示垂直滚动条，始终隐藏水平滚动条
scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
```

一个常见的场景是只允许垂直滚动、禁止水平滚动——这在表单页面和列表式布局中非常常见。做法是禁止水平滚动条，同时让内容控件的宽度跟随 QScrollArea 的宽度（下一节讲）。

### 3.2 setWidgetResizable 内容自适应宽度

默认情况下，QScrollArea 不会自动调整内容控件的大小来匹配自身的宽度。这意味着如果你把一个 QWidget 放进 QScrollArea，这个 QWidget 的宽度会保持它的 sizeHint 或者你手动设置的固定宽度——不会因为 QScrollArea 变宽而变宽、变窄而变窄。在小窗口下内容可能溢出（出现水平滚动条），在大窗口下内容又显得局促（右侧大量留白）。

setWidgetResizable(bool resizable) 就是解决这个问题的。设为 true 之后，QScrollArea 会自动调整内容控件的宽度（对于垂直滚动区域）或高度（对于水平滚动区域）来匹配自身的视口大小。

```cpp
auto *scrollArea = new QScrollArea;
auto *content = new QWidget;
auto *contentLayout = new QVBoxLayout(content);

// ... 往 contentLayout 中添加很多控件 ...

scrollArea->setWidget(content);
scrollArea->setWidgetResizable(true);  // 内容宽度跟随 QScrollArea
```

setWidgetResizable(true) 的效果是：QScrollArea 在计算内容控件的大小时，会把视口（viewport）的可用宽度传给内容控件的 resizeEvent。内容控件内部的布局管理器会根据新的宽度重新排列子控件——QLabel 的自动换行会生效，QVBoxLayout 中的控件会拉伸到全宽。当内容控件的总高度超过视口高度时，垂直滚动条出现，但水平方向始终保持和视口一致——不会出现水平滚动条。

这几乎是所有表单页面和设置页面的标准配置。你希望表单的输入框、标签、按钮都撑满可用宽度，但在垂直方向可以滚动——setWidgetResizable(true) 加上 setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff) 就是标准做法。

有一个细节需要注意：setWidgetResizable(true) 只影响 QScrollArea 对内容控件宽度的调整，不会改变内容控件自身的 sizePolicy。如果你的内容控件中有一个 QLabel 设置了固定宽度（setFixedWidth(800)），那么即使 QScrollArea 努力把内容拉宽，这个 QLabel 也只会保持 800 像素。所以在使用 setWidgetResizable(true) 时，内容控件及其子控件的 sizePolicy 应该配合使用——让它们能自由伸缩，而不是锁死固定尺寸。

还有一个常见的误解是 setWidgetResizable 会让内容控件的高度也跟着视口变。实际上 setWidgetResizable 只调整一个方向——对于垂直滚动区域（绝大多数情况），它只调整宽度。内容控件的高度由其自身的布局和子控件决定。如果你的内容控件只有两个 QLabel，总高度 100 像素，QScrollArea 高度 500 像素，那内容区域就是上面 100 像素有内容、下面 400 像素空白——QScrollArea 不会把内容拉满。

### 3.3 动态添加内容后自动滚动到底部

很多应用有这种需求：一个聊天窗口或者日志面板，新的消息不断追加到底部，滚动条自动跟着往下滚——用户不需要手动拖滚动条就能看到最新的内容。这种"追加内容 + 自动滚动到底部"的交互模式在即时通讯、终端输出、日志查看器中非常常见。

实现方式是：每次向内容控件中添加新内容后，获取 QScrollArea 的垂直滚动条 QScrollBar，调用其 setValue 方法把它滚到最大值。

```cpp
auto *scrollArea = new QScrollArea;
scrollArea->setWidgetResizable(true);

auto *content = new QWidget;
auto *contentLayout = new QVBoxLayout(content);
scrollArea->setWidget(content);

// 追加一行内容并自动滚动到底部
auto *newLabel = new QLabel("新消息...");
contentLayout->addWidget(newLabel);

// 滚动到底部
QScrollBar *vBar = scrollArea->verticalScrollBar();
vBar->setValue(vBar->maximum());
```

这个方法在大多数情况下能正常工作，但有一个时序问题。当你 addWidget 给内容控件的布局添加新控件时，布局并不会立刻重新计算——Qt 的事件循环会在下一次 idle 时才处理布局更新。也就是说，你调用 setValue(vBar->maximum()) 的时候，滚动条的 maximum 可能还是旧值——因为新的内容控件还没被布局系统处理，内容高度还是之前的值。

解决方法是延迟滚动操作——让它在 Qt 处理完布局更新之后再执行。最简单的方式是用 QScrollBar::valueChanged 信号配合 QTimer::singleShot，或者直接用 QScrollArea 的 ensureVisible/ensureWidgetVisible 方法。

```cpp
// 方法一：ensureWidgetVisible（推荐）
// 先添加新控件到布局
auto *newLabel = new QLabel("新消息...");
contentLayout->addWidget(newLabel);

// 让 QScrollArea 确保这个新控件可见
scrollArea->ensureWidgetVisible(newLabel);
```

ensureWidgetVisible(QWidget *childWidget) 是 QScrollArea 提供的便捷方法——它会自动滚动内容区域，使得指定的子控件出现在视口中。这比手动操作滚动条更可靠，因为它不依赖时序——Qt 内部会在合适的时机执行滚动。

如果你想要"总是滚到最底部"的效果（而不是滚到某个特定控件），可以在内容最底部放一个空的"锚点"控件，每次添加新内容后 ensureWidgetVisible(锚点)。

```cpp
// 在内容布局最底部放一个锚点
auto *anchor = new QWidget;
contentLayout->addWidget(anchor);

// 每次添加新内容后滚到锚点
void appendMessage(const QString &text)
{
    auto *label = new QLabel(text);
    // 插入到锚点前面
    contentLayout->insertWidget(
        contentLayout->count() - 1, label);
    scrollArea->ensureWidgetVisible(anchor);
}
```

方法二是用 QTimer::singleShot 延迟执行滚动，给布局系统一个处理的机会。

```cpp
contentLayout->addWidget(new QLabel("新消息..."));
QTimer::singleShot(0, [vBar]() {
    vBar->setValue(vBar->maximum());
});
```

QTimer::singleShot(0, ...) 会把滚动操作放到事件队列的末尾——等当前事件处理完毕、布局更新完成后再执行。这种方法的缺点是引入了异步操作，如果在一帧内连续添加多条内容，可能只有最后一条的滚动生效。

### 3.4 自定义滚动条 QSS

默认的滚动条在不同平台上外观差异很大——Windows 上是传统的灰色条，macOS 上是半透明的覆盖式滚动条，Linux 上取决于 GTK 主题。如果你想统一应用在所有平台上的滚动条外观，或者只是觉得默认滚动条太丑，可以通过 QSS 完全自定义滚动条的视觉表现。

QSS 自定义滚动条涉及三个子控件：滚动条主体（QScrollBar）、滑块（QScrollBar::handle）和箭头按钮（QScrollBar::add-line / sub-line）。水平和垂直滚动条分别用 :horizontal 和 :vertical 伪状态来区分。

```css
/* 垂直滚动条整体 */
QScrollBar:vertical {
    background: #F0F0F0;
    width: 10px;
    margin: 0;
}

/* 垂直滚动条的滑块 */
QScrollBar::handle:vertical {
    background: #C0C0C0;
    min-height: 30px;
    border-radius: 5px;
}

/* 滑块悬浮 */
QScrollBar::handle:vertical:hover {
    background: #A0A0A0;
}

/* 滑块按下 */
QScrollBar::handle:vertical:pressed {
    background: #808080;
}

/* 隐藏上下箭头按钮 */
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0px;
}

/* 滚动条轨道（滑块上下的空白区域） */
QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
    background: none;
}
```

水平滚动条的 QSS 结构完全一样，只需要把 :vertical 换成 :horizontal、width 换成 height、min-height 换成 min-width。

```css
/* 水平滚动条整体 */
QScrollBar:horizontal {
    background: #F0F0F0;
    height: 10px;
    margin: 0;
}

/* 水平滚动条的滑块 */
QScrollBar::handle:horizontal {
    background: #C0C0C0;
    min-width: 30px;
    border-radius: 5px;
}

QScrollBar::handle:horizontal:hover {
    background: #A0A0A0;
}

QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
    width: 0px;
}

QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
    background: none;
}
```

这套 QSS 会让滚动条变成现代应用常见的"窄条 + 圆角滑块"风格——宽度只有 10 像素，滑块是圆角的灰色条，悬浮时变深，按下时更深。上下箭头按钮被隐藏（height 设为 0），轨道背景透明。这种风格在 VS Code、Telegram Desktop 等应用中非常常见。

如果你希望滚动条在不使用时自动隐藏、鼠标悬浮在 QScrollArea 上时才显示，可以通过 QScrollBar 的 opacity 来实现。不过 QSS 没有直接控制透明度的属性——你需要用 QPropertyAnimation 配合自定义的 graphicsEffect，或者干脆用 QWidget::setStyleSheet 在 enterEvent/leaveEvent 中切换不同的 QSS。

有一个实用的技巧是给 QScrollArea 自身设置边框和圆角，让整个滚动区域在视觉上更加独立。

```css
QScrollArea {
    border: 1px solid #DDD;
    border-radius: 6px;
    background-color: white;
}
```

注意，QScrollArea 的 QSS 只影响外层容器的边框和背景，不影响内部滚动条和内容控件的样式。滚动条需要单独设置 QSS。内容控件的 QSS 应该设置在内容控件自身或者 QScrollArea 的 viewport() 上。

## 4. 踩坑预防

第一个坑是 setWidget 的所有权问题。QScrollArea 会对传入 setWidget 的控件接管所有权——如果你之后再调用一次 setWidget(newWidget)，之前那个控件会被自动 delete。如果你在代码中持有旧控件的指针并继续使用它，就是 use-after-free。解决办法是每次 setWidget 之前先判断 widget() 是否已经是你要设置的控件，避免重复设置。

第二个坑是 setWidgetResizable(true) 搭配内容控件中的 fixedWidth/fixedHeight 子控件会导致自适应失效。如果你的内容控件中有一个 QLabel 设置了 setFixedWidth(600)，而 QScrollArea 的宽度只有 400 像素，结果就是内容控件被 QScrollArea 拉到 400 像素宽，但那个 QLabel 仍然保持 600 像素——超出 QScrollArea 的范围，水平方向出现溢出。所以在使用 setWidgetResizable(true) 时，应该避免在内容控件中使用固定宽度，改用 sizePolicy 和 stretch factor 来控制布局。

第三个坑是 ensureWidgetVisible 在内容控件的布局还没有更新时可能滚不到正确位置。这个方法依赖于内容控件的实际几何尺寸——如果布局还没有重新计算（比如你刚刚 addWidget 但还没有回到事件循环），ensureWidgetVisible 可能滚到的是旧位置。解决办法是用 QTimer::singleShot(0, ...) 延迟调用 ensureWidgetVisible，或者在添加大量内容后手动调用 content->layout()->activate() 强制布局更新。

第四个坑是 QSS 自定义滚动条在某些平台上可能不生效。比如 macOS 上 Qt 默认使用原生滚动条渲染，QSS 对原生滚动条无效。如果你在 macOS 上发现 QSS 没有作用，需要在 QApplication 构造之前设置环境变量 QT_ENABLE_GLYPHS_WORKAROUND=1 或者在代码中设置 Qt::AA_UseStyleInstallation attribute。更简单的做法是在 QApplication 初始化后统一用 QApplication::setStyle("Fusion") 切换到 Fusion 风格——Fusion 风格下 QSS 对滚动条完全生效。

## 5. 练习项目

我们来做一个综合练习：创建一个"消息日志查看器"窗口。中央是一个 QScrollArea，内部是一个 QVBoxLayout 的 QWidget，setWidgetResizable(true)。窗口顶部有一行输入框 QLineEdit 和一个"发送"按钮 QPushButton，点击发送后把 QLineEdit 中的文字作为一条新消息追加到 QScrollArea 的内容布局底部（用一个 QLabel 显示），然后通过 ensureWidgetVisible 自动滚动到最新消息。QScrollArea 使用 QSS 自定义滚动条——宽度 8 像素、圆角灰色滑块、隐藏箭头按钮、悬浮时滑块变深。底部有一个"清空消息"按钮，点击后删除所有消息 QLabel（遍历布局，用 removeItem + deleteLater 清理）。窗口大小设为 500x400，禁止水平滚动条。

提示：追加消息时，创建 QLabel 后设置 wordWrap(true) 以支持长文本自动换行。清空消息时需要注意不要删除布局中可能存在的弹簧（addStretch），可以用 qobject_cast<QLabel*> 来判断是否是消息标签。

## 6. 官方文档参考链接

[Qt 文档 -- QScrollArea](https://doc.qt.io/qt-6/qscrollarea.html) -- 滚动区域容器

[Qt 文档 -- QAbstractScrollArea](https://doc.qt.io/qt-6/qabstractscrollarea.html) -- 滚动区域基类

[Qt 文档 -- QScrollBar](https://doc.qt.io/qt-6/qscrollbar.html) -- 滚动条控件（QSS 自定义参考）

---

到这里，QScrollArea 的核心用法就全部讲完了。setWidget 让你把任意内容放入可滚动的容器中，setWidgetResizable(true) 确保内容宽度跟随容器自适应，动态追加内容后通过 ensureWidgetVisible 自动滚动到底部，QSS 让滚动条的外观完全可控。QScrollArea 是 Qt 里处理"内容超出可视区域"这个问题的标准答案——无论是表单页面、日志面板、聊天窗口还是图片浏览器，只要内容可能超出窗口大小，QScrollArea 就是第一选择。
