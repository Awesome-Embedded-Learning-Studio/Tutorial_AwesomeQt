---
title: "3.36 QLCDNumber 进阶"
description: "入门篇我们把 QLCDNumber 的 display 三重载、setDigitCount 位数控制、setMode 进制切换、仪表盘场景都过了一遍。进阶篇要深挖的是那些看似简单的 API 背后藏着的行为边界。"
---

# 现代Qt开发教程（进阶篇）3.36——QLCDNumber 进阶

## 1. 前言 / 七段数码管的边界比你想象的窄

入门篇我们把 QLCDNumber 的 display 三重载、setDigitCount 位数控制、setMode 进制切换、仪表盘场景都过了一遍。说真的，QLCDNumber 的 API 表面上看起来简单得不能再简单了——display 一个值，设好位数，选好进制，就完事了。但如果你在工程中真的拿它做过仪表盘或者调试面板，你大概率踩过这些坑：明明 setDigitCount(5) 了但一个四位数就显示溢出，小数点的位置莫名其妙地偏移了一位，切换进制后显示的内容和预期完全对不上。

这篇文章要深挖的是那些看似简单的 API 背后藏着的行为边界：七段数码管的字形映射和 digitCount 的精确计算规则（含小数点和负号的占位），setSmallDecimalPoint 对字形布局的实际影响，三种 SegmentStyle 的视觉差异和渲染行为，以及 setMode 切换进制时内部存储值与显示值的分离机制。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17 标准。QLCDNumber 属于 QtWidgets 模块，不需要额外的模块依赖。

## 3. 核心概念讲解

### 3.1 七段数码管的字形映射与 digitCount 精确计算

入门篇我们知道了 setDigitCount 设置显示位数，display 超出范围会触发 overflow。现在我们要精确搞清楚的是"位数"到底怎么算的——因为这里的计算规则直接影响你该给 digitCount 设多大的值。

QLCDNumber 内部维护了一个字符槽位数组，每个槽位可以渲染一个七段数码管字符。关键在于"一个槽位"到底对应什么：对于数字 0-9 来说，每个数字占一个槽位，这没问题。但小数点、负号、冒号这些非数字字符呢？答案是：取决于 setSmallDecimalPoint 的设置。

当 setSmallDecimalPoint 为 false（默认值）时，小数点和负号各自独立占用一个槽位。也就是说 display(12.34) 在 digitCount(4) 的情况下会溢出——因为 "12.34" 有 5 个字符（1、2、.、3、4），需要 digitCount(5) 才放得下。display(-999) 需要 digitCount(4)（负号 + 三个数字）。

当 setSmallDecimalPoint 为 true 时，小数点不再独立占位，而是"挤"在相邻两个数字之间的缝隙里。此时 display(12.34) 只需要 digitCount(4) 就够了（1、2、3、4 四个数字，小数点插在 2 和 3 之间）。但负号仍然独立占位。

```cpp
auto *lcd = new QLCDNumber;

// smallDecimalPoint == false（默认）
lcd->setDigitCount(4);
lcd->display(12.34);  // 溢出！需要 5 个槽位
lcd->display(12.3);   // 正常，4 个槽位刚好

// 开启 smallDecimalPoint
lcd->setSmallDecimalPoint(true);
lcd->display(12.34);  // 正常，小数点不占位
lcd->display(-12.3);  // 溢出！负号仍占位，需要 5 个槽位
```

这里有一个容易搞混的地方：display(double) 在传入浮点数时，Qt 内部会先把 double 转换为字符串，转换的精度由当前 digitCount 决定。也就是说 digitCount 不仅控制"能放多少字符"，还会反过来影响"浮点数转字符串时保留多少位小数"。这个双向关系是很多人计算 digitCount 时出错的原因。

### 3.2 setSmallDecimalPoint 的字形布局影响

setSmallDecimalPoint 不仅影响小数点是否独立占位，还会改变七段数码管的字形布局方式。当 smallDecimalPoint 关闭时，小数点被渲染为一个完整的段，占据一个数字槽位的全部宽度——就像一个小的圆点站在一个独立的"房间"里。当 smallDecimalPoint 开启时，小数点被渲染在两个相邻数字之间的间隙中——它不是一个独立的槽位，而是借用右侧数字槽位的左边缘空间来绘制。

