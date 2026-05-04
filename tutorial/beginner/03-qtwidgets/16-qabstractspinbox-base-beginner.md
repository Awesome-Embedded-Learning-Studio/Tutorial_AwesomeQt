# 现代Qt开发教程（新手篇）3.16——QAbstractSpinBox：数字输入基类

## 1. 前言 / 为什么你需要搞懂 QAbstractSpinBox

我们在 Qt 开发中处理数值输入的时候，QSpinBox、QDoubleSpinBox、QDateTimeEdit 这三个控件几乎是标配。它们让用户通过上下箭头按钮或直接键盘输入来调整值，比普通的 QLineEdit 多了一层天然的数值约束。但你知道这三个控件全部继承自同一个基类吗——QAbstractSpinBox。这个基类封装了微调框的核心交互框架：上下箭头按钮、只读模式、输入验证机制、步进控制，以及编辑完成信号。你在 QSpinBox 上调的 `setReadOnly()`、在 QDoubleSpinBox 上连接的 `editingFinished` 信号、在 QDateTimeEdit 上通过子类化重写的 `stepBy()`——这些接口全部定义在 QAbstractSpinBox 上。

说实话，大部分开发者跟 QAbstractSpinBox 的直接接触不多——因为 QSpinBox 和 QDoubleSpinBox 已经覆盖了绝大多数数值输入场景，你很少需要去关心基类提供了什么。但当你需要做一些定制化的数值输入控件——比如一个 16 进制输入框、一个角度选择器、或者一个带有自定义验证逻辑的输入控件——你就必须继承 QAbstractSpinBox，而理解这个基类的内部机制就变成了绕不过去的前置条件。这篇文章我们就把 QAbstractSpinBox 的外观控制、信号与验证、步进机制、以及输入合法性检验这四个方面彻底讲清楚。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QAbstractSpinBox 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。QAbstractSpinBox 的交互行为在所有桌面平台上一致——上下箭头按钮的渲染由当前 QStyle 控制（不同平台视觉上会有些许差异，但行为完全相同）。本篇涉及到的 QValidator 也在 QtWidgets 模块内。如果你已经跑通了之前的 QtWidgets 项目，直接编译运行即可。

## 3. 核心概念讲解

### 3.1 setReadOnly / setButtonSymbols：控件外观控制

QAbstractSpinBox 提供了两个直接影响控件外观的接口，它们决定了用户能看到的"输入框长什么样"。

`setReadOnly(bool)` 控制输入框是否只读。设为 true 之后，用户不能通过键盘直接修改文本内容，但上下箭头按钮仍然可以正常使用——也就是说"只读"只锁定了键盘输入，不锁定按钮步进。这个设计初看有点反直觉——你都只读了为什么还能用按钮调值？但仔细想想，它的使用场景很明确：当你希望用户只能通过预定义的步进值来调整参数，而不允许手动输入任意值的时候（比如只能以 0.5 为步长调整透明度），`setReadOnly(true)` 配合 `setSingleStep(0.5)` 就能做到精确的值域控制。

```cpp
auto *spinBox = new QDoubleSpinBox();
spinBox->setRange(0.0, 1.0);
spinBox->setSingleStep(0.1);
spinBox->setDecimals(1);
spinBox->setReadOnly(true);  // 禁止键盘输入，只能通过上下按钮以 0.1 步长调整
```

`setButtonSymbols(QAbstractSpinBox::ButtonSymbols)` 控制上下箭头按钮的显示方式。有两个可选值：`UpDownArrows` 是默认值——显示传统的上下三角箭头；`NoButtons` 隐藏箭头按钮，输入框看起来就像一个普通的 QLineEdit，但仍然可以通过键盘的上下方向键来触发步进。`NoButtons` 在界面空间紧张或者你需要自定义步进按钮位置的场景下特别有用。

```cpp
auto *spinBox = new QSpinBox();
spinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);  // 隐藏箭头按钮
```

还有一个 `PlusMinus` 枚举值——它会显示 "+"和"-"文字按钮而不是箭头。这个值在 Qt 6 中虽然还存在，但在现代界面设计中几乎没人用，视觉上也不太协调。你知道有这个东西就行，实际项目中基本都用 `UpDownArrows`。

