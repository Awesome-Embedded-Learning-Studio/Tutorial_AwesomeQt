// QtWidgets 入门示例 48: QTreeWidget 便捷树形控件
// 演示：QTreeWidgetItem 构建层级树结构
//       addTopLevelItem / insertChild 增删节点
//       setColumnCount / setHeaderLabels 多列树表
//       itemExpanded / itemCollapsed / itemClicked 信号

#include <QApplication>
#include "MainWindow.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(
        "QLineEdit {"
        "  padding: 4px 8px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 3px;"
        "}"
        "QLineEdit:focus {"
        "  border-color: #4A90D9;"
        "}"
        "QPushButton {"
        "  padding: 5px 10px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 3px;"
        "  background-color: white;"
        "}"
        "QPushButton:hover {"
        "  background-color: #E8E8E8;"
        "}"
        "QTreeWidget {"
        "  border: 1px solid #DDD;"
        "  border-radius: 4px;"
        "}"
        "QTreeWidget::item:selected {"
        "  background-color: #E3F2FD;"
        "  color: #1565C0;"
        "}"
        "QHeaderView::section {"
        "  padding: 4px 8px;"
        "  border: none;"
        "  border-bottom: 1px solid #DDD;"
        "  background-color: #FAFAFA;"
        "  font-weight: bold;"
        "}");

    MainWindow window;
    window.show();

    return app.exec();
}
