#include <QApplication>

#include "MainWindow.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(
        "QPushButton {"
        "  padding: 5px 10px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 3px;"
        "  background-color: white;"
        "}"
        "QPushButton:hover {"
        "  background-color: #E8E8E8;"
        "}"
        "QComboBox {"
        "  padding: 4px 8px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 3px;"
        "  background-color: white;"
        "}"
        "QTableView {"
        "  border: 1px solid #DDD;"
        "  border-radius: 4px;"
        "  gridline-color: #EEE;"
        "}"
        "QTableView::item:selected {"
        "  background-color: #E3F2FD;"
        "  color: #1565C0;"
        "}");

    MainWindow window;
    window.show();

    return app.exec();
}
