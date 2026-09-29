// QtWidgets 入门示例 37: QCalendarWidget 日历控件
// 演示：setSelectedDate / selectedDate 日期选择
//       selectionChanged 信号响应用户选择
//       setMinimumDate / setMaximumDate 可选范围
//       setHeaderTextFormat / setDateTextFormat 自定义外观

#include <QCalendarWidget>
#include <QDateEdit>
#include <QLabel>
#include <QWidget>

// ============================================================================
// CalendarDemoWidget: QCalendarWidget 综合演示窗口
// ============================================================================
class CalendarDemoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CalendarDemoWidget(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();

    /// @brief 更新选中日期标签
    void updateSelectedLabel();

    /// @brief 高亮指定月份的所有周末
    void highlightWeekends(int year, int month);

private:
    QCalendarWidget *m_calendar = nullptr;
    QLabel *m_selectedLabel = nullptr;
    QDateEdit *m_dateEdit = nullptr;
    QDateEdit *m_minDateEdit = nullptr;
    QDateEdit *m_maxDateEdit = nullptr;
    QLabel *m_statusLabel = nullptr;
};
