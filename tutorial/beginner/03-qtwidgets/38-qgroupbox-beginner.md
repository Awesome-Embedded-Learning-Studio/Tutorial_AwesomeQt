# 现代Qt开发教程（新手篇）3.38——QGroupBox：分组框

## 1. 前言 / 最不起眼但使用频率最高的容器

在 Qt 的控件体系里，QGroupBox 大概是最容易被忽视的一个。它不像 QPushButton 那样有点击交互，不像 QTableView 那样承载复杂的数据展示，也不像 QProgressBar 那样有动感的视觉效果。它的全部存在感就是一个带标题的边框——把几个相关的控件框在一起，告诉你"这些控件属于同一组"。但如果你翻开任何一个正经的 Qt 桌面应用，你会发现 QGroupBox 几乎无处不在：设置对话框里"网络配置"一组、"显示选项"一组、"快捷键绑定"一组，登录界面里"服务器信息"一组、"认证方式"一组。它做的事情很朴素——视觉分组，但正是这种朴素的组织能力让复杂界面不至于变成一锅乱炖。

QGroupBox 有两个比较有意思的特性值得单独讲。第一个是 setCheckable——把分组框变成一个可勾选的开关，勾选时组内控件全部启用，取消勾选时全部禁用。这个功能在做"高级选项"折叠面板、可选功能开关这种 UI 时特别实用。第二个是嵌套布局——QGroupBox 本身是一个容器，里面可以放任何布局和控件，也可以在布局里再嵌套 QGroupBox。但嵌套的姿势不对很容易搞出边距叠加、间距混乱的问题，这一块我们也会仔细讲清楚。

今天的内容分四个部分：QGroupBox 带标题边框的基本分组容器用法，setCheckable 可勾选分组框的启用/禁用联动机制，setAlignment 标题对齐的控制，以及嵌套布局的正确姿势。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QGroupBox 属于 QtWidgets 模块，链接 Qt6::Widgets 即可，不需要额外依赖。示例代码中用到了 QLabel、QLineEdit、QCheckBox、QRadioButton、QSpinBox、QComboBox、QVBoxLayout、QHBoxLayout、QGridLayout 和 QGroupBox。

## 3. 核心概念讲解

### 3.1 带标题边框的分组容器

QGroupBox 继承自 QWidget，它的核心功能就是在视觉上把一组控件框起来并标注一个标题。使用方式很直接：创建 QGroupBox 对象，通过 setTitle(const QString &) 设置标题文字，然后在 QGroupBox 上设置一个布局，把需要分组的控件放进这个布局里。

```cpp
// 创建一个"网络设置"分组框
auto *networkGroup = new QGroupBox("网络设置");
auto *layout = new QVBoxLayout(networkGroup);

layout->addWidget(new QLabel("服务器地址:"));
layout->addWidget(new QLineEdit);
layout->addWidget(new QLabel("端口号:"));
layout->addWidget(new QSpinBox);

// 把分组框加入主窗口的布局中
mainLayout->addWidget(networkGroup);
```

QGroupBox 默认会在控件外围画一个带圆角的边框，标题显示在边框的左上角（边框线上方，背景色遮住边框线的位置形成"标题嵌入边框"的效果）。这个视觉效果是 QGroupBox 区别于普通 QWidget 容器的关键——普通 QWidget 没有边框也没有标题，视觉上完全不可见。

QGroupBox 的标题可以为空。如果 setTitle("") 或者不调用 setTitle，QGroupBox 仍然会显示边框，但标题区域为空——边框线是完整的矩形，没有"嵌入标题"的缺口。这种用法在一些不需要文字标题但需要视觉分组的场景下很常见。

```cpp
auto *group = new QGroupBox;    // 无标题，只显示边框
auto *group2 = new QGroupBox("");  // 等价于无标题
```

setTitle 和 title() 分别用于设置和读取标题文字。如果你在运行时动态修改了标题，QGroupBox 会自动更新显示，不需要手动触发重绘。

边框的外观可以通过 QSS 进行定制。QGroupBox 的 QSS 支持比较丰富——你可以控制边框的样式、颜色、圆角、标题的字体和位置等。下面是一个常见的自定义样式：

