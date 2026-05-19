/// @file    main.cpp
/// @brief   QDateTimeEdit Section 感知演示程序入口。
///
/// 启动 SectionAwareDateTime 窗口，展示 currentSection、sectionText、
/// sectionCount、sectionAt 等 Section 相关 API 的实时行为。
///
/// 对应教程：进阶层 03-QtWidgets/30-QDateTimeEdit 进阶。

#include "section_aware_datetime.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    SectionAwareDateTime widget;
    widget.show();

    return app.exec();
}
