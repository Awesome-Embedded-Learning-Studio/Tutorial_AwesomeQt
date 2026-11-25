---
title: "3.66 QFileDialog 进阶"
description: "进阶篇从静态四兄弟读进 QFileDialog 内部：静态函数全是 QFileDialogArgs 流水线造出的整只对话框（qfiledialog.cpp 第 2128-2153、2470-2497、2580-2592 行），原生与 Qt 内置在构造时就分岔（第 2910-2922 行），canBeNativeDialog 三条否决加类名比对（第 694-708 行），setOptions 里 DontUseNativeDialog 当场建控件（第 768-781 行），mimeTypeFilters 是 nameFilter 的翻译层（第 1533-1576 行），保存不补扩展名要靠 defaultSuffix（第 1183-1271、3829-3855 行），accept 把关文件存在与覆盖确认（第 2679-2700、2768-2797 行），setSidebarUrls 在原生路上被静默丢掉（第 401-415、551-572 行），QtProject/FileDialog 设置组记着上回逛到的目录（第 2803-2861、2910-2941 行），示例在 examples/advanced/03-qtwidgets/66-qfiledialog-advanced/"
---

# 现代Qt开发教程（进阶篇）3.66——QFileDialog 进阶

对话框一家从 62 走到 65，静态族每位成员的便捷函数几只就数完了，QFileDialog 一出手就是四个兄弟，getOpenFileName、getSaveFileName、getExistingDirectory、getOpenFileNames，分量最重。笔者把 qfiledialog.cpp 从头读到尾，四千二百多行，读下来的印象是两只对话框住在同一个类名下，平台原生的一只，Qt 自己画的一只。走哪只、什么时候定、哪些设置在哪只身上会失效，答案都在构造与 setVisible 的几行代码里。

引用文件咱们交代清楚。主战场是 qfiledialog.cpp 与 qfiledialog.h（src/widgets/dialogs），界面骨架另有一份 qfiledialog.ui，讲预览面板时会用到。defaultSuffix 这类属性，本体在 gui 一侧的 qplatformdialoghelper.cpp。QFileDialogOptions 就定义在那里，讲保存场景时会下探一次。60 篇 QDialog 的 exec 模态语义本篇挂链不重讲，62 篇消息盒静态族的写法也不重讲。配套示例是一只四按钮演示窗口，完整玩法放结语。

## 静态四兄弟：每一个背后都是一只完整的对话框

咱们从静态函数看起。getOpenFileName（qfiledialog.cpp 第 2085-2099 行）自己不干活，转手调 getOpenFileUrl（第 2128-2153 行），后者把参数装进一只 QFileDialogArgs，mode 填 ExistingFile（第 2140 行），接着 `QAutoPointer<QFileDialog> dialog(new QFileDialog(args))`，构造、exec、取 selectedUrls 的第一项，出了作用域指针自动销毁（第 2143-2153 行）。getSaveFileName 同一条流水线（第 2427-2441 行），只是 getSaveFileUrl 把 mode 换成 AnyFile，再补一句 setAcceptMode(AcceptSave)，两处出处是第 2482 行与第 2487 行。getExistingDirectoryUrl 填 Directory（第 2580-2592 行），getOpenFileNames 则走 getOpenFileUrls 收一串回来，出处是第 2195-2209 行与第 2240 行。

所以静态函数与实例化的分野不在能力，静态函数内部造的就是一只完整实例，您传的 options 也从 args 构造函数原样进了 setOptions，出处是第 361-379 行。分野在生命周期：exec 一把就销毁，显示前调 setSidebarUrls 或 setProxyModel 的机会，静态函数不给您。还有一处默认值您留意，getExistingDirectory 的 options 实参默认是 ShowDirsOnly，出处见 qfiledialog.h 第 189-192 行，其余三兄弟默认空选项，示例里选择目录那个按钮显式传了 ShowDirsOnly 加 DontResolveSymlinks，等的就是这层差别。

## 原生与内置：构造时就分岔，DontUseNativeDialog 当场换路

