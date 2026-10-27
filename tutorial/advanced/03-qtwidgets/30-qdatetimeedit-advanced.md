---
title: "3.30 QDateTimeEdit 进阶"
description: "入门篇把 QDateTimeEdit、QDateEdit、QTimeEdit 当带箭头的日期时间输入框用顺了，进阶篇对着 6.9.1 源码把节段机制认清，格式串由 QDateTimeParser::parseFormat 切成节段与分隔符，汉字天然是分隔符、英文原样文本才要单引号，hh 没配 AP 节段会被换成 24 小时制，重复节段允许出现、数值打架时解析标 conflicts 后重写文本，setCurrentSectionIndex 越界静默忽略，validate 与 interpret 链子落到节段解析引擎上，fixup 只在 CorrectToNearestValue 下重写文本，stepEnabled 拿当前节段双向试步进，calendarPopup 的范围由 syncCalendarWidget 自动同步，日历所有权不转移，timeZone 是控件自己的属性（Qt 6.7 起），默认日期范围 1752-09-14 到 9999-12-31，配套示例在 examples/advanced/03-qtwidgets/30-qdatetimeedit-advanced/。"
---

# 现代Qt开发教程（进阶篇）3.30——QDateTimeEdit 进阶

[3.29 QSpinBox 进阶](./29-qspinbox-advanced.md) 收尾留了一句，QDateTimeEdit 同样是 QAbstractSpinBox 的子类，validate 与 interpret 的链子到它身上，换成按日期节段处理。咱们接着这句往源码里走，节段（Section）是它分解显示文本的基本单位，格式串切成一排可独立选中的小格子，箭头与键盘都只作用于当前格。笔者把 qdatetimeedit.cpp、qdatetimeedit.h 和 qdatetimeparser.cpp 对着源稿的断言逐条过了一遍，源稿有几处说法与代码对不上，时区的内容偏得最远，QDateTimeEdit 自己就有 timeZone 属性，Qt 6.7 起的。

次序咱们摆好，从格式串怎么被切成节段认起，跟着是点击与步进怎么围着节段下标转，validate 与 interpret 换了引擎之后的模样，再往 calendarPopup 的真实机制走，时区收到最后。正文行号都带出处，示例代码在 examples/advanced/03-qtwidgets/30-qdatetimeedit-advanced/，Qt 内部实现指源码树里的 qdatetimeedit.cpp、qdatetimeedit.h 与 qdatetimeparser.cpp。

## 格式串被 parseFormat 切成节段，汉字天然是分隔符

setDisplayFormat 递进来的字符串，第一站是 QDateTimeParser::parseFormat，（qdatetimeparser.cpp 第 433 行起）。它把格式串从头扫到尾，遇到节段字母就落一个节段节点，字母连写的个数决定节段宽度，countRepeat 一次最多数四位（第 410-418 行）。yyyy 四位全重复是四位年，两位年另有两位年节段（第 524-536 行），字母之间的其余字符走 default 分支进分隔符表（第 571-572 行），不占节段下标。年月日三个汉字就是这样成为分隔符的，咱们拿 “yyyy年MM月dd日” 试，一个引号都不用，节段表照样是年、月、日三个。

单引号在格式串里是引用开关，从一枚单引号到下一枚之间的字符不进节段解析（第 452-458 行），unquote 负责把引号剥掉（第 386-408 行）。源稿叮嘱汉字要用单引号包住才安全，那是多虑，汉字根本不在节段字母表里。真要引号护着的是英文原样文本，您在格式里裸写 a、A、t、y、M、d、h、m、s、z 每一枚都是节段字母，QDateTimeEdit 里 t 还专管时区节段（第 561-571 行），格式里裸写 “at” 会被吃成 AmPmSection 加时区节段。

AmPmSection 只有格式里写了 a 或 A（后面可以跟 p）才出现，（第 506-523 行）。源稿说 hh 是 12 小时制、控件会自动补一个 AmPmSection，方向说反了，parseFormat 收尾有一段反向修正，只有 12 小时节段而没有 AmPm 节段时，小时节段全部换成 24 小时（第 579-587 行）。您想让用户在 AM 与 PM 之间切，格式里得亲手写 AP。QDateEdit 与 QTimeEdit 靠 parserType 区分，时间类字母进了 QDateEdit 的格式、日期类字母进了 QTimeEdit 的格式，case 里的 parserType 判定不通过，字母同样落进分隔符，（第 462、524、538、548 行各 case 的写法）。

