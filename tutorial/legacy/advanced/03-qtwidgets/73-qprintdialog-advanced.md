---
title: "3.73 QPrintDialog 进阶"
description: "进阶篇把 QPrintDialog 的行为对到源码行上：对话框自己不存状态，setPrintRange 直写 QPrinter（qabstractprintdialog.cpp 第 149-153 行），不传打印机时对话框自造一只并拥有它（第 244-256 行），打印机已带页码范围会自动打开 PrintPageRange 选项（第 249-250 行），选项默认值文档说全关而私有构造函数给了四项（qabstractprintdialog_p.h 第 37-43 行），PrintDialogOption 六名成员留着 0x0020 缺位（qabstractprintdialog.h 第 32-39 行），Unix 确认键经 _q_checkFields 查完文件名才 accept（qprintdialog_unix.cpp 第 634-638、856-862 行），setupPrinter 把范围与份数逐项写回打印机（第 737-827 行），Windows 对非原生输出直接拒绝（qprintdialog_win.cpp 第 141-145 行），macOS 只翻译两项选项（qprintdialog_mac.mm 第 212-218 行），示例在 examples/advanced/03-qtwidgets/73-qprintdialog-advanced/"
---

# 现代Qt开发教程（进阶篇）3.73——QPrintDialog 进阶

打印的话题，咱们聊到对话框了。QPrintDialog 与 QPrinter 是搭档关系，构造函数拿的就是打印机指针，用户在界面上做的每个决定，最终都写进 QPrinter 本体，您拿到手的就是配置好的打印机对象。笔者把 Qt 6.9.1 打印支持模块的对话框实现从头读到尾，Unix、Windows、macOS 三份平台实现各翻了一遍，本篇从这对搭档怎么共用状态讲起，讲到选项开关的真实默认值、确认键按下之后的写回路径，落到三个平台的分岔。旧稿与网上流传的几处说法，笔者逐一拿行号对过，对不上的当场改讲。

引用文件咱们交代清楚。笔者主读 qabstractprintdialog.cpp 与它的头文件、私有头文件，平台实现是 qprintdialog_unix.cpp、qprintdialog_win.cpp 与 qprintdialog_mac.mm，打印范围那头还要借 qprinter.cpp 对证。入门篇走过的基本三步，造对话框、exec、accept 之后拿配置，本篇不重讲，配套示例是一只多页文本打印器，咱们把玩法放结语。

## 搭档一体：对话框不存状态，状态住在 QPrinter 里

咱们从最反直觉的一处讲起。您调 dialog.setPrintRange，这个值并没有存进对话框，函数体把它原样转给了打印机。

```cpp
void QAbstractPrintDialog::setPrintRange(PrintRange range)
{
    Q_D(QAbstractPrintDialog);
    d->printer->setPrintRange(QPrinter::PrintRange(range));
}
```

（qabstractprintdialog.cpp 第 149-153 行）读取也一样，printRange 读的是打印机里的记录（第 158-162 行）。fromPage 与 toPage 咱们也查了，同样是转发（第 217-231 行）。所以压根不存在“对话框的初始值被用户覆盖”一说，对话框是打印机状态的一张皮，exec 之后您从打印机读到的就是用户改过的终值，这正是搭档设计的用意。

构造函数还有一个不传打印机的重载，只给 parent。您要是这么造，对话框会自己 new 一只 QPrinter 并把 ownsPrinter 置真，析构时替您删掉（第 244-256、84-89 行）。内部造的打印机可以工作，但您拿不到它的配置入口，正经打印流程还是该自己造 QPrinter 传进去。

还有一个联动咱们实测过。构造时要是打印机已经带着页码范围，PrintPageRange 选项会被自动打开。

```cpp
if (newPrinter) {
    printer = newPrinter;
    ownsPrinter = false;
    if (printer->fromPage() || printer->toPage())
        options |= QAbstractPrintDialog::PrintPageRange;
}
```

