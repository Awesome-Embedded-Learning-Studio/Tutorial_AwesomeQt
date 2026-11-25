---
title: "3.27 QComboBox 进阶"
description: "入门篇把 addItem 塞选项、currentIndex 拿值、setEditable 开编辑与 setModel 挂数据源的流程过了一遍，进阶篇对着 6.9.1 的源码把下拉弹窗认清楚，那扇列表是 Qt::Popup 顶层窗口，容器和视图都住在私有头里，屏幕边界检测横竖两个方向默认就做了，覆写 showPopup 换的是放置策略，弹窗宽度跟控件自身的 sizeHint 走，委托自 6.9 起能用 setLabelDrawingMode 请上控件脸面，换显示列后 currentData 认的列跟着变，内置补全默认行内完成不出第二扇弹窗，配套示例在 examples/advanced/03-qtwidgets/27-qcombobox-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.27——QComboBox 进阶

[3.26 QKeySequenceEdit 进阶](./26-qkeysequenceedit-advanced.md) 收尾把话递到了选择类，说录入这件事从键盘回到列表。QComboBox 是选择类的第一站，入门篇把 addItem 塞选项、currentIndex 拿值、setEditable 开编辑、setModel 挂数据源的标准流程跑通了，做个内部工具确实够用，咱们接着往产品级的用法走。

需求一走进正式些的界面，自定义弹窗外观、多列数据模型、可编辑加补全，几处入门篇没写的行为就开始碍事。弹窗跟控件一样宽，长选项从中间被省略，给弹窗画好的委托上不了控件的脸面，setModelColumn 换列之后，currentData 取出来的东西换了含义。这些行为咱们逐一到 6.9.1 的源码里核对过，其中几处流传很广的说法在这个版本站不住，正文里对着行号改。

咱们按这样的次序走，从弹出列表的构造认起，跟着是 showPopup 的定位与宽度，委托怎么从弹窗走上控件的脸面，模型换列与内置补全收尾。正文里的行号都带着出处，示例代码在 examples/advanced/03-qtwidgets/27-qcombobox-advanced/，Qt 内部实现指源码树里的 qcombobox.cpp、qcombobox.h、qcombobox_p.h、qcommonstyle.cpp 与 qcompleter.cpp。

## 弹出的列表是顶层窗口，构造全在私有头里

下拉的那扇列表，咱们按两件东西来认，QComboBoxPrivateContainer 当容器，直接继承 QFrame（qcombobox_p.h 第 179 行），里面装一枚 QComboBoxListView，QListView 的子类（第 46 行）。构造时窗口旗标给的是 Qt::Popup（qcombobox.cpp 第 479-481 行），它是浮在应用上的顶层窗口，不在布局里占位置，鼠标被它抓走，点视图之外它自己收起，收起的活归容器的 mousePressEvent，一律调 hidePopup（第 826-839 行）。容器外观还按风格分岔，菜单式风格（SH_ComboBox_Popup）上下装滚动小条，其余样式给 1 像素的边框（第 500-510 行）。

打开这一侧的条件也值得您记。mousePressEvent 把活转给 showPopupFromMouseEvent，命中测试认到 SC_ComboBoxArrow 才开弹窗，出处 qcombobox.cpp 第 3200-3210 行，不可编辑模式整个控件区域都算，可编辑模式点编辑区出的是光标，点箭头才下拉。您要是收到点控件没反应的报告，命中测试与容器可见两处都得过，容器已经在显示就不再开第二次。

两件东西都住在 qcombobox_p.h，子类 include 不到它们，您想在子类里摸到容器，得按基类找，示例用的就是 findChild<QFrame*>()，出处 custom_combo_popup.cpp 第 33 行。这层东西是懒创建的，view() 或 viewContainer() 第一次被调到，才把容器连同视图造出来（qcombobox.cpp 第 1075-1097 行），构造函数里只有接线，init 挂的是一枚单列 QStandardItemModel，出处第 1068 行。

视图有两处默认设置值得单独说，咱们各记一笔。文本省略模式是 ElideMiddle（第 1085 行），选项装不下时省略号落在文本中间，末尾不带点，选项行连图标区一起进选中高亮，开关在视图的 initViewItemOption 里，出处 qcombobox_p.h 第 63-69 行。