您在代码里定位节段有两条路，setCurrentSection 认枚举，setCurrentSectionIndex 认下标。认枚举的实现从当前下标的下一位往后找，一轮找不着再从头找一轮（qdatetimeedit.cpp 第 717-741 行），格式里同一节段出现多次时，从当前光标往后找到的第一个会被选中。

节段表落定，咱们查节段的口子在 qdatetimeedit.h 摆着，currentSection、sectionAt、currentSectionIndex、sectionCount、sectionText 各占其位（第 36-41、109-123 行），返回的枚举是公开的 Section，从 NoSection 到 YearSection 九个值，另有时间与日期两组掩码（第 47-59 行）。重复节段也合法，类文档自己举例 “yyyy/yy/yyyy” 的 sectionCount 是 3，（qdatetimeedit.cpp 第 763 行），两处年份数值打架时解析会标一个 conflicts，validateAndInterpret 把显示文本重写到一致为止，（第 2067-2073 行）。

走下标定位的话，有两处安静的行为您心里得有数。越界的下标不报错也不崩，函数直接返回（第 796-797 行），源稿说的越界崩溃在代码里找不到着落。换格式时当前下标用 qMin 截到新表长以内（第 950 行），同一个数字在新格式里指的可能是另一个节段，硬编码下标的代码错位就是这么来的。笔者读到 sectionAt 的类文档注释时停了一下，它把 “yyyy/MM/dd” 的 sectionAt(2) 写成了 YearSection，正确答案是 DaySection（第 744-746 行），这里要信代码不信注释。还有一枚 setSelectedSection，源稿说它是 Qt 6 引入的便捷方法，注释写的是 since 4.2（第 852-857 行），实现要求节段在显示列表里，否则一动不动，在的话 setCurrentSection 之后，把该节段选成高亮（第 864-873 行）。

## 点击定节段，箭头与键盘都围着节段下标转

您点进输入框，光标一动，editorCursorPositionChanged 把光标位置翻译成节段下标，拿新光标位置查节段归属（qdatetimeedit.cpp 第 2327 行），落不进节段就吸附到最近的节段（第 2345-2353 行），最后一步，把 currentSectionIndex 换成它（第 2373 行）。节段一切换还顺手 interpret 一轮，敲到一半的值当场结算（第 2356-2358 行）。左右方向键走另一条路，QDateTimeEdit 重写了 focusNextPrevChild，在节段之间挪选区，越过最后一个节段才把焦点交出去（第 1365-1380 行）。

上下箭头与键盘步进都汇进 stepBy，真正干活的一行喂的就是当前节段下标（第 1437 行），步完把该节段重新选中（第 1440 行）。您点中年份时箭头只动年份，全部机制就在这一行里，没有别的开关。

咱们看 stepEnabled 在这个子类里的算法，与 QSpinBox 那版不同，29 篇里默认实现只比边界值与 wrapping，这里换成拿当前节段试步。只读返回空（第 1515-1516 行），当前下标指到空位返回空（第 1553-1557 行），wrapping 开着两面全给（第 1559-1560 行），其余情况朝上试一步、朝下试一步，哪边走得动给哪边标志（第 1562-1568 行）：

```cpp
// qdatetimeedit.cpp stepEnabled 的试步（第 1562-1568 行，只列向上的一支）
QVariant v = d->stepBy(d->currentSectionIndex, 1, true);
if (v != d->value)
    ret |= QAbstractSpinBox::StepUpEnabled;
```

第三参 true 是 test 标志，只算不改值，私有 stepBy 的实现在第 2120 行起。试步不便宜，每帧重绘都要跑，您别在 stepEnabled 里做重活，29 篇数过的调用密度在这里原样成立。

特殊值也在考虑之列，值停在 minimum 且设了 specialValueText 时，stepEnabled 只给向上一步的标志，min 与 max 相等才连它也不给（第 1517-1519 行）。29 篇里咱们见过 specialValueText 整行换掉显示文本，这里的行为接得上，那一档值能不能往下步，控件有自己的判断。

## validate 与 interpret 落到节段解析上，fixup 只在就近修正时改文本

咱们把基类的链子对上，interpret 接手后走 validate，不过就 fixup 一次再验（qabstractspinbox.cpp 第 1899-1916 行），变的是虚函数那一头。QDateTimeEdit::validate 转手给私有 validateAndInterpret，（qdatetimeedit.cpp 第 1480-1486 行），后者把整行文本交给解析器按节段处理（第 2054 行）。空串的判定也在这里，单节段格式或设了 specialValueText 算中间态，其余算无效（第 2019-2027 行），设了 specialValueText 时，还会按前缀逐字符，对输入做大小写不敏感的比对（第 2028-2051 行）。

