---
title: "3.29 QSpinBox 进阶"
description: "入门篇把 setRange、setValue、valueChanged 用顺了，进阶篇对着 6.9.1 源码把 validate、valueFromText、textFromValue 三个自定义口子认清楚，默认 textFromValue 走 locale 且不带前后缀，合成发生在私有 updateEdit，框架递给虚函数的是带前后缀的完整文本，剥离由默认实现内部的 stripped 完成，重写后剥离成了自己的责任，stepEnabled 在键盘、滚轮、长按自重复与重绘每条路径上都查过一遍，默认实现只按边界与 wrapping 给标志，stepBy 本体不查、程序直调不受限，配套示例在 examples/advanced/03-qtwidgets/29-qspinbox-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.29——QSpinBox 进阶

[3.28 QFontComboBox 进阶](./28-qfontcombobox-advanced.md) 收尾把话递了过来，说 QSpinBox 的正题在文本与数值之间的转换与验证，qspinbox.h 里 validate、valueFromText、textFromValue 一组虚函数，跟那篇出厂可编辑的行编辑框接得上。咱们接着这句往源码里走，调用链两头都走到了：显示这一头，私有 updateEdit 把 prefix、textFromValue 的返回值、suffix 拼成一行文本，输入那一头，editorTextChanged 与 interpret 把行编辑框里的完整文本原样递给虚函数，剥前后缀是默认实现自己在内部做的。笔者把 qspinbox.cpp 和 qabstractspinbox.cpp 对着源稿的断言逐条过了一遍，源稿里有几处说法要对着行号改，默认 textFromValue 里没有前后缀，stepEnabled 管的也远不止箭头的灰亮。

次序咱们摆好，从三个虚函数的分工与默认实现认起，跟着是键入路径上 validate 的三条状态与回车后的 interpret，stepEnabled 的默认实现与调用点，重写 valueFromText 之后前后缀的剥离责任收尾。正文行号都带出处，示例代码在 examples/advanced/03-qtwidgets/29-qspinbox-advanced/，Qt 内部实现指源码树里的 qspinbox.cpp、qspinbox.h 与 qabstractspinbox.cpp、qabstractspinbox.h。

## 三个虚函数各管一段，前后缀由 updateEdit 在外面拼

这组口子摆在 qspinbox.h 的 protected 区（第 62-65 行），咱们挨个认，validate 与 fixup 覆盖基类声明，valueFromText 与 textFromValue 是 QSpinBox 自己添的虚函数，各自带着默认实现。QDoubleSpinBox 有完全对应的一组（第 125-128 行），浮点场景沿用同一套思路。

textFromValue 的默认实现在 qspinbox.cpp 第 478-496 行。displayIntegerBase 为 10 的时候，它走 locale().toString(value)，默认把千位分隔符压掉（第 487-492 行），QString::number 只在进制不是 10 时出场，按 qAbs 取绝对值再补负号（第 483-485 行）。类文档把边界写在明面上，返回值里不该带 prefix 与 suffix，specialValueText 也不经过这个函数，出处第 466-470 行。前后缀从哪来，咱们跟着 updateEdit 看，私有实现一行拼齐（qabstractspinbox.cpp 第 1784 行）：

```cpp
// qabstractspinbox.cpp updateEdit 的合成（第 1784 行）
const QString newText = specialValue() ? specialValueText
                                       : prefix + textFromValue(value) + suffix;
```

值等于 minimum 且设了 specialValueText 的时候整行换成那段特殊文本，判定在第 1502-1505 行。您重写 textFromValue 返回 “2.5 MB” 这样的带单位文本，控件显示的就是它本身，外面一层都不加。

valueFromText 的默认实现（qspinbox.cpp 第 513-521 行）转手给私有 validateAndInterpret（第 1078-1146 行），后者头一件事就是 stripped 把前后缀剥掉（第 1091 行），再用 locale().toInt 解析（第 1104-1118 行）。这个机制咱们留到最后一节展开，因为重写之后剥离就换成您的活了。

示例里的 UnitSpinBox 就是这套模式的实物，声明在 unit_spin_box.h 第 25 行，咱们看它怎么落地。内部值存字节数，范围 0 到 100 MB，步进 1 KB，构造函数在 unit_spin_box.cpp 第 19-26 行，一行 prefix、suffix 都没设，固定的 suffix 标不出随值换挡的单位，这件活得交给 textFromValue。textFromValue 按大小选单位（第 32-46 行），过 1 MB 显示 “x.x MB”，过 1 KB 显示 “x.x KB”，其余显示 “x B”。valueFromText 反着来（第 52-101 行），MB、KB、B 三种后缀都认，大小写不敏感，纯数字当字节，全都解析不动时返回当前 value()（第 100 行），控件不会跳去莫名的数。互为逆运算的要求类文档里写得直白，重写了 textFromValue，就该把 valueFromText 与 validate 一起带上，出处 qspinbox.cpp 第 472-473 行。

## validate 守着每一次键入，Acceptable 之外各有去处

