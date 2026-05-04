# 现代Qt开发教程（新手篇）3.30——QDateEdit / QTimeEdit / QDateTimeEdit：日期时间输入三件套

## 1. 前言 / 当你需要用户选一个日期或时间

我们在上一篇刚讲完 QSpinBox 和 QDoubleSpinBox，这两个控件解决的是"用户输入一个数字"的问题。但有一类数字输入场景非常特殊——日期和时间。你当然可以用两个 QSpinBox 分别做年和月，但马上就会发现这根本行不通：不同月份的天数不一样，闰年二月有 29 天，用户选了 2025-02-30 你怎么处理？时区转换、夏令时、12 小时制和 24 小时制的显示切换……这些不是靠 SpinBox 拼凑就能搞定的事情。

Qt 为此提供了一套完整的日期时间编辑控件：QDateEdit 专门处理日期输入，QTimeEdit 专门处理时间输入，QDateTimeEdit 则是前两者的合体——同时编辑日期和时间。它们都继承自 QAbstractSpinBox，所以如果你读过上篇就会发现它们的交互模式非常眼熟：上下箭头按步进调整，也可以直接在输入框里键入数字。但和普通 SpinBox 不同的是，它们内部使用 QDate、QTime、QDateTime 这三个值类型来管理数据，这些类型自带日历计算、格式化和合法性校验，我们完全不需要手动处理"2 月 30 号"这种非法日期。

今天我们要把这三个控件的四个核心维度讲透：它们的继承关系和适用场景，setMinimumDate / setMaximumDate（以及对应的时间版本）如何约束输入范围，setCalendarPopup(true) 如何让用户通过弹出日历选择日期，以及 QDate / QTime / QDateTime 三种数据类型之间如何互相转换。

## 2. 环境说明

本篇代码基于 Qt 6.9.1，CMake 3.26+，C++17 标准。QDateEdit、QTimeEdit、QDateTimeEdit 都属于 QtWidgets 模块，链接 Qt6::Widgets 即可。它们底层依赖 QtCore 中的 QDate、QTime、QDateTime 类型——这些类型在 QtCore 中，无需额外链接。示例代码中用到了 QLabel 和 QFormLayout 来布局和展示效果，同样在 QtWidgets 中。

## 3. 核心概念讲解

### 3.1 继承关系与适用场景

先看继承体系。QDateTimeEdit 继承自 QAbstractSpinBox，而 QDateEdit 和 QTimeEdit 都继承自 QDateTimeEdit。这意味着 QDateTimeEdit 是"完全体"，既能编辑日期又能编辑时间；QDateEdit 是 QDateTimeEdit 的简化版，把时间部分固定不显示；QTimeEdit 也是简化版，把日期部分固定不显示。从 API 角度看，QDateTimeEdit 拥有最完整的方法集，QDateEdit 和 QTimeEdit 只是在构造时预设了不同的显示格式。

```cpp
// QDateEdit: 只编辑日期，默认格式 "yyyy/MM/dd"
auto *dateEdit = new QDateEdit();

// QTimeEdit: 只编辑时间，默认格式 "HH:mm:ss"
auto *timeEdit = new QTimeEdit();

// QDateTimeEdit: 同时编辑日期和时间
auto *dateTimeEdit = new QDateTimeEdit();
```

选择哪个控件的原则很简单。如果你只需要用户选一个日期（生日、入职日期、项目截止日期），用 QDateEdit。如果你只需要用户输入一个时间（闹钟时间、会议开始时间、日志时间戳），用 QTimeEdit。如果两者都要（日程安排、航班出发时间、定时任务启动时刻），用 QDateTimeEdit。

虽然 QDateEdit 和 QTimeEdit 在功能上都可以用 QDateTimeEdit 配合 setDisplayFormat 来模拟，但使用专门的子类有两个好处：语义更清晰——读代码的人一眼就知道这个控件是在编辑什么；默认行为更合理——QDateEdit 默认只显示日期部分，初始值是当前日期，QTimeEdit 默认只显示时间部分，初始值是当前时间。

还有一个细节值得提一下：这三个控件的显示格式都可以通过 `setDisplayFormat(const QString &)` 来定制。格式字符串使用 Qt 的日期时间格式模式，其中 yyyy 表示四位年份，MM 表示两位月份，dd 表示两位日期，HH 表示 24 小时制的小时，mm 表示分钟，ss 表示秒。

