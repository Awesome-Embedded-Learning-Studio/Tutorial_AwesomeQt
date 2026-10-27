---
title: "3.26 QKeySequenceEdit 进阶"
description: "入门篇把创建控件、监听 keySequenceChanged、把 QKeySequence 设到 QAction 的录入流程过了一遍，进阶篇看 keyPressEvent 里的录制状态机与收尾信号的次序，把“焦点一走就丢旧值、半截序列照样提交”的真实行为对着源码确认下来，冲突检测给出系统保留键查表、matches 前缀扫描与 shortcuts 全量遍历，末尾用 Apple 平台的 Ctrl 与 Meta 对调解释 Cmd+S 为什么解析不出、配置存储为什么认 PortableText，配套示例在 examples/advanced/03-qtwidgets/26-qkeysequenceedit-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.26——QKeySequenceEdit 进阶

[3.25 QTextBrowser 进阶](./25-qtextbrowser-advanced.md) 收尾把话递到了最后一站，说本篇是个只管一件事的小控件，录入快捷键、拦截冲突，思路与前四站完全两样。这话不虚。输入类前四站处理的都是文字，本站的 QKeySequenceEdit 收的是按键，您敲下的组合被它收进一个 QKeySequence，再由您设到 QAction 或 QShortcut 上去。入门篇把创建控件、监听 keySequenceChanged、拿到值设给 QAction 的流程跑通了，真放进一个有几十个配置项的设置面板，问题立刻浮出来：值在什么时刻才算落定，两个配置项设成了同一个键谁来报，Windows 上录好的序列拿到 macOS 上还算不算数。本篇按录制、冲突、跨平台的次序过完。

咱们按这样的次序走，从按键在控件里怎么被收下看起，跟着是收尾信号的次序与恢复旧值的位置，再过多键序列与冲突检测，跨平台的解析与显示收尾。正文里的行号都带着出处，示例代码在 examples/advanced/03-qtwidgets/26-qkeysequenceedit-advanced/，Qt 内部实现指源码树里的 qkeysequenceedit.cpp、qkeysequence.cpp、qshortcutmap.cpp 与 qaction.cpp。

## 壳是一枚 QLineEdit，录制全在 keyPressEvent 一处

QKeySequenceEdit 本体是个 QWidget，里面住一枚 QLineEdit（qkeysequenceedit.cpp 第 22-24 行），构造时 setFocusProxy 把焦点交给外壳（第 41 行），键事件全归 keyPressEvent 处理，位置在 qkeysequenceedit.cpp 第 308-377 行。咱们按落下的顺序一段段看这个函数。第一颗键落下时 prevKey 还是 -1，clear() 在这里把旧序列清空，新一轮累积从头开始（第 319-322 行）。跟着是修饰键的关卡，您按下 Ctrl、Shift、Alt、Meta 或者无法识别的键，函数直接返回，什么也不记（第 325-331 行）。主键连同修饰一起入库（第 366-367 行），Ctrl、Meta、Alt 三个修饰键由 translateModifiers 直译（第 51-62 行），Shift 单独走 QKeyMapper::possibleKeys，从当前布局能产出的键值里，挑出带这套修饰的键值（第 343-360 行），Ctrl+Shift+1 才不会跟 Ctrl+! 混在一起。三类键受到的待遇，咱们列成表：

| 按下的键 | keyPressEvent 的处理 |
| ---- | ---- |
| 修饰键单独按下（Ctrl、Shift、Alt、Meta） | 只清掉占位文本，不记入序列 |
| 收尾键（默认 Tab 与 Backtab） | 触发 finishEditing，录制收尾 |
| 其余主键 | 连同修饰存入序列，行编辑框刷新显示 |

event() 里还有两处要看。ShortcutOverride 一律 accept（第 287-289 行），应用里已经绑定的 Ctrl+S 不会趁控件有焦点抢走按键，您才可能把旧快捷键原样录进来替换它。QEvent::Shortcut 则被直接吞掉，返回 true（第 285-286 行）。控件另有一个顺手的开关，setClearButtonEnabled(true) 把清除按钮转发给内部那枚 QLineEdit（第 159-164 行，Qt 6.4 引入），init 里把 lineEdit 的 textChanged 连去了 clear（第 25-29 行），点掉清除图标文本变空，整条序列跟着清空，一键清空是现成的。

