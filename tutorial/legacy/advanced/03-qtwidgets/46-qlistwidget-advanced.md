---
title: "3.46 QListWidget 进阶"
description: "入门篇用 QListWidget 列过字符串配过图标，进阶篇把底翻开对着 6.9.1 读穿：构造里 setup 挂的是私有 QListModel（内部只是一个行指针容器而 setModel 直接断言拒绝）、currentItemChanged、currentTextChanged、currentRowChanged 三个信号由选择模型的 currentChanged 经 emitCurrentItemChanged 依次转发（槽里改模型会影响后续信号）、setItemWidget 只是 setIndexWidget 把 widget 过继给 viewport 并按条目 visualRect 摆几何而行高仍由条目的 SizeHintRole 决定（不设 setSizeHint 就裁切）、行删除时 rowsAboutToBeRemoved 释放静态 editor 回收 widget、InternalMove 拖放走 QListView::dropEvent 的 moveRow 路径发 rowsMoved（落点压条目则退回插入加删除的后备路径）、MIME 默认 application/x-qabstractitemmodeldatalist 而外部交互要重写 mimeData 与 dropMimeData、supportedDropActions 默认 CopyAction 或 MoveAction、sortItems 用 std::stable_sort 按 DisplayRole 比较经 layoutChanged 发布、大列表开关 uniformItemSizes 复用缓存尺寸与 layoutMode 配 batchSize 分批布局（默认 100）而 movement 默认 Static 且 Free 与 Snap 的自由摆放只在 IconMode 生效，配套示例在 examples/advanced/03-qtwidgets/46-qlistwidget-advanced/"
---

# 现代Qt开发教程（进阶篇）3.46——QListWidget 进阶

[3.45 QFrame 进阶](./45-qframe-advanced.md) 结尾把话递了过来：列表部件怎么把条目管起来，本篇接着看。入门篇您用 QListWidget 列过字符串、配过图标，currentRow 一改界面跟着动。进阶篇咱们把它的底翻开：条目存在哪，currentRow 那串信号谁发的，setItemWidget 之后行高为什么老出事，拖放重排时模型里发生了什么。源稿把笔墨压在演示代码上，机制层几乎没碰，本篇按源码重排，改准的地方随文给行号。

本篇引用以 qlistwidget.cpp 与 qlistwidget.h 为主，咱们把私有模型补到 qlistwidget_p.h，拖放与布局补 qlistview.cpp 与 qabstractitemview.cpp，MIME 格式名补 qabstractitemmodel.cpp。配套示例的完整玩法放在结语，正文引用文件名加行号。

## 构造时就定下的事：QListView 挂一个私有 QListModel

咱们从构造读起。QListWidget 的构造只有一行正事，基类 QListView 加一个 setup()，出处 qlistwidget.cpp 第 1354-1359 行。setup 头一件事是 new 一个 QListModel 挂上去，出处第 1069-1073 行。QListModel 不在公开头文件里，声明藏在 qlistwidget_p.h 第 44 行，内部只有一个 QList<QListWidgetItem*> 容器装行指针（第 102 行）。QListWidget 带的是自家的轻量模型，没借 QStandardItemModel 的光，setModel 里那句断言也把换模型的路堵上了，出处 qlistwidget.cpp 第 1884-1886 行。所有权也顺带交代，QListWidget 析构时连条目一起收，文档写在第 1362 行。

条目的数据也存在条目自己身上。私有结构 values 是一个 QList<QWidgetItemData>，每项是角色加值的一对（qlistwidget_p.h 第 139 行）。咱们跟着 setData 看它怎么存，它顺着容器找角色，找到改值，没找到追加，EditRole 顺手折算成 DisplayRole，出处 qlistwidget.cpp 第 656-679 行，改完通知模型发 dataChanged。模型的 data 干脆把活儿整个转回给条目（第 178-183 行）。三个构造函数给条目的默认 flags 一致，Selectable、UserCheckable、Enabled、DragEnabled（第 550-557、577-581、612-615 行），里头没有 Editable，双击默认改不动文字。拿列表指针构造的条目，还会顺手把自己追加到模型末尾，出处第 556-557 行。

## currentRow 那组信号，是选择模型替您转发的

