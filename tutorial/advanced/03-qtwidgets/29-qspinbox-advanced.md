---
title: "3.29 QSpinBox 进阶"
description: "入门篇我们把 QSpinBox 和 QDoubleSpinBox 当作带箭头的数字输入框来用——setRange、setValue、valueChanged 信号，三板斧就能覆盖大多数场景。"
---

# 现代Qt开发教程（进阶篇）3.29——QSpinBox 进阶

## 1. 前言 / 什么时候你需要重写 textFromValue

入门篇我们把 QSpinBox 和 QDoubleSpinBox 当作带箭头的数字输入框来用——setRange、setValue、valueChanged 信号，三板斧就能覆盖大多数场景。但如果你做过稍微专业一点的应用——比如一个角度输入框要显示度数符号、一个内存大小选择器要用 KB/MB/GB 自动切换单位、或者一个时间输入框要用时:分格式显示分钟数——你很快就会发现 prefix/suffix 的表达能力远远不够。prefix 和 suffix 是固定文本，它们不能根据当前值动态变化。

这就是 textFromValue 和 valueFromText 存在的意义。它们是一对虚函数，分别负责"把数值转成显示文本"和"把显示文本解析回数值"。默认实现只做简单的数字转字符串和字符串转数字，但你可以通过子类化重写它们来实现任意复杂的显示格式。配合 stepEnabled 的精确控制，你可以打造出完全定制化的数值输入体验。本篇我们就把这三件事搞透：textFromValue/valueFromText 的自定义格式化、stepEnabled 的步进控制策略，以及 prefix/suffix 在自定义格式化下的边界问题。

## 2. 环境说明

本篇基于 Qt 6.5+ 版本，CMake 3.26+，C++17 标准。依赖 QtWidgets 模块（QSpinBox、QDoubleSpinBox、QAbstractSpinBox）。所有内容不需要额外链接。

## 3. 核心概念讲解

### 3.1 textFromValue / valueFromText 自定义格式化

QSpinBox 的 textFromValue(int value) 是一个虚函数，它的职责是把内部的整数值转换成显示在输入框中的文本。默认实现就是 QString::number(value)，再加上 prefix 和 suffix。valueFromText(const QString& text) 是它的逆操作——把输入框中的文本解析回整数值。

要自定义 SpinBox 的显示格式，我们需要子类化 QSpinBox 并重写这两个函数。来看一个实际的例子：角度输入框，值范围 0-359，显示时带度数符号，但度数符号不是 suffix 而是集成在格式化逻辑中。

```cpp
class AngleSpinBox : public QSpinBox
{
public:
    using QSpinBox::QSpinBox;

protected:
    QString textFromValue(int value) const override
    {
        return QString::number(value) + QChar(0x00B0);  // ° 符号
    }

    int valueFromText(const QString& text) const override
    {
        // 去掉度数符号后解析
        QString cleaned = text;
        cleaned.remove(QChar(0x00B0));
        return cleaned.toInt();
    }
};
```

这里有一个关键的设计原则：textFromValue 和 valueFromText 必须互为逆运算。也就是说，对于任意合法值 v，valueFromText(textFromValue(v)) 必须等于 v。如果你只重写了一个而忘记重写另一个，SpinBox 的输入验证会出问题——用户手动输入的文本无法被正确解析回数值，导致 validate 失败，输入被拒绝。

再来看一个更复杂的例子：内存大小选择器。值为字节数（整数），但显示时根据大小自动选择 KB、MB 或 GB 作为单位。

```cpp
QString MemorySpinBox::textFromValue(int bytes) const
{
    if (bytes >= 1073741824) {  // >= 1 GB
        return QString::number(bytes / 1073741824.0, 'f', 1) + " GB";
    } else if (bytes >= 1048576) {  // >= 1 MB
        return QString::number(bytes / 1048576.0, 'f', 1) + " MB";
    } else {
        return QString::number(bytes / 1024.0, 'f', 1) + " KB";
    }
}

int MemorySpinBox::valueFromText(const QString& text) const override
{
    if (text.endsWith("GB")) {
        return static_cast<int>(
            text.chopped(3).trimmed().toDouble() * 1073741824);
    } else if (text.endsWith("MB")) {
        return static_cast<int>(
            text.chopped(3).trimmed().toDouble() * 1048576);
    } else {
        return static_cast<int>(
            text.chopped(3).trimmed().toDouble() * 1024);
    }
}
```