QFileDialog 构造时，咱们跟着私有类的 init（qfiledialog.cpp 第 2910-2956 行）看它开头做什么：向平台要一名助手，`nativeDialogInUse = platformFileDialogHelper() != nullptr`，要到了就走原生，内置控件一个不建，要不到才 createWidgets（第 2920-2922 行）。真到显示时 setVisible 再核一遍，canBeNativeDialog 放行就试原生，平台那边没铺好就 createWidgets 兜底，Qt 内置的一套当场建起来（第 858-887 行）。

canBeNativeDialog（第 694-708 行）的否决清单咱们数一遍：全局属性 AA_DontUseNativeDialogs，控件属性 WA_DontShowOnScreen，还有选项 DontUseNativeDialog，任一条命中就回 false（第 699-703 行）。末尾还有一道类名比对，实例的类名与 QFileDialog 不完全一致，就回 false（第 707 行）。您要是继承了 QFileDialog，原生路从此对您关闭，strcmp 的比对结果替您做了主。

DontUseNativeDialog 平时怎么设？setOptions 里专门有一段：选项里带 DontUseNativeDialog，就把 nativeDialogInUse 置 false，紧接着 createWidgets（第 778-780 行）。所以示例的调用顺序是对的，setOption(DontUseNativeDialog, true) 一调，内置控件当场建好，后面的 setSidebarUrls 才有落点。setOption 的文档也叮嘱在案，选项要在改其他属性或显示之前设，晚了某些属性就不生效了（第 715-729 行）。Option 枚举一共七项（qfiledialog.h 第 47-56 行），本篇讲到了 DontUseNativeDialog、ShowDirsOnly、DontResolveSymlinks 与 DontConfirmOverwrite，剩下三项是 ReadOnly、HideNameFilterDetails、DontUseCustomDirectoryIcons，您查文档能见到同款处理。

## 双轨过滤：mimeTypeFilters 是 name filter 的翻译层

nameFilter 一侧咱们从写入端看。setNameFilter 收一整串用 `;;` 隔开的过滤描述，切成列表再交给 setNameFilters（qfiledialog.cpp 第 1371-1373 行）。setNameFilters（第 1417-1435 行）把列表做一次 simplified 清洗，然后写进 options。有一处您记一下，name filter 是进了 options 的，原生对话框也拿得到，后面讲侧边栏时会形成对照。内置控件这一路才有动作：喂给 fileTypeCombo，HideNameFilterDetails 开着时只显示括号前面的名字（qt_strip_filters 第 1380-1397 行），最后 useNameFilter(0) 选中第一项（第 1439 行）。文档另有一句提醒值得带上，`*.*` 不跨平台，无扩展名的文件如 Makefile 在有的对话框里匹配不上，想表达任意文件就写 `*`（第 1403-1408 行）。

mimeTypeFilters 一侧，setMimeTypeFilters（第 1565-1576 行）进门就把每个 MIME 类型交给 nameFilterForMime（第 1533-1545 行）：QMimeDatabase 查出 QMimeType，globPatterns 用空格连起来，comment 做显示名，拼成一条普普通通的 name filter，再调 setNameFilters。MIME 一侧没有自己的过滤引擎，它是站在 name filter 机制前面的翻译层，好处是扩展名清单不用您手抄，系统 MIME 数据库认得多少它就写出多少。文档里有个特例，application/octet-stream 翻出来就是 `All files (*)`，出处见第 1547-1562 行。副作用也写明了，调完 setMimeTypeFilters，nameFilters() 的返回值跟着变（第 1560-1562 行），两个接口读的是同一份名单。

## defaultSuffix：过滤器不会替您补扩展名

保存场景有个流传很广的误会：选了下拉里的 `Text files (*.txt)`，输入 notes，确认后拿到的路径自动带 .txt。笔者把内置实现的代码路径走了一遍，不补。您手输的名字走 typedFiles（第 1183-1217 行），它结尾把名单交给 addDefaultSuffixToFiles（第 1246-1271 行），而那个函数只认 defaultSuffix 属性，文件名里不含点才补一个点加后缀（第 1253-1255 行）。属性默认是空的，过滤器选了什么它一概不问。证据在 gui 一侧的 qplatformdialoghelper.cpp 第 410 行，QString defaultSuffix 默认构造为空。想补，自己 setDefaultSuffix 传 `txt`（第 1853-1857 行），文档说实参带前导点会被剥掉。

