---
title: "3.28 QFontComboBox 进阶"
description: "入门篇把 setCurrentFont、currentFontChanged 与 setFontFilters 跑通了，进阶篇对着 6.9.1 的源码把字体枚举的机制认清楚，过滤发生在私有 updateModel 的两道掩码检查里，等宽与比例共用 isFixedPitch 一份判定，掩码两位齐设等于不查，书写系统过滤当场生效，可状态寄存在自家的私有委托身上，委托一换就哑，6.3 起的 setSampleTextForFont 与 setDisplayFont 是定制显示的官方口子，弹窗宽度被 event 撑到控件的 5/3，配套示例在 examples/advanced/03-qtwidgets/28-qfontcombobox-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.28——QFontComboBox 进阶

[3.27 QComboBox 进阶](./27-qcombobox-advanced.md) 收尾把话递了过来，说 QFontComboBox 是 QComboBox 的子类，字体枚举装在现成的模型里，本篇的委托与换列那套到它身上直接落地。咱们接着这句往下核对，落地的情况得分开说，委托那套落了地，接住它的却是一枚私有委托，换列那套落了空，模型是单列的 QStringListModel，没有第二列可换。入门篇把 setCurrentFont、currentFontChanged、setFontFilters 的调用流程跑通了，选个字体设给 QTextEdit 确实够用，咱们把字体枚举背后的机制放到 6.9.1 的源码里认一遍，几处流传很广的说法在正文里对着行号改。

咱们把次序摆成这样，从出厂构造里那几样默认配置认起，跟着是 setFontFilters 的掩码检查与当前字体的去留，书写系统过滤为什么一换委托就哑，6.3 的官方口子与换委托的实操收尾。正文里的行号都带着出处，示例代码在 examples/advanced/03-qtwidgets/28-qfontcombobox-advanced/，Qt 内部实现指源码树里的 qfontcombobox.cpp、qfontcombobox.h 与 qfontdatabase.h。

## 出厂就配好可编辑、单列模型与一枚私有委托

笔者把 6.9.1 的实现对着 27 篇过了一遍，咱们从构造函数认起，几步走完（qfontcombobox.cpp 第 398-419 行）。currentFont 开头就记下控件当前的 font，出处第 402 行，setEditable(true) 打开编辑（第 403 行），setModel 挂一枚 QStringListModel（第 405-406 行），setItemDelegate 装上自家的 QFontFamilyDelegate（第 407 行），视图顺手设了 uniformItemSizes（第 408-410 行），setWritingSystem(Any) 一调，第一轮填列表跟着发生（第 411 行）。

末尾两根连接咱们各记一笔，currentTextChanged 通向私有槽，qApp 的 fontDatabaseChanged 通向 updateModel（第 413-418 行），程序跑着的时候系统装了新字体，列表自己刷新，析构里再把两根连接断开（第 425-430 行）。27 篇讲过的可编辑与内置补全，在它身上出厂就是开着的，行内补全对着家族名补，用的模型就是家族列表本身。

27 篇的换列到这儿没得施展，咱们看取数的接口跟着怎么落。模型单列，DisplayRole 里装的就是家族名，setModelColumn 没有第二个列号可喂，currentData 默认的 Qt::UserRole 上也没存过东西，取选中值的正道就是 currentText 与 currentFont，27 篇那套显式索引用不上。

委托本身咱们细认一遍。QFontFamilyDelegate 直接继承 QAbstractItemDelegate（第 166-168 行），不走 QStyledItemDelegate 那套样式绘制，名称用不用对应字体有一条门槛，writingSystemForFont 查出这个家族支持的书写系统，拉丁在其中才把绘制字体换成家族自己的（第 205-208 行），纯中文字体的名称用控件字体显示，右侧另补一段示例文字，类文档第 371-376 行就是这么写的。

名称的字号咱们接着看，放大到行字体的 1.5 倍（第 201 行）。右侧那段示例默认取 QFontDatabase::writingSystemSample(system)（第 260 行），system 按行解析，偏向系统界面语言对应的书写系统（第 92-102 行），简体繁体互为回退（第 121-129 行），解析成 Any 又没自设示例的行，右侧那截干脆不画（第 257 行）。

弹窗宽度咱们也跟 27 篇对一下。27 篇说常规样式的弹窗跟控件自身 rect 同宽，QFontComboBox 在 event 里拦下 Resize，把弹窗窗口定宽成 min(控件宽 × 5/3, 屏幕可用宽)（第 529-539 行），弹窗比控件宽出一截。sizeHint 的宽度咱们也有数，按 14 个 m 字符的横向宽度来定（第 544-550 行）。

## 过滤是两道掩码检查，等宽与比例共用一份判定

setFontFilters 把掩码存进私有成员，跟着调 updateModel（第 480-485 行），整个过滤机制的正身就在 updateModel（第 286-349 行），咱们按顺序看。起步取 families(system)，system 是书写系统过滤（第 302 行），循环里头一步跳过私有家族 isPrivateFamily（第 309 行），再过两道掩码。

