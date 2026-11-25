---
title: "3.37 QCalendarWidget 进阶"
description: "入门篇用 selectedDate、setMinimumDate/setMaximumDate 与 QTextCharFormat 把 QCalendarWidget 跑了起来，进阶篇对着 6.9.1 源码把外壳拆开，内部是 QCalendarModel 加 QCalendarView 加 QCalendarDelegate 三件套，星期表头住在模型第 0 行、ISO 周号住在第 0 列，视图自带的两个 QHeaderView 被藏掉，NoHorizontalHeader 靠真删模型行实现，表头文字每次 data() 现场从 locale 取、没有缓存，firstDayOfWeek 默认跟 locale、中文环境是周一，setLocale 会把手设的首日重置回去，paintCell 是唯一的绘制口子、调用链经 QCalendarDelegate 的 storedOption，视图在 drawCell 里写入选中态、不调父类连选中高亮一起丢，rect 坐标归内部视图的 viewport，formatForCell 合并次序为表头、星期、日期、随后越界背景强制 Window、非本月前景强制 Disabled Text，日期格式压星期格式、星期格式压表头格式，setDateTextFormat 传空格式不清条目、date 为 null 才清全部、tooltip 走 ToolTipRole，范围钳位时 setMinimumDate 连带抬 max、setMaximumDate 压 min，选中被挪发 selectionChanged、分开设两端可能两次信号、setDateRange 一次到位且 min 大于 max 自动交换，越界格子 flags 为空点不动、月菜单按范围禁用，导航栏构件带 qt_calendar_ 前缀 objectName 可供 QSS 定制，翻月按钮走 addMonths 自带跨年、setCurrentPage 只翻页不动选中，键盘方向键一天七天、dateEditEnabled 默认开、浮框是 QLabel 套 QFrame、1500 毫秒接受，配套示例在 examples/advanced/03-qtwidgets/37-qcalendarwidget-advanced/"
---

# 现代Qt开发教程（进阶篇）3.37——QCalendarWidget 进阶

[3.36 QLCDNumber 进阶](./36-qlcdnumber-advanced.md) 把显示家族送进日期方向。QLCDNumber 把每个像素攥在自己手里，格子坐标、七段多边形全亲手画，QCalendarWidget 的路数整个反过来，整月日历交给一套 Model/View 摆布，控件自己退成一个外壳。外壳里住着三件东西，QCalendarModel 管数据，QCalendarView 是一张 QTableView，QCalendarDelegate 管绘制，出处（qcalendarwidget.cpp 第 821、899、1552 行）。咱们顺着这套构造把定制点逐个找出来，源稿在首日默认值和表头缓存上偏得最远，正文按源码改准。

咱们把默认状态照源码记一遍。短星期名表头、周号列显示、周六周日红字、网格线关闭、可选范围从儒略日 1 到 9999 年 12 月 31 日，出处（qcalendarwidget.cpp 第 936-948、2115-2118 行），dateEditEnabled 在构造收尾被设为真，出处第 2166 行。正文引用以 qcalendarwidget.cpp 为主。

## 星期表头住在模型第 0 行，QCalendarView 藏掉了自带的 QHeaderView

咱们从内部表看起。RowCount 定为 6、ColumnCount 定为 7，另留 HeaderRow 与 HeaderColumn 都是 0，出处（qcalendarwidget.cpp 第 33-37 行），rowCount 与 columnCount 把首行首列算进去，对外是 7 行 8 列，出处第 827-839 行。星期表头与周号不走 QHeaderView，QCalendarView 的构造把自带的横向、纵向两个 header 都 setVisible(false)，出处第 1304-1315 行，表头行和周号列以普通单元格的身份住在模型里，走同一条委托绘制管线。

horizontalHeaderFormat 有四档，SingleLetterDayNames、ShortDayNames、LongDayNames、NoHorizontalHeader，声明出处（qcalendarwidget.h 第 38-43 行）。您想连表头带那一行的空间一起省掉，用的是 NoHorizontalHeader，它走真删一行的路子，模型侧 beginRemoveRows 直接动行数，出处（qcalendarwidget.cpp 第 1248-1265 行）。verticalHeaderFormat 只有 ISOWeekNumbers 与 NoVerticalHeader 两档，切的是模型里的周号开关，出处第 2700-2712 行。

