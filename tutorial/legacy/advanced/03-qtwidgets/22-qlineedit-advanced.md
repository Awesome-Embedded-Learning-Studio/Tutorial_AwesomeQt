---
title: "3.22 QLineEdit 进阶"
description: "入门篇把 QLineEdit 的占位符、回显模式、三个内置验证器和文本信号的分工过了一遍，进阶篇拆 QValidator 的 validate 三态返回与 input 引用改写，拆输入掩码的字符语法与验证器叠用时的冲突，过 editingFinished 的验证前提与 textEdited 的发射条件，末尾收拾 QCompleter 的弹窗层级、模型更新与绑定细节，配套示例在 examples/advanced/03-qtwidgets/22-qlineedit-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.22——QLineEdit 进阶

[入门篇 QLineEdit 基础](../../beginner/03-qtwidgets/22-qlineedit-beginner.md) 过了一遍占位符、echoMode、三个内置验证器，还有文本信号的分工。[3.21 QCommandLinkButton 进阶](./21-qcommandlinkbutton-advanced.md) 收官按钮五连时留了话，输入类五连从单行输入框的验证器与输入掩码讲起，咱们就从这里进去。

笔者摆几个把人问住的场面。您给输入框挂了 QIntValidator(1, 100)，敲个 0 按回车，挂在 editingFinished 上的提交逻辑毫无动静。IP 输入框里 setInputMask 和 QIntValidator(0, 255) 同设，一个数字都敲不进去。搜索框配好 QCompleter，您从弹出列表里选中的词条，挂在 textEdited 上的实时搜索愣是不触发。这三件事的根子分别在 validate 的返回值语义、掩码与验证器的叠加次序、以及 QLineEdit 内部调 setText 的路径上，本篇按验证器、掩码、信号、QCompleter 的顺序逐条过。

## validate 一管三态：Invalid 当场回退，Intermediate 留在框里

自定义验证器从 QValidator 派生，重写 `validate(QString &input, int &pos)`。返回的 QValidator::State 有三个值，每个值在 QLineEdit 那边换来的待遇完全不同，咱们把它列成表，写验证器之前对一眼：

| State | QLineEdit 那边的待遇 |
| ---- | ---- |
| Acceptable | 完全合法，editingFinished 等下游动作放行 |
| Intermediate | 还没输完，暂时放过，字符留在框里 |
| Invalid | 直接拒绝，本次输入被回退，顺带发出 inputRejected |

Invalid 的回退是真实的：QLineEdit 内部每接受一次改动都会调 validate，判成 Invalid 就把文本退回改动之前（emit inputRejected 的分支就在同一条路径上），所以 Invalid 的字符根本落不进框。Intermediate 是“留在框里但还没完”的状态。咱们写验证器时最要紧的一条由此而来：空字符串返回 Intermediate，别返回 Invalid。空串判 Invalid 的验证器打字照常，坏在删除上。您删到最后一个字符那一下，剩下的空串被判 Invalid，这一步被顶回来（顺带发出 inputRejected），输入框从此清不到空，空框按回车、失焦也等不来 Acceptable。空串是合法的中间状态。

validate 的两个参数都是引用，input 尤其值得说道。您在 validate 里改写它，验证器就从“能不能输”升级成“输歪了帮您扶正”：自动去杂字符、插分隔符、转大写，都做得成。改完记得同步调整 pos，光标位置是按原文本算的，文本一变 pos 不挪，光标就跳去错误的地方：

```cpp
// 在 validate(QString &input, int &pos) 里做自动格式化
QString cleaned;
for (const QChar &c : input)
    if (c.isDigit())
        cleaned.append(c);
if (cleaned.size() > 3)
    cleaned.insert(3, '-');   // 凑成 XXX-XXX 的形状
int adjusted = pos;           // 按删掉的字符数把光标也挪过去，这里省略细节
input = cleaned;
pos = adjusted;
return cleaned.size() == 7 ? Acceptable : Intermediate;
```

