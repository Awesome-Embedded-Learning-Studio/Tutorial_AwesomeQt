---
title: "3.37 QCalendarWidget 进阶"
description: "入门篇我们搞定了 selectedDate、日期范围约束和 QTextCharFormat 外观定制，已经能把日历控件跑起来并响应选择。但真正到工程里，产品对日历的视觉要求远不止改改字体颜色那么简单。"
---

# 现代Qt开发教程（进阶篇）3.37——QCalendarWidget 进阶

## 1. 前言 / 为什么日历控件在工程里总需要定制

入门篇我们搞定了 selectedDate、日期范围约束和 QTextCharFormat 外观定制，已经能把日历控件跑起来并响应选择。但真正到工程里，产品对日历的视觉要求远不止改改字体颜色那么简单。你可能需要让日历的表头显示"一月"而不是"1"、让周末的格子带一个浅灰底色、在某个截止日期格子里画一个小红点提醒用户、或者干脆把整个导航栏的风格换掉——这些需求入门篇那一套 QTextCharFormat 搞不定，因为 QTextCharFormat 只能控制文字层面的样式，而格子的背景绘制、额外图形元素的叠加需要更底层的介入。

本篇我们从四个方向深入 QCalendarWidget 的定制能力：setHeaderTextFormat 与导航栏的深度定制（包括 horizontalHeaderFormat 和 firstDayOfWeek 的配合），paintCell 重写实现格子级别的自定义绘制（这是 QCalendarWidget 最强大的扩展点），setMinimumDate/setMaximumDate 在运行时动态更新时的边界行为，以及 gridVisible 和 firstDayOfWeek 这些容易被忽略但确实影响用户体验的属性。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17 标准。QCalendarWidget 属于 QtWidgets 模块，链接 Qt6::Widgets 即可。自定义绘制部分涉及 QPainter 和 QTextCharFormat，均包含在 QtWidgets 中，不需要额外依赖。

## 3. 核心概念讲解

### 3.1 setHeaderTextFormat 与导航栏深度定制

入门篇我们用 setHeaderTextFormat(Qt::DayOfWeek, QTextCharFormat) 改了星期几表头的字体和颜色，但没有展开它和日历整体配置的配合关系。这里我们要把这件事讲透。

horizontalHeaderFormat 属性控制星期几表头的显示模式。它有三个可选值：QCalendarWidget::SingleLetterDayName（单字母，比如"M"），QCalendarWidget::ShortDayName（短名称，比如"Mon"或"周一"，取决于 locale），QCalendarWidget::LongDayName（完整名称，比如"Monday"或"星期一"），以及 QCalendarWidget::NoHorizontalHeader（不显示表头）。这个属性直接影响 setHeaderTextFormat 能作用于多少内容——如果你设了 NoHorizontalHeader，setHeaderTextFormat 自然就没有可见目标了。

```cpp
auto *calendar = new QCalendarWidget;
calendar->setHorizontalHeaderFormat(QCalendarWidget::ShortDayName);
// 中文环境下显示"周一、周二……"，英文环境下显示"Mon、Tue……"
```

这里有一个关键的配合点：horizontalHeaderFormat 的 ShortDayName 和 LongDayName 都受 locale 影响。如果你没有显式设置 calendar 的 locale，它继承的是应用的默认 locale。在中文环境下 ShortDayName 显示"周一"，在英文环境下显示"Mon"。这意味着你用 setHeaderTextFormat 设置的格式需要和 locale 显示的文字长度配合——比如给长名称设了很小的字体宽度却很窄，文字就会被截断。

firstDayOfWeek 属性决定了日历每行从星期几开始。默认值取决于 locale——中文和英文环境默认从周日开始，欧洲一些国家默认从周一开始。你可以手动覆盖：

```cpp
calendar->setFirstDayOfWeek(Qt::Monday);  // 强制周一为每行起始
```

firstDayOfWeek 的变更会影响整个日期网格的排列，同时也会影响表头的顺序——如果 firstDayOfWeek 是 Monday，表头就会从"Mon"开始排列到"Sun"。所以如果你同时自定义了表头格式，要确认格式对应的 Qt::DayOfWeek 和表头的视觉位置是一致的。

gridVisible 属性控制日期格子之间是否显示网格线。默认不显示。在自定义绘制场景下，我建议打开它，因为网格线能为用户提供清晰的日期边界感，特别是当你在格子里画了额外元素之后，没有网格线会让格子之间的内容看起来混在一起。

```cpp
calendar->setGridVisible(true);
```