`setReadOnly` 和 `setButtonSymbols` 可以组合使用。如果你既设了 `setReadOnly(true)` 又设了 `setButtonSymbols(QAbstractSpinBox::NoButtons)`，那这个输入框就完全不可交互了——用户既不能键盘输入，也不能按钮步进，文本内容只能通过代码的 `setValue()` 来改变。这种组合在某些"只展示数值但想保持微调框外观"的场景下会用到。

### 3.2 editingFinished 信号与输入验证

`editingFinished()` 是 QAbstractSpinBox 最常用的信号。它在用户"完成编辑"时触发——具体来说，触发时机有三种：用户按了 Enter 或 Return 键、输入框失去了键盘焦点、或者用户点击了上下箭头按钮（在 Qt 6 中，点击箭头按钮会先改变值再触发 editingFinished）。

这个信号最大的用途是"在用户确认输入之后执行一次性的业务逻辑"。跟 `valueChanged` 信号的持续触发不同，`editingFinished` 只在编辑动作完成时触发一次——如果用户在输入框里删了又改、改了又删，最后按了 Enter，`valueChanged` 可能已经触发了五六次，但 `editingFinished` 只触发一次。

```cpp
auto *spinBox = new QSpinBox();
spinBox->setRange(1, 100);

// valueChanged: 每次值变化都触发
connect(spinBox, &QSpinBox::valueChanged, [](int value) {
    qDebug() << "值变化:" << value;
});

// editingFinished: 编辑完成时触发一次
connect(spinBox, &QSpinBox::editingFinished, [spinBox]() {
    qDebug() << "编辑完成，最终值:" << spinBox->value();
});
```

这里有一个微妙的地方需要说明：当用户输入了一个超出范围的值然后按 Enter，QAbstractSpinBox 的验证机制会自动把值修正到有效范围内的最近值，然后触发 `editingFinished`。也就是说你不需要在 `editingFinished` 的槽函数里再检查值是否合法——到这个信号触发的时候，值已经被修正过了。比如范围是 [1, 100]，用户输入了 999 然后按 Enter，值会被自动修正为 100，然后 `editingFinished` 触发，此时 `value()` 返回的是 100。

但有一种特殊情况：如果用户输入的内容完全无法解析为数值（比如在一个 QSpinBox 里输入了 "abc"），QAbstractSpinBox 不会触发 `editingFinished`——它会恢复到上一次的有效值，并且不发出信号。这个行为设计的初衷是避免"把无效输入当作合法输入处理"，但在实际开发中你可能会遇到用户困惑——"我按了 Enter 怎么什么都没发生"。如果你需要在这种场景下给用户一个明确的反馈，可以重写 `validate()` 或者用 `QTimer::singleShot` 做延迟检查。

### 3.3 stepBy()：步进值控制

`stepBy(int steps)` 是 QAbstractSpinBox 的核心步进方法。参数 `steps` 可以是正数也可以是负数——正数表示增加，负数表示减少。用户点击上箭头按钮时 Qt 内部调用 `stepBy(1)`，点击下箭头时调用 `stepBy(-1)`，按住 Shift 点击上箭头时调用 `stepBy(10)`（这是 QAbstractSpinBox 内置的加速步进行为，Shift 键会将步进量乘以 10）。

对于 QSpinBox 和 QDoubleSpinBox，`stepBy()` 的默认实现是根据 `singleStep()` 属性来计算新值的——新值等于当前值加上 `steps * singleStep()`，然后被钳位到 [minimum, maximum] 范围内。

```cpp
auto *spinBox = new QSpinBox();
spinBox->setRange(0, 100);
spinBox->setSingleStep(5);
spinBox->setValue(20);

// 等效于点击上箭头按钮
spinBox->stepBy(1);   // 值变为 25
spinBox->stepBy(1);   // 值变为 30
spinBox->stepBy(-2);  // 值变为 20

// 步进超出范围时自动钳位
spinBox->stepBy(100); // 值变为 100（上限）
```

真正有趣的地方在于你可以重写 `stepBy()` 来实现自定义的步进逻辑。比如你想做一个"对数步进"的微调框——当值比较大的时候步进幅度也大，值小的时候步进幅度也小——就可以在子类中重写 `stepBy()`。