这个例子中，textFromValue 返回的文本包含动态变化的单位（KB/MB/GB），这是 suffix 根本做不到的——suffix 是固定不变的。但代价是 valueFromText 的解析逻辑要处理多种格式，复杂度显著增加。

### 3.2 stepEnabled 控制步进按钮状态

QAbstractSpinBox 提供了一个虚函数 stepEnabled()，它返回一个 StepEnabledFlag 的组合，用于控制上下箭头按钮是否可用。默认实现会根据当前值是否已到达 minimum 或 maximum 来决定——如果当前值等于 minimum，向下箭头不可用；如果等于 maximum，向上箭头不可用。

你可以重写 stepEnabled 来实现更复杂的步进控制策略。比如，在 wrapping 为 false 的前提下，你想让某个范围的值不允许向上步进但允许向下步进：

```cpp
QAbstractSpinBox::StepEnabled CustomSpinBox::stepEnabled() const
{
    StepEnabled flags;
    if (value() > minimum()) {
        flags |= StepDownEnabled;
    }
    // 某些条件下禁止向上步进
    if (value() < someThreshold() && value() < maximum()) {
        flags |= StepUpEnabled;
    }
    return flags;
}
```

StepEnabled 有两个枚举值：StepUpEnabled（向上箭头可用）和 StepDownEnabled（向下箭头可用）。返回空的 flags 会让两个箭头都灰掉——用户只能通过键盘输入修改值，不能用箭头步进。

现在有一道调试题给大家。你重写了 stepEnabled，让它返回 StepDownEnabled | StepUpEnabled，但发现向上箭头仍然是灰的。检查 value() 确实小于 maximum()。问题可能出在哪里？

问题出在 wrapping 属性上。如果 setWrapping(true) 被设置了，stepEnabled 的默认行为会改变——wrapping 模式下，即使到达了 maximum，向上箭头也应该可用（因为会回绕到 minimum）。但如果你重写了 stepEnabled 而没有考虑 wrapping，你的实现可能和基类的其他逻辑产生冲突。另外，如果你在 stepEnabled 中没有正确处理边界条件——比如在 value() == maximum() 时仍然返回了 StepUpEnabled——QSpinBox 内部的 fixup 机制会把超出范围的值钳回去，但箭头按钮的视觉状态可能和实际行为不一致。

还有一个值得注意的行为：stepEnabled 是每次 SpinBox 需要更新按钮状态时都会调用的。这意味着它在 valueChanged 之后会被调用，在 setRange 之后也会被调用。如果你在 stepEnabled 中做了耗时的计算，会直接影响 SpinBox 的响应速度。

### 3.3 prefix/suffix 解析的边界情况

入门篇我们说过 prefix 和 suffix 纯粹是显示层面的，不影响 value() 的返回值。这个说法在默认情况下是对的，但在自定义 valueFromText 时需要格外小心。

问题在于：当 SpinBox 需要解析用户输入时，它传给 valueFromText 的 text 参数包含了 prefix 和 suffix。比如你设了 setPrefix("$ ")，用户输入了 "$ 50"，valueFromText 拿到的就是 "$ 50" 这个完整字符串。默认的 valueFromText 实现会自动剥离 prefix 和 suffix，然后对中间的数值部分做 toInt()。但如果你重写了 valueFromText，你就需要自己处理 prefix 的剥离。

```cpp
int PriceSpinBox::valueFromText(const QString& text) const
{
    QString cleaned = text;
    // 必须手动剥离 prefix，否则 "$ 50" 解析为 0
    if (!prefix().isEmpty()) {
        cleaned.remove(0, prefix().length());
    }
    if (!suffix().isEmpty()) {
        cleaned.chop(suffix().length());
    }
    return cleaned.trimmed().toInt();
}
```

如果你在重写 valueFromText 时忘记了剥离 prefix，"$ 50" 被 toInt() 解析会返回 0（因为 $ 不是数字字符），然后 SpinBox 的 fixup 机制会把值修正到 minimum 或保持不变，用户输入的 50 就"丢失"了。这个 bug 非常隐蔽，因为看起来一切正常——输入框显示着 "$ 50"，但 value() 返回的是 0 或 minimum。

