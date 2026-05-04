# 现代Qt开发教程（新手篇）3.37——QCalendarWidget：日历控件

## 1. 前言 / 日历控件其实挺能折腾的

说到日历控件，很多人的第一反应是"这不就是一个让用户选日期的组件吗，有什么好讲的"。说实话一开始我也是这么想的，直到有一次做项目管理工具的时候，产品经理要求日历上要高亮显示节假日、周末要灰显、只能选未来七天内的日期、而且选中日期后旁边的面板要实时联动更新。我才发现 QCalendarWidget 远不只是"setSelectedDate 拿到日期就完事"——它的外观定制、日期范围约束、选中信号响应这些功能加在一起，足够单独写一篇教程了。

QCalendarWidget 的定位是一个内嵌在窗口中的月历视图控件，用户可以通过鼠标点击选择日期，也可以通过导航按钮在不同月份之间切换。它不是一个弹出的日期选择对话框（那个是 QDateEdit 或者自己封装的弹窗），而是一个直接嵌入布局的完整日历面板。这种"嵌入式日历"在很多场景下很有用：日程管理界面的侧边栏、酒店预订系统的日期选择面板、考勤系统的月度打卡视图等等。

今天我们要从四个方面拆解 QCalendarWidget：setSelectedDate/selectedDate 的日期读写与 selectionChanged 信号响应，setMinimumDate/setMaximumDate 的可选日期范围约束，setHeaderTextFormat/setDateTextFormat 的外观自定义（包括高亮特定日期、修改星期几的显示样式），以及一个综合性的练习项目把这些知识点串起来。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QCalendarWidget 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。它内部使用了 QDate（QtCore 模块）来处理日期计算，不需要额外链接。示例代码中还用到了 QLabel、QPushButton、QDateEdit、QVBoxLayout、QHBoxLayout、QGroupBox 和 QTextCharFormat 来搭建界面。

## 3. 核心概念讲解

### 3.1 setSelectedDate / selectedDate 与 selectionChanged 信号

QCalendarWidget 的核心交互模式就是"用户点一个日期，程序拿到这个日期"。setSelectedDate(const QDate &) 设置当前选中的日期，selectedDate() 返回当前选中的日期。选中日期会在日历上以高亮背景显示，默认的选中样式取决于当前的平台主题。

```cpp
auto *calendar = new QCalendarWidget;

// 设置选中日期为 2025 年 6 月 15 日
calendar->setSelectedDate(QDate(2025, 6, 15));

// 读取当前选中日期
QDate current = calendar->selectedDate();
// current.toString("yyyy-MM-dd") → "2025-06-15"
```

看起来很简单对吧？但这里有一个需要注意的地方：setSelectedDate 会自动切换日历视图到选中日期所在的月份。也就是说如果当前日历显示的是 2025 年 1 月，你调用 setSelectedDate(QDate(2025, 8, 20))，日历会跳转到 2025 年 8 月并高亮 20 号。这个行为在大部分情况下是符合预期的，但如果你只是想程序化地记住一个日期而不想改变当前显示的月份，就需要先保存 currentPageMonth() 和 currentPageYear()，设置完 selectedDate 之后再 setCurrentPage(year, month) 跳回来。

```cpp
// 程序化设置选中日期但不跳转月份
int year = calendar->currentPageYear();
int month = calendar->currentPageMonth();
calendar->setSelectedDate(QDate(2025, 8, 20));
calendar->setCurrentPage(year, month);  // 跳回原来的月份
```

selectionChanged() 信号在用户点击选择新日期时触发，也在程序调用 setSelectedDate 时触发（如果新日期和当前选中日期不同的话）。这个信号没有参数——你需要通过 selectedDate() 来获取新的选中日期。

```cpp
connect(calendar, &QCalendarWidget::selectionChanged, this, [this]() {
    QDate selected = m_calendar->selectedDate();
    m_label->setText("选中日期: " + selected.toString("yyyy-MM-dd"));
});
```

除了 selectionChanged，QCalendarWidget 还有几个有用的信号。currentPageChanged(int year, int month) 在用户切换月份时触发，参数直接告诉你新的年月。activated(const QDate &) 在用户双击日期（或者在某些平台上按回车）时触发，和 selectionChanged 的区别是 activated 表示用户"确认"了选择，而 selectionChanged 只是"高亮变了"。clicked(const QDate &) 在用户单击日期时触发，和 selectionChanged 类似但带有具体的日期参数。

