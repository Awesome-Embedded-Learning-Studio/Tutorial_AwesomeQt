---
title: "3.42 QSplitter 进阶"
description: "入门篇我们用 QSplitter 搭了左右分栏布局，掌握了水平/垂直分割、addWidget、setSizes/sizes、setCollapsible、saveState/restoreState 持久化等基本操作。"
---

# 现代Qt开发教程（进阶篇）3.42——QSplitter 进阶

## 1. 前言 / 当那条分割线不只是"一条线"

入门篇我们用 QSplitter 搭了左右分栏布局，掌握了水平/垂直分割、addWidget、setSizes/sizes、setCollapsible、saveState/restoreState 持久化等基本操作。说实话，对于大多数桌面应用的分栏需求，入门篇的内容已经够用了。但一旦你开始追求视觉品质——比如想让分割手柄看起来像 VS Code 那种带悬停高亮的宽条而不是 Qt 默认那条几乎看不见的细线，或者想让某个面板在折叠时有一个平滑的收缩动画而不是瞬间消失——你就会发现 QSplitter 默认提供的定制能力相当有限。

今天我们把 QSplitter 的进阶能力拆透。核心内容是四个方面：通过子类化 QSplitterHandle 自定义手柄的绘制外观和行为，setStretchFactor 和 setSizes 配合实现最小宽度约束，handle(int index) 拿到手柄指针后的定制操作，以及嵌套分割器的状态恢复和尺寸协调。

## 2. 环境说明

本篇基于 Qt 6.5+ 版本，CMake 3.26+，C++17 标准。QSplitter 和 QSplitterHandle 都属于 QtWidgets 模块，链接 Qt6::Widgets 即可。自定义手柄绘制涉及 QPainter，需要 QtGui 模块（Qt6::Widgets 已包含）。所有行为在 Qt 6.9.1 上验证通过。

## 3. 核心概念讲解

### 3.1 子类化 QSplitterHandle 自定义手柄外观

QSplitter 的分割手柄默认渲染成一条 2-4 像素的细线（具体宽度取决于平台和 QStyle），在深色主题下几乎不可见。如果你想做一个 VS Code 风格的、有悬停效果、有拖动状态反馈的宽手柄，就需要子类化 QSplitterHandle 并重写 paintEvent。

QSplitterHandle 是 QSplitter 的内部子控件——你不能直接构造它，而是需要重写 QSplitter::createHandle() 方法来返回你的自定义手柄实例。QSplitter 每次需要在两个子控件之间插入手柄时都会调用 createHandle()。

```cpp
class CustomHandle : public QSplitterHandle
{
    Q_OBJECT

public:
    explicit CustomHandle(Qt::Orientation orientation,
                          QSplitter *parent = nullptr)
        : QSplitterHandle(orientation, parent)
        , m_hovered(false)
    {
        setMouseTracking(true);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        QColor bg = m_hovered ? QColor(80, 140, 220, 100)
                               : QColor(60, 60, 60);
        painter.fillRect(rect(), bg);

        // 画中间的拖动指示条
        QColor indicator = m_hovered ? QColor(80, 140, 220)
                                      : QColor(120, 120, 120);
        if (orientation() == Qt::Horizontal) {
            int x = width() / 2 - 1;
            painter.fillRect(x, 0, 2, height(), indicator);
        } else {
            int y = height() / 2 - 1;
            painter.fillRect(0, y, width(), 2, indicator);
        }
    }

    void enterEvent(QEnterEvent *) override
    {
        m_hovered = true;
        update();
    }

    void leaveEvent(QEvent *) override
    {
        m_hovered = false;
        update();
    }

private:
    bool m_hovered;
};

class CustomSplitter : public QSplitter
{
    Q_OBJECT

protected:
    QSplitterHandle *createHandle() override
    {
        return new CustomHandle(orientation(), this);
    }
};
```

手柄宽度通过 setHandleWidth(int) 设置。默认值取决于平台——Windows 上通常是 4 像素，Fusion 风格下可能是 2 像素。如果你需要更宽的拖动热区（比如触摸屏场景），设成 8-12 像素比较合适。注意 setHandleWidth(0) 会完全隐藏手柄——此时用户无法通过拖拽来调整分割比例，但你在代码中仍然可以通过 setSizes 程序化控制。

