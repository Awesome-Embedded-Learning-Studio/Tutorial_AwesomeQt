// QtWidgets 入门示例 30: QDateEdit / QTimeEdit / QDateTimeEdit 日期时间输入
// 演示：setMinimumDate / setMaximumDate 限制范围
//       setCalendarPopup(true) 弹出日历选择器
//       QDate / QTime / QDateTime 数据类型互转
//       valueChanged 信号驱动摘要更新

#include <QDateEdit>
#include <QDateTimeEdit>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QTimeEdit>
#include <QWidget>

// ============================================================================
// SchedulePanel: 日程安排面板
// 覆盖 QDateEdit / QTimeEdit / QDateTimeEdit 的核心用法
// ============================================================================
class SchedulePanel : public QWidget
{
    Q_OBJECT

public:
    explicit SchedulePanel(QWidget *parent = nullptr);

private:
    /// @brief 初始化界面
    void initUi();
    /// @brief 更新时长摘要
    void updateSummary();
    /// @brief 添加日程到列表
    void onAddSchedule();

private:
    QLineEdit *m_titleEdit = nullptr;
    QDateEdit *m_startDateEdit = nullptr;
    QTimeEdit *m_startTimeEdit = nullptr;
    QDateTimeEdit *m_endDateTimeEdit = nullptr;
    QLabel *m_summaryLabel = nullptr;
    QListWidget *m_scheduleList = nullptr;
};