更安全的做法是：如果你要自定义格式化逻辑，干脆不要用 prefix 和 suffix，把所有格式化逻辑都放在 textFromValue 中完成。这样 textFromValue 完全控制输出格式，valueFromText 完全控制输入解析，不需要和 prefix/suffix 的自动剥离逻辑做交互。

## 4. 踩坑预防

第一个坑是重写 valueFromText 时忘记剥离 prefix/suffix。默认的 valueFromText 会自动处理 prefix 和 suffix 的剥离，但一旦你重写了这个函数，剥离的责任就落在了你身上。如果 valueFromText 拿到的文本包含 prefix（比如 "$ 50"）而你没有剥离，toInt() 会返回 0，用户输入的值就"丢失"了。最安全的做法是自定义格式化时完全不用 prefix/suffix，把所有格式逻辑放在 textFromValue 中。

第二个坑是 stepEnabled 返回了错误的标志组合。比如在 value() == maximum() 时仍然返回 StepUpEnabled，或者在 wrapping 为 true 时没有让箭头始终可用。这会导致箭头按钮的视觉状态和实际步进行为不一致——用户看到箭头是可点击的但点了没反应，或者箭头灰了但其实值还有调整空间。记住：stepEnabled 只控制视觉状态，不控制实际步进行为。stepBy 方法的实际执行不受 stepEnabled 返回值的影响。

第三个坑是 setWrapping(true) 后值在 minimum 和 maximum 之间跳跃，但 textFromValue 没有处理回绕后的显示。wrapping 模式下，value 超过 maximum 后会回绕到 minimum。如果你的 textFromValue 有某种"递增显示"的逻辑（比如 "第 1 步"、"第 2 步"），回绕后突然从 "第 N 步" 跳到 "第 1 步" 可能会让用户困惑。wrapping 适合"循环"语义（角度、时钟），不适合"递增"语义。

## 5. 练习项目

练习项目：多功能参数输入面板。我们要实现一个设置面板，包含三种定制的 SpinBox：第一个是角度输入框 AngleSpinBox（范围 0-359，显示 "XX°"，支持 wrapping 循环）；第二个是时间长度输入框 DurationSpinBox（内部存储秒数，范围 0-86400，显示格式根据值自动切换——小于 60 显示 "XX 秒"，60-3599 显示 "XX 分 XX 秒"，3600 以上显示 "XX 时 XX 分"）；第三个是缩放比例输入框（继承 QDoubleSpinBox，显示 "1:2"、"1:1"、"2:1" 这样的比例格式）。

完成标准是三个 SpinBox 都能正确显示格式化文本，用户手动输入格式化文本后能被正确解析回数值，角度 SpinBox 的 wrapping 循环正常工作，时间 SpinBox 的单位自动切换无歧义。提示几个关键点：DurationSpinBox 的 textFromValue 需要做分段格式化，valueFromText 需要支持解析所有三种格式；角度 SpinBox 的 validate 需要接受 "90°" 这种输入；缩放比例的 valueFromText 需要从 "1:2" 格式中提取分子分母。

## 6. 官方文档参考链接

[Qt 文档 · QSpinBox](https://doc.qt.io/qt-6/qspinbox.html) -- 整数步进框，包含 textFromValue/valueFromText 虚函数说明

[Qt 文档 · QDoubleSpinBox](https://doc.qt.io/qt-6/qdoublespinbox.html) -- 浮点数步进框，同样的虚函数接口

[Qt 文档 · QAbstractSpinBox](https://doc.qt.io/qt-6/qabstractspinbox.html) -- 步进框基类，stepEnabled 和 stepBy 的定义

[Qt 文档 · QAbstractSpinBox::StepEnabledFlag](https://doc.qt.io/qt-6/qabstractspinbox.html#StepEnabledFlag-enum) -- 步进按钮状态枚举

---

到这里，QSpinBox 的进阶内容就全部讲完了。textFromValue/valueFromText 这对虚函数是 SpinBox 定制化的核心——它们让你可以完全控制数值和显示文本之间的双向映射，突破 prefix/suffix 只能附加固定文本的限制。stepEnabled 给了你对箭头按钮状态的精确控制权。而 prefix/suffix 在自定义格式化场景下的边界问题，搞清楚后你就知道什么时候该用默认机制、什么时候该完全自己来。把这些能力组合起来，你可以构建出任意复杂的数值输入体验——角度、时间、内存大小、缩放比例，全都不在话下。