```cpp
// 用户切换月份时更新标题
connect(calendar, &QCalendarWidget::currentPageChanged, this,
        [](int year, int month) {
    qDebug() << "当前页面:" << year << "年" << month << "月";
});

// 用户双击确认选择
connect(calendar, &QCalendarWidget::activated, this,
        [](const QDate &date) {
    qDebug() << "确认选择:" << date.toString("yyyy-MM-dd");
});
```

### 3.2 setMinimumDate / setMaximumDate 可选范围

在很多业务场景中，你需要限制用户可选的日期范围。比如酒店预订系统不允许选择过去的日期，会议预约系统只能选未来 30 天内的日期，生日选择器不允许选择未来的日期。QCalendarWidget 通过 setMinimumDate 和 setMaximumDate 来设置这个范围约束。

```cpp
auto *calendar = new QCalendarWidget;

// 不允许选择今天之前的日期
calendar->setMinimumDate(QDate::currentDate());

// 只允许选择未来 30 天
calendar->setMaximumDate(QDate::currentDate().addDays(30));
```

设置范围后，日历上范围之外的日期会显示为灰色且不可点击。用户无法通过鼠标选中这些日期，setSelectedDate 也会对范围外的日期无效。minimumDate() 和 maximumDate() 分别返回当前设置的最小和最大日期。

```cpp
auto *calendar = new QCalendarWidget;
calendar->setMinimumDate(QDate(2025, 1, 1));
calendar->setMaximumDate(QDate(2025, 12, 31));

// 尝试设置范围外的日期——不会生效
calendar->setSelectedDate(QDate(2024, 12, 31));
// selectedDate() 仍然是之前的值，不会变成 2024-12-31
```

有一个容易踩的坑：setMinimumDate 和 setMaximumDate 之间存在隐式的交叉约束。如果你先设了 minimumDate 为 2025-06-01，再设 maximumDate 为 2025-03-31（最大日期比最小日期还早），QCalendarWidget 不会报错，但行为就未定义了——日历可能无法显示、可能显示异常、也可能直接忽略其中一个约束。所以设置范围时一定要保证 minimum <= maximum。

```cpp
// 错误示范：最小日期在最大日期之后
calendar->setMinimumDate(QDate(2025, 6, 1));
calendar->setMaximumDate(QDate(2025, 3, 31));  // 矛盾！

// 正确做法：先设最大值，再设最小值（如果需要的话）
QDate minDate = QDate(2025, 3, 1);
QDate maxDate = QDate(2025, 12, 31);
if (minDate <= maxDate) {
    calendar->setMinimumDate(minDate);
    calendar->setMaximumDate(maxDate);
}
```

还有一个细节是 setMinimumDate 会影响日历的导航按钮。如果 minimumDate 设为 2025-06-01，日历显示 2025 年 6 月时，"上个月"按钮会被禁用（因为 2025-05 没有可选日期）。同理 setMaximumDate 会禁用"下个月"按钮。这个行为是自动的，不需要你手动处理。

默认情况下 minimumDate 和 maximumDate 的范围相当大（从公元前 4714 年到公元 1 亿年左右），实际使用中几乎不会超出。只有在业务明确需要约束时才去设置。

### 3.3 setHeaderTextFormat / setDateTextFormat 自定义外观

QCalendarWidget 的外观定制是通过 QTextCharFormat 来实现的。QTextCharFormat 是 Qt 富文本框架中的字符格式类，可以设置字体、颜色、背景色、粗体、下划线等属性。QCalendarWidget 提供了三个方法来应用不同的格式。

setHeaderTextFormat(Qt::DayOfWeek, const QTextCharFormat &) 设置星期几表头的显示格式。Qt::DayOfWeek 是枚举值，从 Qt::Monday 到 Qt::Sunday。

```cpp
auto *calendar = new QCalendarWidget;

// 把周六和周日的表头设为红色
QTextCharFormat weekendFormat;
weekendFormat.setForeground(QColor("#D32F2F"));
weekendFormat.setFontWeight(QFont::Bold);

calendar->setHeaderTextFormat(Qt::Saturday, weekendFormat);
calendar->setHeaderTextFormat(Qt::Sunday, weekendFormat);
```

setDateTextFormat(const QDate &, const QTextCharFormat &) 设置指定日期的显示格式。这个方法接受一个具体的日期和一个格式对象，可以用来高亮特定的日期——比如节假日、生日、截止日期等。

