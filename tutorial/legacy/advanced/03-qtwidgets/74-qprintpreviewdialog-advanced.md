---
title: "3.74 QPrintPreviewDialog 进阶"
description: "进阶篇把 QPrintPreviewDialog 的机制对到源码行上：对话框内部组装一只 QPrintPreviewWidget 并转发它的 paintRequested（qprintpreviewdialog.cpp 第 199-207 行），预览生成时 QPrinter 临时换上 QPreviewPaintEngine，每一页记录成一只 QPicture 而不是内存 PDF（qprintpreviewwidget.cpp 第 355-377 行，qprinter.cpp 第 137-153 行，qpaintengine_preview.cpp 第 47-62、111-134 行），打印按钮只是再发一次 paintRequested（qprintpreviewwidget.cpp 第 582-587 行），fitToWidth 与 fitInView 是 setZoomMode 的便利函数（第 670-682 行），总览模式列数取页数开方再补成偶数列（第 323-331 行），示例在 examples/advanced/03-qtwidgets/74-qprintpreviewdialog-advanced/"
---

# 现代Qt开发教程（进阶篇）3.74——QPrintPreviewDialog 进阶

上一篇的 QPrintDialog 把挑打印机的活交给了用户，本篇的 QPrintPreviewDialog 再往前送一步，纸还没走到打印机，用户就能在屏幕上翻看每一页。它看着像个黑盒，内部组装得其实很朴素，一只 QPrinter，加一只 QPrintPreviewWidget，外面套一层带工具栏的 QMainWindow。预览机制里，每一页都是拿您的绘制代码现打的，点打印那一刻，同一段代码再跑一遍。

引用文件咱们交代清楚。笔者把打印支持模块里的 qprintpreviewdialog.cpp 与 qprintpreviewwidget.cpp 从头读到尾，预览引擎那头又借 qprinter.cpp 与 qpaintengine_preview.cpp 对证，行号全部对着 Qt 6.9.1 标注。入门篇走过的连接 paintRequested、exec 的基本套路不重讲，本篇从内部组装讲到预览生成的真实路径，再落到缩放实现与工具栏定制，配套示例是一只带水印开关的预览对话框，玩法放在结语。

## 内部组装：一只 QPrinter 加一只 QPrintPreviewWidget

构造时传不传 QPrinter，走的分支咱们在 init 里都能看清。传了，QPrintPreviewWidget 直接拿它建出来，指针原样存下（qprintpreviewdialog.cpp 第 199-201 行）。没传，对话框自己 new 一只 QPrinter，默认落在系统默认打印机上，并记下 ownPrinter 标志（第 203-206 行）。析构时只删除自己造的 QPrinter，您传入的打印机由您自己管（第 681-688 行）。

```cpp
if (_printer) {
    preview = new QPrintPreviewWidget(_printer, q);
    printer = _printer;
} else {
    ownPrinter = true;
    printer = new QPrinter;
    preview = new QPrintPreviewWidget(printer, q);
}
QObject::connect(preview, SIGNAL(paintRequested(QPrinter*)), q, SIGNAL(paintRequested(QPrinter*)));
```

（qprintpreviewdialog.cpp 第 199-207 行）末尾那行 connect，值得咱们停一停。您连到对话框上的 paintRequested，源头在内部的预览控件里，对话框只做转发，转发代码就在第 207 行。本篇后面讲的所有触发时机，起点都在 QPrintPreviewWidget 那一侧。

最外层的组装也顺路看清。对话框内部建了一只 QMainWindow 的派生类，它把 createPopupMenu 重载成返回空指针，免得用户右键点出工具栏菜单（第 47-52 行）。工具栏从适应宽度到打印按钮挨个摆好（第 232-282 行），预览控件当中央部件（第 297 行），主窗口随后被强制改成普通子部件挂进对话框（第 298-300 行）。窗口标题会拼上打印机的 docName（第 307-310 行）。还有一处联动咱们实测会碰到，打印机无效时 Page setup 按钮直接禁用，Windows 与 Apple 平台上输出格式不是原生打印时同样禁用，见第 312-317 行。咱们在没有配置打印机的环境里跑示例，这个钮多半是灰的。

