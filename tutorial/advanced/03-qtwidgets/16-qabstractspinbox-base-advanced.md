---
title: "3.16 QAbstractSpinBox 基类进阶"
description: "入门篇把外观控制、editingFinished、stepBy、validate 过了一遍，进阶篇看微调框一次编辑如何收尾。validate 与 fixup 怎样接力，Invalid 为什么被静默拒绝，stepBy 重写后范围钳位归谁管，stepEnabled 决定箭头什么时候变灰、步进入口放不放行，wrapping 只管步进不管键入，textFromValue 与 valueFromText 要守双向一致，键盘与滚轮的步进入口全在基类手里，配套示例在 examples/advanced/03-qtwidgets/16-qabstractspinbox-base-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.16——QAbstractSpinBox 基类进阶

[入门篇 QAbstractSpinBox 基础](../../beginner/03-qtwidgets/16-qabstractspinbox-base-beginner.md) 把 setReadOnly、setButtonSymbols、editingFinished、stepBy 和 validate 的三态返回值过了一遍，拿 QSpinBox、QDoubleSpinBox 做常规数值输入，入门篇的内容够用。控件基类部分走到第六篇收官，咱们看 QSpinBox、QDoubleSpinBox、QDateTimeEdit 共同的祖先 QAbstractSpinBox。它不从 QFrame、QAbstractScrollArea 那条链下来，直接站在 QWidget 上。上下箭头按钮、内嵌的 QLineEdit、键盘与滚轮的步进入口、编辑结束时的解释与收尾，全由它统管，咱们在子类里重写的 validate、fixup、stepBy，全都声明在它名下。

平时拖现成控件，数值越界有钳位，箭头顶到头自己变灰，咱们不必过问。一旦要定制输入，细节就都摆到眼前了。咱们把最容易把人问住的几个场面摆出来。fixup 重写了，却一次也没被调用过。角度选择器做成了循环步进，359 度时上箭头却灰了。textFromValue 里加了单位后缀，控件的值莫名其妙归了零。按键按下去没反应，也没有任何提示。场面各不相同，病根也各不相同，本篇把四件事讲明白：validate 与 fixup 在编辑收尾时怎么接力，stepBy 的边界行为与 stepEnabled 的箭头状态怎么配，wrapping 把循环步进包成了什么样，textFromValue 与 valueFromText 的双向一致性怎么守。讲完循环、进显示与解析之前，键盘、滚轮与 setAccelerated 的步进入口顺路数一遍，那一层全在基类手里。

## validate 与 fixup：编辑收尾时的两段接力

入门篇见过 validate 的三个返回值，Acceptable、Intermediate、Invalid。咱们拿一张表把三态在编辑收尾时的待遇放回桌上：

| 返回值 | 判定 | 编辑收尾时的走向 |
| -------- | -------- | -------- |
| Acceptable | 文本已经是合法值 | 直接采用 |
| Intermediate | 文本暂不合法，但有修正的余地 | 调用 fixup 修正后再验证一次 |
| Invalid | 文本完全无法解析 | 同样会调 fixup 修正，修不好则恢复旧值 |

当用户完成输入（按 Enter 或焦点离开）时，咱们跟着 QAbstractSpinBox 的内部逻辑走一遍：收尾先跑 interpret()，内部拿 validate 检查当前文本。返回 Acceptable，一切正常。返回 Intermediate 或 Invalid，fixup 都会被请出来尝试把文本修正为合法值，然后再验证一次，过了就采用修正后的值，不过就恢复到上一次的有效值。

editingFinished 这里有两个同名信号，得分开说。lineEdit() 里那个是 QLineEdit::editingFinished，带条件门，文本要过验证器确认 Acceptable 才发。QAbstractSpinBox 自己声明的这个没有门：keyPressEvent 的 Enter 分支和 focusOutEvent 里，都是先 interpret() 再无条件发射，值被恢复成旧值它也照发。接信号的时候咱们要分清来源，QAbstractSpinBox 这个不能拿来当“验证已通过”的凭据。

咱们拿角度输入框当练习场，0 到 359 度，敲进去 400 要在收尾时钳回 359，不能当场把数字拒掉：

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

fixup 的到场时机有讲究，也是本节最要紧的细节。它并不是只在 Intermediate 时被请出来：收尾这条链里，validate 没给出 Acceptable，fixup 就会跑，Intermediate 和 Invalid 都轮得着。咱们平时见到的却总是 Intermediate 触发它，因为 Invalid 的文本根本进不了输入框，收尾时自然碰不着，拦截发生在哪，下一段就看。所以想让 fixup 真有机会干活，关键在 validate 对“可以被修正的输入”返回 Intermediate：写成 Invalid 的字符连框都进不去，修正的机会只能留给 Intermediate 这一支。程序里 setText 直接塞进去的文本不走按键那道闸，真带着 Invalid 状态走到收尾，fixup 一样会跑，机制上它对两种非 Acceptable 一视同仁。