重写 createHandle 时有一个容易忽略的约束：你必须把 orientation() 传给 QSplitterHandle 的构造函数。orientation 决定了手柄的方向——水平分割器中的手柄是垂直的（上下的），垂直分割器中的手柄是水平的（左右的）。如果 orientation 传错了，手柄的鼠标拖动行为会完全反向。

### 3.2 setStretchFactor 和最小宽度约束

QSplitter 在分配空间时有一套优先级系统：先满足所有子控件的 minimumSizeHint，然后把剩余空间按 stretch factor 比例分配。setStretchFactor(int index, int stretch) 设置某个子控件的伸展因子——默认值是 0，表示"不主动要空间"。

但这里有一个很多人搞混的地方：stretch factor 只控制"多余空间怎么分"，不控制"最小尺寸是多少"。如果你的左侧面板需要至少 200 像素宽，右侧面板至少 400 像素，正确的做法是通过子控件自身的 setMinimumWidth 来约束，而不是靠 stretch factor。

```cpp
auto *splitter = new QSplitter(Qt::Horizontal);

auto *sidebar = new QWidget;
sidebar->setMinimumWidth(200);   // 最小宽度约束
auto *editor = new QWidget;
editor->setMinimumWidth(400);

splitter->addWidget(sidebar);
splitter->addWidget(editor);

// stretch factor 控制多余空间的分配：侧栏 1 份，编辑器 3 份
splitter->setStretchFactor(0, 1);
splitter->setStretchFactor(1, 3);
```

这两层约束是叠加的：minimumWidth 决定了"底线"，stretch factor 决定了"底线以上的多余空间怎么分"。当窗口缩小时，QSplitter 会优先保证所有子控件的 minimumSize，实在放不下时才会截断。这意味着如果你设了一个比较大的 minimumWidth，即使窗口很小，那个面板也不会被压缩到最小值以下。

setCollapsible(int index, bool) 控制的是用户能否把某个面板拖到完全折叠（宽度/高度变为 0）。默认情况下所有面板都可以折叠。如果你的某个面板有 minimumWidth 约束（比如设了 200），而 setCollapsible 没有禁用，用户仍然可以把面板拖到 0——minimumWidth 只约束 QSplitter 的自动分配，不约束用户的拖拽行为。如果你需要同时禁止用户拖拽折叠和保证最小宽度，必须同时设 setMinimumWidth 和 setCollapsible(index, false)。

### 3.3 handle() 获取手柄指针与事件拦截

QSplitter::handle(int index) 返回第 index 个手柄的指针。对于有 N 个子控件的 QSplitter，有 N-1 个手柄，索引从 1 开始——handle(1) 是第一个和第二个子控件之间的手柄，handle(2) 是第二个和第三个之间的，以此类推。这个接口让你可以在拿到手柄后做一些额外的操作，比如在手柄上安装事件过滤器或者直接调用手柄的 setCursor。

```cpp
// 获取手柄并设置鼠标光标
if (auto *h = splitter->handle(1)) {
    h->setCursor(Qt::SplitHCursor);  // 水平双向箭头
}

// 在手柄上安装事件过滤器追踪拖动
for (int i = 1; i < splitter->count(); ++i) {
    if (auto *h = splitter->handle(i)) {
        h->installEventFilter(this);
    }
}
```

在事件过滤器中你可以捕获手柄的鼠标按下/移动/松开事件，实现类似"拖动时实时预览布局效果"或者"拖动距离超过阈值才真正触发分割"的功能。但注意不要在事件过滤器中吞掉手柄的核心交互事件——如果你在 mousePressEvent 中返回了 true，手柄的拖动功能就被你截断了，用户完全无法拖动分割线。

### 3.4 嵌套分割器的状态恢复

入门篇讲了 saveState/restoreState 对单个 QSplitter 的持久化。但实际项目中更常见的是嵌套分割器——水平分割器里嵌一个垂直分割器，形成"左 | 右上 + 右下"的三区布局。这种嵌套情况下 saveState 必须在每个 QSplitter 上分别调用，不能只在最外层调一次。

```cpp
// 保存嵌套分割器状态
QSettings settings("MyOrg", "MyApp");
settings.setValue("main_splitter", mainSplitter->saveState());
settings.setValue("right_splitter", rightSplitter->saveState());

// 恢复时必须按从内到外的顺序
rightSplitter->restoreState(
    settings.value("right_splitter").toByteArray());
mainSplitter->restoreState(
    settings.value("main_splitter").toByteArray());
```