```cpp
class LogSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    using QSpinBox::QSpinBox;

protected:
    void stepBy(int steps) override
    {
        int current = value();
        // 对数步进：值的 10% 作为步进量，最小为 1
        int step = qMax(1, current / 10);
        int newValue = current + steps * step;
        newValue = qBound(minimum(), newValue, maximum());
        setValue(newValue);
    }
};
```

还有一个跟步进相关的方法值得了解：`stepEnabled()` 返回一个 `StepEnabledFlags`，指示当前状态下哪些步进按钮是可用的。默认实现会检查当前值是否已经到了范围的边界——如果值等于 maximum，上箭头按钮就会被禁用；如果值等于 minimum，下箭头按钮就会被禁用。如果你重写了 `stepEnabled()` 返回 `StepUpEnabled | StepDownEnabled`，两个按钮就永远可用，不管当前值在哪里。

### 3.4 validate()：输入合法性检验机制

QAbstractSpinBox 的输入验证机制是理解它内部工作方式的关键。当你直接在输入框里键入文字时，Qt 并不是等用户按了 Enter 才去验证——它会在每一次按键后都调用 `validate(QString &input, int &pos)` 方法来实时检查输入内容的合法性。

`validate()` 返回一个 `QValidator::State` 枚举，有三个可能的值。`QValidator::Acceptable` 表示输入完全合法，QAbstractSpinBox 会立即更新内部的值。`QValidator::Intermediate` 表示输入"可能"合法但还不完整——比如范围是 [100, 999]，用户输入了 "5"，这是一个中间状态——"5"本身不在范围内但用户可能还在继续输入"57"然后"572"。`QValidator::Invalid` 表示输入完全不合法，QAbstractSpinBox 会拒绝这次输入（输入框不会显示用户键入的字符）。

```cpp
class HexSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    using QSpinBox::QSpinBox;

protected:
    /// @brief 验证输入是否为合法的 16 进制字符串
    QValidator::State validate(QString &text, int &pos) const override
    {
        // 去掉前缀 "0x" 后验证
        QString hexText = text;
        if (hexText.startsWith("0x", Qt::CaseInsensitive)) {
            hexText = hexText.mid(2);
        }

        if (hexText.isEmpty()) {
            return QValidator::Intermediate;
        }

        // 检查是否全部是 16 进制字符
        for (const QChar &c : hexText) {
            if (!(c.isDigit() || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
                return QValidator::Invalid;
            }
        }

        // 检查转换后的值是否在范围内
        bool ok = false;
        int value = hexText.toInt(&ok, 16);
        if (!ok) {
            return QValidator::Invalid;
        }
        if (value >= minimum() && value <= maximum()) {
            return QValidator::Acceptable;
        }
        return QValidator::Intermediate;
    }

    /// @brief 把模型数值转换为显示文本
    QString textFromValue(int val) const override
    {
        return "0x" + QString::number(val, 16).toUpper();
    }

    /// @brief 把显示文本转换为模型数值
    int valueFromText(const QString &text) const override
    {
        QString hexText = text;
        if (hexText.startsWith("0x", Qt::CaseInsensitive)) {
            hexText = hexText.mid(2);
        }
        return hexText.toInt(nullptr, 16);
    }
};
```

这段代码展示了通过重写 `validate()`、`textFromValue()` 和 `valueFromText()` 三个方法来实现一个 16 进制输入框。`validate()` 负责实时验证用户输入的每个字符是否合法，`textFromValue()` 负责把内部整数值转换为显示在输入框里的字符串（带 "0x" 前缀），`valueFromValue()` 负责把用户输入的字符串解析回整数值。这三个方法配合工作，构成了 QAbstractSpinBox 的"输入-验证-显示"循环。

有一点需要特别说明：`validate()` 在每次按键时都会被调用，所以它的执行效率直接影响输入体验。如果你的验证逻辑涉及复杂的计算或外部查询，用户在快速输入时会感到明显的延迟。保持 `validate()` 的逻辑简洁快速——最好只做字符串格式的检查，复杂的业务验证放到 `editingFinished` 的槽函数里做。

## 4. 踩坑预防

