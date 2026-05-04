# 现代Qt开发教程（新手篇）3.42——QSplitter：可拖动分割容器

## 1. 前言 / 那个藏在 IDE 和文件管理器背后的分割手柄

如果你用过任何一款代码编辑器——不管是 Qt Creator、VS Code 还是 Clion——你一定见过这种交互：窗口中间有一条可拖动的分割线，左边是文件树、右边是编辑区，用鼠标拽着那条线就能自由调节两侧的宽度比例。macOS 的 Finder 有它，Windows 的资源管理器有它，Outlook 的邮件列表和预览区之间还是它。这个看似不起眼的交互控件，几乎是所有"左右分栏"或者"上下分区"布局的核心。

Qt 提供的 QSplitter 就是这个功能的标准实现。它是一个容器控件，你往里面塞两三个子控件，QSplitter 会自动在它们之间画出分割线（handle），用户拖动分割线就能重新分配各子控件的空间大小。你可以选择水平分割（子控件从左到右排列）或者垂直分割（子控件从上到下排列），甚至可以嵌套使用——水平分割器里再嵌一个垂直分割器，就能搭出类似 IDE 那种"左上角文件树 + 左下角输出面板 + 右侧编辑区"的三区布局。

今天的内容分四个部分：水平分割和垂直分割的创建方式与嵌套技巧，setSizes/sizes 对各区域宽度进行程序化设置和读取，setCollapsible 禁止特定区域被完全折叠，以及 saveState/restoreState 把分割比例持久化到 QSettings 中。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QSplitter 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。示例代码中用到了 QSplitter、QTextEdit、QTreeWidget、QListWidget、QLabel、QPushButton、QVBoxLayout、QHBoxLayout 和 QSettings。

## 3. 核心概念讲解

### 3.1 水平分割与垂直分割

QSplitter 的构造函数接受一个 Qt::Orientation 参数来决定分割方向。传入 Qt::Horizontal 创建水平分割器——子控件从左到右排列，分割线是垂直的，用户左右拖动来调整宽度；传入 Qt::Vertical 创建垂直分割器——子控件从上到下排列，分割线是水平的，用户上下拖动来调整高度。

```cpp
// 水平分割：左侧文件树 + 右侧编辑区
auto *hSplitter = new QSplitter(Qt::Horizontal);
auto *treeWidget = new QTreeWidget;
auto *textEdit = new QTextEdit;
hSplitter->addWidget(treeWidget);
hSplitter->addWidget(textEdit);

// 垂直分割：上方列表 + 下方详情
auto *vSplitter = new QSplitter(Qt::Vertical);
auto *listWidget = new QListWidget;
auto *detailLabel = new QLabel("详情区域");
vSplitter->addWidget(listWidget);
vSplitter->addWidget(detailLabel);
```

往 QSplitter 中添加子控件有两种方式。第一种是用 addWidget(QWidget *widget)，控件会被追加到末尾。第二种是用 insertWidget(int index, QWidget *widget)，在指定位置插入。如果 QSplitter 已经有了一个子控件 A，你 insertWidget(0, B)，B 会出现在 A 前面。和 QStackedWidget 不同，QSplitter 的子控件在添加时不需要预先设定大小——QSplitter 会根据自身的可用空间和各子控件的 sizeHint 来分配初始比例。

QSplitter 的子控件数量没有硬性上限——你可以往一个水平分割器里塞五六个控件，每个之间都会有一条可拖动的分割线。不过从用户体验角度看，超过三个子控件的分割器操作起来会变得比较局促——分割线太密集，拖动时容易拽错位置。如果你的布局确实需要四五个区域，建议用嵌套分割器来组织：外层分割器把窗口分成两半，每一半内部再用自己的分割器继续细分。

嵌套分割器的写法很直观——把内层 QSplitter 当作一个普通的 QWidget 添加到外层 QSplitter 中即可。下面这段代码搭建了一个经典的 IDE 三区布局：左侧是文件树、右上角是代码编辑区、右下角是输出面板。

