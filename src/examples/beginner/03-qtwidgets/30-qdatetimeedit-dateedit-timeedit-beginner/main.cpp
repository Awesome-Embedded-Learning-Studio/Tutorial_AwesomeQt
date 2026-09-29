// QtWidgets 入门示例 30: QDateEdit / QTimeEdit / QDateTimeEdit 日期时间输入
// 演示：setMinimumDate / setMaximumDate 限制范围
//       setCalendarPopup(true) 弹出日历选择器
//       QDate / QTime / QDateTime 数据类型互转
//       valueChanged 信号驱动摘要更新

#include <QApplication>

#include "SchedulePanel.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(
        "QGroupBox {"
        "  font-weight: bold;"
        "  border: 1px solid #DDD;"
        "  border-radius: 4px;"
        "  margin-top: 8px;"
        "  padding-top: 16px;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  left: 12px;"
        "  padding: 0 4px;"
        "}"
        "QDateEdit, QTimeEdit, QDateTimeEdit {"
        "  padding: 5px 8px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 4px;"
        "  background-color: #FFF;"
        "  font-size: 13px;"
        "}"
        "QDateEdit:hover, QTimeEdit:hover, QDateTimeEdit:hover {"
        "  border-color: #1976D2;"
        "}");

    SchedulePanel panel;
    panel.show();

    return app.exec();
}