```cpp
auto *dateEdit = new QDateEdit();
dateEdit->setDisplayFormat("yyyy 年 MM 月 dd 日");  // "2025 年 04 月 22 日"

auto *timeEdit = new QTimeEdit();
timeEdit->setDisplayFormat("HH:mm");  // 不显示秒: "14:30"

auto *dateTimeEdit = new QDateTimeEdit();
dateTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");  // "2025-04-22 14:30:00"
```

### 3.2 setMinimumDate / setMaximumDate 约束范围

和 QSpinBox 的 setRange 一样，日期时间编辑控件也支持设置取值范围。对于日期部分，使用 `setMinimumDate(const QDate &)` 和 `setMaximumDate(const QDate &)`；对于时间部分，使用 `setMinimumTime(const QTime &)` 和 `setMaximumTime(const QTime &)`。QDateTimeEdit 则同时支持这两组方法。

```cpp
auto *dateEdit = new QDateEdit();

// 限制日期范围在 2024-01-01 到 2025-12-31 之间
dateEdit->setMinimumDate(QDate(2024, 1, 1));
dateEdit->setMaximumDate(QDate(2025, 12, 31));

auto *timeEdit = new QTimeEdit();

// 限制时间范围在工作时间 08:00 - 18:00
timeEdit->setMinimumTime(QTime(8, 0, 0));
timeEdit->setMaximumTime(QTime(18, 0, 0));
```

也可以用 `setDateRange(const QDate &, const QDate &)` 和 `setTimeRange(const QTime &, const QTime &)` 一次性设置两个边界，效果和分别调用 setMinimum... / setMaximum... 完全等价。如果用户通过键盘输入了一个超出范围的日期或时间，控件会自动把值修正到最近的有效边界——这个行为和 QSpinBox 的钳位机制一样，同样是静默的，不会报错。

有一个比较隐蔽的坑：默认的 minimumDate 是 1752-09-14（这是英国从儒略历切换到格里高利历的日期），默认的 maximumDate 是 9999-12-31。绝大多数场景下我们不需要改 maximumDate，但 minimumDate 经常需要手动设一下——因为如果你的应用场景是"选择未来的日期"（比如预约系统），你不希望用户能选到 1752 年去。

```cpp
// 预约系统: 日期范围从今天到一年后
auto *appointmentDate = new QDateEdit();
appointmentDate->setMinimumDate(QDate::currentDate());
appointmentDate->setMaximumDate(QDate::currentDate().addYears(1));

// 出生年份: 日期范围从 1900-01-01 到今天
auto *birthdayEdit = new QDateEdit();
birthdayEdit->setMinimumDate(QDate(1900, 1, 1));
birthdayEdit->setMaximumDate(QDate::currentDate());
```

QDate 提供了一系列非常实用的静态方法和成员方法来做日期计算。`QDate::currentDate()` 获取当前日期，`addDays(int)` / `addMonths(int)` / `addYears(int)` 做日期偏移，`daysTo(const QDate &)` 计算两个日期之间的天数差，`isValid()` 检查日期是否合法（比如 2025-02-29 会返回 false）。这些方法让范围约束和日期计算变得异常简单。

```cpp
QDate today = QDate::currentDate();
QDate deadline = today.addDays(30);  // 30 天后的日期
int daysLeft = today.daysTo(deadline);  // 30

QDate invalidDate(2025, 2, 29);  // 2025 年不是闰年
bool valid = invalidDate.isValid();  // false
```

### 3.3 setCalendarPopup(true) 弹出日历选择器

这是 QDateEdit 和 QDateTimeEdit 中我最喜欢的一个特性。默认情况下用户只能通过上下箭头或键盘来调整日期——选一个跨月的日期时体验极差，你得不停地按箭头。但只要调用 `setCalendarPopup(true)`，控件右侧就会出现一个小日历图标，点击后弹出一个完整的月历视图，用户可以直接点击选择日期，还可以通过月份和年份的导航按钮快速跳转到目标日期。

```cpp
auto *dateEdit = new QDateEdit();
dateEdit->setCalendarPopup(true);  // 启用弹出日历

auto *dateTimeEdit = new QDateTimeEdit();
dateTimeEdit->setCalendarPopup(true);  // 同样支持
```

弹出日历的视觉样式取决于当前使用的 QSS 和平台主题。在 Windows 上它看起来像系统的月份选择器，在 Linux + Fusion 风格下则是 Qt 自己绘制的日历网格。日历弹出后，用户可以点击左右箭头切换月份，点击月份标题可以快速选择年份，操作体验比纯键盘输入好太多。