流传很广的说法是，按了修饰键切走焦点，控件会留下显示 Ctrl+ 的残留文本，录制却取消了。咱们对着 6.9.1 的源码把这句改掉。行编辑框的文本只在主键入库后才刷新（第 375 行），修饰键单独按下根本不写文本。焦点离开走的是 finishEditing，resetState 按当前 keySequence 重写文本、放回 Press shortcut 占位符（第 64-71 行），残留的 Ctrl+ 在这个版本不存在。真实的行为另有两件，后果都比残留文本重。其一，旧值在第一颗键落下的那一刻就被清掉了（第 320 行），您点进控件按了一下 Ctrl 又反悔，点进别的控件，序列只剩空值，之前的绑定没了。其二，录到一半就点走，比如 Ctrl+S 已入库、框里显示 Ctrl+S, ...，focusOutEvent 照样 finishEditing（第 412-418 行），这半截被当正式值提交。所以恢复逻辑要写在 editingFinished 里，检查 keySequence().isEmpty()，为空就把旧值放回去。示例的 onEditingFinished 是现成样板，出处 shortcut_conflict_panel.cpp 第 207-213 行。

## 收尾的路都汇到 finishEditing，两条信号分了工

finishEditing 是收尾的唯一出口（第 73-80 行），进去做两件事，resetState 把行编辑框文本按 keySequence 重写、占位符放回（第 64-71 行），跟着发两条信号（第 78-79 行），keySequenceChanged 在前，editingFinished 在后。能把录制送进这个出口的路径有四条。收尾键是一条，Tab 与 Backtab 默认在 finishingKeyCombinations 里（init 第 33 行），keyPressEvent 认出它就收尾（第 312-315 行）。计时是一条，最后一颗键松开后 1 秒（keyReleaseEvent 第 386-391 行起 releaseTimer，timerEvent 第 398-407 行收尾），计时从松开那一刻起算，按住不放就永远不收尾。数量是一条，录满上限立即收尾（第 387-390 行）。焦点是一条，focusOutEvent 收尾（第 412-418 行），例外是 Qt::PopupFocusReason 引起的焦点切换不算离开（第 415 行），您在控件旁边弹个提示气泡，录制悬着不收尾，别在气泡的显示逻辑里等 editingFinished。

两条信号的分工值得单独说。程序里调 setKeySequence 只发 keySequenceChanged，不发 editingFinished（第 231-258 行，发射在第 257 行）。所以在 editingFinished 的槽里调 setKeySequence 恢复旧值，不会把自己再触发一遍，恢复逻辑放这里是稳的。反过来，把“这是用户录完的值”这类判定挂到 keySequenceChanged 上就没了依据，程序赋值也会过一遍您的槽。示例把两条连接分开各管各的，editingFinished 管恢复，keySequenceChanged 管刷新，出处 shortcut_conflict_panel.cpp 第 73-78 行，咱们照这个分法走就行。

## 多键序列默认开着，上限四段从构造就在

入门篇说过 QKeySequenceEdit 默认只录单个组合键，这句话 6.9.1 里站不住。maximumSequenceLength 属性 Qt 6.5 引入，默认 4，上限也是 4（qkeysequenceedit.cpp 第 173-181 行），文件头部的 static_assert 把 MaxKeyCount == 4 写成编译期检查（第 16 行），整条录制管线就是按四段铺的。没录满时，长这样：Ctrl+K 落库后框里显示 Ctrl+K, ...（第 371-374 行），下一颗键续上，或者等 1 秒把已有段落收尾。VSCode 把 Ctrl+K, Ctrl+C 给注释代码用，QKeySequenceEdit 默认就能录下这样的序列，您一个属性都不用改。

