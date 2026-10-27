---
title: "3.72 QPrinter 进阶"
description: "进阶篇把 QPrinter 的行为对到源码行上：构造函数请求的是原生输出，找不到有效打印机时静默退回 PdfFormat（qprinter.cpp 第 63、68-74 行），Qt 6 的 OutputFormat 只有 NativeFormat 与 PdfFormat 两值、没有 PostScriptFormat（qprinter.h 第 69 行），PDF 路线一只 QPdfPrintEngine 同时充当打印引擎与绘制引擎（qprinter.cpp 第 87-89 行），画笔原点默认落在可打印区左上角、由 pageMatrix 的平移实现（qpdf.cpp 第 3561-3564 行），setFullPage(true) 才把原点挪到纸角，QMarginsF 的边距顺序是左上右下（qmargins.h 第 274 行），newPage 在引擎不活跃时返回 false、末页之后调用会在文档尾多一张空白页（qprinter.cpp 第 1246-1252、297-301 行），PrinterMode 默认 ScreenResolution、HighResolution 下 PDF 引擎分辨率取 1200（qprintengine_pdf.cpp 第 367-371 行），示例在 examples/advanced/03-qtwidgets/72-qprinter-advanced/"
---

# 现代Qt开发教程（进阶篇）3.72——QPrinter 进阶

71 篇的结语把咱们送到这里，说是打印设备与输出格式的控制在这边等咱们，说的就是 QPrinter。它不住在 QtWidgets 里，归属在 QtPrintSupport 模块，CMake 里要单独引入 PrintSupport 组件并链接 Qt6::PrintSupport，配套示例的构建脚本就是那么写的。模块归属意味着打印这一类功能可以脱离 Widgets 单独用，您在纯 Gui 程序里导 PDF 也不需要 Widgets。

引用文件咱们照例交代清楚。笔者主读 QtPrintSupport 里的 qprinter.cpp，枚举对 qprinter.h，私有默认值对 qprinter_p.h，PDF 引擎的实现在 qprintengine_pdf.cpp，画笔原点一条线还追进了 QtGui 的 qpdf.cpp。入门篇走过的输出格式设置与单页绘制，本篇不重讲，咱们管的是页面配置、分页推进与页眉页脚，示例的完整玩法放结语。

## 构造那一刻就选好了分辨率

PrinterMode 分三档（qprinter.h 第 31 行），构造函数的默认实参，落在 ScreenResolution（第 33 行）。另一档是 HighResolution，第三档 PrinterResolution 已被文档弃用，它在各平台的等效行为不一致（qprinter.cpp 第 337-341 行）。档位落到 PDF 引擎上就是一句分辨率赋值，咱们把这段源码原文贴给您看。

```cpp
resolution = 72;
if (m == QPrinter::HighResolution)
    resolution = 1200;
else if (m == QPrinter::ScreenResolution)
    resolution = qt_defaultDpi();
```

（qprintengine_pdf.cpp 第 367-371 行）72 起步，HighResolution 拉到 1200，ScreenResolution 取屏幕逻辑 dpi，那个函数按主屏的 logicalDotsPerInch 走，桌面环境通常在 96 上下（qfont.cpp 第 105-119 行）。您想中途改，setResolution 要赶在 QPainter::begin() 之前调，文档写明过了这一刻就未必全平台生效（qprinter.cpp 第 1030-1031 行）。示例构造时直接给了 HighResolution（widget.cpp 第 57 行），为的是打印件上文字边缘更干净，代价咱们后面马上看到，所有按像素计的常量都要跟着 dpi 口径走。

字号与分辨率的关系，文档有一条提醒，以 point 计的字号不随设备分辨率变化（qprinter.cpp 第 347-349 行），图文混排想保住相对比例时，反而可以考虑用像素计的字号（第 350-352 行）。咱们在屏幕上惯用的“多少像素”直觉，到了 1200 dpi 的页面上要重新换算，物理单位与设备单位本来就分家，引擎没有替咱们抹平的意思。

