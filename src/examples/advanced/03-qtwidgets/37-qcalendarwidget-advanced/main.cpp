/// @file    main.cpp
/// @brief   QCalendarWidget 进阶演示程序入口。
///
/// 展示 CustomCalendar 的 paintCell 重写、QTextCharFormat 定制和日期范围约束。
///
/// 对应教程：进阶层 03-QtWidgets/37-QCalendarWidget 进阶。

#include "custom_calendar.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    auto* window = new QWidget;
    auto* mainLayout = new QVBoxLayout(window);

    // 标题说明
    auto* title = new QLabel(QStringLiteral(
        "QCalendarWidget 进阶演示\n"
        "观察以下定制效果：\n"
        "1. 周末（周六/周日）有浅红背景和暗红文字\n"
        "2. 今天有浅蓝高亮背景和粗体\n"
        "3. 部分日期底部有红色小圆点标记\n"
        "4. 表头显示中文短名称（周一起始）\n"
        "5. 可选范围限制在当前年份"));
    title->setWordWrap(true);
    mainLayout->addWidget(title);

    // 核心：自定义日历控件
    auto* calendar = new CustomCalendar;
    mainLayout->addWidget(calendar);

    // 操作按钮行
    auto* buttonRow = new QHBoxLayout;

    auto* addMarkBtn =
        new QPushButton(QStringLiteral("标记今天 +5 天"));
    auto* clearMarksBtn =
        new QPushButton(QStringLiteral("清除所有标记"));

    buttonRow->addWidget(addMarkBtn);
    buttonRow->addWidget(clearMarksBtn);
    mainLayout->addLayout(buttonRow);

    // 选中日期显示
    auto* selectedLabel = new QLabel(
        QStringLiteral("选中日期：%1").arg(calendar->selectedDate().toString(Qt::ISODate)));
    mainLayout->addWidget(selectedLabel);

    // 信号槽连接
    QObject::connect(calendar, &QCalendarWidget::selectionChanged, [calendar, selectedLabel]() {
        selectedLabel->setText(
            QStringLiteral("选中日期：%1").arg(calendar->selectedDate().toString(Qt::ISODate)));
    });

    QObject::connect(addMarkBtn, &QPushButton::clicked, [calendar]() {
        calendar->addMarkedDate(QDate::currentDate().addDays(5));
    });

    QObject::connect(clearMarksBtn, &QPushButton::clicked, [calendar]() {
        calendar->clearMarkedDates();
    });

    window->setWindowTitle(QStringLiteral("QCalendarWidget Advanced Demo"));
    window->resize(500, 500);
    window->show();

    return app.exec();
}