数值类的两个内置验证器另有一层 locale 的牵扯。QIntValidator 和 QDoubleValidator 都拿 locale() 去解析数字，QIntValidator 的 validate 拿 locale 做字符校验再转整数，分组符、数位全随 locale 走。QDoubleValidator 的小数点在部分 locale 里是逗号，同一串“3.14”在不同机器上判法不同。产品要跨 locale，这类验证器就不能想当然。QDoubleValidator 还有 setNotation() 切换标准与科学计数法，它的 validate 什么时候给 Intermediate，咱们把 Qt 6 里的规则摆出来：字符过不了 locale 校验判 Invalid，空串、解析到半截、toDouble 失败都算 Intermediate，数值落在范围内才 Acceptable。出了范围多数也只给 Intermediate，只有标准计数法下位数彻底超限才判 Invalid，比如 setDecimals(2) 加上限 9.0，敲到 9.999 或 10.0 都会被退回。科学计数法的出界则一律 Intermediate，改改指数随时救得回来。规则列得出这几条，可要逐条对到手头的输入上并不轻松，与其反复试探它的脾气，格式复杂的场合直接继承 QValidator 自己写，反而省心。

配套示例第一分区就是一枚现成的自定义验证器，IpAddressValidator 逐段验证 IPv4（examples/advanced/03-qtwidgets/22-qlineedit-advanced/，ip_address_validator.cpp 的 validate：四段、每段 0-255、前导零非法，尾段空着算 Intermediate）。您输入 192.168 再敲 3，输入到 300 的第三位时字符进不去、框里停在 30，这就是 Invalid 当场回退的样子。状态标签一路跟着 textChanged 显示 Intermediate 到 Acceptable 的迁移，validate 的三态不用背，敲一遍就长在眼睛里。

## 掩码管格式，验证器管数值：同设要想清楚谁拦谁

QValidator 解决“内容合不合法”，setInputMask 解决“格式长什么样”。掩码是一张逐位的填空模板，分隔符自动摆好，用户只管往空位里填。咱们把字符语法列成表（照 Qt 文档的字符表整理）：