```cpp
// qfontcombobox.cpp updateModel 的过滤骨架（第 312-319 行，外层循环省略）
if ((filters & scalableMask) && (filters & scalableMask) != scalableMask)
    if (bool(filters & QFontComboBox::ScalableFonts) != QFontDatabase::isSmoothlyScalable(family))
        continue;      // 可缩放属性与 ScalableFonts 位对不上，这条家族出局
if ((filters & spacingMask) && (filters & spacingMask) != spacingMask)
    if (bool(filters & QFontComboBox::MonospacedFonts) != QFontDatabase::isFixedPitch(family))
        continue;      // 固定间距属性与 MonospacedFonts 位对不上，这条家族出局
```

两道检查的门槛咱们放一起看，是同一个写法，掩码设了位、又没设全，才真的开查。MonospacedFonts 与 ProportionalFonts 同时传入，掩码设全，这一道整个跳过，效果等同不过滤，流传的解释说成二者并集为全集，根因其实在短路里，判定根本没跑。

还有一层咱们别错过，QFontDatabase 这边没有“比例字体”的独立查询，ProportionalFonts 只是把 isFixedPitch 的结果反过来用，要等宽就收为真的，要比例就收为假的，一份判定两种用法。

可缩放那边同样有个名字上的讲究，6.9.1 用的是 isSmoothlyScalable（平滑缩放），isScalable 会把位图可缩放的字体也算进来，两个接口 QFontDatabase 都提供，笔者对着 qfontdatabase.h 第 95-96 行核过，过滤走的是严的那档。现代系统装的多是 TrueType 与 OpenType，您拿 NonScalableFonts 过滤基本筛不出东西，真正能缩小列表的还是等宽与比例。

当前字体的去留您单独记。updateModel 拿 QFontInfo 解析出的当前家族去结果里找，找到就记下偏移，家族名带 “ [风格]” 后缀的形式也认（第 305-323 行），填完列表 setCurrentIndex 把选中项放回原位（第 347 行），setStringList 那一下信号被 QSignalBlocker 拦着（第 329-339 行），源码注释把权宜写在明面上，TODO 留在第 328 行。换 setFontFilters 不会把已选字体冲掉，掉出过滤结果才落到第一条并触发 currentFontChanged，整份列表被过滤空才把 currentFont 重置成空 QFont 再发一次（第 341-345 行），流传的“一调过滤选中就丢”对不上这个版本。

currentFont 的更新链也顺出来给您，选中文本变了走私有槽 currentChanged，家族名与上回不同才更新并发 currentFontChanged（第 352-360 行），setCurrentFont 字体真变了，重建模型再补发（第 505-515 行）。

程序运行时系统的字体库变了也不用您动手刷新，fontDatabaseChanged 已经接进 updateModel，出处第 416-417 行。

## 书写系统过滤当场生效，状态寄存在私有委托身上

setWritingSystem 的实现就两步，出处 qfontcombobox.cpp 第 442-449 行，把枚举写进委托的 writingSystem 成员，跟着当场调 updateModel。咱们改一处流传的说法，“过滤要等下一轮模型重建才生效”在这个版本站不住，setFontFilters 与 setWritingSystem 都是同步的，调完 model()->rowCount() 拿到的就是过滤后的数。两道过滤取交集，families(system) 头一轮按书写系统收，循环里的掩码再筛一遍，只要支持简体的等宽字体，setWritingSystem 一行，setFontFilters(MonospacedFonts) 一行，就到位。

枚举名您别沿用 Qt 5 的习惯，6 里没有 SimplifiedChineseScript 这个值，简体的值叫 SimplifiedChinese，出处 qfontdatabase.h 第 49 行，整套值表在第 22-62 行。

哑火的地方在状态的住址上。writingSystem 这个值寄存在 QFontFamilyDelegate 身上，setWritingSystem 拿 qobject_cast 找它，找不到就静默跳过（第 445-447 行），updateModel 里同样 qobject_cast 一把，拿不到就回退 Any（第 299-300 行），writingSystem() 读回来也只剩 Any，出处第 451-457 行。您要是像示例那样 setItemDelegate 换成自己的委托，书写系统过滤整条失效，切了枚举列表不动，没有警告也没有断言。模型那边还守着一道，updateModel 开头认准 QStringListModel，不是它就直接返回（第 296-298 行），setModel 换了数据源，字体枚举的整套私有机制退场，列表从此归您管。

自己接管的路子是现成的，QFontDatabase::families(system) 这个重载，直接返回支持指定书写系统的家族列表，出处 qfontdatabase.h 第 85 行，您把结果灌回 model() 就行。

示例面板的结构咱们顺一遍，书写系统下拉的选项，来自 QFontDatabase::writingSystems() 遍历，名字用 writingSystemName 取，出处 filtered_font_combo.cpp 第 120-135 行，切中的枚举从 currentData 取回，把 setWritingSystem 调上（第 141-151 行）。底部预览区拿 currentFont 设字体，示例文字按自带的书写系统映射表换（第 157-171 行）。装着默认委托的时候，这一套就是完整可用的书写系统过滤器，换成自定义委托之后，留给它的只剩预览区那一半还能动。