```cpp
auto *outerSplitter = new QSplitter(Qt::Horizontal);

auto *treeWidget = new QTreeWidget;
treeWidget->setHeaderLabel("项目文件");

// 右侧是一个垂直分割器，内部再分上下
auto *rightSplitter = new QSplitter(Qt::Vertical);
auto *editor = new QTextEdit;
auto *outputPanel = new QTextEdit;
outputPanel->setReadOnly(true);
rightSplitter->addWidget(editor);
rightSplitter->addWidget(rightPanel);

outerSplitter->addWidget(treeWidget);
outerSplitter->addWidget(rightSplitter);
```

这种嵌套方式在复杂布局中非常实用，因为它让每个 QSplitter 只负责一个方向上的分割，逻辑清晰，用户拖动时的行为也是可预测的。

有一个细节值得留意：QSplitter 在计算初始大小时，会按照各子控件的 sizeHint 比例来分配空间。如果你希望某个子控件在一开始就占据更大的比例，可以通过后续要讲的 setSizes 方法来精确控制，也可以给子控件设置 sizePolicy 或者 stretch factor。

### 3.2 setSizes / sizes 程序化控制各区域宽度

QSplitter 提供了 setSizes(const QList<int> &list) 和 sizes() 这对方法来让你精确控制各子控件的大小。setSizes 接受一个 int 列表，列表中第 N 个值对应第 N 个子控件的像素宽度（水平分割器）或高度（垂直分割器）。sizes() 返回当前各子控件的实际像素大小。

```cpp
auto *splitter = new QSplitter(Qt::Horizontal);
auto *left = new QTextEdit;
auto *right = new QTextEdit;
splitter->addWidget(left);
splitter->addWidget(right);

// 设置左侧 250 像素，右侧 550 像素
splitter->setSizes({250, 550});

// 读取当前各区域大小
QList<int> currentSizes = splitter->sizes();
// currentSizes[0] 是左侧宽度，currentSizes[1] 是右侧宽度
```

setSizes 的参数是像素值，但 QSplitter 内部实际上会根据子控件的拉伸因子（stretch factor）来做归一化分配。你可以通过 setStretchFactor(int index, int stretch) 来设置某个子控件的拉伸权重——效果和 QHBoxLayout 的 setStretch 类似。如果 left 的 stretch 是 1、right 的 stretch 是 3，那么无论窗口怎么缩放，右侧始终占据大约四分之三的空间。

```cpp
splitter->setStretchFactor(0, 1);  // 左侧占 1 份
splitter->setStretchFactor(1, 3);  // 右侧占 3 份
```

stretch factor 和 setSizes 的交互方式是：setSizes 设置的是初始大小，之后用户拖动分割线会改变实际大小，但当窗口整体缩放时，QSplitter 会按照 stretch factor 的比例来重新分配空间。如果你没有设置 stretch factor，QSplitter 默认在窗口缩放时保持各子控件的绝对像素大小不变——这意味着缩小窗口时右侧可能被挤到看不见。

一个常见的用法是在窗口初始化时用 setSizes 设置一个合理的初始比例，同时设置 stretch factor 来保证窗口缩放时比例不会跑偏。比如一个文件管理器的左侧目录树你希望初始宽度 200 像素，占总体宽度的四分之一：

```cpp
splitter->setSizes({200, 600});
splitter->setStretchFactor(0, 1);
splitter->setStretchFactor(1, 3);
```

还有一个容易忽略的细节：setSizes 的列表长度不一定非要和子控件数量一致。如果列表比子控件多，多余的值会被忽略；如果列表比子控件少，缺少的子控件会被分配默认大小。但为了代码的清晰性和可维护性，建议始终保持列表长度和子控件数量一致。

### 3.3 setCollapsible 禁止折叠

默认情况下，QSplitter 的每条分割线都可以被拖到极端位置——把某个子控件完全折叠成 0 宽度（或 0 高度）。这在某些场景下是合理的，比如一个可折叠的侧边栏。但在大多数应用中，你不希望用户把编辑区或者文件树拖没——一旦折叠了，新用户可能根本不知道那里原来还有个面板。

setCollapsible(int index, bool collapse) 就是用来控制这个行为的。传入 false 可以禁止指定索引的子控件被折叠。

```cpp
auto *splitter = new QSplitter(Qt::Horizontal);
splitter->addWidget(new QTreeWidget);  // 索引 0
splitter->addWidget(new QTextEdit);     // 索引 1

// 禁止两侧被完全折叠
splitter->setCollapsible(0, false);
splitter->setCollapsible(1, false);
```