（qabstractprintdialog.cpp 第 246-250 行）示例在造对话框之前调了 setFromTo(1, 9999)，所以后面那句 setOption(PrintPageRange, true) 其实是重复劳动，选项早就开了。您把顺序倒过来调，不预设范围而想开页码选项，就得自己设。

范围的另一头咱们顺带对清。setFromTo 有个断言，from 大于 to 直接崩（第 205-206 行），而 QPrinter::setFromTo 的实现是清空页范围后仅在 from 与 to 都非零时才加入新范围，两个 0 等于整篇全打（qprinter.cpp 第 1415-1420 行）。setMinMax 则会顺手把 PrintPageRange 选项打开，出处同样落在基类实现（第 168-176 行）。示例里那句 setMinMax(1, 9999)，因此是一举两得。

## 六个选项与两把开关：单翻一位还是整组替换

PrintDialogOption 的全集咱们直接从头文件数给您，PrintToFile 是 0x0001，PrintSelection 是 0x0002，PrintPageRange 是 0x0004，PrintShowPageSize 是 0x0008，PrintCollateCopies 是 0x0010，PrintCurrentPage 是 0x0040，全都写在 qabstractprintdialog.h 第 32-39 行。成员一共六名，0x0020 缺着位，这是历史遗留的空洞，您别去猜那里本来是什么。旧稿写的 DontUseSheet 在 Qt 6 的头文件里不存在，笔者全库检索过，没有这个枚举名。PrintRange 那边倒是四名成员，AllPages、Selection、PageRange、CurrentPage（第 25-30 行），旧稿只数了三个，漏掉的 CurrentPage，恰好是 Windows 原生对话框里支持得最完整的一个。

两把开关的差别，咱们当成重点讲。setOption 一次翻一位，实现是一个异或。

```cpp
void QPrintDialog::setOption(PrintDialogOption option, bool on)
{
    auto *d = static_cast<QAbstractPrintDialogPrivate *>(d_ptr.data());
    if (!(d->options & option) != !on)
        setOptions(d->options ^ option);
}
```

（qabstractprintdialog.cpp 第 97-102 行）setOptions 则是整组替换，您传什么它就变成什么（第 129-138 行）。您要是把 setOptions(PrintSelection) 当成追加用，默认开着的另外几项会被一次性抹掉，打印到文件的入口当场消失。想留旧项就逐个 setOption，或者把想要的组合凑齐了一次性传给 setOptions。

选项的默认值，笔者发现文档与源码说的是两套话。选项属性的文档写着默认全部关闭（第 117-127 行），咱们到私有类的构造函数里一看，它却给 options 塞了四项。

```cpp
QAbstractPrintDialogPrivate()
    : printer(nullptr), pd(nullptr)
    , options(QAbstractPrintDialog::PrintToFile | QAbstractPrintDialog::PrintPageRange |
            QAbstractPrintDialog::PrintCollateCopies | QAbstractPrintDialog::PrintShowPageSize),
      minPage(0), maxPage(INT_MAX), ownsPrinter(false)
```

（qabstractprintdialog_p.h 第 37-43 行）三个平台的 QPrintDialogPrivate 都没有动过初值，所以您一只选项都不设，Unix 对话框照样亮着页码范围、逐份打印、打印到文件。咱们以源码为准，文档那句落后了。Qt 4 时代的 addEnabledOption 与 setEnabledOptions 在 Qt 6 全库无此函数，笔者只在一处文档示例片段里还见过它，您在网上资料里碰到了别往代码里抄。

## 确认键走过的路：从按钮到写回打印机

Unix 实现里，确认键并不是直接接 accept。按钮盒的 accepted 信号，接的是内部槽 _q_checkFields，咱们在 Unix 实现的第 634-638 行看到接线。它查两件事，一件是打印到文件的文件名是否可写、是否覆盖已有文件，另一件是 CUPS 下每张纸页数与奇偶页集不能同时设（第 1195-1244 行），全过才调 accept（第 856-862 行）。您在文件名框里填一个只读路径，对话框会弹警告并且不关，做检查的就是这段。