## 定制显示走 6.3 的官方口子，换委托放到末档

列表行里名称的字体与右侧的示例文字，6.3 起都有官方接口改，不用动委托。setSampleTextForSystem 按书写系统换示例文字，setSampleTextForFont 按家族换，级别更高（第 552-599 行），setDisplayFont 给指定家族指定显示字体（第 601-610 行），三份表都由私有委托当场消费，名称字体头一步查 displayFontForFontFamily（第 210 行），示例文字从家族的映射查起，落空再落到书写系统的映射（第 256 行）。您只想给少数几个字体换个示例文案或换个显示字体，靠它们就够收工。

要的是每一行右侧都带预览的列表，才轮到换委托，咱们看示例怎么干的。FontPreviewDelegate 头一步，initStyleOption 备好选项，让基类画背景与选中高亮（font_preview_delegate.cpp 第 26-30 行），名称取 DisplayRole 的家族名，构造对应 QFont 加粗画在左半边（第 35-48 行），右半边用同款字体画预览文本，颜色取 PlaceholderText 跟名称区分开（第 51-59 行），行高在 sizeHint 里抬到不低于 30（第 68-74 行）。

装上去就一行，filtered_font_combo.cpp 第 77 行，咱们就是这么接的。

有一处委托永远管不着的您心里有数，QFontComboBox 出厂可编辑，脸面文本住在行编辑框里（27 篇核对过的机制），选中再花哨的字体，弹窗里花，输入框里不花，27 篇讲的 setLabelDrawingMode(UseDelegate)，在它身上也没有出场机会。

开销咱们一起算。每行 paint 现构 QFont，名称与示例各跑一次度量与文本布局，官方委托也这么干，没有做缓存（第 243-251 行），QFont 构造本身只是描述符，重在度量与排版。视图设了 uniformItemSizes（第 408-410 行），行高一致，滚动估算便宜。几百字体的机器现算扛得住，您真要往低端设备上塞长预览文本，再把字体对象与度量缓存起来不迟。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，示例文字与显示字体那组接口要求 Qt 6.3 起，QFontComboBox 与 QStyledItemDelegate 在 QtWidgets，QFontDatabase 在 QtGui，链接 Qt6::Widgets 即可。

[Qt 文档 · QFontComboBox](https://doc.qt.io/qt-6/qfontcombobox.html) -- 字体选择下拉框，fontFilters 与 writingSystem 属性、6.3 的示例文字与显示字体接口都在这页

[Qt 文档 · QFontComboBox::FontFilter](https://doc.qt.io/qt-6/qfontcombobox.html#FontFilter-enum) -- 字体类型过滤器的五个枚举值与组合掩码

[Qt 文档 · QFontDatabase](https://doc.qt.io/qt-6/qfontdatabase.html) -- 字体数据库，families、isFixedPitch、isSmoothlyScalable、writingSystemSample 都在这页

[Qt 文档 · QFontDatabase::WritingSystem](https://doc.qt.io/qt-6/qfontdatabase.html#WritingSystem-enum) -- 书写系统枚举，Qt 6 里简体的值名是 SimplifiedChinese

[Qt 文档 · QStyledItemDelegate](https://doc.qt.io/qt-6/qstyleditemdelegate.html) -- 自定义委托基类，换预览委托时从这里继承

---

选择类到本篇收尾，咱们下一站去 [3.29 QSpinBox 进阶](./29-qspinbox-advanced.md)，输入类从数字开始，QSpinBox 的正题在文本与数值之间的转换与验证（qspinbox.h 里 validate、valueFromText、textFromValue 一组虚函数），跟这篇出厂可编辑的行编辑框正好接得上。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/28-qfontcombobox-advanced/，可执行文件 28-qfontcombobox-advanced），页面上方一枚书写系统下拉、一枚字体下拉，底部一块预览区，切书写系统预览跟着换。

三步实验笔者都照着跑过，咱们顺着正文边改边看。第一步，在 filtered_font_combo.cpp 第 77 行，把 setItemDelegate 注释掉，切书写系统，列表跟着过滤了，把这一行装回去，列表不动而预览标签照样换示例文字，委托哑火的机制在眼前重演一遍（qfontcombobox.cpp 第 445-447 行）。

第二步咱们进构造函数，在 filtered_font_combo.cpp 第 74 行，setSizeAdjustPolicy 之后补一行，setFontFilters(QFontComboBox::MonospacedFonts)，列表只剩等宽字体，改成 MonospacedFonts | ProportionalFonts，列表回全量，掩码设全那道检查不跑（qfontcombobox.cpp 第 316-319 行）。

第三步，咱们进 font_preview_delegate.cpp 第 57 行，把 QPalette::PlaceholderText 换掉，改用 opt.palette.text().color()，名称与预览文本同色，主次一下就糊了，改回即还原。

完成标准咱们列好，书写系统过滤只在默认委托在位时生效，等宽过滤单独传 MonospacedFonts 就够，预览文本与名称的颜色分得开。