设为 false 之后，用户拖动分割线时会发现——无论怎么用力拖，那个区域的最小宽度都不会小于该控件的 minimumSizeHint。如果你想进一步控制最小宽度，可以手动给子控件设置 setMinimumWidth。

```cpp
// 确保文件树至少 150 像素宽
treeWidget->setMinimumWidth(150);
splitter->setCollapsible(0, false);
```

collapsible 属性是按子控件索引设置的，你可以只禁止某些区域被折叠、允许其他区域被折叠。比如一个三区分割器中，你允许左侧导航面板折叠（折叠后整个窗口变成纯编辑区），但禁止中间的编辑区和右侧的属性面板折叠——这是很多绘图工具的标准行为。

还有一个和折叠相关的方法是 childrenCollapsible() 属性，通过 setChildrenCollapsible(bool) 统一设置所有子控件是否可折叠。如果你不希望任何子控件被折叠，一行 setChildrenCollapsible(false) 搞定，不需要逐个调用 setCollapsible。

```cpp
splitter->setChildrenCollapsible(false);  // 禁止所有子控件被折叠
```

有一个小坑需要注意：即使你 setCollapsible(index, false)，如果子控件自身的 minimumSize 被设为 0，QSplitter 仍然允许该区域被压缩到非常小的尺寸——只是不会变成完全的 0。所以如果你的意图是"这个区域至少要保持可见的尺寸"，别忘了同时给子控件设置一个合理的 minimumWidth 或 minimumHeight。

### 3.4 saveState / restoreState 持久化分割比例

用户花时间调好的分割比例，下次打开应用时应该被记住——这是桌面应用的基本体验要求。QSplitter 提供了 saveState() 和 restoreState(const QByteArray &state) 这对方法来实现这个功能，操作的对象是一个 QByteArray，你可以把它存到任何你喜欢的持久化介质中——最常见的是 QSettings。

saveState() 返回一个 QByteArray，里面编码了各子控件的相对大小和分割比例。注意这里说的是"相对大小"而不是绝对像素值——QSplitter 内部存储的是比例关系，不是像素数。这意味着你在分辨率为 1920x1080 的屏幕上保存的状态，恢复到 2560x1440 的屏幕上时，分割比例是一致的，而不是死板地还原像素值。

```cpp
// 保存状态到 QSettings
QSettings settings("MyCompany", "MyApp");
settings.setValue("splitterState", splitter->saveState());
```

恢复状态时调用 restoreState，传入之前保存的 QByteArray：

```cpp
// 从 QSettings 恢复状态
QSettings settings("MyCompany", "MyApp");
QByteArray savedState = settings.value("splitterState").toByteArray();
if (!savedState.isEmpty()) {
    splitter->restoreState(savedState);
}
```

restoreState 的返回值是 bool——如果恢复成功返回 true，如果传入的数据无效（比如格式不对、子控件数量不匹配）则返回 false。你可以在恢复失败时 fallback 到 setSizes 设置的默认值。

有一个非常重要的时序问题：restoreState 必须在所有子控件都被 addWidget 进去之后才能调用。原因很简单——saveState 编码的是"第 0 个子控件占多少、第 1 个子控件占多少"这样的索引关系，如果你在 restoreState 之前没有把所有子控件添加完毕，索引就对不上了，恢复出来的布局就是乱的。

```cpp
auto *splitter = new QSplitter(Qt::Horizontal);
splitter->addWidget(new QTreeWidget);   // 先添加完所有子控件
splitter->addWidget(new QTextEdit);

// 然后再恢复状态
QSettings settings("MyCompany", "MyApp");
splitter->restoreState(settings.value("splitterState").toByteArray());
```

嵌套分割器的状态也需要分别保存和恢复——外层分割器的 saveState 只编码了外层子控件的比例关系，不包含内层分割器的内部状态。所以如果你有一个两层嵌套的分割器布局，需要分别保存外层和内层的状态。

```cpp
QSettings settings("MyCompany", "MyApp");
settings.setValue("outerSplitter", outerSplitter->saveState());
settings.setValue("innerSplitter", rightSplitter->saveState());
```