## 输出格式：要的是原生，拿不到就退回 PDF

构造函数请求的格式是 NativeFormat（qprinter.cpp 第 474 行），但 initEngines 内部的起点写死了 PdfFormat，只有平台打印插件在场、且能找到默认打印机或第一台可用打印机时，才升到 NativeFormat（第 62-74 行）。也就是说，一台没配打印机的机器上，QPrinter 默认构造完 outputFormat() 报出来的就是 PdfFormat，中间没有任何提示。您在开发机上调试时看到的“默认就是 PDF”，其实是上一句讲的回退在兜底。

```cpp
// Default to PdfFormat
outputFormat = QPrinter::PdfFormat;
```

（qprinter.cpp 第 62-63 行）顺着回退路径往下看，还有两条跟目的地联动的格式规则。setOutputFileName 给的文件名以 .pdf 结尾，不分大小写，输出格式直接切到 PdfFormat，给空名则切回 NativeFormat（第 723-726 行）。setPrinterName 走三分支，空名切 PdfFormat，无效打印机名什么也不改，有效名才切 NativeFormat（第 646-655 行）。咱们把这些合起来看，格式跟着输出目的地走。

这里笔者要驳回旧稿的一处说法，它称存在 QPrinter::PostScriptFormat。Qt 6 的枚举里只定义了两个成员，一个是 NativeFormat，另一个是 PdfFormat（qprinter.h 第 69 行），PostScript 输出没有对应枚举值。旧稿还把 NativeFormat 说成无条件默认值，漏掉了上面的静默退回，两处都按源码改讲。PDF 侧另有版本可选，PdfVersion_1_4、PdfVersion_A1b、PdfVersion_1_6 三档，引擎映射的缺省值是 1.4（qprinter.cpp 第 81-86 行）。

打印进行中想改配置，此路是不通的。一排 setter 开头都挂着 ABORT_IF_ACTIVE 宏，引擎一旦处于 Active 状态，宏就 qWarning 一声，然后直接返回（qprinter.cpp 第 30-40 行），setPrinterName 在名单上，setOutputFileName 也在，setResolution 同样（第 641、720、1039 行）。咱们要在 begin 之前把页面、格式、目的地一次配齐。

## 一只引擎两副面孔：PDF 路线的打印与绘制是一体的

PDF 分支里引擎只造了一只，new QPdfPrintEngine 一次，paintEngine 指向它，printEngine 也指向它（qprinter.cpp 第 87-89 行）。原生分支才是两只引擎分工，createNativePrintEngine 造一只，createPaintEngine 再造一只（第 78-79 行）。您用 paintEngine() 和 printEngine() 取回来的，在 PDF 模式下是同一个对象，这件事笔者刚开始也没料到，翻到 initEngines 才确认。

格式切换走 changeEngines，旧引擎的属性挨个搬进新引擎（第 104-117 行），份数与打印机名还有专门的迁移处理（第 109-114 行）。所以咱们中途切格式，页面大小与边距不会丢。自带引擎的话另说，setEngines 文档写明 QPrinter 不接管自定义引擎的所有权，实例得您自己管（第 482-486 行），默认引擎才由析构函数删除（第 512-513 行）。

咱们再把同族的对象认一认。QPdfWriter 归属在 QtGui，不依赖 PrintSupport（qpdfwriter.h 第 22 行），纯导 PDF 的场景它是更轻的选择，页面配置接口出自同一个基类，都挂在 QPagedPaintDevice 名下。QPrinter 也继承它（qprinter.cpp 第 447 行），setPageSize 从基类来，setPageMargins 与 pageRanges 也从基类来。另有一条硬约束，QCoreApplication 出现之前构造 QPrinter，直接 qFatal（第 466-470 行），您在全局对象里放一只 QPrinter 就是给自己埋雷。

## 画笔原点：默认已经在可打印区的左上角