弹窗的高度按内容算，咱们看 showPopup 怎么累加，它把每一行的 visualRect 高度加起来，出处 qcombobox.cpp 第 2697-2719 行，常规样式加到 maxVisibleItems 项就停（第 2711-2714 行），这个数默认 10（qcombobox_p.h 第 402 行），一百个选项也不会弹出一屏高，剩下的交给滚动。菜单式风格不算这个上限，弹窗按全部内容撑开，macOS 的原生菜单那一路（qcombobox.cpp 第 2665-2673 行）连这套几何都不经过，覆写 showPopup 够不着它。

选中怎么落地也收在容器里。回车与 Enter 在 eventFilter 里拦下，当前项有效就 hidePopup 并发 itemSelected（第 745-750 行），F4 与 Alt+Down 同路（第 755-759 行），鼠标在视图里松开的一下，也由 eventFilter 接手（第 788-799 行）。itemSelected 拿索引去 setCurrentIndex，emitActivated 跟着把 activated 与 textActivated 发出去（第 1416-1436 行），您在自己代码里接的 currentIndexChanged 与 activated，出口都在这里。

收起这边也做了防重入，hidePopup 开头挂一枚 hidingPopup 标志，重入直接返回（qcombobox.cpp 第 2903-2910 行），容器没在显示就早退（第 2910-2911 行），菜单式风格带 SH_Menu_FlashTriggeredItem 时，被触发的项闪一遍高亮才收（第 2914-2947 行）。您在子类里连着调几次 hidePopup 也只收一遍，标志到函数出口才复位。

## 屏幕边界检测默认两个方向都做，覆写 showPopup 换的是放置策略

流传很广的说法是，默认实现只查竖直方向，水平截断没人管，咱们对着 6.9.1 把这句改掉。showPopup 的定位段上来就拿弹窗左上角落在的那块屏幕可用区域（第 2678 行，popupGeometry 走 availableScreenGeometry，出处第 283-289 行），水平这边的处理在第 2757-2768 行做完，弹窗比屏幕宽就压到屏宽，右边缘出屏整体左移贴住屏幕右边，左边出屏同理贴回左边缘。

竖直那边咱们分三步看（第 2798-2808 行），下方放得下放下方，放不下而上方放得下就挪到控件上方，两边都放不下挑空余多的一边把高度裁掉，列表视图自己出滚动条。靠近屏幕边缘的组合框拿默认行为已经不容易被截断，无脑覆写 showPopup 再手写一遍 clamp，属于重复劳动。

覆写真正的用武之地是换放置策略，弹窗居中、贴某个参照物、整扇换成自绘窗口都算。文档把纪律写在 showPopup 的注释里，您换成自己的弹窗，收尾得调一次 hidePopup()，内部状态靠它复位（第 2649-2650 行）。示例的 CustomComboPopup 演示的是干预点本身，起手调一次 QComboBox::showPopup() 让默认实现把弹窗造好摆好，随后 findChild 找到容器，改弹窗的 geometry（custom_combo_popup.cpp 第 27-41 行），clampToScreen 把上下左右四条边各自约束进 availableGeometry（第 47-74 行），这一步放在基类调用之后做，容器的几何到那一刻才定下来。

弹窗宽度跟控件走。常规样式从 SC_ComboBoxListBoxPopup 拿到的初始矩形，就是控件自身的 rect（qcommonstyle.cpp 第 4272-4273 行），控件多宽弹窗就多宽。想让长选项完整露出来，路子是让控件本身变宽，三档 SizeAdjustPolicy 的差别咱们摆成表：

| SizeAdjustPolicy | 宽度怎么定 | 何时重算 |
| ---- | ---- | ---- |
| AdjustToContents | 逐项扫描取最宽项（qcombobox.cpp 第 365-379 行） | 内容每次变化作废缓存重算 |
| AdjustToContentsOnFirstShow | 同样逐项扫描 | 默认值，首次显示前作废一次（第 3133-3142 行） |
| AdjustToMinimumContentsLengthWithIcon | 字符数乘字宽加图标位，不逐项扫（第 380-392 行） | minimumContentsLength 改变时 |

