---
title: "3.16 QAbstractSpinBox 基类进阶"
description: "入门篇我们把 QAbstractSpinBox 的外观控制、editingFinished 信号、stepBy 和 validate 这些基础接口用了一遍。进阶篇要深入到微调框的状态机内部，搞清楚 validate 和 fixup 的协作机制、步进按钮的自定义行为、wrapping 循环策略、以及 textFromValue/valueFromText 的双向转换设计。"
---

# 现代Qt开发教程（进阶篇）3.16——QAbstractSpinBox 基类进阶

## 1. 前言 / 为什么微调框的内部机制值得深挖

入门篇我们用 QAbstractSpinBox 的基本 API 做了一个 16 进制输入框，学会了 validate、textFromValue、valueFromText 这三个核心方法。如果你只是用 QSpinBox 和 QDoubleSpinBox 做普通的整数和浮点数输入，那些知识确实够用。但当你需要做一个定制化的输入控件——比如一个角度选择器需要 0-359 度的循环步进、一个 IP 地址输入框需要多段独立验证、或者一个自定义单位转换的输入框需要精确控制用户能输入什么——你就必须理解 QAbstractSpinBox 内部的状态机是怎么运转的。这篇进阶要把 validate 和 fixup 的协作关系、stepBy 的边界行为、wrapping 循环机制、以及 text/value 转换的设计模式彻底讲透。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17 标准。依赖 QtWidgets 模块，涉及 QValidator 的三个状态和 QAbstractSpinBox 的保护方法。自定义子类化部分需要重写 stepBy、validate、fixup 等虚函数。所有内容在桌面平台上行为一致。

## 3. 核心概念讲解

### 3.1 validate + fixup 状态机——输入验证的完整闭环

入门篇我们讲了 validate 的三个返回值：Acceptable、Intermediate、Invalid。但 validate 只是验证环节的一半，另一半是 fixup。这两个方法构成了 QAbstractSpinBox 输入验证的完整闭环。

当用户完成输入（按 Enter 或焦点离开）时，QAbstractSpinBox 的内部逻辑是这样的：先调用 validate 检查当前文本。如果返回 Acceptable，一切正常，editingFinished 信号触发。如果返回 Intermediate，QAbstractSpinBox 会调用 fixup 尝试把"中间状态"的文本修正为合法文本，然后再验证一次。如果 fixup 之后的文本验证通过了，editingFinished 触发；如果还是不通过，QAbstractSpinBox 会恢复到上一次的有效值，不触发 editingFinished。

```cpp
class DegreeSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    using QSpinBox::QSpinBox;

protected:
    QValidator::State validate(QString &text, int &pos) const override
    {
        // 允许空输入（用户正在清空准备重新输入）
        if (text.isEmpty()) return QValidator::Intermediate;

        bool ok = false;
        int val = text.toInt(&ok);
        if (!ok) return QValidator::Invalid;

        // 0-359 范围内合法，负数或 360+ 是中间状态
        if (val >= 0 && val < 360) return QValidator::Acceptable;
        return QValidator::Intermediate;
    }

    void fixup(QString &text) const override
    {
        bool ok = false;
        int val = text.toInt(&ok);
        if (!ok) {
            text = "0";
            return;
        }
        // 把超出范围的值钳位到 0-359
        val = qBound(0, val, 359);
        text = QString::number(val);
    }
};
```

这里有一个极其重要的细节：fixup 只在 validate 返回 Intermediate 时被调用，不会在 validate 返回 Acceptable 或 Invalid 时调用。这意味着如果你的 validate 对所有无效输入都返回 Invalid 而不是 Intermediate，fixup 就永远不会被调用——用户输入了一个无效值，焦点离开后文本直接被恢复到旧值，没有任何修正机会。所以如果你希望 fixup 机制能工作，validate 必须对"可以被修正的输入"返回 Intermediate 而不是 Invalid。

### 3.2 stepBy 的边界行为与自定义步进策略

入门篇我们展示了重写 stepBy 实现对数步进的例子。进阶篇我们要关注的是 stepBy 的边界行为——当步进后的值超出 [minimum, maximum] 范围时会怎样。

QSpinBox 的默认 stepBy 实现会在步进后把值钳位到 [minimum, maximum]：`setValue(qBound(minimum(), newValue, maximum()))`。这意味着即使步进量很大，值也不会越界。如果你重写了 stepBy，你也必须自己做这个钳位——QAbstractSpinBox 的基类实现不会帮你做范围检查。