表头文字没有缓存。dayName 在每次 data() 里现场从 locale 取，短名档用 dayName 的 ShortFormat，单字母档用 standaloneDayName 的 NarrowFormat 再取首字符，出处第 1059-1076 行。笔者拿系统里的 Qt 实测，中文环境短名档印 “周一” 到 “周日”，单字母档印 “一” 到 “日”。

firstDayOfWeek 默认跟 locale 走，模型构造读的是 QLocale().firstDayOfWeek()，出处第 945 行。中文 locale 的一周从周一开始，源稿说中文和英文环境默认从周日开始，数据表与实测都不支持，美式英语是周日，英式英语又回到周一，首日要看 locale，别按语言猜。setFirstDayOfWeek 改的是模型里的 m_firstDay，整张网格经 dateForCell 重新排布，表头顺序跟着列走，出处第 1004-1020、1267-1274 行。还有一处联动容易被忽略，locale 变更时 event() 会重读 locale().firstDayOfWeek() 覆盖 m_firstDay，您手动设的首日会被一次 setLocale 冲掉，出处第 3102-3103 行。

gridVisible 默认关闭，setGridVisible 只是转发给视图的 setShowGrid，出处第 1310、2768-2775 行。您在格子里画了额外内容，就把它打开，边界感会好很多，示例构造里就是这么做的，出处（custom_calendar.cpp 第 33 行）。

## paintCell 是唯一的绘制口子，选中态随 storedOption 进来

调用链笔者从头跟了一遍。视图绘制管线逐格调委托，QCalendarDelegate::paint 算出格子对应的日期，把 option 存进 storedOption，再把画笔、矩形、日期一起递给私有层的 paintCell，出处（qcalendarwidget.cpp 第 1678-1689 行），私有层的转发在第 1976-1980 行，公有虚函数 paintCell 在第 2281-2284 行接住。声明在 qcalendarwidget.h 第 118 行，它是 const 成员，覆写时签名别漏掉这个 const。

只有日期格进得了 paintCell。表头行与周号列的格子，dateForCell 返回无效日期，委托把它们直接交给 QStyledItemDelegate::paint 的默认分支，出处第 1006-1008、1682-1688 行，所以 date 参数永远有效，您也不用判断表头格。

默认实现绕一圈回来，公有 paintCell 把活儿交回委托的 paintCell，后者把 storedOption.rect 换成传入矩形，走 QStyledItemDelegate::paint 画出日期数字与选中高亮，出处第 1691-1699 行。选中高亮的来源值得咱们多看一眼，表格视图的 drawCell 逐格绘制，把 State_Selected 写进 option.state，出处（qtableview.cpp 第 1030-1037 行）。选中状态经 storedOption 一路带到绘制，您覆写 paintCell 又不调父类，日期数字没了，选中格子的高亮也跟着没了。

咱们把覆写的骨架摆出来看：

```cpp
class MarkedCalendar : public QCalendarWidget
{
public:
    using QCalendarWidget::QCalendarWidget;

protected:
    // 注意结尾的 const，与声明保持一致
    void paintCell(QPainter *painter, const QRect &rect, QDate date) const override
    {
        QCalendarWidget::paintCell(painter, rect, date);
        if (m_marks.contains(date)) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(Qt::red);
            const QPoint c = rect.bottomLeft() + QPoint(6, -6);
            painter->drawEllipse(c, 3, 3);
        }
    }

private:
    QSet<QDate> m_marks;
};
```

父类的调用放在覆写的开头，默认画面齐了再往上叠。只有打算接管全部日期绘制时才跳过父类，那时范围外格子的 Window 底色、非本月格子的 Disabled 灰字都得您自己补，出处第 1110-1113 行。