fixup 的实现值得您读一遍（第 1493-1506 行）。它带着 fix 标志把 validateAndInterpret 再跑一遍，注释写明 CorrectToPreviousValue 的回退交给 QAbstractSpinBox，只有 correctionMode 设成 CorrectToNearestValue 时，才把输入文本重写成解析后的样子。注释还点了春令时的名，解析合法的时刻转进控件时区后可能不存在，落空的值会被降级成中间态（第 2060-2063 行），机制的另一半放到时区一节再说。

咱们在 29 篇认过的 valueFromText 与 textFromValue，到这边换成 textFromDateTime 与 dateTimeFromText 一对受保护的虚函数，（qdatetimeedit.h 第 166-167 行）。前者的实现一行转给 locale().toString(dateTime, displayFormat, calendar)（qdatetimeedit.cpp 第 1452-1456 行），后者转手 validateAndInterpret（第 1467-1474 行），类文档照旧提醒，重写这一对就得连 validate 一起重写，（第 1444-1450 行）。

## calendarPopup：范围自动同步，日历的所有权不进编辑器

calendarPopup 打开后，弹出的是 QCalendarPopup，一个 Popup 壳，里面嵌一枚 QCalendarWidget。能不能弹由 calendarPopupEnabled 说了算，条件是 calendarPopup 为 true，且格式里至少有一个日期节段（第 2711-2714 行）。纯时间格式例如 HH:mm:ss 永远弹不出来，箭头回到步进的老路，源稿把它归因于当前点中的节段，实际只看格式里有没有日期节段。日期节段在场时，您点箭头区域直接弹日历，点别处才走基类的处理（第 1579-1596 行）。

setCalendarWidget 收自定义日历有三道前置，日历非空、calendarPopup 已开、格式含日期节段，哪条不满足都给一句 qWarning 然后返回，（第 831-847 行）。所有权这件事源稿说反了，类文档明写编辑器不接手所有权（第 824 行附近），实际动作发生在弹出层内部，QCalendarPopup::setCalendarWidget 把旧日历删掉，再把您给的日历 addWidget 进自己的布局（第 2756-2773 行），日历从此搬了家。同一枚日历塞给另一台 QDateTimeEdit，日历会被搬进新弹出层，旧弹出层里只剩空壳，一器配一个编辑器是干净做法，理由是搬家，与所有权转移无关。

范围同步是全自动的，咱们看 syncCalendarWidget，它拿编辑器的 minimumDate、maximumDate 和当前日期推给弹出层（第 2716-2723 行）：

```cpp
// qdatetimeedit.cpp syncCalendarWidget（第 2719-2722 行）
const QSignalBlocker blocker(monthCalendar);
monthCalendar->setDateRange(q->minimumDate(), q->maximumDate());
monthCalendar->setDate(q->date());
```

setRange 的重载在范围一变时就调它（第 2637-2641 行），每次值提交也会再跑一遍（第 2303 行）。源稿让您手动把 minimumDate 复制给日历的那段代码，写进去也会被下一次同步盖掉，删了干净。默认范围提一句，下限是 1752-09-14，上限 9999-12-31（qdatetimeparser_p.h 第 36-38 行的常量，默认值设在 qdatetimeedit.cpp 第 368、410 行），比 100 年 1 月 1 日还早的下限，setMinimumDateTime 整个静默忽略，连警告都没有（第 371-379 行）。私有构造里也是拿 startOfDay 与 endOfDay 落的默认上下限，（第 1759-1760 行）。

## timeZone 是控件自己的属性，输入解析完都转进它

源稿在时区上偏得最远。它断言 QDateTimeEdit 没有时区属性，只存显 QDateTime，转换全靠您自己倒。属性表明明白白摆着一条 QTimeZone timeZone，（qdatetimeedit.h 第 45 行），Qt 6.7 引入（qdatetimeedit.cpp 第 1043-1044 行），老的 Qt::TimeSpec timeSpec 还在，6.10 起标了废弃（头文件第 132-133 行），从 6.7 起它只是 timeZone 的间接入口（第 1010 行）。控件默认时区是本地，私有构造的默认实参就是 QTimeZone::LocalTime，（qdatetimeedit_p.h 第 39 行）。笔者顺着属性声明往回核对默认值，另一处落在 QTimeEdit，构造时拿到的是 UTC（qdatetimeedit.cpp 第 190-191 行），纯时间的值不挂时区语义。