### 3.2 paintCell 重写——格子级别自定义绘制

这是 QCalendarWidget 最强大的扩展点。QCalendarWidget 提供了一个虚函数 paintCell(QPainter* painter, const QRect& rect, const QDate& date)，每当需要绘制某个日期格子时就会被调用。通过子类化 QCalendarWidget 并重写 paintCell，你可以在每个格子里画任何东西——背景色、图标、小圆点、选中标记、覆盖文字等等。

```cpp
class CustomCalendar : public QCalendarWidget
{
public:
    using QCalendarWidget::QCalendarWidget;

protected:
    void paintCell(QPainter* painter, const QRect& rect,
                   const QDate& date) override
    {
        // 先调用父类实现，绘制默认内容（日期数字等）
        QCalendarWidget::paintCell(painter, rect, date);

        // 在格子底部画一个小圆点标记
        if (m_marked_dates.contains(date)) {
            painter->setBrush(Qt::red);
            painter->setPen(Qt::NoPen);
            int dot_size = 6;
            int cx = rect.center().x();
            int cy = rect.bottom() - 8;
            painter->drawEllipse(cx - dot_size / 2, cy - dot_size / 2,
                                 dot_size, dot_size);
        }
    }

private:
    QSet<QDate> m_marked_dates;
};
```

paintCell 的调用时机是 QCalendarWidget 的 paintEvent 内部遍历当前月份的所有可见日期格子，对每个格子调用一次 paintCell。参数 rect 是该格子在 QCalendarWidget 坐标系中的矩形区域，date 是该格子对应的日期。painter 已经配置好了默认的 clip 区域和变换，你直接在上面画就行。

现在有一道调试题给大家。下面这段 paintCell 重写有什么问题？

```cpp
void paintCell(QPainter* painter, const QRect& rect,
               const QDate& date) override
{
    // 不调用父类，完全自定义绘制
    if (date == QDate::currentDate()) {
        painter->fillRect(rect, QColor("#E3F2FD"));
        painter->drawText(rect, Qt::AlignCenter,
                          QString::number(date.day()));
    }
}
```

问题在于：当 date 不是今天时，这个函数什么都不画——那个格子就变成空白了。如果你不打算完全接管所有日期的绘制（包括范围外的灰色日期、非当月的灰色日期等），就一定要在 paintCell 开头调用 QCalendarWidget::paintCell(painter, rect, date) 让父类先把默认内容画好，然后你在此基础上叠加自定义内容。只有在你确实要完全替代默认渲染时才跳过父类调用，但那时你必须处理所有日期的绘制逻辑，包括 selected 状态、out-of-range 状态、today 标记等。

另外一个重要细节：paintCell 传入的 rect 已经考虑了导航栏和星期几表头的高度偏移。也就是说 rect 的坐标是相对于 QCalendarWidget 内部的日期网格区域的，不是相对于整个控件左上角的。你不需要手动减去表头高度。但是——如果你通过 QSS 给导航栏设了比较大的 padding 或者自定义了高度，rect 的计算可能和你预期的不一致。这种情况下需要用 QCalendarWidget::height() 减去导航栏和表头的实际高度来验证 rect 是否正确。

### 3.3 setMinimumDate / setMaximumDate 的动态边界行为

入门篇我们讲了这两个属性的静态用法——设一次就不管了。但在实际项目中，日期范围往往是动态变化的：用户选了一个起始日期后，结束日期的范围要跟着变；或者后台推送了一个新的可用日期区间，前端要实时更新。

动态更新时的核心问题是：如果你在运行时缩小了范围（比如把 maximumDate 从 12 月 31 号改成了 11 月 30 号），而当前 selectedDate 落在了新范围之外，QCalendarWidget 会自动把 selectedDate 调整到新范围内最近的合法日期。这个调整会触发 selectionChanged 信号。如果你在 selectionChanged 的槽函数里做了业务逻辑（比如发网络请求），这个"自动调整触发的信号"就可能让你困惑——你明明没有用户点击操作，怎么业务逻辑被触发了？

```cpp
// 动态缩小范围
calendar->setMaximumDate(QDate(2025, 6, 30));
// 如果 selectedDate 原本是 2025-08-15，会被自动调整为 2025-06-30
// 并触发 selectionChanged 信号
```