## 预览的每一页都是现打的

预览从哪来，咱们从对话框第一次显示跟起。setVisible 里有个只跑一次的分支，头一回显示时调 preview->updatePreview，分支在第 693-702 行。updatePreview 本体在第 758-768 行，它再调 generatePreview，函数体在第 355-377 行。generatePreview 就是预览的主流程，咱们把核心几行原文摆出来。

```cpp
if (printer->d_func()->previewMode())
    return;
printer->d_func()->setPreviewMode(true);
emit q->paintRequested(printer);
printer->d_func()->setPreviewMode(false);
pictures = printer->d_func()->previewPages();
```

（qprintpreviewwidget.cpp 第 365-370 行）这几行得连着 qprinter.cpp 里的 setPreviewMode 一起读。预览模式打开时，QPrinter 的打印引擎与绘制引擎被临时换成一只 QPreviewPaintEngine，真正的引擎被存到一边（qprinter.cpp 第 137-153 行）。您的绘制槽拿 QPainter 往 QPrinter 上画的时候，驱动的其实是预览引擎，它把每一次 draw 调用记录进一只 QPicture。begin 建头一页（qpaintengine_preview.cpp 第 47-62 行），您每调一次 newPage，它就新起一只 QPicture 记下一页（第 111-134 行）。画完，generatePreview 关掉预览模式，把页表要回来（第 369-370 行）。

记录完的 QPicture 接着被摆进图形场景。populateScene 给每页造一只 PageItem（第 298-315 行），显示时 PageItem::paint 画好纸与阴影，再 drawPicture 把记录的绘制命令原样重放（第 87 行），页边距区还会被盖上一层半透明白，您画进边距的内容看起来就被洗淡了（第 89-95 行）。这里笔者要当面驳一条说法，旧稿与网上流传的讲法都称预览在内存里生成 PDF 再栅格化成图像，笔者在源码里找不到支持的代码，真实路径是 QPicture 记录加重放，从头到尾没有一张 PDF。

哪些动作会让 paintRequested 再来一遍，笔者把源头数了一遍。对话框首次显示算一次。翻转 Portrait 与 Landscape 也算一次，工具栏的方向钮没有经过对话框的私有槽，直接接到预览控件的方向函数上（qprintpreviewdialog.cpp 第 374-375 行），一个接 setPortraitOrientation，另一个接 setLandscapeOrientation。它们落到 setOrientation，改完页面方向就调 generatePreview，函数在 qprintpreviewwidget.cpp 第 572-577 行。Page setup 对话框确认后同样走一遍方向更新，出处是第 570-587 行。点打印按钮时还会来一次，咱们下一节单独讲。所以您的绘制槽在一次预览里可能被叫好几回，绘制逻辑必须每次都从零画到尾。槽里要是往文件里追加内容或者累加全局计数，翻一次方向就多记一份，这类副作用得在入口处清干净。页面几何也得每次从参数给出的 QPrinter 上现读，方向刚被用户改过，缓存的旧矩形只会画错。

## 点打印之后：同一段绘制代码再跑一遍

工具栏的 Print 按钮接到 _q_print，咱们跟着它走一遍 Unix 的路径。它按需建一只 QPrintDialog，exec 等用户确认（qprintpreviewdialog.cpp 第 560-561 行）。用户点了接受，它才调 preview->print()，随后把对话框收下（第 562-568 行）。preview->print() 的函数体只有一行，就是再发一次 paintRequested，出处是 qprintpreviewwidget.cpp 第 582-587 行。源码里还留着一句注释，说本该复用生成好的页，实现没做，就在第 585 行。

所以打印与预览共用的就是同一个槽，区别只在 QPrinter 当前装的是哪只引擎。预览时装 QPreviewPaintEngine，打印时预览模式已关，QPainter 直通真正的打印引擎。类文档把这套用法写得很直白，预览用的代码可以原样拿来当实际打印用，包括 newPage（qprintpreviewdialog.cpp 第 632-640 行）。预览与打印的一致性就靠这一点保证，您要是另写一套打印代码，两边的分页迟早分道扬镳。