setup 还接了一批视图信号，pressed 与 clicked 经它转手，发出时就成了 itemPressed、itemClicked，doubleClicked 和 activated 同理，出处 qlistwidget.cpp 第 1074-1087 行。currentRow 这边走得更绕。QListWidget::setSelectionModel 里接了两条线（第 1375-1392 行）：选择模型的 currentChanged 转给私有类的 emitCurrentItemChanged，selectionChanged 走短线，直接发 itemSelectionChanged。emitCurrentItemChanged 依次发三个信号，currentItemChanged 打头，currentTextChanged 居中，currentRowChanged 收尾，出处第 1138-1155 行。入门篇您连的 currentRowChanged(int)，就是打这儿发出来的。

顺着这路信号还有两个顺手的东西。currentRow 是带 NOTIFY 的属性，您 setCurrentRow(2) 换行，读 currentRow() 问行，都走它，出处 qlistwidget.h 第 167 行。想让某行滚进视野，scrollToItem(item) 默认给 EnsureVisible（第 229 行），不用自己去算滚动条。

这里有个连锁要留神。您在 currentItemChanged 的槽里再改一次模型，后两个信号读到的是改过之后的状态，源码注释专门拿持久索引防了这一手（第 1146-1152 行）。selectedItems 走的也是选择模型，selectedIndexes() 拿到行号，再回模型取条目（第 1709-1721 行）。一次能回来几个由 selectionMode 决定，QListView 构造里默认给了 SingleSelection，出处 qlistview.cpp 第 144-152 行，您换成 ExtendedSelection，selectedItems 才可能一次回一串。ListMode 还有个默认藏着的：橡皮筋选框不显示，私有构造里 showElasticBand 是 false（第 1759-1772 行），您想看到那根框，就调 setSelectionRectVisible(true)，出处第 1692-1710 行。

## setItemWidget：行高跟条目的 sizeHint 走，widget 只是铺上去

setItemWidget 本体三行，转手给基类的 setIndexWidget，出处 qlistwidget.cpp 第 1698-1703 行。setIndexWidget 值得咱们停下来读一遍，出处 qabstractitemview.cpp，第 3325-3353 行。

```cpp
// setIndexWidget 的核心动作（qabstractitemview.cpp 第 3338-3351 行，摘录）
if (widget) {
    widget->setParent(viewport());
    d->persistent.insert(widget);
    d->addEditor(index, widget, true);
    widget->installEventFilter(this);
    widget->show();
    dataChanged(index, index); // update the geometry
    if (!d->delayedPendingLayout) {
        widget->setGeometry(visualRect(index));
        d->doDelayedItemsLayout();  // relayout due to updated geometry
    }
}
```

widget 被过继给 viewport，注册成该索引的静态 editor，再按条目当前的 visualRect 摆好几何。您注意到没有，它全程没碰条目的 sizeHint。行高从哪来？QListView 布局时问委托要 sizeHint，委托回模型查 SizeHintRole，QListModel 又转给条目（qlistwidget.cpp 第 178-183 行）。所以您挂了一张 60 像素高的卡片，条目自己的 hint 还是默认值，卡片就被压进一行矮格子里，下半截裁掉。解法是挂之前给条目 setSizeHint(card->sizeHint())，示例正是这个次序（widget.cpp 第 107-113 行）。撤除有现成的 removeItemWidget，本质是拿空指针再设一遍（qlistwidget.h 第 264 行）。

widget 的身份也容易误会，别当它是 item 的子对象，setParent 给的是 viewport。行删掉时它确实跟着没，依据是视图的 rowsAboutToBeRemoved 槽，它会遍历 editor 哈希，把删除区间里的静态 editor 顺手释放，出处 qabstractitemview.cpp 第 3547-3570 行。量级的事顺便说清：每挂一个 widget 就多一棵真控件树，滚动经过的每一项都要显示隐藏，条目数上了几百，咱们该换 QStyledItemDelegate 重写 paint，委托只画不建控件。文档对 setItemWidget 的告诫同向，只放静态内容，动态内容用 QListView 配委托（qlistwidget.cpp 第 1688-1693 行）。