第一个坑是 `setReadOnly(true)` 之后还期望键盘输入生效。前面已经说过了，只读模式只允许按钮步进和方向键步进，不允许键盘直接编辑文本。如果你需要"允许键盘输入但限制输入格式"，正确的做法不是 `setReadOnly(true)`，而是重写 `validate()` 或设置 `QValidator`。

第二个坑是 `editingFinished` 在程序化调用 `setValue()` 时不会触发。这个信号只在用户交互完成时触发，你通过代码调用 `setValue()` 不会触发它。如果你需要在值被任何方式改变时都得到通知，应该连接 `valueChanged` 信号。

第三个坑是重写 `validate()` 时忘记处理 `Intermediate` 状态。如果你对所有不在最终范围内的输入都返回 `Invalid`，用户在输入过程中会体验到"明明在打字但输入框不接受我的输入"的困惑——比如范围是 [100, 200]，用户想输入 150，当他输入 "1" 时你的验证逻辑返回了 `Invalid`，"1" 就不会被显示出来。正确做法是对"有可能变成合法值的中间输入"返回 `Intermediate`。

第四个坑是 `textFromValue()` 和 `valueFromText()` 不匹配。如果你重写了 `textFromValue()` 让它显示 "0xFF" 格式的文本，就必须同时重写 `valueFromText()` 让它能解析 "0xFF" 格式的文本。如果两个方法不对称，QAbstractSpinBox 内部的"显示-解析-显示"循环会出错——轻则显示不正确，重则导致无限递归或崩溃。

第五个坑是在 `stepBy()` 里忘记调用基类的范围钳位逻辑。如果你完全重写了 `stepBy()` 但没有自己做 `qBound(minimum(), newValue, maximum())` 的范围检查，步进后的值可能超出有效范围。QSpinBox 的默认 `stepBy()` 里面包含了范围钳位，你的重写版本也必须包含。

## 5. 练习项目

我们来做一个综合练习：创建一个窗口，展示 QAbstractSpinBox 的各种能力。窗口分为三个区域。顶部区域是一个 HexSpinBox（16 进制微调框），范围 0x00 到 0xFF，支持直接输入 16 进制字符串或通过上下按钮步进，下方有一个标签实时显示当前值的十进制、十六进制和二进制表示。中部区域是一个 QDoubleSpinBox 演示区，包含一个普通的 QDoubleSpinBox 和一个"锁定步进"版本的 QDoubleSpinBox（setReadOnly + NoButtons 的组合），后者只能通过外部的两个 QPushButton 来调用 stepBy(1) 和 stepBy(-1)。底部区域展示 editingFinished 和 valueChanged 的触发对比——一个 QSpinBox 的两种信号分别连接到两个 QLabel 计数器上，用户操作时可以实时看到两个信号各自的触发次数。

几个提示：HexSpinBox 继承 QSpinBox，重写 validate/textFromValue/valueFromText 三个方法；锁定步进的 QDoubleSpinBox 用 setReadOnly(true) + setButtonSymbols(NoButtons) 配合外部按钮的 stepBy()；信号对比区用两个 int 计数器变量分别累加 valueChanged 和 editingFinished 的触发次数，显示在 QLabel 上。

## 6. 官方文档参考链接

[Qt 文档 · QAbstractSpinBox](https://doc.qt.io/qt-6/qabstractspinbox.html) -- 微调框基类

[Qt 文档 · QSpinBox](https://doc.qt.io/qt-6/qspinbox.html) -- 整数微调框

[Qt 文档 · QDoubleSpinBox](https://doc.qt.io/qt-6/qdoublespinbox.html) -- 浮点数微调框

[Qt 文档 · QValidator](https://doc.qt.io/qt-6/qvalidator.html) -- 输入验证器基类

---

到这里，QAbstractSpinBox 的核心机制你就搞定了。setReadOnly 和 setButtonSymbols 控制控件的外观和交互边界，editingFinished 信号给你"编辑完成"这个关键时刻的入口，stepBy 是步进逻辑的核心方法可被重写来实现自定义步进策略，而 validate 配合 textFromValue/valueFromText 构成了完整的"输入-验证-显示"循环。理解了这些，无论是使用 QSpinBox/QDoubleSpinBox 还是自己继承 QAbstractSpinBox，你都能做到心中有数。