```css
QGroupBox {
    font-weight: bold;
    border: 1px solid #CCCCCC;
    border-radius: 6px;
    margin-top: 10px;
    padding-top: 16px;
}

QGroupBox::title {
    subcontrol-origin: margin;
    subcontrol-position: top left;
    left: 12px;
    padding: 0 6px;
    color: #333333;
}
```

这里面有几个 QSS 属性值得解释。margin-top 给 QGroupBox 顶部留出空间来放标题文字——如果不设 margin-top，标题文字会和边框线重叠在一起。padding-top 控制的是 QGroupBox 内容区域（也就是你放的子控件）距离边框的距离，留够空间才不会让内容控件贴着边框。subcontrol-origin: margin 表示标题的定位基准是 margin 区域（而不是内容区域），subcontrol-position: top left 表示标题放在左上角。left: 12px 控制标题距离左边缘的偏移量。padding: 0 6px 在标题文字左右各留 6px 的间距，避免文字紧贴边框线。

QGroupBox::title 的 subcontrol-position 可以设为 top left、top center、top right 等位置，对应标题在边框上方的左/中/右对齐。但这个 QSS 属性和 setAlignment 是两套机制——setAlignment 在 C++ 层面控制标题对齐，QSS 的 subcontrol-position 在样式层面控制。如果两者同时设置了，QSS 的优先级更高。

### 3.2 setCheckable 可勾选分组框

QGroupBox 有一个很实用但经常被忽略的功能：setCheckable(bool)。设为 true 后，QGroupBox 的标题左侧会出现一个 QCheckBox 风格的勾选框。勾选时，组内的所有子控件处于启用状态；取消勾选时，所有子控件自动变为禁用状态（灰色、不可交互）。

```cpp
auto *proxyGroup = new QGroupBox("使用代理服务器");
proxyGroup->setCheckable(true);
proxyGroup->setChecked(false);  // 默认不勾选

auto *layout = new QVBoxLayout(proxyGroup);
layout->addWidget(new QLabel("代理地址:"));
layout->addWidget(new QLineEdit);
layout->addWidget(new QLabel("端口:"));
layout->addWidget(new QSpinBox);
```

上面这段代码创建了一个"代理设置"分组框。默认不勾选时，组内的 QLineEdit 和 QSpinBox 都是灰色禁用的，用户无法输入。勾选后，所有子控件自动变为可用。这个"勾选 → 子控件启用、取消 → 子控件禁用"的联动是 QGroupBox 内部自动处理的——它通过遍历所有子控件并调用 setEnabled(checked) 来实现，不需要你手动写代码。

setChecked(bool) 设置勾选状态，isChecked() 读取当前状态。toggled(bool) 信号在勾选状态变化时触发（和 QCheckBox 的信号同名，行为一致）。你可以在 toggled 信号的槽函数里做一些额外的逻辑——比如取消勾选时清空输入框的内容，或者勾选时设置默认值。

```cpp
connect(proxyGroup, &QGroupBox::toggled, this,
        [this](bool checked) {
    if (!checked) {
        // 取消勾选时清空输入
        m_proxyAddressEdit->clear();
        m_proxyPortSpin->setValue(8080);
    } else {
        // 勾选时设置默认代理地址
        m_proxyAddressEdit->setText("127.0.0.1");
    }
});
```

这里有一个容易踩的坑：setCheckable(true) 但 setChecked(true) 时（即默认勾选），子控件都是启用的。setCheckable(true) 且 setChecked(false) 时（即默认不勾选），子控件都是禁用的。但如果你在代码里手动调用了某个子控件的 setEnabled(true)，它在当前分组框不勾选的情况下也会被启用——QGroupBox 的启用/禁用联动只在 toggled 信号触发时生效，不会持续监听子控件的 enabled 状态变化。如果你之后需要精确控制单个子控件的启用状态，建议不用 setCheckable，改为手动用 QCheckBox + toggled 信号来管理。

另外一个细节是 setCheckable 的默认值为 false——也就是说普通的 QGroupBox 是不可勾选的，只显示边框和标题，没有勾选框。只有显式调用 setCheckable(true) 才会添加勾选框。checkable() 方法可以查询当前是否可勾选。

setCheckable 在一些 UI 模式下的表现略有不同。如果你的 QGroupBox 使用了自定义 QSS 并且覆盖了 QGroupBox::indicator（勾选框的样式），你需要自己绘制 indicator 的勾选/未勾选状态。默认的 indicator 使用平台原生的 QCheckBox 风格——Windows 上是一个小方框，macOS 上是一个圆形勾选框。