rect 的坐标系归内部视图管。QCalendarView 在布局里与导航栏是平级兄弟，出处第 2160-2163 行，矩形来自视图自己的绘制管线，导航栏与表头行都不用您手工扣除。源稿说 QSS 给导航栏加 padding 会让 rect 与可见区域错位、要用 clipRegion 验证，6.9.1 里找不到依据，删。

## 日期格式压住星期格式，越界与非本月谁也拦不住

咱们把合并的次序排开看。三套格式存在模型三个地方，表头格式一份，星期格式按星期存，日期格式按 QDate 存进 QMap，出处（qcalendarwidget.cpp 第 893-895 行）。formatForCell 把它们按固定次序往上叠，调色板打底，表头格式给表头格，星期格式按列对应的星期叠加，日期格式收尾，出处第 1078-1116 行。QTextFormat::merge 的规则是后来者赢，出处（qtextformat.cpp 第 940-963 行），同一格上日期格式压住星期格式，星期格式又压住表头格式，setHeaderTextFormat 文档里那句星期格式的前景背景压过表头，依据就在这一次序，出处（qcalendarwidget.cpp 第 2842-2845 行）。

星期表头行自己也吃星期格式。叠加只看列号不看行号，出处第 1101-1105 行，您给周六设了红字，表头里的 “周六” 两个字跟着变红，默认构造给周六周日挂的红字就是这么染上去的。

合并之后还有两刀，谁也拦不住。日期落在范围外，背景被强制换成 QPalette::Window，日期不在当前展示的月份，前景被强制换成 Disabled 组的 Text，出处第 1110-1113 行。您给下个月 1 号设了醒目的前景色，它露脸时仍显示灰字。

setDateTextFormat 直接写进 QMap，date 为 null 时清掉全部日期格式，传一个不设任何属性的空格式只是存了个空条目，视觉上恢复默认，条目本身还在，出处第 2908-2917 行。格式还能捎带 tooltip，data() 的 ToolTipRole 取的就是 fmt.toolTip()，出处第 1149-1150 行。示例里标记日期挂的 “已标记日期” 提示走的就是同一机制，您把鼠标悬上去就能看到，出处（custom_calendar.cpp 第 67 行）。

## 钳位会连带改另一端，setDateRange 才是一次性的

模型侧的 setDate 把选中日期往范围里钳，出处（qcalendarwidget.cpp 第 1166-1173 行）。范围本身也会连带变形，咱们看模型侧的 setMinimumDate，您给一个高过当前 max 的日期，max 被直接抬到 min，setMaximumDate 反着来时 min 被压到 max，出处第 1206-1207、1219-1220 行，两头一交叉，可选范围悄悄退化成一个日期。

选中日期被钳走时，控件更新选中格、把页面跳到新日期所在的月份，再发 selectionChanged，出处第 2523-2529 行。您在槽里做联动，一次没有用户点击的信号就来了。源稿说 Qt 内部有合并机制，连着改两端不会真的发两次信号，6.9.1 里没有这样的机制，setMinimumDate 与 setMaximumDate 各自判断各自发，两次都把日期钳到位就收到两次信号，出处第 2523-2529、2579-2585 行。一次性的 API 是 setDateRange，min 大于 max 时自动交换，出处第 2612-2632 行与模型侧第 1230-1231 行。动态联动改走它，比 blockSignals 干净，被阻塞的信号不会补发，选中被挪这件事反而无从得知。

范围外与可选性还牵着几处。您点范围外的格子没有反应，它的 flags 返回空，出处第 1159-1162 行，clicked 信号的文档也写明范围外不发，出处第 3056-3059 行。导航栏的月菜单按范围禁用条目，年份输入框的上下界跟着同步，出处第 1821-1833、2521 行。

## objectName 是导航栏的定制入口，翻月交给现成的槽