交互也跟着换主。挂上去的控件树自己消化鼠标事件，视图看不见那些按压，您点卡片上的按钮，列表既不选中那一行，也不发 itemClicked，选区要靠条目自己露出来的区域去点。事件过滤器装在 widget 一层（qabstractitemview.cpp 第 3342 行），拦的是焦点切换这类编辑器事务，不管选区。示例的 X 按钮正吃了这个特性，点按钮直接删行，那一行不会因此被选中（widget.cpp 第 36、43-55 行）。

## InternalMove 重排：moveRow 发 rowsMoved，还有一条后备路径

setDragDropMode(InternalMove) 一行把拖出和接收同时打开，实现是 setDragEnabled(true)，紧接 setAcceptDrops(true)，出处 qabstractitemview.cpp 第 1528-1534 行。源稿说成默认如此，笔者对着源码核过，改准为这一次调用的效果。

松手后的事在 QListView::dropEvent，出处 qlistview.cpp 第 883-948 行。事件源是自己且是移动时，它把选中项按行号排好，逐个调 model()->moveRow，QListModel::moveRows 里走 beginMoveRows，挪容器，收尾发布，出处 qlistwidget.cpp 第 261-292 行，模型发出的就是 rowsMoved。您在外部另维护一份 QVector，连模型的 rowsMoved 信号最省事，槽里按 count() 重读一遍就是新顺序。

咱们把 moveRow 的调度摘出来看（qlistview.cpp 第 914-927 行）。

```cpp
// QListView::dropEvent 的搬移调度（qlistview.cpp 第 914-927 行，摘录）
for (const QPersistentModelIndex &pIndex : std::as_const(persIndexes)) {
    // only generate a move when not same row or behind itself
    if (r != pIndex.row() && r != pIndex.row() + 1) {
        // try to move (preserves selection)
        const bool moved = model()->moveRow(QModelIndex(), pIndex.row(), QModelIndex(), r);
        if (!moved)
            continue; // maybe it'll work for other rows
        dataMoved = true; // success
    } else {
        // move onto itself is blocked, don't delete anything
        dataMoved = true;
    }
    r = pIndex.row() + 1;   // Dropped items are inserted contiguously and in the right order.
}
```

摘录里有两层保护值得认。落到自己身上或紧挨着自己后面的位置不发搬移，源码注释把它标成 blocked，也不删任何东西。搬移用的 moveRow 保选区，注释那句 preserves selection，就是您不用替选区操心的理由。

落点压在选中条目身上时走不到 moveRow，退回 QAbstractItemView::dropEvent 的老路（第 940-946 行），模型按 MIME 数据插入新行，源行由启动拖动那头的 clearOrRemove 收掉，出处 qabstractitemview.cpp 第 3896 行，这时您收到的是 rowsInserted 加 rowsRemoved。外部副本要么两种信号都接，要么干脆不存副本、用时现读，笔者更倾向后者。

另有一路您也容易认错：movement 的 Free 与 Snap 管的是 IconMode 的自由摆放，图标视图里拖着走只挪视觉位置、发 indexesMoved，行序不动（qlistview.cpp 第 2936-2960 行）。ListMode 下 movement 默认 Static，布局仍是纵向流，切 Free 或 Snap 看不出差异。setMovement 还有个副作用，值给到非 Static，dragEnabled 会跟着打开，viewport 的 acceptDrops 也是，出处第 193-204 行。

## MIME 三件套：默认私有格式，进出都能改

mimeTypes 默认返回 Qt 的私有格式，全名以 application/ 打头，后跟 x-qabstractitemmodeldatalist，出处 qlistwidget.cpp 第 1770-1772 行，格式名登记在 qabstractitemmodel.cpp，第 2120 行。这个格式 Qt 家里互相认，文件管理器不认。想让条目拖出去变成文件，子类化 QListWidget 重写受保护的虚函数 mimeData，把选中项的路径装进 QUrl 列表（声明在 qlistwidget.h 第 249 行，默认实现在 qlistwidget.cpp 第 1785-1801 行）。反过来接收外部文件，您重写 dropMimeData 处理 hasUrls，声明在 qlistwidget.h 第 251 行，默认实现出处第 1812-1823 行。