```cpp
auto *calendar = new QCalendarWidget;

// 高亮 2025 年 10 月 1 日（国庆节）
QTextCharFormat holidayFormat;
holidayFormat.setForeground(QColor("#FFFFFF"));
holidayFormat.setBackground(QColor("#D32F2F"));
holidayFormat.setFontWeight(QFont::Bold);

calendar->setDateTextFormat(QDate(2025, 10, 1), holidayFormat);
calendar->setDateTextFormat(QDate(2025, 10, 2), holidayFormat);
calendar->setDateTextFormat(QDate(2025, 10, 3), holidayFormat);
```

这里有一个关键的限制：setDateTextFormat 只能设置单个日期的格式，没有"设置所有周末格式"这种批量接口。如果你想把当前月份的所有周六和周日都设为灰色，需要自己遍历当前月份的每一天，判断星期几，然后逐个调用 setDateTextFormat。

```cpp
/// @brief 设置指定月份中所有周末的格式
void setWeekendFormat(QCalendarWidget *calendar, int year, int month,
                      const QTextCharFormat &format)
{
    QDate date(year, month, 1);
    QDate endDate = date.addMonths(1).addDays(-1);

    while (date <= endDate) {
        int dayOfWeek = date.dayOfWeek();
        // Qt: Saturday = 6, Sunday = 7
        if (dayOfWeek == 6 || dayOfWeek == 7) {
            calendar->setDateTextFormat(date, format);
        }
        date = date.addDays(1);
    }
}
```

这里面有一个非常容易踩的坑：当你调用 setDateTextFormat 设置了某个日期的格式后，如果用户切换到了其他月份再切回来，之前设置的格式会丢失吗？答案是：不会丢失。QCalendarWidget 内部维护了一个 QMap<QDate, QTextCharFormat> 格式映射表，设置的格式会一直保留，直到你调用 setDateTextFormat(date, QTextCharFormat()) 清除（传入一个空的格式对象）。但如果你只想设置当前月份的格式，用户切换月份后不应该继续保留，那你就需要在 currentPageChanged 信号中手动清除旧格式并设置新月份的格式。

另外一个坑是：setDateTextFormat 对 minimumDate/maximumDate 范围外的日期也有效——你完全可以给一个不可选的日期设置格式。这种情况下日期会显示你设置的格式（比如红色文字），但用户依然无法点击选中它。如果你希望范围外的日期统一显示为灰色，不需要手动设置——QCalendarWidget 会自动处理。

setWeekdayTextFormat(Qt::DayOfWeek, const QTextCharFormat &) 是第三个格式设置方法，它设置的是"某个星期几的全局默认格式"——也就是说设了 setWeekdayTextFormat(Qt::Sunday, format) 后，所有周日的日期都会自动使用这个格式，不需要逐个日期设置。这个方法比 setDateTextFormat 方便多了，但它设置的格式优先级低于 setDateTextFormat——如果你同时用 setDateTextFormat 给某个周日设了特殊格式，那个特殊格式会覆盖 setWeekdayTextFormat 的全局格式。

```cpp
// 全局设置所有周日为灰色
QTextCharFormat sundayFormat;
sundayFormat.setForeground(QColor("#999999"));
calendar->setWeekdayTextFormat(Qt::Sunday, sundayFormat);

// 但某个特定的周日（比如 10-5 国庆假期）特殊高亮
QTextCharFormat holidayFormat;
holidayFormat.setBackground(QColor("#D32F2F"));
holidayFormat.setForeground(QColor("white"));
calendar->setDateTextFormat(QDate(2025, 10, 5), holidayFormat);
// 10-5 仍然是周日，但使用 holidayFormat 而不是 sundayFormat
```

QCalendarWidget 的 QSS 支持也非常有限，基本上只能控制整体背景色、导航栏的按钮和工具条。日期格内的文字颜色和背景色必须通过 QTextCharFormat 来设置，不能通过 QSS。如果你需要完全自定义日历的外观（比如把日历改成深色主题），需要同时用 QSS 控制外部框架和 QTextCharFormat 控制日期单元格。

```css
QCalendarWidget {
    background-color: #FAFAFA;
}
QCalendarWidget QToolButton {
    color: #333;
    font-size: 14px;
    font-weight: bold;
}
QCalendarWidget QMenu {
    background-color: white;
}
#qt_calendar_navigationbar {
    background-color: #E3F2FD;
}
#qt_calendar_prevmonth {
    qproperty-icon: none;
}
#qt_calendar_nextmonth {
    qproperty-icon: none;
}
#qt_calendar_monthbutton {
    color: #1976D2;
    font-weight: bold;
}
#qt_calendar_yearbutton {
    color: #1976D2;
    font-weight: bold;
}
```