旧稿正文称，画笔绑上 QPrinter 后原点在纸张左上角，要从 pageRect 的左上角起画才不会压到边距。这话不成立，而且它与旧稿后面自认的另一条说法直接打架。咱们拿两处源码对正。setFullPage 的文档写明，默认关闭时坐标系原点与可打印区左上角重合，开启后才与纸张左上角重合（qprinter.cpp 第 982-988 行）。引擎侧的实现就是页矩阵里的一次平移。

```cpp
QTransform tmp(scale, 0.0, 0.0, -scale, 0.0, m_pageLayout.fullRectPoints().height() / userUnit);
if (m_pageLayout.mode() != QPageLayout::FullPageMode) {
    QRect r = m_pageLayout.paintRectPixels(resolution);
    tmp.translate(r.left(), r.top());
}
```

（qpdf.cpp 第 3561-3564 行）标准模式下 pageMatrix 把 pageRect 的左上偏移叠进变换，咱们从 (0, 0) 落笔，起点就在边距内侧，不必自己加偏移。FullPageMode 不平移，原点回到纸角，按 QPageLayout 的规则（qpagelayout.cpp 第 220-222 行），paintRect 直接等于 fullRect，pageRect 与 paperRect 两个查询就此重合，边距全归您自己管。两个查询的入口在 QPrinter::pageRect 与 paperRect，单位自选，DevicePixel 口径下，pageRect 内部走 paintRectPixels，paperRect 走 fullRectPixels（qprinter.cpp 第 1157-1182 行，属性侧见 qprintengine_pdf.cpp 第 285-290 行）。

边距的填法还有一处方向性细节要纠正。QMarginsF 的构造顺序是左、上、右、下（qmargins.h 第 274 行），旧稿写成上、右、下、左，照它填边距，四条边就转了位。setPageMargins 的第二参，默认值就是毫米（qpagedpaintdevice.h 第 38 行）。示例没有另设边距，只给了 A4 纸张（widget.cpp 第 60 行），页眉页脚全按 pageRect 排布，咱们看输出就知道默认边距留了多少。

## newPage 只管推进纸，画笔状态您自己管

咱们从 newPage 本体看起，QPrinter::newPage 开头就查引擎状态，引擎不处于 Active 就直接返回 false（qprinter.cpp 第 1249-1250 行）。类文档另给两条使用语义，第一页在 QPainter::begin() 时已经备好，前面不必调 newPage，末页画完再调一次，文档尾会多出一张空白页（第 297-301 行）。

引擎侧 QPdfEngine::newPage 做的事是写完当前页、新开页对象、重发三类脏标记，画刷、画笔、裁剪路径（qpdf.cpp 第 1318-1333 行，脏标记在第 1325 行）。画笔的 worldTransform 不在那批脏标记里，它是 QPainter 一侧的状态，换了页也没人动，您第一页 translate 过，第二页那个平移还跟着，笔尖可能已经出了页外。稳妥做法是每页从同一个基准起笔，示例正是这么写的，每页开头画页眉（widget.cpp 第 103 行），正文 y 从 bodyTop 起算（第 107 行），不依赖上一页留下的任何坐标。

分页推进本身是咱们自己的活，引擎不代劳。drawText 不会替您进位，同一个矩形里画五十次就是五十层文字叠在一起，行距要自己拿 QFontMetrics 算。度量必须绑打印设备构造：QFontMetricsF(bodyFont, &printer)（widget.cpp 第 72 行），绑屏幕算出来的行高，放到 1200 dpi 的页面上会差出一个量级，分页立刻全乱。示例的推进逻辑朴素而够用，按行切段，每页行数用可用高度除以行距得出（第 81 行），切段与逐页循环画满就 newPage（第 88-117 行）。

## 页眉页脚与总页数：一遍数页，一遍落笔

