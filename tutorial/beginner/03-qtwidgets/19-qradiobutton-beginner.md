# 现代Qt开发教程（新手篇）3.19——QRadioButton：单选按钮

## 1. 前言 / 为什么单选按钮不像你想象的那么简单

QRadioButton 大概是所有 GUI 框架里最"看着简单但用起来有坑"的控件之一。表面上看，它就是一个圆形的选择框——选中就填充一个小圆点，没选就空着。你把它往界面上放几个，同一组里只能选一个，完事。但实际在 Qt 项目中，"同一组"这个概念本身就是一个需要仔细处理的命题。QRadioButton 的互斥机制依赖于 QObject 的 parent-child 层次关系，这意味着如果两个单选按钮的 parent 不同，它们天然就不在同一组——即使它们在视觉上看起来是挨在一起的。当你需要把分布在不同容器中的单选按钮归为一组时，就需要引入 `QButtonGroup` 来手动管理互斥关系。

还有一个容易忽略的点：QRadioButton 的 `toggled(bool)` 信号会在每次选中状态变化时触发，包括"从选中变成取消选中"的情况。如果你只关心"哪个按钮被选中了"而不关心"哪个按钮被取消了"，你需要在信号处理中做判断。这篇文章我们就把 QRadioButton 的四个核心问题讲清楚：Qt 的自动互斥机制是怎么工作的、QButtonGroup 如何跨 parent 实现互斥分组、`toggled(bool)` 信号的正确监听方式，以及如何用 QSS 把默认的方形复选框外观改成真正的圆形按钮。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QRadioButton 和 QButtonGroup 都属于 QtWidgets 模块，链接 Qt6::Widgets 即可，不需要额外引入其他模块。QRadioButton 在所有桌面平台上的行为一致，视觉外观由 QStyle 决定——不同主题下圆点的大小和颜色可能有微妙差异，但交互逻辑完全相同。

## 3. 核心概念讲解

### 3.1 自动互斥：同一 parent 下单选按钮天然互斥

QRadioButton 继承自 QAbstractButton，而 QAbstractButton 内部有一个 `autoExclusive` 属性。对于 QRadioButton，这个属性默认为 true。这意味着：当多个 QRadioButton 拥有同一个 parent Widget 时，它们自动构成一个互斥组——选中其中一个，其他的自动取消选中。

```cpp
auto *container = new QWidget();
auto *layout = new QVBoxLayout(container);

auto *radio1 = new QRadioButton("选项 A", container);
auto *radio2 = new QRadioButton("选项 B", container);
auto *radio3 = new QRadioButton("选项 C", container);

radio1->setChecked(true);  // 默认选中 A

layout->addWidget(radio1);
layout->addWidget(radio2);
layout->addWidget(radio3);
```

这三个按钮的 parent 都是 `container`，所以它们自动互斥。用户点击"选项 B"后，"选项 A"的选中状态会被自动取消。这个过程完全由 Qt 的 QAbstractButton 内部逻辑处理——当一个 `autoExclusive` 为 true 的按钮被 `setChecked(true)` 时，Qt 会遍历 parent 的所有子控件，找到同属 `autoExclusive` 的按钮，把它们全部 `setChecked(false)`。

这里有一个你可能踩的坑：`autoExclusive` 的范围是"同一个 parent 的所有子控件"，而不是"同一个 layout 内的所有控件"。在 Qt 中，layout 不拥有控件——控件是 parent Widget 的子对象。所以如果你把两个 QRadioButton 放在不同的 layout 中，但它们的 parent 是同一个 Widget，它们仍然互斥。反过来，如果你把两个 QRadioButton 放在同一个 layout 中，但它们的 parent 不同（通过 `setParent()` 手动改过），它们就不互斥。

另一个需要注意的地方：`autoExclusive` 只对 QRadioButton 默认为 true。QPushButton 和 QCheckBox 的 `autoExclusive` 默认为 false。如果你想让 QPushButton 或 QCheckBox 也实现类似单选的互斥行为，需要手动调用 `setAutoExclusive(true)`——但说实话，这种需求用 QButtonGroup 来实现更合理。

### 3.2 QButtonGroup 跨 parent 实现互斥分组

自动互斥在面对复杂界面时会遇到一个明显的短板：如果你的单选按钮分布在不同容器中（比如一个在 QGroupBox 里，另一个在 QFrame 里），它们的 parent 不同，自动互斥就失效了。这个时候你需要 `QButtonGroup`。

`QButtonGroup` 是一个不可见的逻辑分组容器——它不会在界面上显示任何东西，只负责管理一组按钮的互斥关系和选中状态。你把按钮 `addButton()` 进同一个 QButtonGroup，无论这些按钮的 parent 是谁，它们都会互斥。

