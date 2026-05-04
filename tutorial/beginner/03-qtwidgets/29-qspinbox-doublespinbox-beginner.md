# 现代Qt开发教程（新手篇）3.29——QSpinBox / QDoubleSpinBox：数字步进框

## 1. 前言 / 当你需要用户输入一个精确的数字

我们在前面两篇分别讲了 QComboBox（下拉选择框）和 QFontComboBox（字体选择下拉框）。这两个控件解决的是"从有限候选集中选一个"的问题。但还有很多场景下你需要用户直接输入一个数值——图片缩放的百分比、日志保留的天数、导出图片的 DPI、网络请求的超时秒数、商品价格、屏幕坐标。这些场景的共同特征是：输入值是一个数字，有一定的取值范围，有一定的步进粒度。

你当然可以用一个 QLineEdit 加上 QIntValidator 或 QDoubleValidator 来做数字输入——我自己一开始也是这么干的。但很快你就会发现这种方案有一堆体验问题：用户可以输入非法字符（字母、多个小数点），你需要在槽函数里反复做合法性校验；没有上下箭头让用户快速微调数值；没有前缀后缀来提示输入的含义（比如"100 %"或者"$ 9.99"）；无法限制小数位数导致浮点精度问题层出不穷。QSpinBox 和 QDoubleSpinBox 就是 Qt 为解决这些问题而提供的专用控件。

QSpinBox 处理整数输入，QDoubleSpinBox 处理浮点数输入。它们都继承自 QAbstractSpinBox，共享同一套交互模式和 API 设计——上下箭头按钮（或者键盘上下键）按步进值增减数值，也可以直接在输入框里键入数字。我们今天把这两个控件的四个核心维度讲清楚：setRange / setSingleStep / setPrefix / setSuffix 设置数值约束和显示格式，setValue / value 进行取值与设值操作，QDoubleSpinBox::setDecimals 控制浮点精度，以及 valueChanged 信号的响应机制。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QSpinBox 和 QDoubleSpinBox 都属于 QtWidgets 模块，链接 Qt6::Widgets 即可。它们的核心数据类型是 int 和 double，属于 C++ 内建类型，不需要额外依赖。示例代码中用到了 QLabel 和 QTextEdit 来展示数值变化效果，这些同样在 QtWidgets 中。

## 3. 核心概念讲解

### 3.1 setRange / setSingleStep / setPrefix / setSuffix

我们从最基础的四件套开始——这四个方法定义了 SpinBox 的"行为边界"和"显示外观"。

`setRange(int min, int max)` 设定数值的取值范围。对于 QSpinBox，参数是 int 类型；对于 QDoubleSpinBox，参数是 double 类型。当用户通过上下箭头或键盘输入试图越过范围边界时，值会被钳位（clamped）到最近的有效值。比如你设了 setRange(0, 100)，用户在输入框里手动打了 200，当输入框失去焦点后值会自动回退到 100。这个钳位行为是通过 QSpinBox 内部的 validate 和 fixup 机制实现的——validate 检查输入是否合法，fixup 把不合法的输入修正为最近的有效值。

```cpp
auto *spinBox = new QSpinBox();
spinBox->setRange(0, 100);    // 值域 [0, 100]

auto *doubleSpinBox = new QDoubleSpinBox();
doubleSpinBox->setRange(-999.99, 999.99);  // 支持负值和小数
```

有一个替代写法是分别调用 `setMinimum(int)` 和 `setMaximum(int)`，效果和 setRange 一样。setRange 的好处是一行代码搞定两个边界，减少遗漏某个方向约束的可能性。默认的 range 是 (0, 99)——如果你不设范围，SpinBox 的值只能在 0 到 99 之间，这个默认值在实际开发中经常被忽略然后踩坑。

`setSingleStep(int)` 设定步进值——每次点击上下箭头或者按键盘上下键时数值变化的幅度。对于 QSpinBox 参数是 int，对于 QDoubleSpinBox 参数是 double。