总页数是页脚的难点，咱们画到第几页才知道一共几页，“Page X / Y” 的 Y 却要每页当场写。示例的解法是纯算术预数，全文按换行切段，每页行数算出来后总页数整除向上取整（widget.cpp 第 88-93 行），一遍画完。这个口径对等行距的纯文本成立，富文本或表格高度不定时，就得走两遍遍历，第一遍只算不画数出页数，第二遍才落笔，工程里这是常见代价。

页眉三件，标题居左、日期居右、下方一条分隔线（widget.cpp 第 122-147 行），页脚页码居中（第 149-167 行）。您留意示例每画一段就重设一次字体，页眉、正文、页脚三套字号各自 setFont（第 106、127、153 行），这正是把跨页状态依赖压到零的做法，与新页重发脏标记的机制相配，谁也不指望谁。

收尾前咱们把打印范围这组接口归拢一下。PrintRange 枚举有四个值（qprinter.h 第 72 行），私有侧的默认值是 AllPages（qprinter_p.h 第 51 行），另有 Selection、PageRange，还有 CurrentPage。读写页范围的接口在基类（qpagedpaintdevice.cpp 第 188-201 行），旧的 setFromTo 内部写的也是同一份页范围（qprinter.cpp 第 1415-1420 行）。选打印机另有入口，QPrinterInfo 的 availablePrinters()，拿来就是系统清单（qprinterinfo.h 第 58 行）。您要让这些值真正生效，得配合打印对话框让用户确认，那是下一篇的主角。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，构建需引入 Qt6 的 PrintSupport 组件，行为咱们在 Qt 6.9.1 源码核对过。

[Qt 文档 · QPrinter](https://doc.qt.io/qt-6/qprinter.html) -- 打印设备类，输出格式、页面配置与 newPage 的语义出处

[Qt 文档 · QPagedPaintDevice](https://doc.qt.io/qt-6/qpagedpaintdevice.html) -- 分页绘制设备基类，setPageMargins 与 pageRanges 的定义处

[Qt 文档 · QPageLayout](https://doc.qt.io/qt-6/qpagelayout.html) -- 页面布局类，pageRect 与 paperRect 的计算依据

[Qt 文档 · QPdfWriter](https://doc.qt.io/qt-6/qpdfwriter.html) -- QtGui 一侧的 PDF 写出类，不依赖 PrintSupport

[Qt 文档 · QPrinterInfo](https://doc.qt.io/qt-6/qprinterinfo.html) -- 打印机信息类，可用打印机清单的入口

---

widgets 卷的打印地带咱们走到一半，下一篇 [3.73 QPrintDialog 进阶](./73-qprintdialog-advanced.md)，打印范围、份数、选哪台打印机，这些交给用户拍板的活都归它管。本篇留下的 printRange，还有 pageRanges，正好是它要操作的接口。

想动手的您把示例跑起来（examples/advanced/03-qtwidgets/72-qprinter-advanced/，可执行文件 72-qprinter-advanced）。窗口是一块文本编辑器加一只 Print to PDF 按钮，粘上几屏长文，按下按钮选好保存路径，输出的 PDF 每页顶部是标题加日期加分隔线，底部居中是页码，正文按固定行距排布。有个一分钟的实验值得做，把逐页循环里 page > 1 的判断去掉（widget.cpp 第 98-100 行），再跑一次，PDF 开头会多出一张空白页，这正是类文档写明的行为，第一页本来就不需要 newPage（qprinter.cpp 第 297-299 行）。您再对比把构造参数换成 ScreenResolution 的一份输出，同样的文本页数变多或变少，因为 pageRect 的设备像素尺寸随分辨率变了，行数每页跟着变。

完成标准照旧。默认构造拿不到打印机时 outputFormat() 报什么，您答得出。画笔默认原点落在哪、setFullPage(true) 把它挪到哪，您指得准。QMarginsF 四个值按什么顺序填，您写得对。newPage 之后哪些状态被引擎重发、哪些原样留着，您数得清。
