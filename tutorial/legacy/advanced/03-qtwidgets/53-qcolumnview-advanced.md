---
title: "3.53 QColumnView 进阶"
description: "进阶篇把 QColumnView 的分栏机制读明白，每列是一只独立的 QListView，出生路口在 hasChildren（qcolumnview.cpp 第 666-675 行），预览是 previewColumn 与 previewWidget 两层部件（第 828-847 行、qcolumnview_p.h 第 40-111 行），更新钩子 updatePreviewWidget 信号只对叶子触发（qcolumnview.cpp 第 940-941 行），setColumnWidths 只改现有列、多余值存进 columnSizes 留给未来的列（第 857-871 行），新列出生时查 columnSizes、查不到用 sizeHint（第 700-710 行），用户调宽唯一入口是列角 grip 且出厂开（第 1042 行），拖动写回 columnSizes 并平移右侧列（第 508-535 行），双击回 sizeHint（qcolumnviewgrip.cpp 第 98-106 行），doLayout 只排不改宽（第 1092-1118 行），导航引擎在 currentChanged（第 901-944 行），跨枝由 closeColumns 收列重建且预览列例外（第 544-620 行），共享选中模型全视图一份、由 changeCurrentColumn 在列间搬家（第 950-1006 行），QTreeView 双击展开同样以 hasChildren 为闸门（qtreeview.cpp 第 2036、4041-4060 行），配套示例在 examples/advanced/03-qtwidgets/53-qcolumnview-advanced/"
---

# 现代Qt开发教程（进阶篇）3.53——QColumnView 进阶

[3.52 QHeaderView 进阶](./52-qheaderview-advanced.md) 结尾说，条目视图家族只剩 QColumnView 一位，浏览器式的分栏形态在本篇等着。macOS Finder 的列视图就是它，点开一个有孩子的节点，右边多出一列，一路向右，层级有多深列就有多少只。入门篇咱们搭过最简版，setModel、setPreviewWidget，加上 clicked 信号，就停在那一层。进阶篇把分栏机制的里子读明白：列是怎么出生的，预览部件住在哪里，宽度由谁说了算。47 到 51 篇讲过的选中行为、委托绘制、模型接口，本篇只挂链不重讲。

引用文件咱们交代清楚。本体读 qcolumnview.cpp 与 qcolumnview.h，私有结构与预览列在 qcolumnview_p.h，列角手柄另有 qcolumnviewgrip.cpp，树视图那一侧的对读落到 qtreeview.cpp。配套示例是一只文件分类浏览器，完整玩法放结语，正文引用文件名加行号。

## 每列是一只独立的 QListView，出生路口在 hasChildren

QColumnView 自己几乎不画条目，每个层级发一只 QListView，全部塞进自己的视口里水平排开。出生的路口在私有的 createColumn（qcolumnview.cpp 第 661-720 行），笔者把分岔抄给您：

```cpp
// 列与预览的分岔（qcolumnview.cpp 第 666-675 行，摘录）
if (model->hasChildren(index)) {
    view = q->createColumn(index);
    clickedConnection = QObjectPrivate::connect(view, &QAbstractItemView::clicked,
                                                this, &QColumnViewPrivate::clicked);
} else {
    if (!previewColumn)
        setPreviewWidget(new QWidget(q));
    view = previewColumn;
    view->setMinimumWidth(qMax(view->minimumWidth(), previewWidget->minimumWidth()));
}
```

模型说有孩子，就走受保护的虚函数 createColumn，里面 new 一只 QListView，setRootIndex 挂上该层，顺手把 canFetchMore 与 fetchMore 各调一次，出处第 740-751 行。留的口子是给懒加载模型的，QFileSystemModel 这类按需取数的模型跟 QColumnView 天生一对，列出生当场触发一次抓取。模型说没孩子，就不发新列，复用预览列，您要是从没设过预览部件，视图自作主张 new 一个空 QWidget 顶上，出处第 671-672 行。

新列出生后，initializeColumn 给它配齐家当（第 761-797 行）：去边框、横向滚动条常关、竖向常开、最小宽度 100（第 765-768 行），再把本视图的图标尺寸、选中模式、拖放设置、文本省略模式挨个复制过去，行级委托也不例外（第 790-791 行）。省略模式出厂是 ElideMiddle（第 68 行），列里的长文件名中间打点，跟 QListView 默认的右侧省略不一样。想让某一层换掉 QListView，您重写 createColumn 返回自己的视图，再调 initializeColumn 沾一遍设置，这是官方留的正门（qcolumnview.h 第 64-65 行，protected）。

出生的同时还有一条信号转发线，activated、clicked、doubleClicked、entered、pressed 五个信号从每只列视图原样转给 QColumnView（第 677-684 行），所以您在外层接 QColumnView 的 clicked，任意列里的点击都能收到。

## 预览是两层部件，官方的更新钩子叫 updatePreviewWidget