逐项扫描的成本落在内容变化上，弹出的时候不算。sizeHint 算完缓存在私有数据里（recomputeSizeHint 开头的 isValid 判断，第 347-350 行），AdjustToContents 下 dataChanged、rowsInserted、rowsRemoved 各有一处作废（第 1111-1115、1139-1143、1178-1182 行），字体与图标尺寸变化也各有一处（第 3075-3077、1722-1723 行），作废之后还有一枚 20 毫秒的定时器垫在中间（adjustComboBoxSize 第 418-421 行，timerEvent 里 adjustSize，第 437-447 行）。您要是拿几千项的模型逐行灌进去，重算跟着定时器的节奏走，弹出那一下不用替它操心。

弹窗自己加宽的情形只对菜单式风格开，showPopup 里把 computeWidthHint 与控件宽度的差值直接加到弹窗上（第 2744-2748 行），Windows 与 Linux 的常规样式轮不到。您手里要是选项又多又长的界面，AdjustToMinimumContentsLengthWithIcon 配 setMinimumContentsLength 是折中，宽度按字符数乘字宽加图标位定（第 387-392 行），模型多大都不影响这一趟计算。改 setSizeAdjustPolicy 的生效不绕路，实现里当场清掉 sizeHint 缓存，跟着再调 adjustComboBoxSize 与 updateGeometry，出处 qcombobox.cpp 第 1651-1660 行。

## 委托只有一份，控件脸面走不走它是 6.9 的新开关

QComboBox::setItemDelegate 的实现只有一行，转手调 view() 的 setItemDelegate，出处 qcombobox.cpp 第 2039-2046 行，itemDelegate() 返回的，也是弹窗视图的委托（第 2019-2022 行）。您把委托设上去，它就管着弹窗里的每一行，示例的三列绘制委托装上去，名称、编码、地区从同一行的 sibling 索引里取出来，行区域三等分各画各的（combo_item_delegate.cpp 第 36-44 行），行高在 sizeHint 里抬到 32，给三列留出垂直空间（第 67-74 行）。

文档在 setItemDelegate 的警告里写明，同一份委托实例别在多个组合框之间共用（第 2031-2035 行），closeEditor 信号会串门。还有一层容易忽略，Qt 自己也会换委托，样式带 SH_ComboBox_Popup 时默认委托换成菜单式的 QComboMenuDelegate，常规样式换回 QComboBoxDelegate（updateDelegate 第 1791-1803 行），您没设过自定义委托它才动手，设过就不碰。

控件自己的脸面另算。paintEvent 头一步把 CC_ComboBox 整个交给样式画（第 3108-3110 行），跟着分岔，默认 LabelDrawingMode 是 UseStyle，出处 qcombobox_p.h 第 399 行，当前项的文本与图标由 CE_ComboBoxLabel 画（第 3124-3126 行），您的委托在这里不出场。后果是弹窗与脸面长得不一样，弹窗里三列并排，咱们选中一项把弹窗关掉，脸面上还是单列普通样式。Qt 6.9 给这个分岔添了开关：

```cpp
// qcombobox.cpp paintEvent 的分岔，UseDelegate 是 6.9 新增的一支
if (itemDelegate() && labelDrawingMode() == QComboBox::LabelDrawingMode::UseDelegate)
    itemDelegate()->paint(&painter, itemOption, d->currentIndex);  // 脸面交给委托，矩形取 SC_ComboBoxEditField（第 3118-3123 行）
else
    painter.drawControl(QStyle::CE_ComboBoxLabel, opt);            // 样式画文本与图标（第 3124-3126 行）
```

setLabelDrawingMode 一调，参数取 LabelDrawingMode::UseDelegate，出处 qcombobox.cpp 第 3654-3660 行，枚举与属性的文档块在第 3622-3646 行，里外都标着 \since 6.9，脸面与弹窗的绘制就统一到一份委托上。官方注释还提醒，UseDelegate 模式下得配一个合适的委托，样式给的默认委托未必画得动（第 3641-3644 行），6.9 之前的版本没有这枚开关，要统一就得覆写 paintEvent 自己走一遍这个分岔。有一处委托永远管不着，您把组合框设成可编辑之后，脸面的文本装在那枚 QLineEdit 里，显示、补全、校验都是它的地盘。

## currentData 认的是显示列，跨列取数自己拼索引

