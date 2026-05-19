---
title: "3.22 QLineEdit 进阶"
description: "入门篇我们把 QLineEdit 的基础输入控制、回显模式、验证器和信号机制过了一遍。这次我们要把视野拉到更深的层面——自定义 QValidator 子类、输入掩码的精确控制、QCompleter 的异步集成，以及三个文本信号的精确时序分析。"
---

# 现代Qt开发教程（进阶篇）3.22——QLineEdit 进阶

## 1. 前言 / QLineEdit 的进阶水位在哪里

入门篇我们把 QLineEdit 的基础能力过了一遍——placeholder、echoMode、validator 三个内置验证器、textChanged/textEdited/editingFinished 的区别。说实话，那些知识足够你应付八成以上的 QLineEdit 使用场景了。但如果你做过稍微正式一点的产品，你大概率会遇到这些让血压升高的时刻：内置的 QIntValidator 不支持千位分隔符，QDoubleValidator 的 locale 行为在不同平台上表现不一致，IP 地址输入框需要精确到每一位的格式限制，还有搜索框里那个死活弹不到最上层的 QCompleter 下拉列表。

这篇文章我们就把 QLineEdit 的四个进阶维度拆清楚：自定义 QValidator 子类（包括 QIntValidator 和 QDoubleValidator 的陷阱）、输入掩码 setInputMask 与 validator 的关系和冲突、QCompleter 的正确集成方式、以及三个文本变化信号的精确时序和线程安全考量。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17 标准。涉及 Qt6::Widgets 和 Qt6::Gui（QValidator、QRegularExpression 定义在 Gui 中）。QCompleter 同样属于 QtWidgets。所有示例在 Linux / macOS / Windows 桌面平台上行为一致。

## 3. 核心概念讲解

### 3.1 自定义 QValidator 子类——QIntValidator 和 QDoubleValidator 不够用的时候

入门篇我们用了 QIntValidator 和 QRegularExpressionValidator，但实际项目中它们的局限性很快就暴露出来。QIntValidator 的一个经典问题是它对 locale 敏感——在某些 locale 下小数点用逗号，在另一些 locale 下用句号，而 QIntValidator 的范围验证行为会受此影响。QDoubleValidator 更甚，它的 `setNotation()` 可以切换 Scientific 还是 Standard，但它的 `validate()` 返回 Intermediate 的条件在不同 Qt 小版本之间有微妙差异。与其在这些问题上反复踩坑，很多团队选择直接继承 QValidator 写自己的验证器。

QValidator 有一个核心虚函数需要重写：`validate(QString &input, int &pos)`。它返回 QValidator::State 枚举——Acceptable 表示完全合法，Intermediate 表示"还没输完，暂时放过"，Invalid 表示直接拒绝。参数 input 是当前 QLineEdit 的完整文本（引用传递，你甚至可以在验证过程中修改它来实现自动纠错），pos 是光标位置。

```cpp
class HexValidator : public QValidator
{
    Q_OBJECT

public:
    explicit HexValidator(QObject *parent = nullptr)
        : QValidator(parent) {}

    State validate(QString &input, int &pos) const override
    {
        Q_UNUSED(pos);
        if (input.isEmpty()) {
            return Intermediate;
        }
        // 只允许十六进制字符
        for (int i = 0; i < input.size(); ++i) {
            auto c = input[i].toUpper().toLatin1();
            bool hex = (c >= '0' && c <= '9')
                    || (c >= 'A' && c <= 'F');
            if (!hex) {
                return Invalid;
            }
        }
        return Acceptable;
    }
};
```

这个 HexValidator 只允许用户输入 0-9 和 A-F（不区分大小写）。关键点在于空字符串返回 Intermediate 而不是 Invalid——如果你返回 Invalid，用户连开始输入的机会都没有。空字符串是合法的中间状态。另外一个实战技巧是利用 input 引用来做自动格式化——比如自动转大写、自动插入分隔符：

