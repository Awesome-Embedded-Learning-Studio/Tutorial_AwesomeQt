#include <QApplication>

#include "MainWindow.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(
        "QColumnView {"
        "  border: 1px solid #DDD;"
        "  border-radius: 4px;"
        "}"
        "QColumnView::item:selected {"
        "  background-color: #E3F2FD;"
        "  color: #1565C0;"
        "}"
        "QColumnView::item:hover {"
        "  background-color: #F5F5F5;"
        "}");

    MainWindow window;
    window.show();

    return app.exec();
}