还有一个细节是 saveState/restoreState 保存的不仅仅是大小比例，还包括分割线的位置、各子控件是否被折叠的状态。所以当你 restoreState 之后，之前 setCollapsible 的设置仍然有效——如果保存时某个面板是折叠的、恢复后该面板仍然被折叠，前提是你没有在 restoreState 之前把 setCollapsible 改成 false。

## 4. 踩坑预防

第一个坑是 restoreState 的时序问题。这一点前面已经反复强调了，但还是要再说一遍——必须在所有子控件都 addWidget 完毕之后再调用 restoreState。如果你在构造函数中先 restoreState 再 addWidget，恢复操作会静默失败，你看到的还是默认大小。这种 bug 的恶心之处在于不会报错、不会崩溃，就是分割比例不对，排查起来很费劲。

第二个坑是嵌套分割器的状态保存容易遗漏。如果你有三层嵌套的分割器布局，必须对每一层都分别调用 saveState 并存储到不同的 QSettings key 中。如果只保存了最外层，内层分割器的比例在重启后会回到默认值。建议用一个命名约定来管理——比如 key 名字用 "splitter/main"、"splitter/left"、"splitter/right" 这种层级结构，避免 key 冲突和遗漏。

第三个坑是 QSplitter 的分割线在 Fusion 风格下非常细，用户可能不太容易注意到它是可拖动的。默认的 handle 宽度只有几个像素，在高 DPI 屏幕上更是几乎看不见。如果你希望分割线更明显，可以通过 setHandleWidth(int) 设置一个更大的宽度，或者通过 QSS 给分割线的 handle 区域加上背景色。

```cpp
splitter->setHandleWidth(4);  // 把分割手柄加宽到 4 像素
```

QSS 方式：

```css
QSplitter::handle {
    background-color: #C0C0C0;
}
QSplitter::handle:horizontal {
    width: 4px;
}
QSplitter::handle:vertical {
    height: 4px;
}
```

第四个坑是 setSizes 在 QSplitter 尚未显示时调用的效果可能和预期不同。QSplitter 在 show 之前可能还没有正确的几何尺寸信息，setSizes 传入的像素值会被按比例映射到实际的可用空间中。如果你需要在窗口显示后立即设置精确的分割比例，可以考虑在 showEvent 中调用 setSizes，或者直接用 restoreState。

## 5. 练习项目

我们来做一个综合练习：创建一个模拟 IDE 布局的主窗口，采用三层嵌套的 QSplitter 结构。最外层是水平分割器，左侧放一个 QTreeWidget 充当项目文件树，右侧是一个垂直分割器。右侧垂直分割器的上半部分放一个 QTextEdit 充当代码编辑区，下半部分放一个 QTextEdit（只读）充当输出面板。文件树初始宽度 200 像素，输出面板初始高度 150 像素。三个区域都通过 setCollapsible 禁止折叠，文件树设置 setMinimumWidth(120) 保证最小可用宽度。窗口关闭时通过 saveState 把外层和内层分割器的状态都保存到 QSettings，下次启动时通过 restoreState 恢复。在窗口顶部添加一个按钮，点击后调用 setSizes 把布局重置为默认值（200 像素文件树、150 像素输出面板），覆盖用户之前的手动调整。

提示：重置默认值的按钮点击后，不需要清空 QSettings 中保存的状态——只是在当前会话中临时重置。用户下次关闭窗口时，当前状态（包括重置后的状态）会被重新保存。

## 6. 官方文档参考链接

[Qt 文档 -- QSplitter](https://doc.qt.io/qt-6/qsplitter.html) -- 可拖动分割容器

[Qt 文档 -- QSettings](https://doc.qt.io/qt-6/qsettings.html) -- 持久化存储（配合 saveState/restoreState）

---

到这里，QSplitter 的核心用法就全部讲完了。水平和垂直分割加上嵌套组合可以搭建出几乎所有常见的多区域布局，setSizes 让你有精确的初始大小控制能力，setCollapsible 防止用户把关键面板拖没，saveState/restoreState 配合 QSettings 实现了跨会话的布局持久化。QSplitter 是 Qt 里构建复杂界面布局的基石之一——掌握了它，再加上布局管理器和 QStackedWidget，基本上任何桌面应用的界面骨架你都能搭出来。
