// QtWidgets 入门示例 24: QPlainTextEdit 纯文本高性能编辑器
// 演示：appendPlainText 追加日志的正确用法
//       setMaximumBlockCount 限制行数防内存溢出
//       highlightCurrentLine 实现行高亮效果

#include <QApplication>
#include "log_terminal.h"

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
        "QPlainTextEdit {"
        "  border: 1px solid #CCC;"
        "  border-radius: 4px;"
        "  padding: 8px;"
        "  font-size: 12px;"
        "}"
        "QComboBox {"
        "  padding: 4px 8px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 4px;"
        "}");

    LogTerminal terminal;
    terminal.show();

    return app.exec();
}