支持的动作默认就带移动。supportedDropActions 的默认也在这儿定，基类给 CopyAction，QListWidget 再或上 MoveAction，出处 qlistwidget.cpp 第 1837-1841 行。基类的默认在 qabstractitemmodel.cpp，第 2252-2255 行。您只想给复制，就重写 supportedDropActions，让它只返回 Qt::CopyAction。QListWidget::dropEvent 只是转手 QListView::dropEvent（第 1827-1831 行），想拦拖放别在这一层做。

## sortItems 与大列表的布局开关

sortItems 默认升序（qlistwidget.h 第 202 行的默认参数），实现存下您给的顺序再调模型的 sort（qlistwidget.cpp 第 1588-1593 行）。QListModel::sort 把指针和原始行号配成对，交给 std::stable_sort（第 295-320 行），相等的条目保持原序，排序经 layoutAboutToBeChanged 与 layoutChanged 一对信号发布，没有删行插行。比较器是 operator<，拿 DisplayRole 走 variantLessThan（第 698-702 行），也就是按显示文本比。sortingEnabled 默认 false，您把它开成 true 之后，插入即自动排序。

布局开关都在 QListView 身上。uniformItemSizes 默认 false，条目等高时开成 true，布局直接复用第一项的缓存尺寸，跳过逐项算 sizeHint（qlistview.cpp 第 2484、2750 行）。layoutMode 默认 SinglePass 一次布完，您改成 Batched 后按 batchSize 分批布局，批间回事件循环，可见条目早早能交互，batchSize 默认给 100，出处第 303-312、352-372 行。批量 addItem 前后用 setUpdatesEnabled 包一层是通用手段，省的是逐次重绘。这几个开关救的是布局与绘制，救不了内存，几万条往上就得 QListView 配自定义模型了。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，行为在 6.9.1 源码核对。QListWidget 属 QtWidgets，链接 Qt6::Widgets 即可，拖放另涉 QtGui 的 QMimeData 与 QDrag。

[Qt 文档 · QListWidget](https://doc.qt.io/qt-6/qlistwidget.html) -- setItemWidget 与 currentRowChanged 的定义处

[Qt 文档 · QListWidgetItem](https://doc.qt.io/qt-6/qlistwidgetitem.html) -- 条目的 flags 与 sizeHint 所属

[Qt 文档 · QListView](https://doc.qt.io/qt-6/qlistview.html) -- uniformItemSizes、batchSize 与 selectionRectVisible

[Qt 文档 · QAbstractItemView](https://doc.qt.io/qt-6/qabstractitemview.html) -- DragDropMode 枚举与 setIndexWidget

[Qt 文档 · QMimeData](https://doc.qt.io/qt-6/qmimedata.html) -- setUrls 与 hasUrls，外部拖放的数据载体

---

私有模型、信号转发、行高归属，加上 moveRow 与 MIME，还有布局开关，都对上了号。QListWidget 把模型藏起来换省心，代价是模型一层您碰不着，条目一多，内存头一个顶不住。把这层壳翻到底正是下一篇的事，[3.47 QListView 进阶](./47-qlistview-advanced.md)，咱们接着看。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/46-qlistwidget-advanced/，可执行文件 46-qlistwidget-advanced）。窗口一张列表五张卡片，构造里那几行配置值得您回头对照正文，InternalMove 开重排，单选加 40 像素网格，出处 widget.cpp 第 71-77 行。addCard 三步走，建条目、设 sizeHint(QSize(0, 40))、再挂 CardWidget（第 104-114 行），行高的归属就这么演给您看。卡片右上的 “X” 按钮靠 itemWidget 反查自己属于哪一行（第 43-55 行），行一删，widget 会被视图一并回收，依据就是 rowsAboutToBeRemoved 那个槽。底部标签连着 currentRowChanged，行一变就报数（第 89-90、131-145 行）。顶部按钮在 Free 与 Snap 之间切 movement（第 116-129 行），不过示例是 ListMode，切换看不出差异，自由摆放要 IconMode 才登场。

完成标准照旧。QListWidget 的模型是哪个类、声明在哪个头文件，您说得出。currentRowChanged 从哪条内部信号转来，您数得出。挂了 itemWidget 的行高度由谁定，您答得上。内部重排在模型里发哪个信号，后备路径发哪两个，您对得出。