setModelColumn 一趟把列号喂给三处，私有成员 d->modelColumn（qcombobox.cpp 第 3611 行），弹窗视图的 setModelColumn（第 3612-3614 行），可编辑时内置补全的 completionColumn（第 3615-3618 行），末尾一轮 setCurrentIndex(currentIndex())，把脸面文本刷成新列的值（第 3619 行）。行号不变，显示换列，咱们看取数的接口跟着怎么动。

currentData 的实现是 d->currentIndex.data(role)（第 2276-2280 行），那个索引是持久化的 QModelIndex，每次 setCurrentIndex 都按 modelColumn 重建（第 2186 行），私有侧落索引前还有一次 sibling 归一化（第 2205 行），所以换列之后 currentData 取的就是新列的数据。role 默认是 Qt::UserRole（qcombobox.h 第 136 行），新列在 UserRole 上没存过值，拿回来的自然是无效 QVariant。您手上常见的调试题在这儿有答案，模型第二列明明有文本，setModelColumn(1) 之后 currentData(Qt::UserRole) 变无效，链路没坏，新列在那个角色上本来就没存东西。

控件自带的 itemText 与 itemData 也认显示列，两个实现内部都按 modelColumn 拼索引，出处 qcombobox.cpp 第 2285-2311 行，itemData 的 role 由您传，itemText 固定取显示文本。咱们数下来，取数的几个接口全绑在显示列上，多列模型的读取绕不开显式索引。

查找这组接口同样认显示列。findData 从显示列起步做 match（第 1605-1606 行），findText 就是 findData 换了个默认角色，findText 查 DisplayRole，findData 查 Qt::UserRole（qcombobox.h 第 62-66 行的默认参数），您按数据反查条目时，存下的那个值得在显示列上找得到。

稳的写法是把列写显式。取别的列的数据一律 model()->index(currentIndex(), column) 自己拼索引，currentData 留给您确实要显示列数据的场合。示例的信息标签用的就是显式索引，城市、编码、地区三列各自 model->index(row, N)（main.cpp 第 111-113 行），显示列怎么换标签都不乱。setModel 还有一处连带效果，可编辑时新模型同时设给内置补全器（setModel 的注释在第 2066-2067 行），换数据源不用您再补一遍。

## 内置补全默认行内完成，第二扇弹窗要您自己调出来

您把 setEditable(true) 打开，控件里装进一枚 QLineEdit（第 1829-1831 行）。

setLineEdit 给它配默认补全器，咱们看配了什么：模型用组合框自己的 model（第 1900 行），大小写不敏感（第 1901 行），完成模式 QCompleter::InlineCompletion（第 1902 行），完成列跟着 modelColumn（第 1903 行）。文档把行为说全了，可编辑组合框自动带上一个 QCompleter，做大小写不敏感的行内补全（第 1976-1977 行）。

行内补全在行编辑框里把余下的文本补上并选中，不弹任何列表，可编辑组合框默认只有自己那一扇弹窗。流传的“补全弹窗跟下拉弹窗打架”在默认配置下无从谈起，您想让补全也做成列表，completer()->setCompletionMode(QCompleter::PopupCompletion) 一调，第二扇窗才出现，两个弹窗叠加的管理从这一刻起才归您管，要么收回行内，要么接管 completer() 的 popup 自己安排位置。

那扇补全列表由 QCompleter 自动创建，您把 completionMode 切到 PopupCompletion 或 UnfilteredPopupCompletion，它就生成一枚 QListView，出处是 qcompleter.cpp 第 1200-1202 行的注释。

咱们把它跟下拉容器分清楚，两扇窗各归各家，下拉的容器住 qcombobox_p.h，补全的列表归 QCompleter 所有，窗口旗标同样是 Qt::Popup，焦点代理设回行编辑框，委托换成 QCompleterItemDelegate，列跟着 completionColumn 走，出处集中在 qcompleter.cpp 第 1241-1255 行。

补全器的归属另有一条线。它挂在 lineEdit 名下，setLineEdit 换行编辑框时旧的一起被扔掉（第 1871 行，注释在第 1856-1857 行），您给不可编辑的组合框 setCompleter，收到的是一句 qWarning（第 1986-1988 行）。当前项一变，补全前缀跟着同步成新文本（第 2216-2218 行），程序里 setCurrentText 之后补全是对得上的。editingFinished 里还有一道防抢，补全弹窗正开着时焦点离开，它看见弹窗可见就返回，把选择交给 QCompleter 的 activated，出处第 1327-1341 行。