这个布局差异导致了几个实际影响。第一，开启 smallDecimalPoint 后，右侧数字的视觉位置会微微向右偏移，因为左侧空间被小数点占了一部分。在 digitCount 较大（比如 8 位以上）时这个偏移不太明显，但在 digitCount 较小（比如 3-4 位）时会比较显眼。第二，某些 QStyle 下开启 smallDecimalPoint 后小数点会渲染得非常小，在低分辨率屏幕上几乎看不见。如果你需要在不同平台和分辨率下保持一致的显示效果，建议实际编译测试。

```cpp
auto *lcd = new QLCDNumber;
lcd->setDigitCount(6);
lcd->setSmallDecimalPoint(true);

// 这些值在 smallDecimalPoint 开启时的实际显示
lcd->display(123.45);   // "123.45" — 5 个数字 + 1 个挤入的小数点
lcd->display(0.12345);  // "0.12345" — 刚好 6 个数字
lcd->display(0.123456); // 可能溢出，取决于精度截断
```

现在有一道调试题给大家。下面这段代码有什么问题？

```cpp
auto *lcd = new QLCDNumber;
lcd->setDigitCount(4);
lcd->setSmallDecimalPoint(true);
lcd->display(-9.9);
```

看起来 "-9.9" 只有两个数字、一个负号、一个小数点——开启 smallDecimalPoint 后小数点不占位，应该只需要 3 个槽位（负号 + 9 + 9）。但实际情况是：负号始终独立占位，所以需要 3 个槽位（负号 + 数字9 + 数字9），digitCount(4) 足够。这段代码实际上没有问题，能正常显示。那如果 display(-99.9) 呢？负号 + 9 + 9 + 小数点 + 9 = 负号(1) + 三个数字(3) = 4 个槽位，digitCount(4) 刚好放得下。但 display(-999.9) 就溢出了——负号 + 四个数字需要 5 个槽位。

### 3.3 SegmentStyle 三种风格

QLCDNumber 提供了三种段样式（SegmentStyle），通过 setSegmentStyle 设置，影响七段数码管的视觉外观。

QLCDNumber::Outline 是默认样式，每个线段只画轮廓边框，内部是空的或者浅色的。这种样式在浅色背景上看起来比较清晰，但在深色背景上线段的轮廓可能和背景融为一体。QLCDNumber::Filled 在 Outline 的基础上把线段内部填充为前景色，看起来像一个实心的发光数码管——这是工业仪表盘最常用的样式，配合深色背景和绿色/红色前景色效果很好。QLCDNumber::Flat 和 Filled 类似，但不画轮廓边框，线段直接以前景色扁平绘制，看起来更简洁但缺少立体感。

```cpp
auto *lcd = new QLCDNumber;
lcd->setSegmentStyle(QLCDNumber::Filled);
lcd->setStyleSheet("QLCDNumber { background-color: #1a1a2e; color: #00ff88; }");
lcd->setDigitCount(6);
lcd->display(1234.56);
```

三种样式的选择主要取决于你的界面风格。Outline 适合传统的 LCD 模拟效果，Filled 适合现代工业仪表盘风格，Flat 适合极简风格。QSS 的 color 属性影响线段的前景色（以及 Filled 模式下的填充色），background-color 影响整个控件的背景。

有一点需要注意：SegmentStyle 只影响七段数码管的线段绘制方式，不影响文字内容的显示。也就是说不管你选哪种 SegmentStyle，display(QString("HELLO")) 都能用同样的方式渲染字母——字母也是用七段线段拼出来的。

### 3.4 setMode 与内部存储值的分离

入门篇我们知道 setMode 可以切换十进制/十六进制/八进制/二进制显示。现在我们要搞清楚一个容易混淆的关键点：setMode 只影响显示，不影响内部存储的值。

QLCDNumber 内部用 double 存储最后一次 display(int) 或 display(double) 传入的值。当你调用 setMode(QLCDNumber::Hex) 时，QLCDNumber 不是把内部值转换成十六进制重新存储，而是每次需要渲染时用当前的 mode 去格式化那个 double 值。这意味着你可以随时切换 mode 而不会丢失原始值。

```cpp
auto *lcd = new QLCDNumber;
lcd->setDigitCount(8);
lcd->display(255);

lcd->setMode(QLCDNumber::Dec);  // 显示 "255"
qDebug() << lcd->value();       // 255.0

lcd->setMode(QLCDNumber::Hex);  // 显示 "FF"
qDebug() << lcd->value();       // 依然是 255.0，内部值没变

lcd->setMode(QLCDNumber::Oct);  // 显示 "377"
lcd->setMode(QLCDNumber::Bin);  // 显示 "11111111"
```