平台差异咱们也交代一句。Windows 与 Apple 上输出格式不是原生打印时，Print 按钮走的是导出 PDF 的路，弹文件保存框，后缀补上 .pdf，随后照样调 preview->print()。出处 qprintpreviewdialog.cpp 第 542-559 行。预览终究是记录下来的绘制命令在屏幕上重放，几何一致，设备渲染不完全一致，打印机的分辨率与色彩和屏幕有物理差别，版面照预览走，成色以打印机为准。

## fitToWidth 与 fitInView：缩放的实现路径

工具栏的 Fit width 与 Fit page 两个钮都接进 _q_fit，咱们看槽里的转发。Fit page 走 fitInView()，Fit width 走 fitToWidth()，转发逻辑在 qprintpreviewdialog.cpp 第 470-477 行。这一对函数在预览控件那边都是一行的便利函数。

```cpp
void QPrintPreviewWidget::fitToWidth()
{
    setZoomMode(FitToWidth);
}
```

（qprintpreviewwidget.cpp 第 670-673 行）fitInView 同款，只是换成 setZoomMode(FitInView)，位置在第 679-682 行。咱们接着看 setZoomMode，缩放走哪条路由它定。模式是 FitInView 或 FitToWidth 时，fitting 置真并立刻调 _q_fit(true)。是 CustomZoom 时，fitting 置假（第 689-699 行）。

_q_fit 里的活分两条路，咱们对着源码看。FitToWidth 拿视口宽度除以目标页宽得到比例，套上变换矩阵，再把视野对到页顶（第 220-229 行）。另一条路走 graphicsView 的 fitInView，KeepAspectRatio，整页塞进视口（第 231 行）。算完把 zoomFactor 按变换矩阵的 m11 乘上打印机与控件的逻辑 DPI 之比回写（第 239 行），打印页从打印机分辨率缩到屏幕分辨率，靠的就是这一步。

手动缩放那一路咱们顺带看清。zoomIn 与 zoomOut 的默认系数是 1.1（qprintpreviewwidget.h 第 52-53 行）。实现那头把模式切成 CustomZoom，再乘上变换，出处是 qprintpreviewwidget.cpp 第 400-404 与 593-611 行。工具栏上的加减号钮带自动重复，间隔 200 毫秒（qprintpreviewdialog.cpp 第 284-294 行）。缩放下拉框预置了 12.5% 到 800% 的档位（第 224-226 行，表存的是两倍值、显示时再除回）。手输超界的值，验证器在 1 到 1000 的范围上拦（第 222 行），Intermediate 状态回车没反应，焦点移走时打回原值（第 101-105 行），qprintpreviewdialog.cpp 第 596-607 行的夹取只在别的路径兜底。

## 视图模式与当前页

视图模式枚举有三个成员，咱们在 qprintpreviewwidget.h 第 24-28 行里能数全。单页是 SinglePageView。对页是 FacingPagesView。总览是 AllPagesView。私有构造里默认 SinglePageView，缩放模式默认 FitInView，缩放系数 1（qprintpreviewwidget.cpp 第 147-152 行）。对话框工具栏的初始勾选与它对得上，Fit page 与单页显示是选中态，对应 qprintpreviewdialog.cpp 第 403-409 行。三个显示钮接到 _q_setMode，选总览时切 AllPagesView，把适应、翻页与页码框一并禁用（第 515-523 行），切回单页或对页再启用并重新适应（第 529-535 行）。setViewMode 那头，总览分支还会把缩放模式改成 CustomZoom，fitting 关掉（第 546-551 行）。