控件时区管两头。您 setDateTime 传进来什么时区的值，convertTimeZone 会把它转进控件时区再入库（第 235 行调用，实现只有一行 toTimeZone，第 1764-1767 行）。用户敲完文本，解析出的时刻也统一转进控件时区（第 2060 行），转完失效的值降级成中间态，fixup 一节提过的春令时注释说的就是它。想让用户直接在 UTC 下编辑，老写法是存取两端自己 toLocalTime、toUTC 来回倒，现在给控件 setTimeZone(QTimeZone::utc()) 就行，显示与取值都落在 UTC 里。枚举本身还有一层数目要改准，Qt::TimeSpec 有四个值，LocalTime、UTC、OffsetFromUTC、TimeZone，（qnamespace.h 第 1247-1252 行），源稿只数了三个。

限制也有。格式串里写了 t 这个时区节段字母，用户输入的时区信息仍然作不了主，解析完一律按控件时区重述，源码里留着一条 TODO（第 1041 行），类文档把它写在明处（第 1047-1050 行）。界面看不出值活在哪个时区，跨时区的场景您在旁边摆一枚标签注明时区，这个建议源稿提得对，留着。

## 官方文档参考

本篇基于 Qt 6.5+ / CMake 3.26+ / C++17，QDateTimeEdit、QDateEdit、QTimeEdit 与 QCalendarWidget 都在 QtWidgets，链接 Qt6::Widgets 即可。

[Qt 文档 · QDateTimeEdit](https://doc.qt.io/qt-6/qdatetimeedit.html) -- 日期时间编辑控件，Section、calendarPopup、timeZone 属性的正式说明

[Qt 文档 · QDateTimeEdit::Section](https://doc.qt.io/qt-6/qdatetimeedit.html#Section-enum) -- Section 枚举值与节段相关接口

[Qt 文档 · QCalendarWidget](https://doc.qt.io/qt-6/qcalendarwidget.html) -- 日历控件，弹出日历的定制接口

[Qt 文档 · QDateTime](https://doc.qt.io/qt-6/qdatetime.html) -- 日期时间值类型，格式串与 toString 的写法

[Qt 文档 · QTimeZone](https://doc.qt.io/qt-6/qtimezone.html) -- 时区类，QTimeZone::utc() 与 IANA 时区 ID

---

输入类的键盘阵营到这里收尾，下一站 [3.31 QSlider 进阶](./31-qslider-advanced.md)。它换到 QAbstractSlider 这边（qslider.h 第 17 行），没有行编辑框也没有节段，值的进出走滑条自己的路，咱们去那边接着认。

想动手的，您把示例跑起来（examples/advanced/03-qtwidgets/30-qdatetimeedit-advanced/，可执行文件 30-qdatetimeedit-advanced），窗口里一枚 QDateTimeEdit 加一块信息面板，格式是 “yyyy-MM-dd HH:mm:ss”，六个节段（section_aware_datetime.cpp 第 30 行）。面板列着当前节段名、索引与各节段文本（第 117-141 行），总数见 rebuildSectionList（第 113-114 行），点输入框的不同部位，面板跟着换，上下箭头只动当前格。

实验咱们顺着走三步。第一步，点年份看索引归 0，点分钟看索引归 4，光标位置换节段下标的出处就是第 2327 行那一句，箭头只动一格的原因在第 1437 行。第二步，把第 30 行的格式改成 “hh:mm:ss AP”，面板里多出 AmPmSection（示例的名字映射在第 186 行），再把 AP 拿掉只留 hh，节段数少一个，小时照样按 24 小时制走，出处 qdatetimeparser.cpp 第 579-587 行。换 “yyyy年MM月dd日” 也试一下，汉字不加引号，节段表照常三个。第三步，看事件过滤器（第 147-175 行），示例拦鼠标按下与按键释放，刷新用 QueuedConnection 排到控件处理完点击之后（第 153、166 行），因为 QDateTimeEdit 的信号只有 dateTimeChanged、timeChanged、dateChanged 三个，出处 qdatetimeedit.h 第 146-149 行，没有 currentSectionChanged，节段切换只能这样旁听。

完成标准照旧，点哪个节段您说得清下标怎么来的，格式改完能预判节段列表，AP 的有无能讲出为什么。