```cpp
auto *group = new QButtonGroup(this);

auto *groupBox1 = new QGroupBox("外观设置");
auto *layout1 = new QVBoxLayout(groupBox1);
auto *lightRadio = new QRadioButton("浅色主题");
auto *darkRadio = new QRadioButton("深色主题");
layout1->addWidget(lightRadio);
layout1->addWidget(darkRadio);

auto *groupBox2 = new QGroupBox("高级设置");
auto *layout2 = new QVBoxLayout(groupBox2);
auto *autoRadio = new QRadioButton("跟随系统");
layout2->addWidget(autoRadio);

// 三个按钮在不同的 QGroupBox 中，parent 不同
// 自动互斥失效，但 QButtonGroup 让它们互斥
group->addButton(lightRadio, 0);
group->addButton(darkRadio, 1);
group->addButton(autoRadio, 2);
```

`addButton()` 的第二个参数是一个可选的整数 ID。给每个按钮分配一个 ID 之后，你可以通过 `group->checkedId()` 快速获取当前选中按钮的 ID，而不用挨个调用 `isChecked()` 来判断。这个 ID 在连接信号时特别方便。

```cpp
connect(group, &QButtonGroup::idClicked, this, [](int id) {
    switch (id) {
        case 0: qDebug() << "选中: 浅色主题"; break;
        case 1: qDebug() << "选中: 深色主题"; break;
        case 2: qDebug() << "选中: 跟随系统"; break;
    }
});
```

`QButtonGroup` 提供了三个选中相关的信号。`idClicked(int)` 在按钮被点击时触发，参数是按钮的 ID。`idToggled(int, bool)` 在按钮的选中状态变化时触发，第一个参数是 ID，第二个参数是是否选中。`buttonClicked(QAbstractButton *)` 跟 `idClicked` 类似，但参数是按钮指针而不是 ID。实际项目中 `idClicked(int)` 用得最多——你只需要一个整数就能确定用户选了什么，代码最简洁。

还有一个细节：当你把按钮加入 QButtonGroup 后，QButtonGroup 会自动把这些按钮的 `autoExclusive` 设为 true。所以如果你之前手动设置过 `autoExclusive`，加入 QButtonGroup 后会被覆盖。

QButtonGroup 默认是互斥的（`exclusive` 属性为 true）。如果你把它设为非互斥（`setExclusive(false)`），它就退化成一个纯粹的按钮集合——每个按钮可以独立选中/取消选中，行为跟 QCheckBox 一样。这个模式用得很少，但偶尔在"允许不选"的场景下有用——因为 QRadioButton 默认是"组内必须有一个被选中"的，而 `setExclusive(false)` 可以实现"一个都不选"的状态。

说到"允许不选"这个话题——QRadioButton 有一个不太直观的行为：一旦组内有一个按钮被选中，用户就无法通过点击来取消选中它（再点一次不会取消）。这是单选按钮的标准行为——它的语义是"从多个选项中选一个"而不是"可以选择或不选"。如果你需要"不选任何选项"的能力，要么在组内加一个"无"或"不适用"的 QRadioButton，要么使用 QButtonGroup 的 `setExclusive(false)` 配合手动状态管理。

### 3.3 toggled(bool) 信号监听状态变化

QRadioButton 从 QAbstractButton 继承了两个常用的信号：`clicked()` 和 `toggled(bool)`。`clicked()` 在按钮被鼠标点击或键盘空格键按下时触发。`toggled(bool)` 在按钮的选中状态发生变化时触发，参数 `true` 表示选中，`false` 表示取消选中。

对于 QRadioButton 来说，`toggled(bool)` 比 `clicked()` 更适合用来监听状态变化。原因是：当用户点击一个单选按钮时，实际上有两个按钮的状态发生了变化——新点击的按钮从"未选中"变为"选中"（toggled 收到 true），之前选中的按钮从"选中"变为"未选中"（toggled 收到 false）。如果你用 `clicked()` 来监听，你只会收到新按钮的信号；如果你用 `toggled()` 来监听，你会收到两个信号。

```cpp
auto *radioA = new QRadioButton("选项 A");
auto *radioB = new QRadioButton("选项 B");

connect(radioA, &QRadioButton::toggled, this, [](bool checked) {
    if (checked) {
        qDebug() << "选项 A 被选中";
    } else {
        qDebug() << "选项 A 被取消选中";
    }
});

connect(radioB, &QRadioButton::toggled, this, [](bool checked) {
    if (checked) {
        qDebug() << "选项 B 被选中";
    }
});
```