### 3.3 setAlignment 标题对齐

setTitle 在默认情况下显示在边框的左上角。但你可以通过 setAlignment(Qt::Alignment) 来改变标题的对齐位置。支持的对齐方式包括 Qt::AlignLeft（左对齐，默认）、Qt::AlignHCenter（水平居中）、Qt::AlignRight（右对齐），以及按位组合的 Qt::AlignLeft | Qt::AlignTop 等等。

```cpp
auto *group = new QGroupBox("标题");

group->setAlignment(Qt::AlignLeft);      // 左对齐（默认）
group->setAlignment(Qt::AlignHCenter);   // 居中
group->setAlignment(Qt::AlignRight);     // 右对齐
```

setAlignment 只影响标题文字的水平对齐，不影响垂直位置——标题始终在边框上方。alignment() 方法返回当前的对齐方式。

实际使用中，setAlignment(Qt::AlignHCenter) 是最常见的变体——有些 UI 设计规范偏好分组框标题居中显示。Qt::AlignRight 很少用到，但在某些从右到左（RTL）语言布局下可能会用到。

setAlignment 和 QSS 的 QGroupBox::title 的 subcontrol-position 会互相影响。如果 QSS 中设置了 subcontrol-position: top center，而 C++ 中 setAlignment(Qt::AlignLeft)，最终表现取决于 QSS 引擎的优先级。一般建议只在一处设置对齐方式，不要同时用 C++ 和 QSS 控制——容易造成混乱。

还有一个相关的属性是 setFlat(bool)。设为 true 后，QGroupBox 不显示边框，只显示标题。这种"扁平"模式下分组框的视觉存在感很弱，适合在不想要边框但想保留分组标题的场景下使用。

```cpp
auto *group = new QGroupBox("高级选项");
group->setFlat(true);  // 不显示边框，只显示标题
```

isFlat() 查询当前的扁平模式状态。setFlat 后 QGroupBox 的 QSS 边框样式不再生效，但标题样式仍然有效。扁平模式配合 setCheckable 使用时，勾选框仍然显示在标题旁边。

### 3.4 嵌套布局的正确姿势

QGroupBox 作为容器，可以嵌套使用——一个 QGroupBox 的布局中包含另一个 QGroupBox。这种嵌套在复杂的设置界面中非常常见：外层是"网络设置"分组框，内层嵌套"代理设置"和"DNS 设置"两个子分组框。

嵌套布局最大的问题是边距叠加。QGroupBox 默认有 contentsMargins（内边距），默认值大约是 (9, 9, 9, 9) 像素。如果你在 QGroupBox 里再放一个 QGroupBox，外层的内边距加上内层的内边距，再加上边框的宽度，叠加起来可能有 20-30 像素的浪费空间。嵌套三层以上时界面就会显得非常"空"，控件只占中间一小块。

解决方案是手动控制每层 QGroupBox 的 contentsMargins。外层保持默认或者适当加大，内层的 margins 要缩小——特别是嵌套层数较多时，内层 QGroupBox 的 margins 可以设为 (4, 4, 4, 4) 甚至 (2, 2, 2, 2)。

```cpp
// 外层分组框
auto *outerGroup = new QGroupBox("系统设置");
outerGroup->setContentsMargins(12, 16, 12, 12);

// 内层分组框（缩小边距）
auto *networkGroup = new QGroupBox("网络配置");
networkGroup->setContentsMargins(8, 12, 8, 8);

auto *displayGroup = new QGroupBox("显示选项");
displayGroup->setContentsMargins(8, 12, 8, 8);

auto *outerLayout = new QVBoxLayout(outerGroup);
outerLayout->addWidget(networkGroup);
outerLayout->addWidget(displayGroup);
```

QSS 中的 padding 和 C++ 的 contentsMargins 是叠加关系——QSS 的 padding 作用在边框内部，contentsMargins 作用在内容区域外围。如果你用 QSS 给 QGroupBox 设了 padding: 16px，再加上 C++ 的 contentsMargins(9, 9, 9, 9)，实际的内边距是 16 + 9 = 25 像素。嵌套时这个问题会更严重。建议在 QSS 中把 padding 设为 0 或者很小的值，把边距控制权交给 C++ 的 contentsMargins。

