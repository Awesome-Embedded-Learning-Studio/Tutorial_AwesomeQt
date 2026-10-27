// QtWidgets 入门示例 03: Model/View 架构入门
// 演示：QStringListModel + QListView / QStandardItemModel + QTableView
//       data() / setData() / rowCount() 三大核心接口的协作流程

#include <QApplication>
#include <QWidget>

#include "mainwindow.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 全局样式
    app.setStyleSheet(
        "QTableView {"
        "  gridline-color: #DDD;"
        "  selection-background-color: #3498DB;"
        "  selection-color: white;"
        "}"
        "QListView {"
        "  border: 1px solid #CCC;"
        "  selection-background-color: #3498DB;"
        "  selection-color: white;"
        "}"
        "QListView::item:alternate {"
        "  background-color: #F8F9FA;"
        "}"
        "QPushButton {"
        "  padding: 6px 16px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 4px;"
        "  background-color: #FAFAFA;"
        "}"
        "QPushButton:hover {"
        "  background-color: #E8E8E8;"
        "}"
        "QSplitter::handle {"
        "  background-color: #DDD;"
        "  height: 3px;"
        "}"
    );

    MainWindow window;
    window.show();

    return app.exec();
}