keyboardTracking 默认开着，出处 qabstractspinbox.cpp 第 1406 行，行编辑框每敲一个字，都会进 editorTextChanged，出处第 1523-1541 行。validate 返回 Acceptable 才当场调 valueFromText 更新值，否则只挂起一个 pendingEmit，等下一轮处理，出处第 1530-1537 行。您按回车或让控件失焦，interpret 接手（第 1887-1920 行），validate 过不了就调一次 fixup 再验（第 1899-1907 行），QSpinBox 的 fixup 只删千位分隔符（qspinbox.cpp 第 539-543 行），救不了别的，再验还不过，就按 correctionMode 落回旧值或就近边界，出处 qabstractspinbox.cpp 第 1907-1911 行，默认是 CorrectToPreviousValue。

三条状态怎么划分，默认 validateAndInterpret 里看得最清楚，出处 qspinbox.cpp 第 1093-1134 行。落在 min 与 max 之间是 Acceptable（第 1122-1123 行），空串与光杆正负号是 Intermediate（第 1096-1099 行），解析不动、或者越过边界背向零，是 Invalid，出处第 1101-1102、1120-1121、1127-1129 行。范围外的另一种落点您留意，值往零的方向够不着 min 时算 Intermediate，再敲一位数字可能就进范围了，比如范围 10 到 99 时敲了个 “1”，控件放行等您继续。

示例的 validate 是自己写的正则版（unit_spin_box.cpp 第 107-150 行）。主模式认数字加可选单位（第 112-114 行），单位是可选的，纯数字自己就够 Acceptable，实测敲 “1” 值立刻变成 1，示例里真正落到 Intermediate 的，是格式合法、换算出的字节数却出了范围的输入（第 146-149 行），纯数字一路敲、越过 100 MB 上限就是一例，格式两条正则都不认才回 Invalid（第 125 行）。代码里还备了一条只认光杆数字的部分匹配（第 119-123 行），本意是接住敲到一半的中间态，可纯数字主模式直接就收了，它成了主模式的真子集，一次都轮不到跑。您照着示例扩写时多想一步，每条分支在什么输入下真的可达，中间态别判 Invalid 的原则不变，判了用户就打不完字。

## stepEnabled 在每条步进路径上把门，默认实现只认边界与 wrapping

StepEnabledFlag 有三个值，摆在 qabstractspinbox.h 第 40-42 行，StepNone、StepUpEnabled、StepDownEnabled，咱们从默认实现认起（qabstractspinbox.cpp 第 541-556 行）：

```cpp
// qabstractspinbox.cpp stepEnabled 的默认实现（第 541-556 行，只读分支省略）
if (d->wrapping)
    return StepEnabled(StepUpEnabled | StepDownEnabled);
StepEnabled ret = StepNone;
if (variantCompare(d->value, d->maximum) < 0)
    ret |= StepUpEnabled;
if (variantCompare(d->value, d->minimum) > 0)
    ret |= StepDownEnabled;
return ret;
```

只读直接 StepNone（第 544-545 行），wrapping 一开两面标志都给（第 546-547 行），剩下按边界判。回绕的实现在私有 bound 里，wrapping 下越过 max 落到 min、跌破 min 抬到 max（第 1721-1727 行），角度、时钟这类循环量您给 setWrapping(true) 正合适，步数这类只增不减的量别给，从 “第 99 步” 绕回 “第 0 步” 用户会愣一下。

笔者数过它的调用点，流传的说法把 stepEnabled 当成纯视觉开关，这个版本对不上。键盘 Up、Down、PageUp、PageDown 进 stepBy 之前要查对应标志，缺了直接 return（第 1012-1013 行），滚轮同样查过才步进（第 1159-1160 行），按住不放的自重复定时器每一拍重新取一遍，标志丢了就 reset（第 1259-1272 行），鼠标按下也查（第 1374-1381 行），连右键菜单里的 Step up、Step down 两项，都按它置灰（第 1308-1312 行）。灰亮的绘制走 paintEvent 里的 initStyleOption（第 949-955、1703-1705 行），样式提示 SH_SpinControls_DisableOnBounds 为真才采用 stepEnabled 的结果，为假恒画两面可用。不查的只有 stepBy 本体（第 616-647 行），stepUp、stepDown 两个槽直通它（第 587-601 行），程序里手动调不受标志限制。

调用频率您心里有数，每次重绘、每次滚轮、每次方向键、自重复每一拍都会跑一遍 stepEnabled，在里面做重活会直接拖响应。反过来这也是个便宜的通知点，想让箭头随外部条件变化，重写它返回相应标志就行，绘制与交互路径会自己跟上。

## 重写 valueFromText 之后，剥前后缀成了自己的活