Invalid 的输入其实在更早的环节就被拦下了。基类构造时给内嵌的 QLineEdit 装了内部验证器，按键造成的文本变化要过 validate，返回 Invalid 的按键被直接拒绝，输入框不会显示用户键入的字符，也没有任何提示，用户只觉得“我按了键但什么都没发生”。笔者翻了 qlineedit 侧的实现，被拒的按键其实发了 QLineEdit::inputRejected 信号，只是默认没人监听。要在非法输入时给反馈，接上这个信号弹个提示、临时变个样式都行，更舒服的做法还是尽量拿 Intermediate 代替 Invalid，让输入过程宽容一些，收尾时再用 fixup 收拢。

配套示例正是照分工写的，HexSpinBox 的源文件是 hex_spin_box.h 与 hex_spin_box.cpp，在 examples/advanced/03-qtwidgets/16-qabstractspinbox-base-advanced/ 下，可执行文件 16-qabstractspinbox-base-advanced。HexSpinBox 继承 QAbstractSpinBox 而不是 QSpinBox，自己拿 quintptr 存值。它的 validate 里，空输入与超出范围的数返回 Intermediate（用户正在清空、越界可钳位），混进非十六进制字符才返回 Invalid。咱们跑一个实验：程序启动值 0x00FF00A0，范围顶到 0x00FFFFFF。把光标点到文本末尾，按一个 F，文本变成 0xFF00A0F，越界了，Intermediate 放它进去。按 Enter，fixup 把它钳到 0xFFFFFF，下方标签同步成十进制 16777215。再把 validate 里越界那支 Intermediate 改成 Invalid，重新编译按同一个 F，字符进不去了，按键无声无息。Intermediate 与 Invalid 的分界，亲手按一遍就有体感。看完把改动改回去。

## stepBy 的边界：钳位归自己，箭头听 stepEnabled

入门篇重写 stepBy 做过对数步进，进阶要回答的是边界问题，步进后的值超出 [minimum, maximum] 范围会怎样。这件事只在一个地方处理：QAbstractSpinBox::stepBy 把候选值交给私有函数 bound()，由它钳回 [minimum, maximum]，步进量再大，值也不会越界。QSpinBox 连 stepBy 都没有重写，用的就是基类这一份。一旦咱们重写了 stepBy，bound() 那套钳位就被整个绕开，范围处理落到自己手里。角度选择器要的还不是钳位，是循环，359 再往上回 0，0 再往下到 359：

