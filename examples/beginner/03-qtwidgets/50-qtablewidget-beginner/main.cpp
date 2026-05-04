// QtWidgets 入门示例 50: QTableWidget 便捷表格控件
// 演示：setRowCount / setColumnCount 设置行列数
//       setItem / item 单元格读写
//       setHorizontalHeaderLabels / setVerticalHeaderLabels 表头
//       cellChanged / cellClicked / currentCellChanged 信号

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
        "QTableWidget {"
        "  border: 1px solid #DDD;"
        "  border-radius: 4px;"
        "  gridline-color: #EEE;"
        "}"
        "QTableWidget::item:selected {"
        "  background-color: #E3F2FD;"
        "  color: #1565C0;"
        "}"
        "QHeaderView::section {"
        "  padding: 4px 8px;"
        "  border: none;"
        "  border-bottom: 1px solid #DDD;"
        "  border-right: 1px solid #EEE;"
        "  background-color: #FAFAFA;"
        "  font-weight: bold;"
        "}");

    MainWindow window;
    window.show();

    return app.exec();
}