有一个动作倒是真的：保存模式下切换过滤器，useNameFilter 会把文件名里已有的扩展名替换成新过滤器第一个扩展名（第 3841-3851 行），notes.txt 切到 `*.json` 就变 notes.json。但前提是名字里本来有点，没有点的名字它不碰。咱们用一段伪代码把保存姿势立起来。

```cpp
// 伪代码：保存时自己保扩展名，selectedFilter 指针顺手收下用户选的过滤器
QString selectedFilter;
const QString path = QFileDialog::getSaveFileName(
    this, tr("保存"), QString(),
    tr("文本 (*.txt);;JSON (*.json)"), &selectedFilter);
// path 不含点时按 selectedFilter 补齐后缀，比赌对话框的默认行为可靠
```

## accept 的把关：文件不存在不关门，覆盖要问过您

内置实现的确认按钮按下后，accept（qfiledialog.cpp 第 2708-2797 行）按 FileMode 分三截把关。ExistingFile 与 ExistingFiles 一截：挨个查文件存在，不存在就弹 itemNotFound 的提示并且直接 return，对话框不关（第 2780-2790、2692-2700 行），您输错文件名想蒙混过去，窗是关不上的。选中的是个目录，它也不报错，当成导航钻进去（第 2791-2794 行）。AnyFile 一截管保存，目标已存在、又没开 DontConfirmOverwrite，就弹 itemAlreadyExists 问您要不要替换，QMessageBox 摆出 Yes 与 No，默认落在 No 上（第 2768-2776、2679-2690 行），替换要的是一次明确的 Yes。开了 DontConfirmOverwrite，问话整个跳过（第 2768 行）。Directory 一截最短，路径存在且真是目录就收下，不存在照旧 itemNotFound（第 2737-2751 行）。

这三截都只管内置实现，原生对话框的确认行为归平台管，qfiledialog.cpp 的代码够不着。咱们把边界记下，跨平台测试时它就在差异清单的头几行里。

## 侧边栏与预览：setSidebarUrls 在原生路上会被静默丢掉

对照来了。setNameFilters 写 options，原生拿得到，setSidebarUrls（第 401-406 行）只干一件事：内置控件在的时候把 URL 列表写进 qFileDialogUi->sidebar，nativeDialogInUse 为真时整个函数体直接跳过，一行警告都没有。sidebarUrls 同一套待遇，原生在用时返回一张空表（第 411-415 行）。原生助手那边也不是全无通道，helperPrepareShow 只有在 usingWidgets 的情况下才把侧边栏塞进 options（第 551-562 行），helperDone 在接受后把 options 里的写回控件（第 564-572 行），一来一回都建立在控件存在这个前提上。您把上头几段连起来读就明白，侧边栏是内置实现的私产，想在原生对话框上设置它，原生路径根本不读这些设置。

嵌预览面板的套路就顺理成章了：必须关掉原生路。界面骨架在 qfiledialog.ui 里写得明白，根布局是一枚 QGridLayout（第 19 行），中部一道 QSplitter（第 135 行），左边挂 QSidebar（第 148 行），右边 QStackedWidget 里装 listView 与 treeView 两个视图页，行号在第 180 行到第 228 行。咱们的做法是 setOption(DontUseNativeDialog, true) 之后用 findChild 摸到根布局，把预览面板作为新的一列加进去，currentChanged 信号驱动刷新，信号声明在 qfiledialog.h 第 147 行，文档在第 279-284 行。这个信号原生路上也有转发，私有类 initHelper 把平台助手的 currentChanged 接到 nativeCurrentChanged（第 542-543 行），但您自己造的 widget 塞不进系统管理的窗口，信号再勤快也没有落脚的地方。

该说清的另一面是，findChild 找内部布局是踩在私有实现上的做法，qfiledialog.ui 的结构 Qt 没承诺稳定，版本升级挪了位置，您那行 addWidget 就找不着北。稳妥的写法是找不到布局就退回一只普通对话框，预览功能静默缺席，别让主流程跟着受牵连。咱们给一段骨架。

