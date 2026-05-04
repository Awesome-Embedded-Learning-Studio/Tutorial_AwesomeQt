#include <QApplication>
#include "HelpBrowser.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(
        "QGroupBox {"
        "  font-weight: bold;"
        "  border: 1px solid #DDD;"
        "  border-radius: 4px;"
        "  margin-top: 8px;"
        "  padding-top: 16px;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  left: 12px;"
        "  padding: 0 4px;"
        "}"
        "QPushButton {"
        "  padding: 5px 14px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 4px;"
        "  background-color: #FFF;"
        "  font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #E8E8E8;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #DDD;"
        "}"
        "QPushButton:disabled {"
        "  color: #AAA;"
        "  background-color: #F5F5F5;"
        "}"
        "QTextBrowser {"
        "  border: 1px solid #CCC;"
        "  border-radius: 4px;"
        "  padding: 12px;"
        "  font-size: 13px;"
        "  line-height: 1.5;"
        "}");

    HelpBrowser browser;
    browser.show();

    return app.exec();
}