入门篇把预览说成一个部件，源码里其实是两层。QColumnViewPreviewColumn 是一只空壳视图，住在 qcolumnview_p.h 第 40-111 行，visualRect、indexAt、scrollTo 这些交互接口全部给空实现（第 77-108 行），它存在的意义是当一个容器。您交给 setPreviewWidget 的部件，会被 reparent 进它的视口（qcolumnview.cpp 第 846 行），尺寸变化和滚动条，全由它的 resizeEvent 打理，出处 qcolumnview_p.h 第 51-65 行。预览列的最小宽度也跟着您的部件走，QColumnViewPreviewColumn 的 setPreviewWidget，直接取部件的 minimumWidth，出处 qcolumnview_p.h 第 45-48 行。私有 createColumn 再拿 qMax 抬一次，出处 qcolumnview.cpp 第 674 行，示例面板里的 setMinimumWidth(200)，出处 column_model.cpp 第 34 行，就是打这里生效的。换预览部件时，旧的连列一起 deleteLater（qcolumnview.cpp 第 831-835 行），所以预览永远只有一份，想按节点类型换界面，得在部件内部用 QStackedWidget 自己切。

内容更新走信号，咱们回到 currentChanged 里看判定：模型说当前项没孩子，才 emit updatePreviewWidget(current)（第 940-941 行），信号声明在 qcolumnview.h 第 22 行。中间节点永远等不来它，因为预览列只在叶子被选中时顶到最右，管门的还是 createColumn 的分岔。想让中间节点也出预览，只能在模型的 hasChildren 上说谎，代价是子列也不展开了，官方没留别的口子。

配套示例接的却是 selectionModel 的 currentChanged，出处 column_model.cpp 第 100-101 行，跑起来您就能看到差别：点 Documents，子列展开的同时预览面板也换上了分类描述，因为 currentChanged 对任何节点都触发。改接 updatePreviewWidget 的话，面板只在点到 readme.txt 这类叶子时更新，时机跟列视图的原生行为对齐。示例把面板直接交给 setPreviewWidget，没再包 QScrollArea——预览列自带滚动管理，多包一层就是双层滚动，列宽原样从 setColumnWidths 起步（第 91 行）。

## 列宽是绝对值，宽度的存取全走 columnSizes

QColumnView 没有 QHeaderView，列宽的事它自己操办，登记册叫 columnSizes（qcolumnview_p.h 第 139 行，注释写明用于初始化与角部拖动）。setColumnWidths 的语义，咱们对着实现读，出处 qcolumnview.cpp 第 857-871 行。前 min(清单长度, 现有列数) 列当场 resize，清单里多出的值不丢，追加进 columnSizes，等更深的列出生时领用，新列出生时查登记册，查不到才用自己的 sizeHint（第 700-710 行）。清单比现有列短，多出来的列完全不动。谁跟您讲“没给宽度的列用默认值”，那是讲错了。想问当前各列多宽，columnWidths() 返回的就是登记册里已入册的值（第 878-887 行），新列出生时 sizeHint 起算的宽度也记了，读回来是全的。

用户调宽的唯一入口是列角的 grip。resizeGripsVisible 出厂 true（第 1042 行），私有 createColumn 给每只列的角部装一只 QColumnViewGrip，出处第 691-698 行。手柄的活在 qcolumnviewgrip.cpp：拖动时 moveGrip 直接 resize 父列并 emit gripMoved（第 58-80 行），QColumnViewPrivate::gripMoved 把新宽度写回 columnSizes，再把右侧所有列整体平移（qcolumnview.cpp 第 508-535 行）。双击手柄，列宽跳回 sizeHint（qcolumnviewgrip.cpp 第 98-106 行）。您想禁掉手势调宽，setResizeGripsVisible(false) 一关，角部手柄全部卸下（第 111-132 行），此后宽度的写入口就只剩 setColumnWidths。

窗口伸缩时列宽纹丝不动，doLayout 只按现有宽度把列排开（第 1092-1118 行），所以窗口拉宽右侧留白，压窄出横向滚动条。想要列宽跟着窗口走，您得自己接 resizeEvent 重排，Qt 没替您做。

## 导航就是列的生灭，引擎在 currentChanged

咱们点一个节点，右边发生什么，全看 currentChanged（qcolumnview.cpp 第 901-944 行）。在同一层里上下移动走快速通道，直接放行（第 911-922 行）。选中一个有孩子的节点，视图在它右侧造一列新的，而且造出来是藏着的，等滚动到位再亮出来（createColumn(current, false)，第 924-936 行）。跳到另一条枝，closeColumns 上场（第 544-620 行）：沿父链找还活着的列，右侧不合路径的列逐个 deleteLater，唯独预览列例外，藏起来留着复用（第 596-604 行），再按路径把缺的列补齐。setRootIndex 与 setModel 同样进门就 closeColumns 清场后重建，出处第 148、161 行。