```cpp
auto *spinBox = new QSpinBox();
spinBox->setRange(0, 1000);
spinBox->setSingleStep(10);  // 每次点击箭头 ±10

auto *doubleSpinBox = new QDoubleSpinBox();
doubleSpinBox->setRange(0.0, 1.0);
doubleSpinBox->setSingleStep(0.01);  // 每次点击 ±0.01
```

步进值的选取直接影响用户的操作效率。如果范围是 0 到 10000，步进值设为 1 的话用户要点 5000 次才能从中间调到最大值——这显然不现实。通常的做法是把步进值设为范围的 1% 到 5% 左右，或者根据业务含义选择一个合理的粒度（比如价格步进 0.01 元，天数步进 1 天，百分比步进 5%）。默认的 singleStep 是 1。

接下来是显示格式相关的两个方法。`setPrefix(const QString &)` 在数值前面添加固定文本，`setSuffix(const QString &)` 在数值后面添加固定文本。前缀和后缀纯粹是显示层面的——它们不会影响 value() 的返回值，也不会参与数值计算。

```cpp
auto *percentSpin = new QSpinBox();
percentSpin->setRange(0, 100);
percentSpin->setSuffix(" %");       // 显示为 "50 %"
// percentSpin->value() 返回 50，不包含 "%" 字符

auto *priceSpin = new QDoubleSpinBox();
priceSpin->setRange(0.0, 99999.99);
priceSpin->setPrefix("$ ");         // 显示为 "$ 29.99"
priceSpin->setDecimals(2);

auto *timeoutSpin = new QSpinBox();
timeoutSpin->setRange(1, 3600);
timeoutSpin->setSuffix(" 秒");       // 显示为 "30 秒"
```

前缀后缀在实际项目中的使用频率非常高，因为它们让用户一眼就能理解这个数字代表什么——没有单位的裸数字很容易引起歧义（"100"是 100 秒还是 100 毫秒？）。而且当你用 valueChanged 信号拿到数值的时候，前缀后缀已经被 SpinBox 自动剥离了，你拿到的是纯净的 int 或 double，不需要自己从字符串里解析。这是一个经常被忽视但用起来非常顺手的设计。

### 3.2 setValue / value 取值与设值

`setValue(int)` 是 QSpinBox 的 setter，`value()` 是对应的 getter。QDoubleSpinBox 的版本是 `setValue(double)` 和 `value()` 返回 double。这两个方法的用法直观到不需要解释，但有几个行为细节值得注意。

```cpp
auto *spinBox = new QSpinBox();
spinBox->setRange(0, 100);
spinBox->setValue(50);

int currentVal = spinBox->value();  // 50

// 如果设的值超出了 range，会被钳位
spinBox->setValue(200);
int clampedVal = spinBox->value();  // 100（被 maximum 钳位了）
```

setValue 时传入的值如果超出了 setRange 设定的范围，SpinBox 会自动把值钳位到最近的有效边界。setValue(200) 在 range (0, 100) 下的效果等同于 setValue(100)。这个钳位是静默发生的——不会报错，不会弹警告。如果你需要在设值时知道值是否被修改了，可以在 setValue 之后调用 value() 检查实际值是否和预期一致。

`setValue()` 会触发 `valueChanged` 信号。这一点和 QComboBox 的 setCurrentIndex 类似——程序设置值和用户操作都会触发同一个信号。如果你需要区分"程序主动设值"和"用户手动修改"，可以用 blockSignals(true) 临时屏蔽信号，或者用一个布尔标志位。

```cpp
// 方式 1: blockSignals
spinBox->blockSignals(true);
spinBox->setValue(50);      // 不触发 valueChanged
spinBox->blockSignals(false);

// 方式 2: 标志位
m_programmaticChange = true;
spinBox->setValue(50);
m_programmaticChange = false;

// 在槽函数中:
void onValueChanged(int val)
{
    if (m_programmaticChange) return;
    // 处理用户操作
}
```