```cpp
class AngleSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    using QSpinBox::QSpinBox;

protected:
    void stepBy(int steps) override
    {
        int newVal = value() + steps * singleStep();

        // 不做钳位，而是循环：359 + 1 -> 0，0 - 1 -> 359
        while (newVal >= 360) newVal -= 360;
        while (newVal < 0) newVal += 360;

        setValue(newVal);
    }

    // 永远允许上下步进（不因为到达边界而禁用按钮）
    StepEnabled stepEnabled() const override
    {
        return StepUpEnabled | StepDownEnabled;
    }
};
```

注意 stepEnabled 的重写——默认实现在 value 等于 maximum 时会禁用上箭头按钮，在 value 等于 minimum 时禁用下箭头按钮。但我们的角度选择器是循环的，不存在"到达边界"的概念，所以需要重写 stepEnabled 让两个按钮永远可用。如果你只重写了 stepBy 实现了循环但忘了重写 stepEnabled，用户会看到上箭头在 359 度时变灰，虽然点击它值确实会循环到 0——视觉上会给用户"不能点了"的错误暗示。

### 3.3 Wrapping 循环策略

从 Qt 5.12 开始，QAbstractSpinBox 新增了 `setWrapping(bool)` 属性。当 wrapping 为 true 时，步进到最大值后再步进会回到最小值，反之亦然。这个功能让前面那个手动重写 stepBy 实现循环的做法变得不必要了——直接设置 wrapping 就行。

```cpp
auto *spinBox = new QSpinBox();
spinBox->setRange(0, 359);
spinBox->setWrapping(true);  // 359 + 1 = 0，0 - 1 = 359
```

wrapping 的底层实现是修改了 QSpinBox 的 stepBy 默认行为——当步进后的值大于 maximum 时，值被设为 minimum；小于 minimum 时被设为 maximum。但要注意 wrapping 只影响通过步进按钮或键盘方向键触发的值变化，不影响用户在输入框中直接键入的数字。用户仍然可以手动输入 999，这时候 validate/fixup 会处理——如果 validate 返回 Intermediate，fixup 会被调用来修正值。

现在有一道行为分析题给大家。一个 QSpinBox，范围 [0, 59]，wrapping 为 true，singleStep 为 1，当前值为 58。用户点击上箭头一次，值变成 59。再点击一次，值变成 0。此时用户直接在输入框中输入 100 然后按 Enter。最终值是多少？答案是——fixup 会被调用（因为 100 超出范围，validate 返回 Intermediate），fixup 把 100 钳位到 maximum 即 59，所以最终值是 59。wrapping 只影响 stepBy，不影响 validate/fixup 的钳位逻辑。

### 3.4 textFromValue / valueFromText 的双向转换设计

入门篇我们讲了这两个方法的基本用法——textFromValue 把内部值转为显示文本，valueFromText 把显示文本解析回内部值。进阶篇我们要强调的是这两个方法必须满足双向一致性：对任何合法值 v，`valueFromText(textFromValue(v))` 必须等于 v；对任何合法文本 t，`textFromValue(valueFromText(t))` 必须等价于 t（格式上可以有微小差异，比如大小写，但数值必须相同）。

```cpp
// textFromValue：内部值 → 显示文本
QString textFromValue(double val) const override
{
    return QString("%1°C").arg(val, 0, 'f', 1);
}

// valueFromText：显示文本 → 内部值（必须去掉后缀再解析）
double valueFromText(const QString &text) const override
{
    QString cleaned = text;
    cleaned.remove("°C").remove("°").remove("C");
    return cleaned.trimmed().toDouble();
}
```

这里最容易犯的错误是在 textFromValue 里加了单位后缀，但在 valueFromText 里忘了去掉后缀。结果就是 QAbstractSpinBox 内部的"显示-解析-显示"循环会把后缀叠加——第一次 textFromValue 返回 "36.5°C"，用户编辑后 valueFromText 解析出来 36.5，然后 textFromValue 又加了 "°C" 变成 "36.5°C"——看起来没问题。但如果 valueFromText 没有正确去掉 "°C"，它可能把 "36.5°C" 整个字符串传给 toDouble，toDouble 返回 0.0，然后 textFromValue 把 0.0 格式化为 "0.0°C"，原来的值就丢了。