您把可编辑开关往回拨，也看得见清理。setEditable(false) 关掉输入法使能，行编辑框 hide 掉再 deleteLater，指针清成空（qcombobox.cpp 第 1837-1840 行），补全器归行编辑框所有，跟着一起销毁。两个分支的末尾都调 updateDelegate（第 1843 行），脸面用不用委托，随可编辑状态一起重算。

插入策略默认是 InsertAtBottom（qcombobox_p.h 第 395 行），可编辑组合框里敲一段新文本按回车，returnPressed 把它插成新条目，设为当前项（qcombobox.cpp 第 1352-1414 行）。您把它设成 NoInsert，returnPressed 进门就返回（第 1360-1361 行），敲什么都不会变成列表里的新选项。

有两处咱们别跟着想漏，补全列表里选中一项走 completerActivated，照常 setCurrentIndex 并发 activated（第 207-233 行），焦点离开时 editingFinished 拿行编辑框的文本 findText，找得到就切过去（第 1343-1347 行），NoInsert 挡的是插入，按文本定位当前项它不管。查重的口径跟着补全器走，matchFlags 按补全器是否大小写敏感，决定加不加 MatchCaseSensitive，出处第 1309-1316 行。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，QComboBox、QCompleter 与 QStyledItemDelegate 都在 QtWidgets，链接 Qt6::Widgets 即可，labelDrawingMode 一节要求 Qt 6.9 起的版本。

[Qt 文档 · QComboBox](https://doc.qt.io/qt-6/qcombobox.html) -- 下拉选择框，showPopup、setModelColumn、setSizeAdjustPolicy 与 labelDrawingMode 都在这页

[Qt 文档 · QComboBox::SizeAdjustPolicy](https://doc.qt.io/qt-6/qcombobox.html#SizeAdjustPolicy-enum) -- 尺寸自适应三档策略，弹窗宽度问题的主入口

[Qt 文档 · QStyledItemDelegate](https://doc.qt.io/qt-6/qstyleditemdelegate.html) -- 标准项委托，弹窗行与 6.9 的脸面绘制都用它

[Qt 文档 · QCompleter](https://doc.qt.io/qt-6/qcompleter.html) -- 自动补全，completionMode 与 filterMode 的说明

[Qt 文档 · QStandardItemModel](https://doc.qt.io/qt-6/qstandarditemmodel.html) -- 多列数据模型，示例的数据源

---

选择类到本篇开了个头。咱们下一站去 [3.28 QFontComboBox 进阶](./28-qfontcombobox-advanced.md)，QFontComboBox 是 QComboBox 的子类，字体枚举装在现成的模型里，本篇的委托与换列那套到它身上直接落地。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/27-qcombobox-advanced/，可执行文件 27-qcombobox-advanced），页面上一个多列选择器，十五座城市三列信息，弹窗里三列并排，选中后标签拼出整行。

三步实验顺着正文走，咱们边改边看。第一步把 main.cpp 第 87 行的 setModelColumn(0) 改成 setModelColumn(1)，控件与弹窗的显示列一起换成编码，弹窗里三段的内容跟着错位（委托把传入索引当名称列，combo_item_delegate.cpp 第 36-38 行），底部标签纹丝不动，它走的是显式索引。第二步把 custom_combo_popup.cpp 第 20 行的 AdjustToContents 注释掉，窗口拉窄再点开弹窗，弹窗跟着控件变窄，还原这一行，宽度回到最宽选项。城市的名字就两个字，太短，省略号出不来，您想亲眼看 ElideMiddle 从中间截断（qcombobox.cpp 第 1085 行），往列表里塞一个长选项再拉窄窗口。第三步进 CustomComboPopup 的构造函数，即 custom_combo_popup.cpp 第 16-21 行，补一行 setLabelDrawingMode，参数取 LabelDrawingMode::UseDelegate，选中一项，控件脸面也按三段画（qcombobox.cpp 第 3118-3123 行），脸面只有一行的高度，三段字挤而齐，属正常现象。完成标准咱们列好，三列弹窗完整不被屏幕边缘裁掉，换列后标签数据不乱，脸面与弹窗的绘制出自同一份委托。