你会发现，用 `toggled` 信号时，绝大多数情况下你只关心 `checked == true` 的情况——你只想知道"哪个按钮被选中了"，而不关心"哪个按钮被取消了"。所以在 lambda 中加一个 `if (checked)` 判断是几乎必写的样板代码。

如果你觉得给每个按钮都连一个 toggled 信号太麻烦，QButtonGroup 提供了更优雅的方式。前面提到的 `idClicked(int)` 信号只在按钮被实际点击选中时触发（不会在取消选中时触发），而且直接给你按钮的 ID——这省去了 `if (checked)` 的判断。所以在实际项目中，推荐的模式是：用 QButtonGroup 管理按钮组，连接 `idClicked(int)` 信号，用 ID 来区分选项。

```cpp
auto *group = new QButtonGroup(this);
group->addButton(radioA, 0);
group->addButton(radioB, 1);

// idClicked 只在按钮被选中时触发，参数直接是按钮 ID
connect(group, &QButtonGroup::idClicked, this, [](int id) {
    qDebug() << "用户选中了 ID =" << id;
});
```

有一个特殊情况需要注意：`setChecked()` 是通过代码设置选中状态的，它会触发 `toggled(bool)` 信号，但不会触发 `clicked()` 信号。同样，QButtonGroup 的 `idToggled(int, bool)` 也会被触发，但 `idClicked(int)` 不会被触发——因为按钮不是被"点击"选中的，而是被程序设置选中的。所以如果你需要在代码初始化时就设置默认选中状态，并且不想触发响应逻辑，你需要在 `connect()` 之前调用 `setChecked()`。

### 3.4 自定义样式 QSS 圆形按钮美化

QRadioButton 的默认外观在不同平台上差异比较大。Windows 上是一个小圆形加右侧文字，macOS 上是蓝色填充的圆形，Linux 上取决于主题。大多数情况下默认外观就够了，但如果你需要自定义颜色、大小或者做出更精致的设计，就需要用 QSS 来美化。

QRadioButton 的 QSS 选择器支持几个子控件：`::indicator` 是圆形选择框本身，剩余的空间用来绘制文字。自定义样式时，你主要操作的就是 `::indicator`。

```cpp
radioButton->setStyleSheet(
    "QRadioButton::indicator {"
    "  width: 16px;"
    "  height: 16px;"
    "  border-radius: 8px;"        // 圆形：半径 = 宽度/2
    "  border: 2px solid #BDBDBD;"
    "  background-color: white;"
    "}"
    "QRadioButton::indicator:checked {"
    "  border-color: #1976D2;"
    "  background-color: #1976D2;"  // 选中时整个圆填充蓝色
    "}"
    "QRadioButton::indicator:hover {"
    "  border-color: #90CAF9;"
    "}"
);
```

这段 QSS 把 indicator 设为一个 16x16 的圆形，未选中时白色填充加灰色边框，选中时蓝色填充。`border-radius: 8px` 等于宽度的一半，确保 indicator 是正圆。

如果你想让选中状态保持外圈空心但内部加一个小圆点（类似标准单选按钮的样式），可以用 `background-color` 配合 `border` 来实现——外圈用白色背景加深色边框，选中时在内部绘制一个小圆点。但 QSS 本身不支持"在圆形内部再画一个更小的圆形"这种操作。要实现这个效果，你有两个选择：一是用 QSS 的 `image` 属性直接用图片替换 indicator（提供选中和未选中两种状态的图片），二是子类化 QStyle 并重写绘制逻辑。在实际项目中，用 `image` 属性加 SVG 图标是最简单高效的做法。

```cpp
radioButton->setStyleSheet(
    "QRadioButton::indicator {"
    "  width: 18px;"
    "  height: 18px;"
    "}"
    "QRadioButton::indicator:unchecked {"
    "  image: url(:/icons/radio_unchecked.svg);"
    "}"
    "QRadioButton::indicator:checked {"
    "  image: url(:/icons/radio_checked.svg);"
    "}"
);
```

还有一种常见的做法是把 QRadioButton 美化成"卡片式选择器"——把整个按钮（包括文字区域）做成一个带圆角边框的卡片，选中时边框高亮。这种样式在设置页面和选项表单中很流行。

```cpp
// 卡片式单选按钮（需要给每个按钮单独设置 objectName 以区分样式）
radioButton->setStyleSheet(
    "QRadioButton {"
    "  spacing: 8px;"
    "  padding: 12px 16px;"
    "  border: 2px solid #E0E0E0;"
    "  border-radius: 8px;"
    "  background-color: #FAFAFA;"
    "}"
    "QRadioButton:checked {"
    "  border-color: #1976D2;"
    "  background-color: #E3F2FD;"
    "}"
    "QRadioButton:hover {"
    "  border-color: #90CAF9;"
    "}"
    "QRadioButton::indicator {"
    "  width: 16px;"
    "  height: 16px;"
    "  border-radius: 8px;"
    "  border: 2px solid #BDBDBD;"
    "  background-color: white;"
    "}"
    "QRadioButton::indicator:checked {"
    "  border-color: #1976D2;"
    "  background-color: #1976D2;"
    "}"
);
```

