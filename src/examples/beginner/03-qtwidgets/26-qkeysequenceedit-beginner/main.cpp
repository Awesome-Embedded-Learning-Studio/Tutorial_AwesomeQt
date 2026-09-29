// QtWidgets 入门示例 26: QKeySequenceEdit 快捷键录入控件
// 演示：keySequenceChanged 信号获取录入结果
//       setKeySequence 设置默认快捷键
//       与 QAction::setShortcut 结合的完整热键配置流程
//       冲突检测 + 日志记录

#include <QApplication>

#include "shortcut_config_window.h"

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
        "QKeySequenceEdit {"
        "  padding: 4px 8px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 4px;"
        "  font-size: 12px;"
        "}"
        "QPlainTextEdit {"
        "  border: 1px solid #CCC;"
        "  border-radius: 4px;"
        "  padding: 6px;"
        "}"
        "QMenuBar {"
        "  background-color: #F5F5F5;"
        "  border-bottom: 1px solid #DDD;"
        "}"
        "QMenuBar::item:selected {"
        "  background-color: #E0E0E0;"
        "}");

    ShortcutConfigWindow window;
    window.show();

    return app.exec();
}