这个属性的边界有两处要留神。setMaximumSequenceLength 收到越界值不抛异常，qWarning 之后保留旧值（第 192-196 行），您给配置面板传 0 或者 8 都等于白设。把上限往小调时，已有的超长序列被当场截断（第 198-203 行）。收尾键列表 finishingKeyCombinations，同样 Qt 6.5 引入（第 206-229 行），默认 {Qt::Key_Tab, Qt::Key_Backtab}（init 第 33 行）。您想让 Tab 不再收尾，改由别的键收尾，换这个列表就行。普通桌面应用的快捷键配置单段组合占绝大多数，把上限收到 1 是设置面板更常见的选择，编辑器体系才需要四段。

## 冲突要在设上去之前查，Qt 的兜底只兜一半

咱们把冲突检测的几个维度摆成表，每层查的东西不同，示例的 refreshAllStatus 走了保留键、精确重复两道，前缀一道留到结语实验再补上，出处 shortcut_conflict_panel.cpp 第 172-188 行：

| 检测维度 | 查法 |
| ---- | ---- |
| 系统保留键 | 对照已知保留键列表，命中只警告不拒绝 |
| 前缀冲突 | matches() 双向扫描，出现 PartialMatch 即报 |
| 重复绑定 | 与面板内其他条目比精确相等 |
| 真冲突落地 | Qt 自己处理，谁都不触发，stderr 一行警告 |

### 系统保留键查表拦在前头

操作系统有一批快捷键应用抢不过：Windows 的 Win+L 锁屏、Win+D 显示桌面，macOS 的 Cmd+Space 聚焦搜索，Ctrl+Space 的输入法切换两边平台都有。后果是把这些键设给某项操作后，按键被系统截走，应用收不到事件，界面上却显示设置成功，用户按了没反应还找不到原因。检测没有跨平台的统一方案，Windows 上可以用 RegisterHotKey 试注册，注册失败即被占用，这是 Win32 的常规做法，跟 Qt 无关。macOS 的系统级快捷键由系统接走，基本无从探测。咱们能做的是维护一份已知保留键列表，检测时加一道比对，示例的 kSystemReservedShortcuts 列的就是这些键，出处 shortcut_conflict_panel.cpp 第 20-25 行。表永远列不全，您在界面上标注“可能被系统拦截”，比断言不可用诚实，示例的橙色标签用的就是这个措辞（第 173-179 行）。

### == 比较查不出前缀冲突，matches() 才看得见

QKeySequence 的 == 是整条序列的精确比较，Ctrl+S 与 Ctrl+Shift+S 不相等，Ctrl+K 与 Ctrl+K, Ctrl+C 也不相等。可 Qt 的快捷键分发认前缀，QShortcutMap::find 扫注册表时，找到 ExactMatch 就不再收 PartialMatch，出处 qshortcutmap.cpp 第 437-439 行，注释原话是有了 identicals 就用不着 partials。于是 Ctrl+K 单步与 Ctrl+K, Ctrl+C 两步并存时，您按下 Ctrl+K，单步立即命中收走按键，两步序列永远等不到第二段。前缀冲突用 == 永远查不出来，得用 matches()，它返回三态：

| matches 返回值 | 含义 |
| ---- | ---- |
| NoMatch | 两条序列沾不上边 |
| PartialMatch | 一条是另一条的前缀 |
| ExactMatch | 完全相同 |

方向是单向的，短序列 matches 长序列给 PartialMatch，反过来在开头就返回 NoMatch（qkeysequence.cpp 第 1354-1359 行），咱们扫描时得两个方向都做，漏了方向就漏了冲突。示例第 141 行用的是精确相等，前缀冲突它查不出，结语里的实验会让面板当场报出来。

### 检测面要扫 shortcuts()，不止 shortcut() 一个

QAction::setShortcuts 一次能挂多个快捷键，QKeySequence::Copy 这类 StandardKey 在 Windows 上本身就带两个绑定，Ctrl+C 之外还有 Ctrl+Ins（qkeysequence.cpp 第 229 行的标准表里写着），Cut 那行同样带两个，Ctrl+X 与 Shift+Del 并用（第 228 行）。您检测时如果只拿 shortcut() 比，漏掉的就可能是真实的冲突，shortcut() 的实现只回 shortcuts() 列表的第一个元素，出处 qaction.cpp 第 343-350 行，后面那些它不管。遍历 shortcuts() 全量比对，检测面才算扫全。

### 真冲突落地时 Qt 谁都不触发，只在 stderr 喊一声