另外一个常见的嵌套问题是 spacing（布局间距）。QVBoxLayout 和 QHBoxLayout 默认的 spacing 大约是 6-8 像素。嵌套布局中，每一层布局的 spacing 都会在视觉上产生间隔。如果你觉得子分组框之间的间距太大，可以手动设 layout->setSpacing(4) 来缩小。

```cpp
auto *outerLayout = new QVBoxLayout(outerGroup);
outerLayout->setSpacing(8);  // 子分组框之间的间距

auto *innerLayout = new QVBoxLayout(networkGroup);
innerLayout->setSpacing(4);  // 网络配置组内控件间距更紧凑
```

最后一个关于嵌套的建议是：不要嵌套超过三层 QGroupBox。三层以上时界面会变得非常嵌套和空旷，用户体验很差。如果确实需要组织大量控件，考虑使用 QTabWidget（标签页切换）或者 QScrollArea + 扁平布局来替代深层嵌套。

## 4. 踩坑预防

第一个坑是 contentsMargins 和 QSS padding 叠加导致内边距过大。嵌套 QGroupBox 时一定要检查每一层的实际内边距，必要时手动设 setContentsMargins 或者 QSS 中设 padding: 0。

第二个坑是 setCheckable(false) 不会自动恢复子控件的 enabled 状态。如果你先 setCheckable(true) 再 setCheckable(false)，子控件的 enabled 状态停留在最后一次 toggled 信号触发的状态——不会自动恢复为全部 enabled。如果需要恢复，手动遍历子控件调用 setEnabled(true)。

第三个坑是 QGroupBox 的标题在 QSS 自定义时容易和 C++ 的 setAlignment 冲突。建议只在一处控制标题对齐——要么只用 setAlignment，要么只用 QSS 的 subcontrol-position，不要两处都设。

第四个坑是 setFlat(true) 后 QSS 中的边框样式不生效。flat 模式下 QGroupBox 不画边框，QSS 中的 border 属性被忽略。如果需要在扁平模式下加下划线之类的视觉分隔，需要用 QSS 的 border-bottom 或者额外的 QFrame 来实现。

第五个坑是嵌套层级过多导致界面空旷。QGroupBox 每层都有边框和内边距，嵌套三层以上控件区域会被严重压缩。如果界面复杂到需要三层以上分组，建议改用 QTabWidget 或者折叠面板。

## 5. 练习项目

我们来做一个综合练习：创建一个"应用设置"对话框，包含多个 QGroupBox 分组。窗口分为三个主要分组区域："基本设置"分组包含用户名 QLineEdit、语言选择 QComboBox、自动启动 QCheckBox；"网络配置"分组是一个 setCheckable(true) 的可勾选分组框，勾选后启用内部的代理地址 QLineEdit 和端口 QSpinBox；"界面偏好"分组嵌套了两个子分组框"主题设置"和"字体设置"，每个子分组框内各有自己的控件。外层分组框使用正常的内边距，内层子分组框缩小内边距避免空间浪费。所有 QGroupBox 通过 QSS 统一设置为蓝色边框、标题加粗的视觉风格。

提示：可勾选分组框通过 setCheckable(true) + setChecked(false) 实现，组内控件的启用/禁用是自动联动的。嵌套时内层 QGroupBox 调用 setContentsMargins(6, 14, 6, 6) 缩小边距。

## 6. 官方文档参考链接

[Qt 文档 -- QGroupBox](https://doc.qt.io/qt-6/qgroupbox.html) -- 分组框控件

[Qt 文档 -- QLayout](https://doc.qt.io/qt-6/qlayout.html) -- 布局基类（contentsMargins 和 spacing 的文档）

---

到这里，QGroupBox 的核心用法就全部讲完了。它虽然只是一个"带标题边框的容器"，但 setCheckable 让它能充当整组启用/禁用的开关，setAlignment 和 QSS 让标题的视觉位置灵活可控，嵌套布局在合理控制边距的前提下可以组织出层次分明的复杂界面。QGroupBox 不是什么高深的控件，但它是 Qt 界面布局的"骨架"——几乎所有需要把控件分组呈现的界面都会用到它。掌握它的边距控制、可勾选联动和 QSS 定制，你就能把设置对话框、选项面板这类常见界面做得整洁有序。
