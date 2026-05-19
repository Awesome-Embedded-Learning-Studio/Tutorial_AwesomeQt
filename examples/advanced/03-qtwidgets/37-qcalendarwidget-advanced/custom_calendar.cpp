/// @file    custom_calendar.cpp
/// @brief   CustomCalendar 类实现——QTextCharFormat 定制与日期范围约束。
///
/// 对应教程：进阶层 03-QtWidgets/37-QCalendarWidget 进阶。

#include "custom_calendar.h"

#include <QDate>
#include <QTextCharFormat>

// ─────────────────────────────────────────────────────────────────────────────
// 常量
// ─────────────────────────────────────────────────────────────────────────────

namespace
{
    const QColor kWeekendTextColor = QColor(0xCC, 0x33, 0x33);    // 周末文字颜色（暗红）
    const QColor kWeekendBgColor = QColor(0xFD, 0xF0, 0xF0);     // 周末背景色（浅粉）
    const QColor kTodayBgColor = QColor(0xE3, 0xF2, 0xFD);       // 今日高亮背景色（浅蓝）
    const QColor kHeaderTextColor = QColor("#2E75B6");            // 表头文字颜色
    const QColor kMarkBgColor = QColor(0xFF, 0xF3, 0xE0);        // 标记日期背景色（浅橙）
    const QColor kMarkTextColor = QColor(0xE6, 0x5C, 0x00);      // 标记日期文字颜色（橙色）
}

// ─────────────────────────────────────────────────────────────────────────────
// 构造函数
// ─────────────────────────────────────────────────────────────────────────────

CustomCalendar::CustomCalendar(QWidget* parent)
    : QCalendarWidget(parent)
{
    // 显示网格线，为自定义格式提供清晰的日期边界感
    setGridVisible(true);

    // 周一起始——符合中国和欧洲用户习惯
    setFirstDayOfWeek(Qt::Monday);

    // 短名称表头（中文环境下显示"周一"至"周日"）
    setHorizontalHeaderFormat(QCalendarWidget::ShortDayNames);

    // 设置默认 locale 确保中文表头正确显示
    setLocale(QLocale(QLocale::Chinese));

    setupTextFormats();
    setupDateRange();

    // 添加几个示例标记日期
    addMarkedDate(QDate::currentDate().addDays(3));
    addMarkedDate(QDate::currentDate().addDays(7));
    addMarkedDate(QDate::currentDate().addDays(14));
}

// ─────────────────────────────────────────────────────────────────────────────
// 公有接口
// ─────────────────────────────────────────────────────────────────────────────

void CustomCalendar::addMarkedDate(const QDate& date)
{
    if (date.isValid()) {
        m_markedDates.insert(date);

        // 使用 setDateTextFormat 为该日期设置特殊格式
        QTextCharFormat markFormat;
        markFormat.setBackground(kMarkBgColor);
        markFormat.setForeground(kMarkTextColor);
        markFormat.setFontWeight(QFont::Bold);
        markFormat.setToolTip(QStringLiteral("已标记日期"));
        setDateTextFormat(date, markFormat);

        update();  // 触发重绘
    }
}

void CustomCalendar::clearMarkedDates()
{
    // 清除格式前先备份，再逐个恢复默认
    for (const auto& date : m_markedDates) {
        // 传入空的 QTextCharFormat 恢复为默认格式
        QTextCharFormat defaultFormat;
        setDateTextFormat(date, defaultFormat);
    }
    m_markedDates.clear();
    update();
}

// ─────────────────────────────────────────────────────────────────────────────
// 私有方法
// ─────────────────────────────────────────────────────────────────────────────

void CustomCalendar::setupTextFormats()
{
    // 周末单元格样式——暗红文字 + 浅粉背景
    QTextCharFormat weekendFormat;
    weekendFormat.setForeground(kWeekendTextColor);
    weekendFormat.setBackground(kWeekendBgColor);
    setWeekdayTextFormat(Qt::Saturday, weekendFormat);
    setWeekdayTextFormat(Qt::Sunday, weekendFormat);

    // 今日高亮样式——浅蓝背景 + 粗体
    QTextCharFormat todayFormat;
    todayFormat.setBackground(kTodayBgColor);
    todayFormat.setFontWeight(QFont::Bold);
    setDateTextFormat(QDate::currentDate(), todayFormat);

    // 表头（星期几导航栏）样式——蓝色文字 + 粗体
    QTextCharFormat headerFormat;
    headerFormat.setForeground(kHeaderTextColor);
    headerFormat.setFontWeight(QFont::Bold);
    setHeaderTextFormat(headerFormat);
}

void CustomCalendar::setupDateRange()
{
    // 限制可选日期范围为当前年份
    const QDate today = QDate::currentDate();
    setMinimumDate(QDate(today.year(), 1, 1));
    setMaximumDate(QDate(today.year(), 12, 31));
}

void CustomCalendar::refreshMarkedFormats()
{
    for (const auto& date : m_markedDates) {
        QTextCharFormat markFormat;
        markFormat.setBackground(kMarkBgColor);
        markFormat.setForeground(kMarkTextColor);
        markFormat.setFontWeight(QFont::Bold);
        setDateTextFormat(date, markFormat);
    }
}