最坏的情况咱们也交代一下。前面三道都漏了、两个 QAction 真挂上同一个键，QShortcutMap 找出一组 identicals，dispatchEvent 从里挑一个 owner，把带歧义标记的 QShortcutEvent 发过去，出处 qshortcutmap.cpp 第 605-606 行，QAction::event 收到 isAmbiguous 的事件，打一行 Ambiguous shortcut overload 警告，拒绝激活（qaction.cpp 第 1043-1047 行）。后果是两个动作都不触发，界面上零提示，警告只在 stderr 里，用户看到的就是快捷键失灵。所以面板必须在设上去之前自查，Qt 的兜底保证不出乱子，不保证能用。

## Cmd+S 解析不出，Apple 平台把 Ctrl 和 Meta 对调

有一道流传很广的调试题，`QKeySequence a("Ctrl+S")` 与 `b("Cmd+S")` 在 macOS 上比较，相等吗？常见的解释是 Cmd 在 macOS 上解析成 Qt::META，跟 Ctrl 的 Qt::CTRL 不同，所以不等。这个解释两头都不对，咱们对着源码过一遍。解析这头，字符串里认得的修饰键名只有 Ctrl、Shift、Alt、Meta。NativeText 与 PortableText 的表里都没有 Cmd，出处 qkeysequence.cpp 第 1061-1065 与 1070-1074 行。解析不出的段落落成 Qt::Key_unknown（第 1147-1148 行），存进序列（第 1018 行）。b 非空、却是按不出来的无效值，isEmpty 查的只是首键是否为零（第 918-921 行），== 自然不等。要表达 Command+Space，写的该是 Ctrl+Space，Qt 把 Ctrl 对调到了 Command 键上。麻烦跟着就来，这串放到 Windows 和 Linux 又落回物理 Ctrl+Space，恰好是输入法切换，跨平台保留键表没法用同一个串表达，得按平台分别建，示例的保留键列表就是这么给各条目注明平台的。

咱们再看对调。QKeySequence 的类文档把规则写在明面上，Apple 平台上，“Ctrl”、Qt::CTRL 与 Qt::ControlModifier 对应键盘上的 Command 键，“Meta”与 Qt::MetaModifier 对应 Control 键，出处 qkeysequence.cpp 第 199-204 行，您写一遍快捷键描述，到 macOS 上自动落对位置。解析侧的 decodeString 建 NativeText 修饰表时，起手就查 Qt::AA_MacDontSwapCtrlAndMeta（第 1048-1058 行），默认对调，⌘ 符号记在 Qt::CTRL 名下。物理层面 macOS 的按键翻译做同一件事，用户按下 Command，事件报上来的修饰就是 ControlModifier。想回到字面对应，您把这个属性设回去，适配就关了。跨平台还有更省事的一档，QKeySequence(QKeySequence::Save) 这类 StandardKey 构造，示例十行默认值全用它（shortcut_conflict_panel.cpp 第 100-111 行），保存、撤销、复制在各平台自动到位。

## 存储认 PortableText，显示认 NativeText

入门篇说 toString() 默认给 NativeText，这个说法咱们得纠正：6.9.1 的默认参数是 PortableText（qkeysequence.h 第 141 行），存配置直接 seq.toString() 拿到的就是可移植格式。NativeText 要显式给，给显示用。QKeySequenceEdit 刷新行编辑框时传的就是它，出处 qkeysequenceedit.cpp 第 69 行，macOS 上出来的是菜单栏里那套 ⌘、⌥、⇧、⌃ 符号（encodeString 走 appleSymbolForQtKey，qkeysequence.cpp 第 1233-1258 行）。两种格式各有各的去处：

| SequenceFormat | 输出长相与去处 |
| ---- | ---- |
| NativeText | 平台符号与翻译名，macOS 出 ⌘，给界面显示 |
| PortableText | Ctrl、Meta 这类拉丁名，写进配置文件 |