这里有一个重要的行为边界：setMode 对 display(QString) 传入的内容没有影响。如果你用 display(QString("HELLO")) 显示了一个字符串，然后调用 setMode，显示内容不会变化——因为 QString 模式不走进制转换逻辑。value() 在 display(QString) 之后返回 0，因为字符串内容无法转换为数值。

另外一个需要特别注意的边界是 display(double) 在非十进制模式下的行为。setMode(Hex) 后调用 display(3.14) 不会显示 "3.14" 的十六进制——它会把 3.14 截断为整数 3，然后显示十六进制的 "3"。十六进制浮点数表示不是 QLCDNumber 支持的功能。如果你需要在十六进制模式下显示带小数的值，唯一的办法是自己格式化好字符串然后用 display(QString)。

```cpp
auto *lcd = new QLCDNumber;
lcd->setDigitCount(8);
lcd->setMode(QLCDNumber::Hex);

lcd->display(255);      // 显示 "FF"
lcd->display(255.0);    // 也是 "FF"（255.0 截断为 255）
lcd->display(255.99);   // 还是 "FF"（截断为 255）

// 显示十六进制带前缀——只能用 QString
lcd->display(QString("0xFF"));  // 显示 "0xFF"
```

## 4. 踩坑预防

第一个坑是 value 超出 digitCount 静默溢出。QLCDNumber 不会自动扩展位数，超出范围就显示全横线并触发 overflow 信号。更坑的是 display(double) 的精度截断行为和 digitCount 互相影响，很容易算错需要的位数。解决方案是手动计算最大位数时采用一个简单公式：整数位数 + （可能有的负号 1 位）+ （如果 smallDecimalPoint 关闭则小数点 1 位）+ 小数位数 = 最小 digitCount，宁多勿少。

第二个坑是 smallDecimalPoint 改变后的布局偏移。开启 smallDecimalPoint 后小数点挤入数字间隙，右侧数字会微微偏移。在 digitCount 较大时不太明显，但在 3-4 位时偏移可能让数字看起来"歪了"。如果你对显示精度和视觉对齐有严格要求，建议实际运行测试各个值的表现。

第三个坑是 setMode 不改变存储值但改变 display(double) 的截断行为。setMode(Hex) 后 display(3.14) 会截断为整数 3 再显示十六进制——如果你期望看到浮点数的十六进制表示，这是不可能的，QLCDNumber 不支持。需要在非十进制模式下显示带小数的值时，必须自己格式化字符串用 display(QString)。

## 5. 练习项目

练习项目：多功能进制转换器面板。我们要实现一个窗口，中间是一个大号的 QLCDNumber（digitCount 设为 12），下方是四个 QPushButton 分别标注 "Dec"、"Hex"、"Oct"、"Bin"。用户通过一个 QSpinBox 输入一个 0-65535 的整数，LCD 实时显示对应的进制值。切换进制时自动调整 digitCount——十进制最多 5 位，十六进制最多 4 位，八进制最多 6 位，二进制最多 16 位。LCD 使用 Filled 段样式配深色背景绿色数字，同时在下方用独立的 QLabel 显示当前 mode 和 digitCount 的值，方便验证。

完成标准是：切换进制时 LCD 显示正确的值且不会溢出，digitCount 随进制自动调整，二进制模式下 16 位数字不溢出。提示几个关键点：每次切换 mode 前先调整 digitCount 再 display，二进制 16 位需要 setDigitCount(16)；用 value() 获取当前存储值而不是重新从 SpinBox 读取，这样可以验证 setMode 不改变内部值的行为。

## 6. 官方文档参考链接

[Qt 文档 · QLCDNumber](https://doc.qt.io/qt-6/qlcdnumber.html) -- 液晶数字显示控件，包含 SegmentStyle、Mode、smallDecimalPoint 属性说明

[Qt 文档 · QFrame](https://doc.qt.io/qt-6/qframe.html) -- QLCDNumber 的父类，frameShape 和 frameShadow 影响边框外观

---

到这里，QLCDNumber 的进阶内容就过了一遍。digitCount 的计算不是一个简单的"几位数字"问题——小数点是否独立占位取决于 smallDecimalPoint，负号永远独立占位，display(double) 的精度截断和 digitCount 互相影响。setSmallDecimalPoint 不仅改变占位规则，还会导致数字的视觉位置偏移。三种 SegmentStyle 各有适用场景，Filled 配深色背景是工业仪表盘的标配。setMode 与内部存储值完全分离，切换进制只影响显示格式，不影响 value() 的返回值。把这些边界搞清楚，QLCDNumber 在你的仪表盘里就不会再出幺蛾子了。