更隐蔽的情况是：如果你同时更新 minimumDate 和 maximumDate，比如把范围从"全年"缩小到"本月"，两个 set 调用各可能触发一次 selectedDate 调整。虽然 Qt 内部有合并机制不会真的触发两次信号，但你不能依赖这个行为——在某些 Qt 版本上确实可能触发两次。建议在动态更新范围之前 blockSignals(true)，更新完了再 blockSignals(false)，然后手动检查 selectedDate 是否被调整了。

### 3.4 navigationBarVisible 与导航栏定制

QCalendarWidget 顶部的导航栏（包含前后翻月按钮、月份年份按钮）可以通过 setNavigationBarVisible(false) 完全隐藏。隐藏后你可以自己实现导航控件，然后通过 setCurrentPage(int year, int month) 来控制日历显示的月份。这种方式在外观要求高度统一的商业项目中很常见——用自定义的导航按钮替换默认的 QToolButton 导航栏。

```cpp
calendar->setNavigationBarVisible(false);

// 自定义导航按钮
connect(prev_btn, &QPushButton::clicked, this, [this]() {
    int y = m_calendar->currentPageYear();
    int m = m_calendar->currentPageMonth();
    m_calendar->setCurrentPage(y, m - 1 > 0 ? m - 1 : 12);
    // 月份减到 0 时需要年份也减 1
});
```

## 4. 踩坑预防

第一个坑是 paintCell 中 rect 的坐标不包含导航栏和表头偏移，但在某些 QSS 自定义场景下 rect 的计算会和你预期不一致。具体来说，如果你通过 QSS 给导航栏加了额外的 margin 或 padding，QCalendarWidget 内部的布局计算可能会把这些额外空间计入日期网格的起始位置，导致 paintCell 拿到的 rect 和实际可见区域有微小偏差。解决方案是尽量不在 QSS 中修改导航栏的几何属性，如果必须修改，在 paintCell 中先用 painter->clipRegion() 验证一下实际绘制区域。

第二个坑是 setMinimumDate 导致 selectedDate 跳转并触发 selectionChanged。这个在动态更新范围的场景中特别容易踩——你以为只是改了个约束条件，结果把整个业务状态都带跑了。解决方案是在动态更新范围前后配合 blockSignals 使用，或者用一个标志位标记"这次 selectedDate 变更是自动调整的，不是用户操作"，在槽函数里区分处理。

第三个坑是 horizontalHeaderFormat 设为 ShortDayName 但 locale 不生效。这种情况通常出现在你给 QCalendarWidget 设了自定义 locale 但没有同步更新 horizontalHeaderFormat。QCalendarWidget 在第一次显示时会缓存表头文字，之后改 locale 不会自动刷新缓存的表头。解决方案是先设 locale，再设 horizontalHeaderFormat，或者设完 locale 后调用 update() 强制刷新。

## 5. 练习项目

练习项目：带标记的自定义日历面板。我们要实现一个日历组件，继承 QCalendarWidget，支持在指定日期格子中显示小圆点标记（模拟日程提醒），周末格子有浅色背景，表头使用中文短名称，导航栏替换为自定义控件。

完成标准是：继承 CustomCalendar 后调用 addMarkedDate 即可在日历上看到红色小圆点，周末背景正确渲染不影响日期数字的可读性，自定义导航按钮能正确切换月份并处理跨年边界（比如 2026 年 1 月往前翻应该到 2025 年 12 月），horizontalHeaderFormat 配合中文 locale 正确显示"周一"到"周日"。

提示几个关键点：重写 paintCell 时先调用父类实现再叠加自定义绘制；周末判断用 date.dayOfWeek() == 6 || date.dayOfWeek() == 7；自定义导航栏需要隐藏默认导航栏后用 setCurrentPage 控制页面切换。

## 6. 官方文档参考链接

[Qt 文档 · QCalendarWidget](https://doc.qt.io/qt-6/qcalendarwidget.html) -- 日历控件完整 API，包含 paintCell 虚函数和所有属性

[Qt 文档 · QTextCharFormat](https://doc.qt.io/qt-6/qtextcharformat.html) -- 字符格式类，用于表头和日期文字的样式设置

[Qt 文档 · QDate](https://doc.qt.io/qt-6/qdate.html) -- 日期类，日期计算和星期判断的基础

---

到这里，QCalendarWidget 的进阶定制能力我们就过了一遍。paintCell 重写是整个控件最核心的扩展点——掌握了它，你就能在日历格子里画出任何你想要的视觉效果。horizontalHeaderFormat 和 firstDayOfWeek 的配合决定了日历的基本布局结构，动态范围更新时的信号控制则是工程实践中必须注意的稳定性问题。下一篇我们来看 QGroupBox 的进阶用法。