QSS 美化 QRadioButton 时有一个容易被忽略的属性：`spacing`。`QRadioButton { spacing: 8px; }` 控制 indicator 和文字之间的间距。默认的间距在不同 QStyle 下不一样，如果你觉得文字离圆形太近或太远，调整 `spacing` 就行。

## 4. 踩坑预防

第一个坑是 parent 不同导致自动互斥失效。如果你把 QRadioButton 放在不同的 QGroupBox 或其他容器中，它们不会自动互斥。解决方案是用 QButtonGroup 手动分组。这个坑特别隐蔽，因为界面上看起来按钮是挨在一起的，但逻辑上它们不在同一组。

第二个坑是 `toggled(bool)` 在初始化时也会触发。如果你在构造函数中先 `connect()` 再 `setChecked()`，`setChecked()` 会触发 `toggled` 信号，你的处理逻辑会在初始化阶段被执行一次。如果你不希望这样，要么在 `connect()` 之前完成所有 `setChecked()` 调用，要么用一个 `m_initializing` 标志位在信号处理中跳过初始化阶段。

第三个坑是 QRadioButton 的"不可取消选中"行为。一旦组内有一个按钮被选中，用户无法通过点击来取消它。如果你的业务需求是"可以不选任何选项"，加一个"无"选项比改用其他控件更合理。

第四个坑是 QSS 美化时 `::indicator` 的尺寸。如果你在 QSS 中给 indicator 设置了固定宽高，但没有同时设置 `border-radius` 为宽度的一半，indicator 会变成方形而不是圆形——因为 QSS 的 `border-radius` 默认是 0。这是一个细节上的坑，`border-radius` 一定要手动设置。

第五个坑是 QButtonGroup 的 `checkedId()` 在没有任何按钮被选中时返回 -1。如果你在代码中用 `checkedId()` 来获取当前选项，记得处理返回 -1 的情况——否则你的 switch 语句可能走入意外的分支。

## 5. 练习项目

我们来做一个综合练习：创建一个"应用设置"窗口，展示 QRadioButton 的各种用法。窗口左侧是一个 QGroupBox "主题选择"，包含三个 QRadioButton（浅色、深色、跟随系统），它们在同一个 parent 下自动互斥。窗口右侧分为上下两个 QGroupBox，上方的 QGroupBox 叫"语言选择"包含两个 QRadioButton（中文、英文），下方的 QGroupBox 叫"字体大小"包含三个 QRadioButton（小、中、大）。左右两侧的按钮需要用 QButtonGroup 跨 parent 实现互斥——也就是说"主题选择"和"语言选择"的按钮是同一组（虽然分布在不同容器中），"字体大小"是另一组。窗口底部有一个 QLabel 实时显示当前所有选项的状态，以及一个 QPushButton "重置为默认值"。使用 QSS 给所有 QRadioButton 的 indicator 做圆形美化（自定义颜色和尺寸）。

几个提示：跨 parent 互斥用 QButtonGroup；实时状态显示连接 `QButtonGroup::idClicked` 信号；重置默认值用 `group->button(defaultId)->setChecked(true)`；QSS 圆形 indicator 的关键是 `border-radius: width/2`。

## 6. 官方文档参考链接

[Qt 文档 · QRadioButton](https://doc.qt.io/qt-6/qradiobutton.html) -- 单选按钮

[Qt 文档 · QButtonGroup](https://doc.qt.io/qt-6/qbuttongroup.html) -- 按钮分组

[Qt 文档 · QAbstractButton](https://doc.qt.io/qt-6/qabstractbutton.html) -- 按钮基类（autoExclusive / toggled / clicked）

[Qt 文档 · QStyle](https://doc.qt.io/qt-6/qstyle.html) -- 样式系统

---

到这里，QRadioButton 的四个核心问题就全部梳理清楚了。自动互斥机制让你在简单场景下不需要任何额外代码就能实现单选逻辑，QButtonGroup 在复杂界面中提供了跨 parent 的互斥分组能力，`toggled(bool)` 和 `idClicked(int)` 两套信号机制覆盖了从单个按钮到整组按钮的状态监听需求，而 QSS 则让你能把默认外观美化成圆形按钮或卡片式选择器。QRadioButton 看起来是最简单的控件之一，但用好它的互斥分组和状态监听需要的细节比你预想的多不少。