上面的 QSS 片段展示了 QCalendarWidget 的几个可定制元素：qt_calendar_navigationbar 是顶部导航栏，qt_calendar_prevmonth / qt_calendar_nextmonth 是前后翻月按钮，qt_calendar_monthbutton / qt_calendar_yearbutton 是月份和年份的显示按钮。这些 object name 是 QCalendarWidget 内部硬编码的，可以通过 QSS 来修改样式。但日期网格内部的样式仍然需要 QTextCharFormat。

## 4. 踩坑预防

第一个坑是 setDateTextFormat 设置的格式不会随月份切换自动清理。如果你只给 1 月份的周末设了灰色格式，用户切到 2 月份时 1 月份的格式仍然保留在内部映射表中。虽然不影响 2 月份的显示（2 月份的日期没有对应的格式映射），但如果你每个月都大量调用 setDateTextFormat，映射表会越来越大。建议在 currentPageChanged 中做一次清理。

第二个坑是 setSelectedDate 会自动跳转月份。如果你在程序中有"记住用户最后选择的日期"这种需求，恢复日期时要注意保存和恢复当前显示的月份。

第三个坑是 setMinimumDate 大于 setMaximumDate 时的未定义行为。设置范围约束时一定要保证最小值不超过最大值，最好加一个断言或者条件判断。

第四个坑是 QCalendarWidget 的尺寸比较大。默认情况下它至少需要 300x200 像素的空间才能正常显示，如果你的布局空间紧张，可能需要配合 QScrollArea 或者用 QDateEdit 替代。QCalendarWidget 不适合用在工具栏或者侧边栏这种紧凑布局中。

第五个坑是 QSS 对 QCalendarWidget 的支持非常有限。日期单元格的文字和背景必须用 QTextCharFormat，QSS 只能控制导航栏和整体背景。如果你试图用 QSS 改变日期文字颜色，大概率不会生效。

## 5. 练习项目

我们来做一个综合练习：创建一个"日期选择演示"窗口。窗口左侧是一个 QCalendarWidget，右侧是一个信息面板。信息面板包含：一个 QLabel 显示当前选中日期（格式 yyyy-MM-dd 星期几），一个 QDateEdit 让用户手动输入日期并与日历联动，两个 QDateEdit 分别设置可选范围的最小和最大日期（修改后立即生效），以及一个"高亮当前月份的周末"按钮。点击按钮后，当前月份的所有周六周日会用灰色文字显示。日历下方的导航栏通过 QSS 改为蓝色主题。

提示：高亮周末需要遍历当前月份的所有日期，通过 QDate::dayOfWeek() 判断是否为周末（6 和 7），然后调用 setDateTextFormat。currentPageChanged 信号中可以清理上个月的格式并重新设置新月份的格式。

## 6. 官方文档参考链接

[Qt 文档 -- QCalendarWidget](https://doc.qt.io/qt-6/qcalendarwidget.html) -- 日历控件

[Qt 文档 -- QDate](https://doc.qt.io/qt-6/qdate.html) -- 日期类（日历控件的日期操作基础）

[Qt 文档 -- QTextCharFormat](https://doc.qt.io/qt-6/qtextcharformat.html) -- 字符格式类（自定义日历外观）

---

到这里，QCalendarWidget 的核心功能就全部覆盖了。setSelectedDate 和 selectedDate 是日历控件的"输入输出接口"，配合 selectionChanged 信号可以实现"用户选日期、程序响应"的基本交互。setMinimumDate 和 setMaximumDate 提供了日期范围约束的能力，范围外的日期自动变灰不可选。setHeaderTextFormat、setDateTextFormat 和 setWeekdayTextFormat 三种格式设置方法各有分工——表头用 setHeader、特定日期用 setDate、全局星期格式用 setWeekday，优先级是 setDateTextFormat > setWeekdayTextFormat。虽然 QCalendarWidget 的 QSS 定制空间有限，但配合 QTextCharFormat 和导航栏的 object name 选择器，依然可以做出相当不错的视觉效果。搞清楚这些之后，无论是日程管理面板、预订系统日历还是考勤打卡视图，你都能自如地使用 QCalendarWidget 了。
