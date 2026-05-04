// QtWidgets 入门示例 37: QCalendarWidget 日历控件
// 演示：setSelectedDate / selectedDate 日期选择
//       selectionChanged 信号响应用户选择
//       setMinimumDate / setMaximumDate 可选范围
//       setHeaderTextFormat / setDateTextFormat 自定义外观

#include <QApplication>

#include "CalendarDemoWidget.h"

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
        "#qt_calendar_navigationbar {"
        "  background-color: #E3F2FD;"
        "}"
    );

    CalendarDemoWidget demo;
    demo.show();

    return app.exec();
}