NativeText 的麻烦在翻译名与符号。非 Apple 平台的 encodeString，对 NativeText 会取翻译名（qkeysequence.cpp 第 1263-1270 行）。德语版把 Ctrl 译作 Strg，Shift 译作 Umschalt，qtbase_de.ts 的 QShortcut 段里就能对上，Qt 的键序列测试也拿 Strg+K 当译名样本。装上德语语言包跑 toString(NativeText)，出来的是 Strg+S。把这个返回值存进文件，换一台语言不同的机器，解析时翻译表对不上，段落照旧落成 Key_unknown，序列就废了。PortableText 的表永远是五个拉丁名（第 1070-1074 行），到哪台机器都认得。读回这边还有个默认值的不对称，字符串构造函数默认 NativeText（qkeysequence.h 第 121 行），fromString 默认 PortableText（第 142 行）。存的是 PortableText 串，读回用 fromString 显式带上格式最稳，字符串构造其实也认拉丁名（NativeText 的解析表把未翻译名排在最后，qkeysequence.cpp 第 1087 行），您只是别让翻译名混进存档。

自定义快捷键显示组件时还有一档可查：想拿某个标准操作在当前平台的真实绑定列表，QKeySequence::keyBindings(QKeySequence::Save) 返回的就是它，出处 qkeysequence.h 第 149 行，按列表顺序展示比您自己硬编码可靠。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，QKeySequenceEdit 在 QtWidgets，QKeySequence 在 QtGui，链接 Qt6::Widgets 即可，多键序列相关属性要求 Qt 6.5 起的版本。

[Qt 文档 · QKeySequenceEdit](https://doc.qt.io/qt-6/qkeysequenceedit.html) -- 录入控件，maximumSequenceLength、finishingKeyCombinations 与 editingFinished 都在这页

[Qt 文档 · QKeySequence](https://doc.qt.io/qt-6/qkeysequence.html) -- 序列数据类型，SequenceFormat、matches 与 keyBindings 的说明

[Qt 文档 · QKeySequence::StandardKey](https://doc.qt.io/qt-6/qkeysequence.html#StandardKey-enum) -- 跨平台标准快捷键枚举

[Qt 文档 · QAction](https://doc.qt.io/qt-6/qaction.html) -- 动作抽象，setShortcuts 与 shortcuts 的说明

---

输入类五连到本篇收官。QLineEdit 管单行文字，QTextEdit 与 QPlainTextEdit 管多行，QTextBrowser 管只读文档，QKeySequenceEdit 管按键，五件控件共用编辑这层外壳，各自收的东西并不相同。下一站换到选择类，[3.27 QComboBox 进阶](./27-qcombobox-advanced.md) 讲下拉框，录入这件事从键盘回到列表。想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/26-qkeysequenceedit-advanced/，可执行文件 26-qkeysequenceedit-advanced），十行配置项在源码里对应十个 StandardKey 默认值，界面就是一面冲突检测面板。实验顺着正文走，改哪行、看什么现象，咱们给您列好。第一步把 shortcut_conflict_panel.cpp 第 141 行的 existing == sequence，换成 existing.matches(sequence) != QKeySequence::NoMatch，再把“新建文件”设成 Ctrl+K、“查找”设成 Ctrl+K, Ctrl+C。原来 == 扫不出，面板不报，换完之后“查找”一行的标签当场变红，报“冲突：新建文件”，前缀冲突就落进了面板。第二步在第 56 行 new QKeySequenceEdit 后面补一行，m_entries[i].sequenceEdit->setMaximumSequenceLength(1)，再按 Ctrl+K、Ctrl+C。第二段进不来（qkeysequenceedit.cpp 第 340-341 行），Ctrl+K 一松手就收尾（第 387-390 行），Ctrl+K, ... 的等待形态根本不出现。第三步把第 207-213 行 onEditingFinished 里空值恢复的分支注释掉，在一行里录一个值（比如第一步那个 Ctrl+K）再回来做本步：点进那一行按一下 Ctrl，再点进另一行的录入框。标签停在“未设置”，旧值回不来，因为第一颗键落下时 clear() 就把它清掉了，出处 qkeysequenceedit.cpp 第 319-322 行。还原代码再做一次，旧值恢复。完成标准咱们列好，十行配置各自恢复正确，Ctrl+K 与 Ctrl+K, Ctrl+C 能报出前缀冲突，保留键给橙色提示。