| 掩码字符 | 含义 |
| ---- | ---- |
| `A` | 必填字母 |
| `a` | 字母，可留空 |
| `N` | 必填字母或数字 |
| `n` | 字母或数字，可留空 |
| `X` | 必填任意非空白字符 |
| `x` | 任意非空白字符，可留空 |
| `9` | 必填数字 |
| `0` | 数字，可留空 |
| `D` | 必填 1-9 |
| `d` | 1-9，可留空 |
| `#` | 数字或正负号，可留空 |
| `H` | 必填十六进制字符 |
| `h` | 十六进制字符，可留空 |
| `B` | 必填二进制字符 |
| `b` | 二进制字符，可留空 |
| `>` | 后续字母转大写 |
| `<` | 后续字母转小写 |
| `!` | 关闭大小写转换 |
| `;c` | 掩码到此结束，空白占位符定为 c |
| `\` | 转义上面的元字符 |

表里有几个格子跟不少人的直觉相反，咱们单独拎出来。小写 a 是可留空的字母，必填字母是大写 A，方向别记反。自动转大写跟大小写无关，靠的是 `>`。IP 掩码 `000.000.000.000;_` 末尾的 `;_` 用的就是 `;c` 的语法：0 是可留空的数字，空白位显示成下划线，每一组数字的边界一眼可见：

```cpp
auto *phoneEdit = new QLineEdit;
phoneEdit->setInputMask("(999) 999-9999");      // 括号、空格、连字符自动摆好
auto *ipEdit = new QLineEdit;
ipEdit->setInputMask("000.000.000.000;_");      // 空白位显示为下划线
```

掩码设下之后，读值有两个口子，选错一个就带着占位符入库：text() 返回时把空白占位符剥掉，displayText() 原样返回框里看见的内容，连分隔符和没填满的空位一起。您提交前用 text()，想在界面上展示原始形态才轮到 displayText()。不过 text() 剥的只是空位与占位符，字面分隔符原样留下，电话掩码只敲到 12，text() 给的就是“(12) -”这样的半成品，提交前仍要以 editingFinished 为准，或者拿 hasAcceptableInput 查一道。

现在回到开篇第二个场面：掩码和验证器同设，咱们得弄清两层谁拦在前。次序是按键过掩码的字符检查，每一位允不允许这个字符由掩码说了算，改动落进文本之后，validate 再对整串做值级判定。两层各有职责本是好事，可冲突也出在这里：掩码放过的字符可能被验证器拒绝，验证器更吃不消掩码带进来的分隔符。开篇那道调试题就是典型：

```cpp
auto *ip = new QLineEdit;
ip->setInputMask("000.000.000.000;_");
ip->setValidator(new QIntValidator(0, 255, ip));   // 一个数字都敲不进去，为什么
```

QIntValidator 是对整串做解析的，它拿到的是“192.168.001.___”这样的文本，点号和下划线混在里面，locale 的整数解析根本变不出一个数，validate 判成 Invalid，QLineEdit 当场把这串文本回退，连第一个数字都落不进去。它的出路要么照示例写自定义验证器逐段验证，要么只用掩码、提交时再做范围检查。咱们自己试验时，把示例第二分区的 setInputMask 换成 IP 掩码，custom_validator_demo.cpp 第 101 行现在是电话号码 `(999) 999-9999`。再补一行 setValidator(new QIntValidator(0, 255, m_maskEdit))，敲数字全部被顶回来，这个冲突就复现在眼前。

## editingFinished 有验证前提：Intermediate 也算没输完

editingFinished 的发射点在按 Return 或 Enter，以及失去焦点，两处都挂着一个前提：输入框上有 validator 时，当前文本必须是 Acceptable。Return 路径的检查是看 hasAcceptableInput，不行再试 fixup（QValidator 的修复接口，下表还会带一笔），两关都过不去，事件直接被忽略，信号不响，失焦路径同样过这道检查。开篇第一个场面就出在这个前提上：您给输入框挂 QIntValidator(1, 100) 敲个 0，0 虽在范围外，位数却没超上限，validate 给的是 Intermediate，字符留在框里，回车和失焦都等不来 editingFinished。

这里笔者要修正一个流传很广的说法：validator 会把焦点拦下。实际 Tab 照样把焦点带走，QLineEdit 没有任何拦下焦点的分支，真正丢掉的是 editingFinished。生产代码常把“数据落库”挂在它上面，半成品状态一失焦，提交逻辑就静默错过，表单看着填完了，数据没进去。处理路子有几条，代价各不相同：

| 处理方式 | 代价与边界 |
| ---- | ---- |
| 提交按钮里自己做最终校验，不依赖 editingFinished | 最稳，校验逻辑自己写一遍 |
| validator 对“接近合法”的输入放宽为 Acceptable，不完整判定推迟到提交时 | 输入体验顺，把关后移 |
| 给 validator 写 fixup，失焦前把半成品修成合法值 | QIntValidator 自带 fixup 会按 locale 归一数字，自定义验证器得自己实现 |

还有一条和 QCompleter 相关的细节：焦点若是因为您点了补全弹出列表而离开输入框，editingFinished 同样不发，QLineEdit 在失焦分支里专门排除了点弹层这件事，选完词接着编辑，不算编辑结束。

想在示例里验证这个前提，请您在第一分区补一行连接：custom_validator_demo.cpp 第 75 行的 connect 后面照样子再连一个 editingFinished，槽里 qDebug 打个点。IP 填到 192.168 按 Tab，输出没动静，补全第四段再按一次，打点就出现了。

## textEdited 只认人手：程序与补全改的字都不算

入门篇区分过 textChanged 和 textEdited 的分工，咱们把内部发射机制看一眼，就知道这分工是怎么实现的。QLineEdit 内部维护一个 edited 标志，用户键入的路径上它为真，程序调 setText 的路径上它为假，改动收尾时统一检查：edited 为真才发 textEdited，textChanged 则无论来路都发。同一个内部函数里两条 emit 挨着，一条看标志，一条不看，分工就落在 edited 这个布尔量上。

QCompleter 的选中恰好走程序路径。QLineEdit 给自己接补全器时，把 completer 的 activated(const QString &) 直接连到 QLineEdit::setText，您从列表里选中的词条是 setText 塞进去的，edited 标志为假，textEdited 不响。开篇第三个场面的来路在这里：实时搜索挂在 textEdited 上，补全选中的词就漏搜，得在 activated 里自己补一发搜索逻辑。顺序也是确定的，activated 在前、textChanged 在后，连接本身定下了这个次序，信号发出之后槽里才去调 setText。由此有个联动的提醒：您要是在 textChanged 里清状态，activated 的处理函数可能读到已经被清掉的数据，表单联动复杂时留个心眼。

验证 edited 的行为最直观，示例第三分区的搜索框现成。m_completerEdit 已经接上 completer，在 custom_validator_demo.cpp 第 174 行。textEdited 和 textChanged，您在它后面各接一个 qDebug 打点，手动键入时两条都响，从列表里选中词条时只有 textChanged 响。

## QCompleter 落地时的弹窗、模型与绑定

配置本身不复杂，咱们看搜索框的标配，忽略大小写加中间匹配：

```cpp
auto *completer = new QCompleter(words, this);        // words 是 QStringList 词表
completer->setCaseSensitivity(Qt::CaseInsensitive);
completer->setFilterMode(Qt::MatchContains);           // 词腹里含关键词也算命中
searchEdit->setCompleter(completer);
```

filterMode 只认 MatchStartsWith、MatchContains、MatchEndsWith 三个值，传别的 Qt::MatchFlag 只换来一条警告，什么也不改。大小写也不归它管，Qt::MatchCaseSensitive 放进去没有作用，得走 setCaseSensitivity。咱们在示例里换一次就能看见差别。custom_validator_demo.cpp 第 171 行的 MatchContains 改回默认的 MatchStartsWith，词表不动。敲 thread 一个藏在词腹里的关键词，列表当场空掉，改回去又出来。

弹出列表的出身咱们交代一句，它默认是 QCompleter::popup() 现场建的一枚 QListView，窗口标志置了 Qt::Popup，焦点代理交回输入框，要换自绘弹层走 setPopup()。它的 transient parent 挂在输入框所在窗口，显出来那一刻按输入框的全局坐标贴边摆放，超出屏幕就按屏幕可用区夹取。显示期间不跟着输入框挪，输入框滚动或移动，弹层停在原位。

补全数据来自数据库或网络时，构造期固定一个 QStringList 不够用，标准做法是 QSortFilterProxyModel 配自定义的 QAbstractItemModel，数据就绪后更新模型内容。直接 completer->setModel(newModel) 换模型也能跑，动的只有匹配引擎，当前的过滤结果清空，弹层对象原地不动也不隐藏，filterMode、大小写、补全模式这些配置原样保留，旧模型要是以 completer 为父对象，会被顺带释放。绑定则要记一件事：QLineEdit::setCompleter 发现 completer 还没绑控件时，会替您调 setWidget 绑上自己。可一枚 completer 伺候不了两个输入框，同一枚 completer 复用给别的框，焦点一进新框，focusInEvent 就把 setWidget 抢到新框上，弹层跟着焦点走，旧框手里的补全当场失效。想让两个框共用一份词表，各配一枚 completer，底下的模型可以共享。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17。QValidator、QIntValidator、QDoubleValidator 定义在 QtGui，QCompleter 属于 QtWidgets，示例可在 Linux、macOS、Windows 桌面平台编译运行。

[Qt 文档 · QLineEdit](https://doc.qt.io/qt-6/qlineedit.html) -- 单行输入框，掩码字符表、text 与 displayText 的差别、editingFinished 的验证前提都在类文档里

[Qt 文档 · QValidator](https://doc.qt.io/qt-6/qvalidator.html) -- 验证器基类，validate 三态返回与 fixup 的语义

[Qt 文档 · QIntValidator](https://doc.qt.io/qt-6/qintvalidator.html) -- 整数验证器，locale 对解析的影响与自带的 fixup

[Qt 文档 · QDoubleValidator](https://doc.qt.io/qt-6/qdoublevalidator.html) -- 浮点验证器，notation 设置与小数点的 locale 牵扯

[Qt 文档 · QCompleter](https://doc.qt.io/qt-6/qcompleter.html) -- 自动补全组件，filterMode、popup 弹层与模型绑定的配置

---

输入类五连在 QLineEdit 这里开了个头，验证、掩码、信号时序这套功夫后面几篇全用得上。下一篇 [3.23 QTextEdit 进阶](./23-qtextedit-advanced.md) 进多行文本，从富文本文档结构讲起。想再动手的，请您把示例（examples/advanced/03-qtwidgets/22-qlineedit-advanced/，可执行文件 22-qlineedit-advanced）完整跑一轮：第一分区敲半个 IP 看 Intermediate、敲到 300 看 Invalid 回退，第二分区换 IP 掩码再叠 QIntValidator 复现敲不进字的冲突，第三分区切 filterMode 看匹配方式的变化，末了照 editingFinished 一节补连接，亲手抓住信号被前提拦住的那一刻。想加码的，照第一分区的 IpAddressValidator 拼一个 IpPortValidator，验证 `192.168.1.1:8080` 这样的地址加端口，IP 每段 0-255、端口 1-65535，输入中途给 Intermediate，提交时用 QToolTip::showText 在输入框旁提示哪一段超了范围，补全词表顺手换成最近用过的十条地址，activated 里接上验证逻辑。您手头要是有正被 validator 卡住回车的输入框，拿 Intermediate 与 Invalid 的分野对一对，八成就有答案。