在实际开发中，setValue 最常见的场景是从配置文件或网络请求中恢复之前的数值设置。比如你的应用有一个"设置"页面，启动时从 QSettings 读取上次的值：

```cpp
QSettings settings;
int savedTimeout = settings.value("network/timeout", 30).toInt();
timeoutSpin->setValue(savedTimeout);
```

这里第二个参数 30 是默认值——如果 QSettings 中没有存储过这个键，就会使用 30 作为初始值。

还有一个文本相关的 getter 是 `text()`，它返回 SpinBox 输入框中显示的完整文本，包括前缀、数值、后缀。而 `cleanText()` 返回的是去除前缀和后缀之后的纯数值文本。在大多数情况下你应该用 value() 而不是 text() 来获取数值，因为 value() 已经帮你做了字符串到数值的解析和类型转换。

### 3.3 QDoubleSpinBox::setDecimals() 控制小数位

QSpinBox 和 QDoubleSpinBox 之间最本质的区别在于精度——前者是整数，后者是浮点数。而控制 QDoubleSpinBox 精度的核心方法就是 `setDecimals(int)`，它设定显示和编辑的小数位数。

```cpp
auto *doubleSpin = new QDoubleSpinBox();
doubleSpin->setRange(0.0, 100.0);
doubleSpin->setDecimals(2);   // 显示两位小数，如 "3.14"
doubleSpin->setSingleStep(0.01);
```

setDecimals 的效果体现在两个方面。第一是显示精度：SpinBox 的输入框中会始终显示指定数量的小数位，不足的补零。比如 decimals 设为 2 时，值 3 会显示为 "3.00"，值 3.1 会显示为 "3.10"。第二是输入精度：用户在输入框中键入的数字会按照 decimals 指定的位数进行四舍五入。如果 decimals 是 2，用户输入 3.14159，最终 value() 返回的是 3.14。

```cpp
auto *doubleSpin = new QDoubleSpinBox();
doubleSpin->setDecimals(3);
doubleSpin->setValue(3.14159);
// value() 返回 3.142，text() 显示 "3.142"
```

默认的 decimals 是 2。这个默认值对货币场景（精确到分）刚好合适，但如果你做的是科学计算或者工程参数输入，可能需要更多小数位。decimals 的最大值没有硬性上限，但超过 10 位以后浮点精度本身就会出问题——double 类型只有大约 15 到 17 位有效数字，设太多位没有实际意义。

这里有一个很容易踩的坑：setDecimals 会影响 setSingleStep 的实际表现。如果你的 decimals 设为 0（整数模式），但 singleStep 设为 0.5，那每次点击箭头的步进效果会被四舍五入抹掉——点击上箭头可能从 3 变成 4 而不是 3.5，因为 decimals 为 0 时所有小数都被舍去了。所以在设 setDecimals 的时候，一定要确保 singleStep 的精度和 decimals 匹配。

还有一个和 QDoubleSpinBox 相关的格式化方法是 `setDecimals` 搭配 `setGroupSeparatorShown(bool)`。设为 true 后，SpinBox 会在大数值中自动插入千位分隔符（比如 1,000.00 而不是 1000.00）。这个分隔符会根据系统语言环境自动选择——中文环境下用逗号。

```cpp
auto *doubleSpin = new QDoubleSpinBox();
doubleSpin->setRange(0.0, 999999.99);
doubleSpin->setDecimals(2);
doubleSpin->setGroupSeparatorShown(true);
// 值为 12345.67 时显示为 "12,345.67"
```

### 3.4 valueChanged 信号响应

SpinBox 的核心信号是 `valueChanged`。对于 QSpinBox，信号签名是 `valueChanged(int)`；对于 QDoubleSpinBox，信号签名是 `valueChanged(double)`。每当 SpinBox 的值发生变化时——无论是用户点击了箭头、用户在输入框中键入了新值并确认，还是代码调用了 setValue——这个信号都会被触发。