```cpp
State validate(QString &input, int &pos) const override
{
    QString cleaned;
    for (auto c : input) {
        if (c.isDigit()) cleaned.append(c);
    }
    // 格式化为 XXX-XXX
    if (cleaned.size() > 3) {
        cleaned.insert(3, '-');
    }
    int new_pos = pos;  // 需要根据格式调整光标
    input = cleaned;
    pos = new_pos;
    return cleaned.size() == 7 ? Acceptable : Intermediate;
}
```

通过修改 input 引用，你在 validate 里实现的不仅仅是"能不能输入"，还包括"自动纠错成什么样"。这个能力在格式化输入场景（电话号码、身份证号、银行卡号）中非常好用。但要注意每次修改 input 后，pos 也要做相应调整，否则光标会跳到错误的位置。

### 3.2 输入掩码 setInputMask——精确的字符级格式控制

QValidator 解决的是"内容是否合法"的问题，而 setInputMask 解决的是"格式长什么样"的问题。输入掩码用一套简单的字符语法来定义每一位应该输入什么类型的字符。`9` 表示必须输入数字、`a` 表示必须输入字母、`A` 表示必须输入字母（自动转大写）、`N` 表示字母或数字、`X` 表示任意字符、`H` 表示十六进制字符、`>` 表示后续字符自动转大写、`<` 表示后续字符自动转小写。

```cpp
auto *phoneEdit = new QLineEdit();
phoneEdit->setInputMask("(999) 999-9999");
phoneEdit->setPlaceholderText("Phone number");

auto *ipEdit = new QLineEdit();
ipEdit->setInputMask("000.000.000.000;_");
```

掩码中 `0` 表示必须输入数字（不允许为空），`;` 后面的字符是空白占位符——在 IP 掩码中用了下划线 `_`，这样用户能清楚地看到每一组数字的边界。掩码会自动在对应位置显示括号、点号等分隔符，用户只需要填空。

现在问题来了：setInputMask 和 setValidator 能不能同时使用？答案是可以，但行为非常微妙。当两者同时设置时，QLineEdit 先应用掩码的格式过滤，再调用 validator 的 validate 方法。也就是说掩码在第一层把关"字符类型"，validator 在第二层把关"值的合法性"。但这也意味着两者可能产生冲突——掩码允许的字符可能被 validator 拒绝，或者 validator 的 Intermediate 状态和掩码的"必须填满"要求互相打架。实际上，我的建议是除非你非常清楚两者之间的交互，否则不要同时使用。对于 IP 地址这种场景，用掩码控制三位一组的格式，用 validator 检查每组是否在 0-255 范围内，确实可以做到；但如果你发现输入行为变得诡异（某些字符无法输入、光标跳动异常），第一时间怀疑的就是掩码和 validator 的冲突。

现在有一道调试题给大家。下面这段代码在 IP 输入框中同时设置了掩码和验证器，但用户发现输入 "192.168.001" 之后无法继续输入了——为什么？

```cpp
auto *ip = new QLineEdit();
ip->setInputMask("000.000.000.000;_");
ip->setValidator(new QIntValidator(0, 255, ip));
```

问题出在 QIntValidator 是对整个字符串做范围验证的，而 IP 掩码产生的字符串是 "192.168.001.___" 这样的格式——点号和下划线混在里面，QIntValidator 根本无法把它解析成整数，直接返回 Invalid。正确的做法是写一个自定义的 IpValidator 逐段验证，或者干脆只用掩码不用 validator，在提交时做最终检查。

### 3.3 QCompleter 集成——搜索框自动补全的正确姿势

QCompleter 是 QLineEdit 的自动补全搭档，用法看上去很简单——创建 QCompleter、设置 QStringList 模型、绑定到 QLineEdit：

```cpp
auto *completer = new QCompleter(this);
completer->setModel(new QStringListModel(
    {"Apple", "Banana", "Cherry", "Date", "Elderberry"}, completer));
completer->setCaseSensitivity(Qt::CaseInsensitive);
completer->setFilterMode(Qt::MatchContains);

auto *searchEdit = new QLineEdit();
searchEdit->setCompleter(completer);
```