有一点需要注意：QTimeEdit 没有日历弹出功能——这很好理解，时间不需要日历。setCalendarPopup 是 QDateEdit 和 QDateTimeEdit 的功能。

你还可以通过 `setCalendarWidget(QCalendarWidget *)` 方法提供一个自定义的 QCalendarWidget 作为弹出日历。这样就可以对日历的外观和行为做更细粒度的控制——比如设置每周的第一天是周一还是周日，设置导航栏是否可见，或者高亮某些特定日期。

```cpp
auto *dateEdit = new QDateEdit();
dateEdit->setCalendarPopup(true);

auto *calendar = new QCalendarWidget();
calendar->setFirstDayOfWeek(Qt::Monday);  // 周一作为一周的第一天
calendar->setGridVisible(true);           // 显示日期网格线
calendar->setNavigationBarVisible(true);  // 显示月份导航栏

dateEdit->setCalendarWidget(calendar);
```

QCalendarWidget 还有一些高级功能，比如 `setDateRange` 限制日历的可选范围、`setHorizontalHeaderFormat` 设置星期标题的显示格式（短名称如"一/二/三"还是单个字母）、`setVerticalHeaderFormat` 控制是否显示周数。如果你的应用对日期选择体验有较高要求，花点时间定制 QCalendarWidget 是值得的。

### 3.4 QDate / QTime / QDateTime 数据类型互转

QDateEdit、QTimeEdit、QDateTimeEdit 内部分别使用 QDate、QTime、QDateTime 来存储数据。这三者之间的转换关系很明确。

QDateTime 是"日期 + 时间"的组合体，你可以把它拆开，也可以用 QDate 和 QTime 组装出一个 QDateTime。

```cpp
// QDateTime → QDate / QTime
QDateTime dateTime = QDateTime::currentDateTime();
QDate date = dateTime.date();    // 提取日期部分
QTime time = dateTime.time();    // 提取时间部分

// QDate + QTime → QDateTime
QDate today = QDate::currentDate();
QTime morning(9, 0, 0);
QDateTime combined(today, morning);  // 今天的 09:00:00

// 控件之间的转换
auto *dateEdit = new QDateEdit();
auto *timeEdit = new QTimeEdit();
auto *dateTimeEdit = new QDateTimeEdit();

// 从 QDateEdit 和 QTimeEdit 的值构建 QDateTime
QDateTime fromParts(dateEdit->date(), timeEdit->time());

// 从 QDateTimeEdit 的值分别提取日期和时间
QDate d = dateTimeEdit->dateTime().date();
QTime t = dateTimeEdit->dateTime().time();
```

QDate 只关心年月日，QTime 只关心时分秒毫秒，QDateTime 则同时包含两者并且携带时区信息。当你需要做跨时区的日期时间计算时，QDateTime 提供了 `toUTC()`、`toLocalTime()`、`toTimeZone(const QTimeZone &)` 等方法。在大多数桌面应用开发中我们用不到时区转换，但如果你的应用需要处理国际化的时间数据，这些方法会派上大用场。

还有一个实用的方法是 `QDateTime::fromString(const QString &, const QString &)` 和 `QDateTime::toString(const QString &)`，它们可以在字符串和日期时间对象之间做相互转换。这在读取配置文件、解析网络接口返回的时间戳时非常常用。

```cpp
// 字符串 → QDate
QDate date = QDate::fromString("2025-04-22", "yyyy-MM-dd");

// QDate → 字符串
QString dateStr = date.toString("yyyy 年 MM 月 dd 日");  // "2025 年 04 月 22 日"

// 字符串 → QDateTime
QDateTime dt = QDateTime::fromString("2025-04-22 14:30:00",
                                      "yyyy-MM-dd HH:mm:ss");

// QDateTime → 字符串
QString dtStr = dt.toString("yyyy-MM-dd HH:mm:ss");
```

要注意的是 fromString 如果格式不匹配会返回一个无效的 QDate 或 QDateTime，所以在使用转换结果之前最好调用 `isValid()` 检查一下。尤其是在解析用户输入或者外部数据源时，永远不要假设格式一定正确。

在实际项目中，三个控件的取值方法也对应它们各自的数据类型：`QDateEdit::date()` 返回 QDate，`QTimeEdit::time()` 返回 QTime，`QDateTimeEdit::dateTime()` 返回 QDateTime。设值方法同理：`setDate(const QDate &)`、`setTime(const QTime &)`、`setDateTime(const QDateTime &)`。和 QSpinBox 一样，程序调用 set 方法也会触发对应的 `dateChanged`、`timeChanged`、`dateTimeChanged` 信号。