```cpp
auto *spinBox = new QSpinBox();
spinBox->setRange(0, 100);

connect(spinBox, &QSpinBox::valueChanged,
        this, [](int value) {
    qDebug() << "SpinBox 值变为:" << value;
});

auto *doubleSpin = new QDoubleSpinBox();
doubleSpin->setRange(0.0, 1.0);
doubleSpin->setDecimals(3);
doubleSpin->setSingleStep(0.001);

connect(doubleSpin, &QDoubleSpinBox::valueChanged,
        this, [](double value) {
    qDebug() << "DoubleSpinBox 值变为:" << value;
});
```

这里有一个非常常见的陷阱：QSpinBox 和 QDoubleSpinBox 还有一个重载信号 `textChanged(const QString &)`，以及从 QAbstractSpinBox 继承的信号。如果你在连接信号时使用函数重载的方式（比如 `SIGNAL(valueChanged(int))`），在新式 connect 语法中需要特别注意类型匹配。现代 Qt 推荐的做法是使用编译期类型安全的 connect 语法：

```cpp
// 正确: 使用 static_cast 明确指定重载版本
connect(spinBox,
        static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
        this, [this](int value) {
    // 处理整数变化
});
```

不过在实际开发中，因为 QSpinBox 只有一个 valueChanged(int) 信号（textChanged 是另一个名字），所以直接用 `&QSpinBox::valueChanged` 通常不会产生歧义。真正容易出问题的是 QDoubleSpinBox——它的 valueChanged 信号有一个 double 参数版本，而有时候你可能不小心把槽函数的参数类型写成了 int，导致隐式截断丢失小数精度。

valueChanged 的触发时机需要特别留意。当用户在输入框中键入内容时，valueChanged 不是每敲一个字符就触发一次——它是在用户完成输入并确认后触发的（比如按下 Enter 键、Tab 键离开焦点、或者点击了上下箭头）。这意味着如果你需要"实时"监控用户的每一次键盘输入，valueChanged 不够用，你需要连接 QLineEdit 的 textEdited 信号（通过 spinBox->lineEdit() 获取内部 QLineEdit）。

```cpp
// 实时监控输入框中的文本变化
connect(spinBox->lineEdit(), &QLineEdit::textEdited,
        this, [spinBox](const QString &text) {
    qDebug() << "用户正在输入:" << text;
});
```

在实际项目中，valueChanged 最典型的应用模式是"联动"——一个 SpinBox 的值变化后，更新另一个控件的状态。比如一个"图像缩放"面板中有宽度 SpinBox 和高度 SpinBox，当用户勾选"锁定比例"并修改宽度时，高度应该自动按比例调整：

```cpp
connect(m_widthSpin, &QSpinBox::valueChanged,
        this, [this](int width) {
    if (m_lockRatioCheck->isChecked()) {
        int newHeight = static_cast<int>(
            width * m_aspectRatio + 0.5);
        m_heightSpin->blockSignals(true);
        m_heightSpin->setValue(newHeight);
        m_heightSpin->blockSignals(false);
    }
});
```

这里用 blockSignals 的原因是防止 heightSpin 的 valueChanged 又反过来触发 widthSpin 的更新，形成无限循环。这种"交叉更新"的场景在 SpinBox 联动中非常常见，blockSignals 是最干净利落的解法。

## 4. 踩坑预防

第一个坑是 setRange 的默认值是 (0, 99)。如果你忘记调用 setRange，SpinBox 的值被限制在 0 到 99 之间。这在大多数实际场景下都太小了，用户输入的值很容易被钳位。养成习惯：创建 SpinBox 后第一件事就是设 range。

