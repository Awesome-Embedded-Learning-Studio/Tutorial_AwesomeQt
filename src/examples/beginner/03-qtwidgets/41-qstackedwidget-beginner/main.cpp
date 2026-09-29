// QtWidgets 入门示例 41: QStackedWidget 堆叠页面控件
// 演示：addWidget 添加页面 / setCurrentIndex 切换页面
//       与 QComboBox / QListWidget 组合做导航菜单
//       currentChanged 信号响应页面切换
//       区别于 QTabWidget（无标签头，适合自定义导航）

#include <QApplication>

#include "StackedWidgetDemoWidget.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(
        "QListWidget {"
        "  border: 1px solid #E0E0E0;"
        "  border-right: none;"
        "  background-color: #FAFAFA;"
        "  font-size: 13px;"
        "  outline: none;"
        "}"
        "QListWidget::item {"
        "  padding: 10px 12px;"
        "  border-bottom: 1px solid #F0F0F0;"
        "}"
        "QListWidget::item:selected {"
        "  background-color: #E3F2FD;"
        "  color: #1976D2;"
        "  font-weight: bold;"
        "}"
        "QListWidget::item:hover {"
        "  background-color: #F0F4F8;"
        "}"
        "QStackedWidget {"
        "  border: 1px solid #E0E0E0;"
        "  border-left: none;"
        "  background-color: white;"
        "}"
        "QLineEdit {"
        "  border: 1px solid #E0E0E0;"
        "  border-radius: 4px;"
        "  padding: 6px 10px;"
        "  font-size: 13px;"
        "}"
        "QLineEdit:focus {"
        "  border-color: #1976D2;"
        "}"
        "QSpinBox {"
        "  border: 1px solid #E0E0E0;"
        "  border-radius: 4px;"
        "  padding: 6px 10px;"
        "  font-size: 13px;"
        "}"
        "QComboBox {"
        "  border: 1px solid #E0E0E0;"
        "  border-radius: 4px;"
        "  padding: 6px 10px;"
        "  font-size: 13px;"
        "}"
        "QCheckBox {"
        "  font-size: 13px;"
        "  spacing: 8px;"
        "}"
    );

    StackedWidgetDemoWidget demo;
    demo.show();

    return app.exec();
}