总览的网格排布写在 layoutPages 里。纵向纸取列数为页数开方向上取整，横向纸向下取整，随后再补成偶数列，注释写明偶数列更好看（第 323-331 行）。对页模式固定两列，位置表多算一格，首页没有对页，注释也写明白了（第 332-345 行）。当前页怎么定，是滚动时算的，calcCurrentPage 拿视口与每页的重叠面积比大小，面积最大的那页胜出，打平取页码小的（第 257-275 行）。滚动条的 valueChanged 就接在这个槽上，连线在第 285-286 行。咱们拖着滚动条翻看，工具栏的页码框跟着变，动力就在这里。

## 给工具栏塞自己的按钮

内部工具栏并没有藏进私有类，它是内部主窗口的子对象，主窗口又挂在对话框底下（qprintpreviewdialog.cpp 第 232-233 与 296-300 行），所以 findChild<QToolBar*>() 能把它找出来，addAction 就能加您自己的按钮。示例加的是一个可勾选的 Add Watermark 开关。

勾选水印的现象值得您亲手试。示例的开关只翻一个 bool，不碰预览，paintRequested 的触发权在预览控件手里，所以勾上 Add Watermark 的那一刻预览纹丝不动。您点一下工具栏的 Portrait 或 Landscape，方向一翻，generatePreview 重跑，斜向的 DRAFT 水印连同新方向一起出现在每页中央。想勾完立刻见效，咱们可以拿 findChild 找到内部预览控件，模板参数写 QPrintPreviewWidget，然后调它的 updatePreview()，这是公开槽（qprintpreviewwidget.cpp 第 758-768 行），语义就是重发生一次 paintRequested，比借方向翻转绕路要体面。

依赖内部控件结构的做法要留余地。findChild 找不到就返回空指针，工具栏与预览控件的组装方式属于实现细节，大版本之间可能挪窝，判空加降级是您工程里该有的姿势，示例里 if (toolBar) 的判空就是这个意思。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，行为咱们在 Qt 6.9.1 源码核对过。

[Qt 文档 · QPrintPreviewDialog](https://doc.qt.io/qt-6/qprintpreviewdialog.html) -- 打印预览对话框，paintRequested 信号与内部组装的文档出处

[Qt 文档 · QPrintPreviewWidget](https://doc.qt.io/qt-6/qprintpreviewwidget.html) -- 预览控件，ViewMode 与 ZoomMode 枚举以及缩放便利函数的定义处

[Qt 文档 · QPrinter](https://doc.qt.io/qt-6/qprinter.html) -- 打印机类，预览模式换引擎就发生在它身上

[Qt 文档 · QPageSetupDialog](https://doc.qt.io/qt-6/qpagesetupdialog.html) -- 页面设置对话框，预览工具栏 Page setup 按钮弹出的就是它

---

03-qtwidgets 卷到这里就走完了，从布局与事件一路走到打印预览，控件的家底咱们翻过了一遍。往后的去处，教程线进 [05 其他模块卷的 QtSql 一篇](../05-other-modules/01-qtsql-advanced.md)，QtSql 与 QtCharts 在那边开卷。实例库那头则是成品级的练手货，widget 与 app 两栏可逛，拿成品对照本卷学过的控件，正好补上从学会到做出的那一段。

想动手的您把示例跑起来（`examples/advanced/03-qtwidgets/74-qprintpreviewdialog-advanced/`，可执行文件与工程同名）。窗口里是一框示例文本加一个 Print Preview 按钮，点开预览，示例文本在 A4 上排出三四页，页码框能看到总数，没有打印机的环境里 Page setup 是灰的。您勾上 Add Watermark，预览没动静，翻转一次 Portrait，水印才在每页中央现身，这正是 paintRequested 只在预览重生成时发生的活证据。缩放框里手输 5000 回车，框里没反应，把焦点移走，它打回原值，超界的输入由验证器在回车那一步就拦下了。再点总览钮，所有页排成偶数列的网格，适应与翻页钮一起变灰，切回单页再拖动滚动条，页码框自己往上走。

完成标准照旧。paintRequested 的源头在哪一行、被谁转发，您说得出。预览页是 QPicture 还是内存 PDF，您答得准。打印那一刻有没有复用预览生成好的页，您讲得清。fitToWidth 最终落到哪个函数、缩放比例怎么算，您能对着源码指出来。