第二个坑是 setValue 超出范围会被静默钳位。setValue(200) 在 range (0, 100) 下不会报任何错误，但实际值会被设为 100。如果你依赖 setValue 的"设了什么就是什么"语义，一定要确认 range 已经设对了。

第三个坑是 setDecimals 和 setSingleStep 的精度不匹配。如果 decimals 设为 0 但 singleStep 设为 0.5，步进效果会被舍入吞掉。确保 singleStep 的精度不低于 decimals 指定的小数位数。

第四个坑是 valueChanged 信号在 setValue 时也会触发。如果你在初始化阶段通过代码设值，信号处理代码会被意外执行。可以用 blockSignals 或者"先连接信号再设值"的策略来规避。

第五个坑是 QDoubleSpinBox 的浮点精度问题。double 类型在表示某些十进制小数时存在精度误差（比如 0.1 在二进制浮点中无法精确表示）。如果你需要精确的十进制运算（比如货币计算），不要直接拿 value() 的返回值做加减法，应该乘以 10 的幂次转换成整数后再计算，或者使用专门的十进制数学库。

## 5. 练习项目

我们来做一个综合练习：创建一个"图像导出设置面板"窗口，覆盖 QSpinBox 和 QDoubleSpinBox 的核心用法。窗口左侧是设置区域，包含：宽度 QSpinBox（范围 1-4096，默认 1920，后缀 " px"）、高度 QSpinBox（范围 1-4096，默认 1080，后缀 " px"）、一个"锁定宽高比"QCheckBox（默认勾选）、DPI QSpinBox（范围 72-1200，默认 300，后缀 " DPI"，步进 72）、缩放比例 QDoubleSpinBox（范围 0.1-5.0，默认 1.0，步进 0.1，decimals 1，后缀 "x"）、质量 QSpinBox（范围 1-100，默认 85，后缀 " %"）。窗口右侧是一个 QTextEdit（只读），实时显示当前的导出设置摘要——格式为"宽度 x 高度 @ DPI，缩放 Nx，质量 %"，外加估算的文件大小（简单用一个公式模拟即可）。每当任何一个 SpinBox 的值发生变化时，摘要区域自动更新。如果勾选了锁定宽高比，修改宽度时高度自动按比例调整，反之亦然。

几个提示：宽高比联动需要 blockSignals 防止循环更新；DPI 变化时不需要联动宽高；缩放比例变化时可以联动宽度（宽度 = 原始宽度 * 缩放比例）；初始化顺序要注意——先设完所有 range 和 singleStep，再设初始 value，最后连接信号，避免初始化过程中的信号乱飞。

## 6. 官方文档参考链接

[Qt 文档 · QSpinBox](https://doc.qt.io/qt-6/qspinbox.html) -- 整数步进框

[Qt 文档 · QDoubleSpinBox](https://doc.qt.io/qt-6/qdoubleSpinbox.html) -- 浮点数步进框

[Qt 文档 · QAbstractSpinBox](https://doc.qt.io/qt-6/qabstractspinbox.html) -- 步进框基类（共享 API）

[Qt 文档 · QAbstractSpinBox::ButtonSymbols](https://doc.qt.io/qt-6/qabstractspinbox.html#ButtonSymbols-enum) -- 按钮符号样式枚举

---

到这里，QSpinBox 和 QDoubleSpinBox 的四个核心维度我们就全部讲完了。setRange / setSingleStep / setPrefix / setSuffix 构成了 SpinBox 的行为和显示配置基础，setValue / value 是取值设值的标准接口，setDecimals 让 QDoubleSpinBox 可以精确控制浮点显示精度，valueChanged 信号则是所有数值联动逻辑的出发点。这两个控件虽然看起来简单——不就是一个带箭头的数字输入框嘛——但把它用对、用好，涉及到的细节比想象中多：范围钳位、信号触发时机、浮点精度、联动防循环，每一环都可能成为你调试半天的坑。掌握这些内容后，基本上所有需要用户输入数值的场景都能优雅地处理了。
