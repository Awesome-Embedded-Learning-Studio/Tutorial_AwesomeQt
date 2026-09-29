/// @file    custom_calendar.h
/// @brief   演示 QCalendarWidget 的 QTextCharFormat 定制与日期范围约束。
///
/// 对应教程：进阶层 03-QtWidgets/37-QCalendarWidget 进阶。

#pragma once

#include <QCalendarWidget>
#include <QDate>
#include <QSet>

/// 自定义日历控件——带标记日期、周末背景色和自定义表头格式。
///
/// 展示四个核心知识点：
/// - setDateRange 限制可选日期范围
/// - QTextCharFormat 定制单元格外观（周末颜色、今日高亮、表头格式）
/// - setWeekdayTextFormat / setHeaderTextFormat 定制星期和导航栏
/// - setDateTextFormat 为标记日期设置特殊格式（橙色背景 + 粗体）
class CustomCalendar : public QCalendarWidget
{
    Q_OBJECT

public:
    /// @brief 构造函数，配置日期范围、表头格式和默认标记。
    /// @param[in] parent 父控件指针。
    explicit CustomCalendar(QWidget* parent = nullptr);

    /// @brief 添加一个标记日期，该日期格子以橙色背景 + 粗体显示。
    /// @param[in] date 要标记的日期。
    /// @note 内部通过 setDateTextFormat 为指定日期设置 QTextCharFormat。
    void addMarkedDate(const QDate& date);

    /// @brief 清除所有标记日期，恢复为默认格式。
    void clearMarkedDates();

private:
    /// @brief 配置 QTextCharFormat——周末颜色、今日高亮、表头字体。
    /// @note horizontalHeaderFormat 必须与 locale 配合，否则表头文字可能被截断。
    void setupTextFormats();

    /// @brief 设置可选日期范围为当前年份。
    /// @note 动态缩小范围时 selectedDate 可能被自动调整并触发 selectionChanged 信号。
    void setupDateRange();

    /// @brief 刷新所有标记日期的 QTextCharFormat。
    /// @note 每次添加/清除标记后调用，确保格式同步。
    void refreshMarkedFormats();

private:
    QSet<QDate> m_markedDates;  // 需要高亮标记的日期集合
};