accept 之后还要再查一遍，查 CUPS 页面语法的合法性与双面打印的 PPD 冲突（第 988-1013 行），查完才执行写回，也就是 setupPrinter（第 737-827 行）。咱们把范围分支原文摘给您。

```cpp
// print range
if (options.printAll->isChecked()) {
    p->setPrintRange(QPrinter::AllPages);
    p->setPageRanges(QPageRanges());
} else if (options.printSelection->isChecked()) {
    p->setPrintRange(QPrinter::Selection);
    p->setPageRanges(QPageRanges());
} else if (options.printCurrentPage->isChecked()) {
    p->setPrintRange(QPrinter::CurrentPage);
    p->setPageRanges(QPageRanges());
} else if (options.printRange->isChecked()) {
    if (q->testOption(QPrintDialog::PrintPageRange)) {
        p->setPrintRange(QPrinter::PageRange);
        p->setFromTo(options.from->value(), qMax(options.from->value(), options.to->value()));
    } else {
        // This case happens when CUPS server-side page range is enabled
        // Setting the range to the printer occurs below
        p->setPrintRange(QPrinter::AllPages);
        p->setPageRanges(QPageRanges());
    }
}
```

（qprintdialog_unix.cpp 第 763-783 行）四选一的单选钮决定 printRange 写哪个值，其余配置也是同样节奏逐项落笔。咱们在源码里数到的有双面模式（第 746-753 行），有页序（第 761 行），还有份数与逐份打印（第 824-826 行）。对话框上半区的选择由 QUnixPrintWidgetPrivate 的同名 setupPrinter 写回，打印机名、输出格式、输出文件名都在那里定（第 1291-1314 行）。全部写完，QDialog::accept 才被调用（第 1011-1012 行）。您之后从 QPrinter 读到的每样东西，都是 setupPrinter 一项项写进去的。

Selection 这个值咱们要单独提醒。写回的只是 printRange 等于 Selection 这个事实，打印机并不知道用户选中了哪些内容，选区由谁提供，全靠应用自己实现。您若没提供选区打印的逻辑，用户选了 Selection，输出多半是空页。不支持选区的应用干脆别开 PrintSelection 选项，按钮藏掉比事后解释便宜。

## 打印到文件：Unix 列表尾上的那一项

PrintToFile 选项在 Unix 上的样子，咱们追了一遍。选项开着时，打印机下拉列表的末尾会被追加一个分隔线和一个“Print to File (PDF)”条目（第 1077-1085 行），选中它，输出格式当场切到 PdfFormat，文件名输入框同时点亮（第 1134-1147 行）。xcb 平台下文件名还有个贴心默认值，取打印机的 docName 拼上 .pdf 后缀，docName 空就用 print.pdf（第 1318-1349 行）。

打印到文件的路有平台边界，您得当心。Windows 实现里有个检查函数 warnIfNotNative，输出格式不是原生格式，它就打一句警告，内容是 QPrintDialog: Cannot be used on non-native printers，然后什么都不做（qprintdialog_win.cpp 第 141-145 行）。构造函数里接着设 WA_DontShowOnScreen（第 154-158 行），整个对话框形同虚设。macOS 的 setVisible 对非原生格式同样直接返回，出处是 qprintdialog_mac.mm 的第 305 行。所以想打印到 PDF，在 Unix 之外得靠 QPrinter 自己的 PdfFormat 输出，别指望对话框。