setFilterMode(Qt::MatchContains) 让补全器在任意位置匹配关键词，而不是只在开头匹配。setCaseSensitivity(Qt::CaseInsensitive) 忽略大小写。这两个设置几乎是搜索框的标配。

但在实际项目中有几个地方容易踩雷。第一是 QCompleter 的弹出窗口（popup）层级问题。如果 QLineEdit 在一个设置了 `Qt::Tool` 标记的窗口中，或者在一个浮动面板中，QCompleter 的弹出列表可能被其他窗口遮挡。这是因为 QCompleter 的弹出窗口默认是一个 QFrame with Qt::Popup 标记，它的 parent 是 QLineEdit 所在的窗口。如果你的 QLineEdit 在一个特殊的顶层窗口中，popup 的 Z-order 可能不对。解决方案是调用 `completer->setWidget(lineEdit)` 确保绑定正确，或者在必要时重写 completer 的 `popup()` 方法自定义弹出行为。

第二是 QCompleter 的模型动态更新。如果你的补全数据来自数据库或网络，你不能在构造时就固定 QStringList——你需要动态更新 model。标准做法是用 QSortFilterProxyModel 配合一个自定义的 QAbstractItemModel，在数据就绪后更新 model 的内容。如果你直接替换 completer 的 model（`completer->setModel(newModel)`），需要注意这会重置 completer 的内部状态，包括当前的过滤条件和弹出窗口的位置。

第三是 completion 信号和 textEdited 信号的交互。QCompleter 在用户选中一个补全项时会调用 QLineEdit 的 setText，这会触发 textChanged 但不会触发 textEdited。如果你在 textEdited 中做实时搜索，用户通过补全选择的结果不会触发搜索——这可能不是你想要的行为。你需要在 completer 的 `activated(const QString &text)` 信号中手动触发搜索逻辑。

### 3.4 三个文本信号的精确时序——textChanged vs textEdited vs editingFinished 的内部机制

入门篇我们从使用角度区分了三个信号，进阶篇我们要看看它们在 Qt 内部的发射机制。textChanged 在 QLineEdit 的内部 setText 末尾发射，无论是来自用户输入（QLineEdit 内部的 keyPressEvent 处理后调用 setText）还是程序调用 setText。textEdited 只在 QLineEdit 处理用户输入事件（QInputMethodEvent 和 QKeyEvent）的路径上发射，程序调用 setText 的代码路径上不经过 textEdited 的发射点。editingFinished 在 QLineEdit 的 focusOutEvent 和 keyPressEvent（检测到 Enter/Return）中发射，但有一个前提条件——如果设置了 validator 且当前文本的 validate 返回的不是 Acceptable，editingFinished 不会被发射。

这个 validator 阻塞 editingFinished 的行为在很多项目中造成了困惑。用户在输入框里填了一个"半成品"（比如只输入了 "192." 的 IP 地址），然后按 Tab 想跳到下一个输入框——结果焦点根本不跳，因为 validator 返回了 Intermediate，QLineEdit 认为编辑还没结束。这在表单中特别讨厌：用户想先跳过某个字段回头再填，但 validator 卡住了焦点。解决方案是在 focusOutEvent 中不依赖 editingFinished，而是自己处理焦点切换逻辑；或者让 validator 在 Intermediate 时也返回 Acceptable，把"不完整"的判断推迟到提交时。

另外一个需要注意的时序细节是：当你用 QCompleter 选中补全项时，信号顺序是 activated -> textChanged。如果你在 textChanged 中清空了某些状态，activated 的处理函数中可能读到已经被清空的数据。这种情况虽然少见，但在复杂的表单联动中确实可能出现。

## 4. 踩坑预防

第一个坑是 QValidator 返回 Intermediate 时 editingFinished 信号不发射。很多开发者给 QLineEdit 设了 validator 后发现"按回车没反应"或者"Tab 键跳不过去"，原因就是 validator 认为当前文本还没输完。这个行为在 QIntValidator 范围下限大于 0 时尤其容易出现——比如 QIntValidator(1, 100)，用户输入 "0" 后按回车，editingFinished 不会触发，因为 "0" 不在合法范围内。解决方案是：要么在提交按钮的点击处理中手动触发验证逻辑，不依赖 editingFinished；要么在 validator 中对"接近合法但还不完整"的输入返回 Acceptable 并在提交时做二次检查。