恢复顺序很重要。如果你先恢复了外层的 mainSplitter，外层分割器会按照保存的状态设置子控件的大小——但此时内层的 rightSplitter 还没有恢复，它的子控件尺寸是默认值。外层分配给 rightSplitter 的空间可能和 rightSplitter 恢复后的需求不一致，导致布局闪跳。正确的做法是先恢复内层，再恢复外层。

restoreState 有一个隐含的限制：它只在 QSplitter 的子控件数量和保存时一致的情况下才能正确恢复。如果你在两次运行之间添加或删除了一个子控件，restoreState 会静默失败（返回 false），分割器回到默认布局。如果你的应用有"可选面板"的功能——用户可以在设置中启用或禁用某些面板——必须在恢复状态之前先确定最终的子控件列表，然后再调用 restoreState。

## 4. 踩坑预防

第一个坑是 setCollapsible 和 minimumSize 之间的行为差异。setCollapsible(index, false) 禁止用户通过拖拽把手柄把面板折叠到 0，但它不会自动给面板设最小尺寸。反过来，setMinimumWidth(200) 约束了 QSplitter 的空间分配，但不阻止用户把手柄拖到 0。如果你需要"面板有最小尺寸且不能被折叠"，必须同时设两者。只设一个的后果是用户仍然能把面板拖成一条缝，或者面板虽然有最小尺寸但用户可以把它拖没。

第二个坑是 restoreState 在子控件数量变化时静默失败。如果你在上次保存时有 3 个子控件，这次运行时只 addWidget 了 2 个（比如某个面板在设置中被禁用了），restoreState 会返回 false 并且不做任何恢复。如果你没有检查返回值，用户看到的就是默认布局，而不是上次保存的布局。解决方案是在 restoreState 返回 false 时用 setSizes 设置一个合理的默认布局。

第三个坑是 setHandleWidth 设得太小导致触摸设备上几乎无法拖动。桌面鼠标的精度足够点中 2-4 像素的手柄，但触摸屏上手指的热区通常至少需要 12-16 像素。如果你的应用需要支持触摸操作，把手柄宽度设到至少 8 像素，或者用自定义手柄在 paintEvent 中画一个视觉上细但点击热区宽的手柄。

## 5. 练习项目

练习项目：IDE 风格三区布局面板。我们要实现一个具备完整分割管理能力的面板。

完成标准是：最外层一个水平 QSplitter，左侧是文件树面板（QTreeWidget），右侧是一个垂直 QSplitter 嵌套。右侧垂直 QSplitter 上半部分是代码编辑区（QPlainTextEdit），下半部分是输出面板（QTextEdit）。所有分割手柄使用自定义外观——悬停时变蓝色，默认灰色，手柄宽度 6 像素。左侧面板最小宽度 200 且不可折叠，代码编辑区最小高度 300，输出面板最小高度 100。三个面板的伸展因子分别是 1:3:1。程序关闭时保存两层 QSplitter 的状态到 QSettings，启动时恢复。恢复失败时（比如面板数量变化）使用默认尺寸比例 250:500:150。

提示几个关键点：createHandle 里传 orientation()，水平分割器的手柄方向是 Qt::Horizontal；嵌套分割器的 restore 顺序是从内到外；判断 restoreState 返回值来决定是否用 setSizes 设默认值。

## 6. 官方文档参考链接

[Qt 文档 · QSplitter](https://doc.qt.io/qt-6/qsplitter.html) -- 分割容器控件，包含 setSizes/setStretchFactor/saveState/restoreState 等接口

[Qt 文档 · QSplitterHandle](https://doc.qt.io/qt-6/qsplitterhandle.html) -- 分割手柄控件，用于自定义手柄外观

[Qt 文档 · QSplitter::createHandle](https://doc.qt.io/qt-6/qsplitter.html#createHandle) -- 创建自定义手柄的虚函数

---

到这里，QSplitter 的进阶内容就拆完了。子类化 QSplitterHandle 并重写 createHandle 是自定义手柄外观的标准路径——你可以在 paintEvent 里画任何你想要的效果。setStretchFactor 控制多余空间的分配，setMinimumWidth 控制底线，两者必须配合使用。嵌套分割器的状态恢复要注意从内到外的顺序和子控件数量的一致性。把这些机制搞透后，你就能做出专业级的分栏布局——而不只是"能拖但丑"的分割界面。