框架递给虚函数的，是行编辑框的完整文本。interpret 从 displayText 起步，出处 qabstractspinbox.cpp 第 1895 行，validate 与 valueFromText 拿到的是同一份带前后缀的串，见第 1899、1914 行，editorTextChanged 的 keyboardTracking 路径同样如此，出处第 1528-1532 行。默认实现不怕，validateAndInterpret 内部走 stripped，出处 qspinbox.cpp 第 1091 行，写法在 qabstractspinbox.cpp 第 1460-1486 行，开头匹配 prefix 就去头，结尾匹配 suffix 就去尾，顺手 trim 两边空白。笔者核到这里停了一下，匹配不上的前后缀它不硬剥，剥得掉的前提是文本真的以 prefix 开头、以 suffix 结尾。

您一旦重写 valueFromText，默认的剥离不再替您跑。设了前缀 “$ ” 的框里敲 “50”，行编辑框交给虚函数的文本是 “$ 50”，默认 validate 内部自己剥、照样给 Acceptable，您的解析拿它去 toInt 只会得 0，于是键入路径上第一个字符落地，editorTextChanged 的 Acceptable 分支就把 0 设进了值里（上一节的第 1530-1534 行），50 从头到尾没进过值。事故隐蔽在输入框还显示着您敲的 “$ 50”，回车后 interpret 只是把显示刷成 “$ 0”，丢的值这才露出来。还有一层在 QSpinBoxValidator，行编辑框的验证器发现输入缺 prefix 会当场补回头、缺 suffix 补到尾（第 1960-1966 行），设了前缀的框里用户几乎敲不出不带前缀的文本，您的解析函数必然遇到它。

示例的处理是防御性的，valueFromText 开头把 prefix、suffix 剥掉，再往下解析（unit_spin_box.cpp 第 54-62 行），尽管构造里一个都没设。您要自定义格式，最省心的路子是跟示例一致，别用 prefix、suffix，格式的进出全部收进 textFromValue 与 valueFromText，两边不打架。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，QSpinBox、QDoubleSpinBox 与 QAbstractSpinBox 都在 QtWidgets，链接 Qt6::Widgets 即可。

[Qt 文档 · QSpinBox](https://doc.qt.io/qt-6/qspinbox.html) -- 整数步进框，textFromValue、valueFromText、validate 与 displayIntegerBase 的正式说明

[Qt 文档 · QDoubleSpinBox](https://doc.qt.io/qt-6/qdoublespinbox.html) -- 浮点版同一组虚函数，decimals 与取整行为在这页

[Qt 文档 · QAbstractSpinBox](https://doc.qt.io/qt-6/qabstractspinbox.html) -- 步进框基类，stepEnabled、stepBy、correctionMode、wrapping 的定义

[Qt 文档 · QAbstractSpinBox::StepEnabledFlag](https://doc.qt.io/qt-6/qabstractspinbox.html#StepEnabledFlag-enum) -- StepNone、StepUpEnabled、StepDownEnabled 三个枚举值

---

输入类刚开个头，下一站 [3.30 QDateTimeEdit 进阶](./30-qdatetimeedit-advanced.md)。它同样是 QAbstractSpinBox 的子类，qdatetimeedit.h 第 21 行写得分明，今天认的 validate 与 interpret 链子，到它身上换成按日期节段处理，咱们去那边接着认。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/29-qspinbox-advanced/，可执行文件 29-qspinbox-advanced），窗口里一枚 UnitSpinBox 加两块标签，初始值 512 KB（main.cpp 第 30 行），标签实时显示显示文本与内部值（第 41-45 行）。

实验咱们顺着正文走三步。第一步，您点几下上箭头，显示从 “512.0 KB” 一格一格往上走，这是 textFromValue 的 KB 分支在出文本，出处 unit_spin_box.cpp 第 39-43 行。第二步，全选后敲 “1048576”，值当场就在变，底部标签逐键跟上，keyboardTracking 确实在每敲一键时就收值，输入框里却始终是裸数字 “1048576”，回车或失焦后才跳成 “1.0 MB”。值已变、显示未变，这个错位本身就是个好的观察点，根因一句话，示例的 validate 只判状态、不改写文本，editorTextChanged 里 setValue(v, EmitIfChanged, tmp != t) 的第三参算出来是 false，updateEdit 不跑（qabstractspinbox.cpp 第 1533 行），收值与刷显示是两条路，回车后的 interpret 换成 setValue(v, ep, true)，才无条件把显示刷成 textFromValue 的样子（第 1917 行），单位切换由值的大小决定，出处第 34-38 行。第三步，敲 “2.5 ”，能进，主模式认纯数字带尾随空格，再补一个 “M”，进不去，敲到 “2.5 M” 时主模式与部分匹配两条正则都不认，validate 判 Invalid，行编辑框直接拒收，不是中间态漏算了它，是哪条分支都没给它留位置。这是写 validate 时最容易漏的一类中间态，单位的前缀得一个字符一个字符敲上去，您要是想让 “2.5 MB” 能手敲进去，实测 “2.5 B” 敲得出、卡住的只是单位字母，把单位的前缀补进部分匹配的清单就行。

完成标准咱们照旧列好，单位随值切换无歧义，手敲的纯数字按字节收，进不去的字符您能说出卡在哪条状态上。
