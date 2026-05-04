// QtWidgets 入门示例 39: QTabWidget 标签页控件
// 演示：addTab / insertTab / removeTab 动态管理标签页
//       setTabPosition 上/下/左/右 与 setTabShape
//       currentChanged 信号响应标签切换
//       setTabIcon / setTabToolTip 标签美化

#include <QApplication>

#include "tabwidgetdemowidget.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(
        "QTabWidget::pane {"
        "  border: 1px solid #BDBDBD;"
        "  border-radius: 4px;"
        "  padding: 4px;"
        "}"
        "QTabBar::tab {"
        "  background-color: #F5F5F5;"
        "  border: 1px solid #BDBDBD;"
        "  border-bottom: none;"
        "  border-top-left-radius: 6px;"
        "  border-top-right-radius: 6px;"
        "  padding: 6px 14px;"
        "  margin-right: 2px;"
        "  font-size: 13px;"
        "}"
        "QTabBar::tab:selected {"
        "  background-color: white;"
        "  border-bottom: 2px solid #1976D2;"
        "  font-weight: bold;"
        "  color: #1976D2;"
        "}"
        "QTabBar::tab:hover {"
        "  background-color: #E3F2FD;"
        "}"
        "QTabBar::tab:disabled {"
        "  color: #BDBDBD;"
        "  background-color: #FAFAFA;"
        "}"
    );

    TabWidgetDemoWidget demo;
    demo.show();

    return app.exec();
}