```cpp
// 伪代码：给内置文件对话框嵌一块预览面板
dialog->setOption(QFileDialog::DontUseNativeDialog, true);   // 内置控件当场建好
auto* grid = dialog->findChild<QGridLayout*>();              // 摸 qfiledialog.ui 的根布局
if (grid) {
    auto* preview = new ImagePreview(dialog);
    grid->addWidget(preview, 0, grid->columnCount(),
                    grid->rowCount(), 1);                    // 挂到最右侧新列
    connect(dialog, &QFileDialog::currentChanged, this,
            [preview](const QString& path) { preview->show(path); });
}
```

## 上回逛到的目录：QSettings 里那份 QtProject/FileDialog

有种说法是静态函数每次都从默认目录起步，想把上次的目录留下来得实例化自己存。笔者翻到的实情相反。私有类的 init 在构造时就读 QSettings（UserScope，组织名 QtProject，组名 FileDialog），把 lastVisited、viewMode、shortcuts、history 一并恢复（第 2910-2941 行，restoreFromSettings 第 2826-2861 行）。析构函数再调 saveSettings 写回，出处是第 373-378 行与第 2803-2824 行。而 lastVisitedDir 是一只进程级静态 QUrl（Q_GLOBAL_STATIC 在第 57 行，赋值点第 593、3960 行），同进程里几只对话框共享，跨进程重启靠 QSettings 续上。所以您什么都不传，弹出来的多半就是上回逛到的目录，程序重启了也一样。

想要每次都从固定目录起步，办法是把 dir 实参传足，静态函数与构造函数都收这个参数（qfiledialog.h 第 159-165 行），或者实例化之后 setDirectory（qfiledialog.cpp 第 925 行）。再顺一句，saveSettings 记的键里还有 sidebarWidth 与 treeViewHeader（第 2803-2813 行），用户拖过的侧边栏宽度和列头布局它也存。存的位置不在您应用的 QSettings 里，Qt 把它们私底下记在 QtProject 名下。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，行为咱们在 6.9.1 源码核对过。

[Qt 文档 · QFileDialog](https://doc.qt.io/qt-6/qfiledialog.html) -- 文件对话框类，静态函数族与选项、属性的定义处

[Qt 文档 · QMimeType](https://doc.qt.io/qt-6/qmimetype.html) -- MIME 类型查询，setMimeTypeFilters 的数据来源

[Qt 文档 · QSettings](https://doc.qt.io/qt-6/qsettings.html) -- 对话框记忆的存放机制，组织名 QtProject

[Qt 文档 · QUrl](https://doc.qt.io/qt-6/qurl.html) -- 侧边栏与 Url 系静态函数共用的地址类型

---

62 起步的这套静态便捷函数到本篇收官，下一站 [3.67 QProgressDialog 进阶](./67-qprogressdialog-advanced.md)。它跟本篇走的是两条路，文件对话框等用户表态，进度对话框跟着长任务跑表，任务预计够慢它才肯露面，那边等咱们。

想动手的您把示例跑起来（examples/advanced/03-qtwidgets/66-qfiledialog-advanced/，可执行文件 66-qfiledialog-advanced）。四枚按钮挨个点。Open File (with filters) 弹出的框，过滤器下拉摆着 Images、Text files、All files 三项，出处见 file_dialog_demo.cpp 第 60-63 行，选中文件后路径回显在 Result 标签。Save File 里输入一个不带点的名字比如 notes，若弹出来的是 Qt 内置实现，Result 里的路径就是裸的，defaultSuffix 没设，过滤器也没补，defaultSuffix 一节的说法当场应验。Select Directory 传了 ShowDirsOnly 与 DontResolveSymlinks（第 96 行），框里只列目录。Open File (Qt Dialog) 强制内置实现（第 110 行），侧边栏多出 /tmp 与主目录两条快捷（第 114-117 行），跟头一枚按钮弹出的框并排一比，两条路的差别就摆在您眼前，若头一枚弹的是系统原生长相，这一枚就是朴素得多的 Qt 画框。

完成标准照旧。静态四兄弟各自把 FileMode 设成什么，谁还多设了 AcceptSave，您说得出。构造时原生与内置怎么分岔，哪三个条件能否决原生，子类为什么回不去原生，您讲得明。保存时扩展名谁补谁不补，您分得清。侧边栏设置在原生路上落在哪里，您答得上。