Windows 与 macOS 走原生对话框，写回逻辑咱们也对了一遍。Windows 用系统的 PrintDlgEx，选项被翻译成 `PD_` 前缀的系统标志，没开 PrintSelection 就设 PD_NOSELECTION，没开 PrintToFile 就设 PD_DISABLEPRINTTOFILE，没开 PrintCurrentPage 就设 PD_NOCURRENTPAGE（第 64-93 行），用户点完打印，另一段函数把结果读回来，用的还是对话框自己的 setPrintRange 与 setFromTo（第 115-139 行），绕一圈终究写进打印机。范围填反了它还会弹“The 'From' value cannot be greater than the 'To' value”并让您重填（第 218-242 行），这层校验是 Unix 那边没有的。macOS 用 NSPrintPanel，咱们数下来 Qt 只翻译了两项，PrintPageRange 对应显示页码范围，PrintShowPageSize 对应纸张尺寸加方向加页面设置附件，份数永远显示，其余选项没人翻译（qprintdialog_mac.mm 第 212-218 行），类文档也写明，macOS 原生对话框不支持 setOptions，咱们在基类实现的第 294-297 行核到这句话。macOS 确认后的范围回读还有个自作主张的映射，from 是 1 且 to 是 INT_MAX，就改写成 AllPages（第 79-93 行）。

## open 与 accepted 信号：不阻塞也能拿到打印机

exec 之外的另一条路咱们收尾讲。QPrintDialog 自己加了一个带打印机参数的 accepted 信号，done 里用单发连接把 QDialog 的 accepted 桥过去，信号参数就是配置好的打印机本身，出处是基类实现的第 365-383 行。配套还有一个 open 重载，您把接收者和槽传进去，对话框关闭时自动断连（第 394-401 行）。类文档还留了一句历史注脚，macOS 的 sheet 老用法已经撤掉，想要 sheet 的窗口模态观感，改用 open 就能拿到（第 299-301 行）。

非阻塞的生命周期归您管，这是用 open 换灵活性的代价。对话框没有 parent 又没人 deleteLater，它就一直留在内存里。示例走的是最普通的 exec 流程，您想改造成 open，接 accepted(QPrinter*) 信号是最省事的接法，槽里拿到的指针，正是您传进构造函数的打印机。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，行为咱们在 Qt 6.9.1 源码核对过。

[Qt 文档 · QPrintDialog](https://doc.qt.io/qt-6/qprintdialog.html) -- 打印对话框类，选项与 accepted 信号的说明处

[Qt 文档 · QAbstractPrintDialog](https://doc.qt.io/qt-6/qabstractprintdialog.html) -- 对话框基类，PrintRange 与 PrintDialogOption 枚举的定义处

[Qt 文档 · QPrinter](https://doc.qt.io/qt-6/qprinter.html) -- 打印机类，fromPage 与 toPage 的语义出处

---

打印对话框走完，下一站 [3.74 QPrintPreviewDialog 进阶](./74-qprintpreviewdialog-advanced.md)，那边要接的话咱们递一句，预览对话框的打印按钮内部就是 new 一只 QPrintDialog 然后 exec，本篇理清的写回路径在那边原样再用。在 Windows 与 macOS 上输出到 PDF 时，它还会绕开对话框，弹个保存框选好路径后直接存成 PDF。

想动手的您把示例跑起来（examples/advanced/03-qtwidgets/73-qprintdialog-advanced/，可执行文件 73-qprintdialog-advanced）。编辑器里躺着四段文本，每段开头一行“--- Page N Section ---”。您点 Print 按钮，Linux 上对话框起步是收拢的，选项区藏在右下角的 Options >> 按钮后面，点开才露出来，因为 qprintdialog_unix.cpp 第 623 行的初始化代码把下半区隐藏了。打印机列表末尾若有一项 Print to File (PDF)，选中它再点 Print，会在您填的路径下生成 PDF，您在范围里填 2 到 3，输出只保留落在所选页上的内容，页脚的 Page X / Y 分母仍是全文总页数，不随所选范围变，因为示例的页脚拿总页数当分母。连着打印两次，第二次对话框里默认选中的仍是全部页面，因为示例每次都重设 AllPages（widget.cpp 第 74 行）。

完成标准照旧。setPrintRange 的值最终存在谁身上，您说得出。setOption 与 setOptions 的差别，以及 setOptions 误当追加用的后果，您答得准。Unix 确认键到写回打印机之间隔着哪两道检查，您数得清。Windows 上拿 QPrintDialog 输出 PDF 会发生什么，您讲得明。