setNavigationBarVisible 只把 navBarBackground 一个子控件藏起来，出处（qcalendarwidget.cpp 第 3083-3090 行），sizeHint 重算时不再计入它的高度，出处第 2246-2247 行。想给导航栏换外观，里面的构件带着现成的 objectName，qt_calendar_navigationbar、qt_calendar_prevmonth、qt_calendar_nextmonth、qt_calendar_monthbutton、qt_calendar_yearbutton、qt_calendar_yearedit，出处第 1743、1804-1808 行，您拿 QSS 按名字打即可，视图自己叫 qt_calendar_calendarview，出处第 2120 行。

翻月不用手写跨年。showPreviousMonth 与 showNextMonth 内部处理年份滚动，出处第 2387-2399、2408-2419 行，导航按钮的槽把当前格日期加减一个月，超范围的日期还会被钳回边界，出处第 1894-1899、1858-1877 行。源稿那段手写月份减一又漏掉年份减一的示例，删。setCurrentPage(year, month) 只翻页不动选中，它把视图的当前格挪到新月份的同一天，用的还是 NoUpdate，出处第 2359-2378 行，月份传得不合法时 QDate 造不出来，当前格的定位直接跳过，别指望它替您滚年份。

键盘导航在视图的 moveCursor 里，左右一天、上下七天、PageUp 与 PageDown 翻月、Home 与 End 跳到月首月末，RTL 布局下左右翻转，出处第 1317-1373 行。dateEditEnabled 默认开着，日历有焦点时您敲数字，浮出一个 QLabel 套 QFrame 的编辑框，居中盖在控件上，1500 毫秒没有新输入就按框里的日期接受，出处第 613、636-637、667、671-674 行。开关落在 setNavigatorEnabled 上，往视图装卸事件过滤器，出处第 1714-1737 行，selectionMode 设成 NoSelection 时编辑框一并关闭，出处第 2951 行。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，QCalendarWidget 在 QtWidgets，格式定制用的 QTextCharFormat 同在 QtWidgets，链接 Qt6::Widgets 即可。

[Qt 文档 · QCalendarWidget](https://doc.qt.io/qt-6/qcalendarwidget.html) -- 日历控件，paintCell 虚函数、表头格式与范围属性说明

[Qt 文档 · QTextCharFormat](https://doc.qt.io/qt-6/qtextcharformat.html) -- 字符格式，表头、星期、日期三层格式的载体

[Qt 文档 · QTableView](https://doc.qt.io/qt-6/qtableview.html) -- 表格视图，QCalendarView 的父类，网格与委托绘制管线

[Qt 文档 · QLocale](https://doc.qt.io/qt-6/qlocale.html) -- 区域设置，firstDayOfWeek 与星期、月份名称

[Qt 文档 · QDate](https://doc.qt.io/qt-6/qdate.html) -- 日期类，星期判断与月份运算的基础

---

日历到这就翻完了，下一程咱们回到容器家族，QGroupBox 把一组控件圈进带标题的框里，入口在 [3.38 QGroupBox 进阶](./38-qgroupbox-advanced.md)。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/37-qcalendarwidget-advanced/，可执行文件 37-qcalendarwidget-advanced）。构造里一步到位，网格打开、周一起始、中文短名表头、locale 设成中文，出处（custom_calendar.cpp 第 33-42 行）。标记日期橙底加粗还带 tooltip，周末两列红字配浅粉底，今天浅蓝加粗，出处第 63-68、93-103 行，可选范围锁在当年，出处第 116-117 行。两只按钮验证动态加标记与清标记，下方标签跟着 selectionChanged 走，出处（main.cpp 第 53-69 行）。示例里没有 paintCell 覆写，标记全走 setDateTextFormat 的橙底加粗，您想看圆点标记，把前文 paintCell 的覆写骨架接进去就是现成练习。

完成标准照旧。星期表头住在模型第几行、NoHorizontalHeader 靠什么实现，您指得出行号。中文 locale 下默认首日是星期几、setLocale 为什么能冲掉手设的 firstDayOfWeek，您答得上来。覆写 paintCell 不调父类会丢什么，您说得出来。分开设 minimumDate 与 maximumDate 为什么可能收到两次 selectionChanged、setDateRange 交换两端的条件是什么，您不跑程序就能推。