```cpp
class AngleSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    using QSpinBox::QSpinBox;
    // 用前先 setRange(0, 359)：范围若停在默认的 0-99，
    // 取模得到的 359 会被 setValue 自己的钳位压回 99，循环到不了顶

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

重写时忘了处理范围，症状比较隐蔽。咱们继承 QSpinBox 又完全重写了 stepBy，等于把基类的钳位整个绕开，实现里就必须自己处理范围，要么钳位到 [minimum, maximum]，要么取模做循环。忘了处理，越界值传给 setValue，会被它内部的范围检查压回边界，程序不崩，但循环转不起来，359 永远到不了。越界的分支在 stepBy 里就地钳位或取模，比指望下游的 setValue 兜底踏实。

箭头按钮的状态由另一个虚函数管。stepEnabled 决定上下箭头可用还是变灰，默认实现在值等于 maximum 时禁用上箭头按钮，在值等于 minimum 时禁用下箭头按钮。它管的不只是灰不灰，是整个步进入口：鼠标点箭头、键盘方向键、滚轮，动手前都要先查 stepEnabled，禁用的方向一律不放行。角度选择器是循环的，不存在到达边界的概念，所以要重写 stepEnabled 让两个按钮永远可用。咱们要是只重写了 stepBy、实现了循环，却忘了 stepEnabled，用户会看到上箭头在 359 度时变灰，此时点它、按方向键都走不动，循环逻辑写得再好也轮不到执行，控件到顶就停了。

配套示例把两件活都自己扛了。HexSpinBox 的步进不看 singleStep，看光标位置，光标落在哪个十六进制数位上，一步就走那一位的权重，个位 ±0x1，十位 ±0x10，光标停在 0x 前缀上则走最高位。值与范围都是自己存的 quintptr，所以 stepBy 里防溢出钳位、stepEnabled 里拿 m_value 与 m_min、m_max 比大小禁箭头，全没依赖基类。咱们再跑一个实验：把 main.cpp 里的 setHexValue(0x00FF00A0) 改成 setHexValue(0x00FFFFFF)，重新编译，窗口一出来上箭头就是灰的，值正好顶在边界上，下箭头照常可用。看完把值改回去。

## wrapping：官方给的循环步进开关

循环未必要咱们自己写。QAbstractSpinBox 一直带着 setWrapping(bool) 属性，Qt 4 时代就有，比 Qt 4.2 才引入的 setAccelerated 资历还老。wrapping 为 true 时，步进到最大值再步进会回到最小值，反过来也一样。前面手动重写 stepBy 实现循环的做法就未必必要了，直接设置 wrapping：

```cpp
auto *spinBox = new QSpinBox();
spinBox->setRange(0, 359);
spinBox->setWrapping(true);  // 359 + 1 = 0，0 - 1 = 359
```

wrapping 的底层就在 QAbstractSpinBox::stepBy 那条链上：候选值交给私有函数 bound()，没开 wrapping 时它做钳位，开了之后换成回绕，大于 maximum 设为 minimum，小于 minimum 设为 maximum。开 wrapping 还顺带改了 stepEnabled 的默认行为，两个方向永远放行，箭头不再因顶格变灰，上一步手写 stepEnabled 的活儿也省了。但它的管辖范围值得咱们看清，wrapping 只影响通过步进按钮、键盘方向键或滚轮触发的值变化，不影响用户在输入框中直接键入的数字。键入的路径归 validate 与 fixup 管，validate 给不出 Acceptable 时 fixup 就会来修正，与 wrapping 无关。步进与键入两套机制各管各的，互不打架。

### 一道行为分析题：wrapping 开着，键入 100 落到几

咱们把边界串起来做一道题。一个 QSpinBox，范围 [0, 59]，wrapping 为 true，singleStep 为 1，当前值 58，validate 按 DegreeSpinBox 的写法自定义过了，可解析但越界的文本返回 Intermediate。点一次上箭头，值变成 59。再点一次，值变成 0。此时用户直接在输入框中输入 100 然后按 Enter，最终值是多少？您先自己想一遍再往下读。答案是 fixup 会被调用，100 超出范围，validate 返回 Intermediate，fixup 把 100 钳位到 maximum 即 59，最终值是 59。wrapping 只影响 stepBy，不影响 validate 与 fixup 的钳位逻辑，步进的循环与键入的收拢是两条独立的路。自定义 validate 是必要前提，这里得点明：内置的 validate 对 [0, 59] 里的 100 返回 Invalid，第三个 0 根本敲不进框，题目里那个 100，只有在本篇这种自定义写法下才会出现。

咱们翻配套示例的构造函数，里面有一行 setWrapping(false)，注释写着不使用 wrapping，靠 stepEnabled 控制箭头状态。示例的范围顶在 0x00FFFFFF，要的是值到顶就禁掉上箭头，循环跳回 0 不是这套界面要的行为。要不要 wrapping 由业务定，基类把两条路都留好了。

## 键盘、滚轮与加速：步进入口全在基类手里

stepBy 的 steps 参数从哪来，咱们平时未必细想。笔者翻了 qabstractspinbox.cpp 的 keyPressEvent 与 wheelEvent，入口全部由基类接好，子类一步都不用操心：

| 操作 | stepBy 收到的步数 |
| -------- | -------- |
| 上、下方向键 | +1、-1 |
| PageUp、PageDown | +10、-10 |
| 步进修饰键配合上、下方向键 | ±10，修饰键默认 Ctrl，来自样式提示 SH_SpinBox_StepModifier |
| 滚轮 | 每攒够 120 个 angleDelta 单位计一步，余数记下来下次续上 |
| 步进修饰键配合滚轮 | 步数乘 10 |

子类拿到的 steps 就是按这张表算好的。配套示例给咱们演示了一种换算参数的方式，光标位步进，steps 的绝对值乘上光标所在数位的权重，上箭头加、下箭头减。基类把入口统一，步进量怎么解释，主动权在子类的 stepBy 手里。

按住不放的体验另有一个开关。setAccelerated(true) 开启加速，按住上箭头或下箭头不放，按住方向键等它自动重复也一样，步进的频率会越来越快。笔者在 timerEvent 里看到算法：重复定时器每走一次，间隔按当前重复率的 5% 缩短，最短压到 10 毫秒。注意加速改变的是步进密度，每一步的步长不变，别指望 singleStep 越滚越大。在大范围里翻数值时开着它很省手指，这个属性从 Qt 4.2 就有。

## textFromValue 与 valueFromText：显示与解析必须互逆

显示与解析是另一对搭档，textFromValue 把内部值转成显示文本，valueFromText 把显示文本解析回内部值。咱们把归属说清，两个虚函数不在 QAbstractSpinBox 上，QSpinBox 持有 int 版本，QDoubleSpinBox 持有 double 版本，基类自己没有。直接子类化 QAbstractSpinBox 的控件想用同样的思路，就得像配套示例那样自己写一对私有函数，在 lineEdit() 的 editingFinished 里手工同步。

双向一致性是硬要求，咱们把它写成两条式子。对任何合法值 v，valueFromText(textFromValue(v)) 必须等于 v。对任何合法文本 t，textFromValue(valueFromText(t)) 必须等价于 t，格式上允许微小差异，比如大小写，数值必须相同。

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

最容易犯的错是在 textFromValue 里加了单位后缀，valueFromText 里忘了去掉。咱们顺着控件内部的显示、解析、显示循环走一遍。第一次 textFromValue 返回 “36.5°C”，用户编辑后 valueFromText 解析出 36.5，textFromValue 再把 “°C” 接回去，看着没问题。可要是 valueFromText 没把 “°C” 去干净，整个字符串传给 toDouble 会返回 0.0，textFromValue 再把 0.0 格式化成 “0.0°C”，原来的值就丢了。加后缀与去后缀必须成对写，写了一个，另一个就得回头核对。

调用频率咱们也得掂量。valueFromText 的频率跟着键盘跟踪走：跟踪开着时每敲一键，编辑框的 editorTextChanged 都会拿着当前文本跑一次 valueFromText，所以它有轻量的 O(1) 硬要求，别在里头做复杂的格式化或查询。它的时机不在 validate 里：QSpinBox::validate 走 validateAndInterpret，拿 locale().toInt() 解析文本，从头到尾不碰 valueFromText。textFromValue 的节奏反而慢，逐键编辑时一次都不跑，等编辑收尾重新设值时才把显示文本更新出来。

多单位温度转换是这两个函数的典型用武之地，咱们把设计要点摆出来：内部始终以摄氏度存值，textFromValue 按当前单位换算并格式化，valueFromText 按当前单位反向换算，单位切换时 setValue 重新设值触发显示更新，步进与验证再按当前单位各自生效。textFromValue 的花式格式化，动态单位、度数符号这类 suffix 做不到的玩法，连同自定义 valueFromText 时 prefix/suffix 的剥离边界，[3.29 QSpinBox 进阶](./29-qspinbox-advanced.md) 里展开。QDateTimeEdit 的日期时间编辑归 [3.30 QDateTimeEdit 进阶](./30-qdatetimeedit-advanced.md)。各有分工，本篇不重复。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17。QAbstractSpinBox 在 QtWidgets 模块，验证部分涉及 QValidator 的三个状态与 QAbstractSpinBox 的保护方法，自定义子类化需要重写 stepBy、validate、fixup 等虚函数，所有内容在桌面平台上行为一致。

[Qt 文档 · QAbstractSpinBox](https://doc.qt.io/qt-6/qabstractspinbox.html) -- 微调框基类，stepBy/validate/fixup/wrapping 接口

[Qt 文档 · QSpinBox](https://doc.qt.io/qt-6/qspinbox.html) -- 整数微调框，textFromValue/valueFromText 接口

[Qt 文档 · QDoubleSpinBox](https://doc.qt.io/qt-6/qdoublespinbox.html) -- 浮点数微调框

[Qt 文档 · QValidator](https://doc.qt.io/qt-6/qvalidator.html) -- 验证器基类，Acceptable/Intermediate/Invalid 三种状态

---

控件基类六篇到这里收官，QAbstractSpinBox 交出的是交互框架，验证的接力、步进的入口、显示与解析的换算，数值语义留给子类填。想再动手的，请您把配套示例（examples/advanced/03-qtwidgets/16-qabstractspinbox-base-advanced/，可执行文件 16-qabstractspinbox-base-advanced）完整玩一轮。光标在不同数位间点来点去，上下箭头一步跨的幅度跟着变。把值推到 0x00FFFFFF，看上箭头变灰。再按正文里的实验把 Intermediate 改成 Invalid，体会按键被无声拒回的差别。下一篇 [3.17 QPushButton 进阶](./17-qpushbutton-advanced.md) 从基类转到具体按钮，QPushButton 在 QDialog 里的键盘拦截与 autoDefault，正是 [3.12 QAbstractButton 基类进阶](./12-qabstractbutton-base-advanced.md) 里留下的那个 Enter 话题。