另一个需要注意的点是 textFromValue 和 valueFromText 的调用频率。textFromValue 在每次值变化时都会被调用（用来更新输入框的显示文本），valueFromText 在每次 validate 被调用时都会被调用（validate 内部可能调 valueFromText 来检查输入能否被解析）。所以这两个方法必须是轻量的 O(1) 操作，不能做复杂的格式化或查询。

## 4. 踩坑预防

第一个坑是 fixup 在正常编辑完成时不会被调用。fixup 只在 validate 返回 Intermediate 时被调用。如果你的 validate 对越界值返回 Invalid 而不是 Intermediate，fixup 永远没有机会执行，用户输入越界值后文本会被直接恢复到旧值，没有任何修正。正确做法是对"可以被修正到合法范围"的输入返回 Intermediate，只对"完全无法解析"的输入返回 Invalid。

第二个坑是 validate 返回 Invalid 时用户完全不知道发生了什么。当 validate 返回 Invalid 时，QAbstractSpinBox 会静默拒绝输入——输入框不会显示用户键入的字符，也没有任何提示。这对用户来说非常困惑——"我按了键但什么都没发生"。如果你需要在输入非法时给用户反馈，可以考虑在 validate 返回 Invalid 时通过某种方式（比如临时改变输入框的样式或显示 tooltip）通知用户。但更好的做法是尽量用 Intermediate 代替 Invalid，让输入过程更宽容。

第三个坑是 stepBy 的重写版本忘记做范围钳位。QAbstractSpinBox::stepBy 的基类实现不做范围检查（它只是调用 setValue），但 QSpinBox::stepBy 的实现会做 qBound 钳位。如果你继承 QSpinBox 并完全重写了 stepBy，你的实现必须自己处理范围——要么钳位到 [minimum, maximum]，要么配合 wrapping 做循环。否则 setValue 在超出范围时可能不会生效（因为 QSpinBox 的 setValue 内部也会做范围检查），但行为可能不符合你的预期。

## 5. 练习项目

练习项目：多单位温度转换微调框。我们要实现一个 TemperatureConverterSpinBox，继承 QDoubleSpinBox，支持三种温度单位：摄氏度（C）、华氏度（F）、开尔文（K）。控件内部始终以摄氏度存储值，但显示和输入的单位可以通过一个外部 QComboBox 切换。当单位切换时，输入框的显示文本自动转换为对应单位的值。步进量为当前单位的 1 度（不是摄氏度的 1 度）。控件的 range 以摄氏度为基准（-273.15 到 1000），但在华氏度模式下输入框显示的是换算后的范围。

完成标准是三种单位切换时数值正确转换且不丢失精度，步进按钮在每种单位模式下步进量为该单位的 1 度，输入验证对每种单位独立生效。提示几个关键点：内部用摄氏度存储，textFromValue 根据当前单位做换算和格式化，valueFromText 根据当前单位反向换算，validate 对每种单位分别检查范围，单位切换时调用 setValue 重新设值以触发显示更新。

## 6. 官方文档参考链接

[Qt 文档 · QAbstractSpinBox](https://doc.qt.io/qt-6/qabstractspinbox.html) -- 微调框基类，stepBy/validate/fixup/wrapping 接口

[Qt 文档 · QSpinBox](https://doc.qt.io/qt-6/qspinbox.html) -- 整数微调框，textFromValue/valueFromText 接口

[Qt 文档 · QDoubleSpinBox](https://doc.qt.io/qt-6/qdoublespinbox.html) -- 浮点数微调框

[Qt 文档 · QValidator](https://doc.qt.io/qt-6/qvalidator.html) -- 验证器基类，Acceptable/Intermediate/Invalid 三种状态

---

到这里，QAbstractSpinBox 的进阶内容就搞定了。validate + fixup 的状态机闭环搞清楚后就不会再困惑"为什么我的修正函数没被调用"，stepBy 的边界行为和 wrapping 循环策略让你能实现各种定制化的步进逻辑，textFromValue/valueFromText 的双向一致性是自定义显示格式的铁律。这些知识在做一个真正可用的自定义微调框控件时会反复用到，把它们吃透了，以后遇到任何输入验证的需求都能从容应对。