键盘导航是对称的，moveCursor 里左键返回父节点，右键下探长子，没孩子就跳到下一个兄弟（第 344-355 行），咱们用方向键就能在层级里游。

模型晚到一步也有补救。rowsInserted 会转发进私有的 checkColumnCreation（第 892-896 行），咱们选中的一项后来才添上孩子、又还占着预览列的位置，视图就把它收掉，换成一只真列（第 1069-1086 行），动态填数的模型不至于卡在预览上。

您再留意每只列的条目右侧那个小箭头，它指示有孩子，是 QColumnViewDelegate 在 paint 里补画的，画的时候走 PE_IndicatorColumnViewArrow，出处第 1153-1158 行。

列多到出滚动条时，滚过去还带动画。scrollTo 会把目标列与它右邻一列的可视宽度算出来，再动横向滚动条的 value，动画时长问样式要 SH_Widget_Animation_Duration，出处第 237-322 行。整只视图的 sizeHint 也不神秘，就是把各列的 sizeHint 逐个相加（第 494-502 行），咱们给它多少初始空间，它心里有数。

## 选中模型全视图只有一份，在列与列之间搬家

有说法讲 QColumnView 每列有自己的选中项，咱们把话说准。列视图确实给每只列都配了 QItemSelectionModel 实例，但算数的只有一份，就是 selectionModel() 返回的共享模型，全视图独一份，谁当当前列谁持有它。搬家发生在 changeCurrentColumn（第 950-1006 行）：倒数第二列升为父列，接走共享模型与焦点（第 964-968 行），此前持有它的列领一只替补实例，选区原样拷走（第 971-980 行），父列还会把当前项的父节点设为选中，让来路保持高亮（第 993-998 行）。所以您在 selectionModel 上接 currentChanged，导航到哪一层，信号都从同一份模型里来，示例正是这么接的。

selectAll 也有它的版本，咱们全选时圈的是当前选中项所在的那一整层，选中的恰好是个有孩子的节点，它自己也会一并圈进来（第 1011-1035 行），拿到的不会是整棵树。

## hasChildren 与 rowCount 脱节，列视图与树视图一起遭殃

createColumn 的分岔只问 hasChildren，不看 rowCount。模型要是宣称有孩子、实际 rowCount 是 0，点开的分岔照样发新列，咱们会看到一只空列杵在右边。反过来宣称没孩子，分岔走向预览列，用户永远到不了那一层。自定义模型重写 hasChildren 时，让它跟 rowCount 同步，两个方向的谎都会直接砸掉导航。

有人讲树视图在这方面更宽容，双击能展开没声明 hasChildren 的节点，笔者去 qtreeview.cpp 核过，不成立。双击展开前的闸门是 hasVisibleChildren（第 2036 行），它的实现同样以 model->hasChildren 为准，出处第 4041-4060 行。两个视图站在同一条依赖上，谁也不比谁多一层容错。QAbstractItemModel 的默认实现就是看 rowCount 大于零，平时不会脱节，重写了才有出事的面。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，行为咱们在 6.9.1 源码核对过。QColumnView 在 QtWidgets，应用链接 Qt6::Widgets 即可。

[Qt 文档 · QColumnView](https://doc.qt.io/qt-6/qcolumnview.html) -- 分栏视图，预览部件与列宽接口的定义处

[Qt 文档 · QAbstractItemView](https://doc.qt.io/qt-6/qabstractitemview.html) -- selectionModel 与 currentChanged 的基类语义

[Qt 文档 · QListView](https://doc.qt.io/qt-6/qlistview.html) -- 每一列的实际视图类型，47 篇细读过

[Qt 文档 · QFileSystemModel](https://doc.qt.io/qt-6/qfilesystemmodel.html) -- 懒加载模型，与 createColumn 的 fetchMore 天然配合

[Qt 文档 · QStandardItemModel](https://doc.qt.io/qt-6/qstandarditemmodel.html) -- 示例用的层级数据来源

---

条目视图家族，咱们到这里收队。下一站 [3.54 QUndoView 进阶](./54-qundoview-advanced.md)，视图换成 QUndoStack 的镜子，撤销栈里每一步操作在列表里各占一行。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/53-qcolumnview-advanced/，可执行文件 53-qcolumnview-advanced）。做三个实验。点 Documents 再点 readme.txt，预览列顶到最右、面板换内容，currentChanged 与 updatePreviewWidget 的时机差别，您对照着体会。拖任意一列右下角的手柄调宽，再双击，宽度写回与回跳 sizeHint 各验一遍。往深处再点一层，看 setColumnWidths 预存的宽度什么时候被新列领走。

完成标准照旧。列与预览的分岔在哪一行判决，您指得出。预览的两层部件谁管滚动，您说得出。列宽的写入与领用走哪本登记册，您数得清。hasChildren 说谎砸掉哪两处导航，您讲得明。