## 4. 踩坑预防

第一个坑是 minimumDate 的默认值是 1752-09-14。这个日期对绝大多数现代应用来说没有意义，如果你的业务场景涉及"未来日期"的选择（预约、排期），一定要手动设置 minimumDate 为一个合理的起点。

第二个坑是 setCalendarPopup 只对 QDateEdit 和 QDateTimeEdit 有效。QTimeEdit 调用这个方法不会有任何效果——时间选择需要的是时/分/秒的调整，不是日历。

第三个坑是 QDate 的构造函数参数顺序是 (year, month, day)，而不是 (day, month, year)。这一点跟 C 标准库的 tm 结构体不一样，跟 Java 的 LocalDate 也不一样。写 QDate(22, 4, 2025) 编译不会报错，但语义完全不对。

第四个坑是 fromString 的格式字符串是区分大小写的。MM 是两位月份，mm 是两位分钟，dd 是两位日期。如果你把 MM 写成了 mm，fromString 返回的结果就是一个无效的 QDate。这种笔误很难通过肉眼检查出来，而且编译器帮不了你。

第五个坑是 QDateTime 的默认构造会使用本地时区。如果你需要存储和传输"绝对时刻"（比如网络接口返回的 UTC 时间戳），要么显式使用 `QDateTime(date, time, QTimeZone::UTC)`，要么在转换前调用 `toUTC()`。时区不一致导致的 bug 极其隐蔽——本地测试好好的，换一个时区的机器就出问题。

## 5. 练习项目

我们来做一个综合练习：创建一个"日程安排面板"窗口，覆盖 QDateEdit、QTimeEdit、QDateTimeEdit 的核心用法。窗口上方是一个日程信息输入区域，包含：开始日期 QDateEdit（弹出日历，范围从今天到一年后）、开始时间 QTimeEdit（范围 06:00-23:00，步进 15 分钟）、截止日期时间 QDateTimeEdit（弹出日历，显示格式 "yyyy-MM-dd HH:mm"）、一个标题 QLineEdit、一个备注 QTextEdit。窗口中间是一个 QLabel，实时显示"从 XXX 到 XXX，共 N 天 N 小时 N 分钟"的摘要信息。窗口下方是一个"添加日程"按钮，点击后将当前输入的日程信息追加到一个 QListWidget 中。

几个提示：计算两个 QDateTime 之间的时间差可以用 `QDateTime::secsTo(const QDateTime &)` 方法，它返回秒数差，然后你自己换算成天/小时/分钟；步进时间的粒度可以通过重写 stepBy 或者用包装类来实现，QTimeEdit 本身的 setSingleStep 对分钟步进有原生支持；弹出日历记得对 QDateEdit 和 QDateTimeEdit 都调用 setCalendarPopup(true)。

## 6. 官方文档参考链接

[Qt 文档 -- QDateEdit](https://doc.qt.io/qt-6/qdateedit.html) -- 日期编辑控件

[Qt 文档 -- QTimeEdit](https://doc.qt.io/qt-6/qtimeedit.html) -- 时间编辑控件

[Qt 文档 -- QDateTimeEdit](https://doc.qt.io/qt-6/qdatetimeedit.html) -- 日期时间编辑控件基类

[Qt 文档 -- QDate](https://doc.qt.io/qt-6/qdate.html) -- 日期值类型

[Qt 文档 -- QTime](https://doc.qt.io/qt-6/qtime.html) -- 时间值类型

[Qt 文档 -- QDateTime](https://doc.qt.io/qt-6/qdatetime.html) -- 日期时间值类型

[Qt 文档 -- QCalendarWidget](https://doc.qt.io/qt-6/qcalendarwidget.html) -- 日历控件

---

到这里，QDateEdit / QTimeEdit / QDateTimeEdit 的四个核心维度就全部讲完了。继承体系决定了你应该选择哪个控件——纯日期用 QDateEdit，纯时间用 QTimeEdit，都要用 QDateTimeEdit。setMinimumDate / setMaximumDate 约束了用户的输入范围，防止出现不合常理的日期选择。setCalendarPopup(true) 是提升日期选择体验的最简手段，一行代码就能让用户从"痛苦的键盘微调"变成"直观的日历点选"。QDate / QTime / QDateTime 三种类型之间的拆分和组装操作，是处理日期时间数据的基础功。这三个控件加上背后的值类型，构成了 Qt 中日期时间输入的完整解决方案——下次再遇到"用户选日期"的场景，不需要再考虑 SpinBox 拼凑方案了。