第二个坑是 QCompleter 的 popup 被其他窗口遮挡。如果 QLineEdit 所在的窗口有特殊的 window flags（比如 Qt::Tool、Qt::Popup、Qt::FramelessWindowHint），QCompleter 的弹出列表可能出现在其他窗口后面。这是因为 Qt::Popup 类型的窗口的 Z-order 受 parent 窗口影响。解决方案是检查 QLineEdit 所在窗口的 windowFlags，必要时调用 `completer->popup()->raise()` 或者换用自定义的弹出控件。

第三个坑是 setInputMask 和 setValidator 同时使用时产生冲突。掩码负责字符级过滤，validator 负责值级验证，两者的判定逻辑可能互相矛盾——掩码允许通过的字符被 validator 拒绝，或者 validator 对带分隔符的文本无法正确解析。我的建议是：二选一，不要同时使用。如果格式要求可以用掩码表达，就用掩码；如果验证逻辑比较复杂（比如 IP 每段 0-255），就用自定义 validator。

## 5. 练习项目

练习项目：智能地址输入组件。我们要实现一个用于输入网络地址的复合 QLineEdit，支持 IP 地址和端口号的输入。核心功能包括：自定义 IpPortValidator 验证器，能够正确验证 "192.168.1.1:8080" 格式的地址（IP 每段 0-255，端口 1-65535），在输入过程中对不完整的输入返回 Intermediate（比如用户输入了 "192.168" 时不应被拒绝），在提交时对不合法的输入给出明确的错误提示（通过 QToolTip 显示"IP 第 N 段超出范围"或"端口号超出范围"）。输入框右侧集成一个 QCompleter，补全数据是用户最近使用的十个地址（保存在 QStringList 中），补全模式为中间匹配、忽略大小写。

完成标准是：用户可以流畅输入 IP 和端口，中间状态不会被 validator 卡住，按回车后能正确判断合法性，补全弹出列表能正确显示和选择，选中的补全项能正确填入输入框。提示几个关键点：IpPortValidator 的 validate 方法需要先把 input 按 `:` 拆分出 IP 和端口两部分，IP 部分按 `.` 拆分出四段逐段验证；QCompleter 的 activated 信号需要连接到输入验证逻辑；错误提示用 QToolTip::showText 在输入框位置显示。

## 6. 官方文档参考链接

[Qt 文档 · QLineEdit](https://doc.qt.io/qt-6/qlineedit.html) -- 单行文本输入框，包含 echoMode、inputMask、completer 属性说明

[Qt 文档 · QValidator](https://doc.qt.io/qt-6/qvalidator.html) -- 验证器基类，validate 虚函数的返回值语义

[Qt 文档 · QIntValidator](https://doc.qt.io/qt-6/qintvalidator.html) -- 整数验证器，locale 对范围验证的影响

[Qt 文档 · QDoubleValidator](https://doc.qt.io/qt-6/qdoublevalidator.html) -- 浮点数验证器，notation 设置的差异

[Qt 文档 · QCompleter](https://doc.qt.io/qt-6/qcompleter.html) -- 自动补全组件，model/filter/popup 的配置方式

---

到这里，QLineEdit 的进阶内容就过完了。自定义 QValidator 子类让你能实现任何格式的输入验证，包括自动纠错和格式化；输入掩码 setInputMask 提供了字符级的格式模板，但和 validator 同时使用时要格外小心冲突；QCompleter 的集成看似简单，但 popup 的 Z-order 和信号时序在复杂窗口中容易出问题；三个文本信号的内部发射机制搞清楚了，你就知道为什么 editingFinished 有时候"不生效"——那多半是 validator 的 Intermediate 状态在作怪。QLineEdit 的体量不大，但细节拉满，值得认真对待。
