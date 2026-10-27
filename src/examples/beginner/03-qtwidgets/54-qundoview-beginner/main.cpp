#include <QApplication>

#include "MainWindow.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(
        "QToolBar { spacing: 4px; padding: 2px; }"
        "QPushButton {"
        "  padding: 5px 12px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 3px;"
        "  background: #FAFAFA;"
        "}"
        "QPushButton:hover {"
        "  background: #E3F2FD;"
        "  border-color: #90CAF9;"
        "}"
        "QPushButton:pressed {"
        "  background: #BBDEFB;"
        "}"
        "QUndoView {"
        "  border: 1px solid #DDD;"
        "  border-radius: 4px;"
        "}"
        "QUndoView::item:selected {"
        "  background-color: #E3F2FD;"
        "  color: #1565C0;"
        "}"
        "QUndoView::item:hover {"
        "  background-color: #F5F5F5;"
        "}"
        "QPlainTextEdit {"
        "  border: 1px solid #DDD;"
        "  border-radius: 4px;"
        "  padding: 8px;"
        "  font-family: monospace;"
        "  font-size: 14px;"
        "}");

    MainWindow window;
    window.show();

    return app.exec();
}
