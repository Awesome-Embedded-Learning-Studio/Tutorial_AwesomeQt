// QtWidgets 入门示例 55: QMainWindow 主窗口完整配置
// 演示：setCentralWidget 中央控件
//       菜单栏/工具栏/状态栏/Dock 完整搭建
//       saveGeometry/restoreGeometry 窗口尺寸持久化
//       多 Dock 窗口布局策略（tabify、嵌套）

#include <QApplication>
#include <QCoreApplication>

#include "MainWindow.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    // 设置组织名和应用名（QSettings 使用）
    QCoreApplication::setOrganizationName("AwesomeQt");
    QCoreApplication::setApplicationName("QMainWindowDemo");

    QApplication app(argc, argv);

    app.setStyleSheet(
        "QMainWindow { background-color: #FAFAFA; }"
        "QToolBar { spacing: 4px; padding: 2px; }"
        "QDockWidget::title {"
        "  background-color: #E8EAF6;"
        "  padding: 4px 8px;"
        "  border: none;"
        "}"
        "QPlainTextEdit {"
        "  border: 1px solid #DDD;"
        "  border-radius: 4px;"
        "  padding: 8px;"
        "  font-family: monospace;"
        "  font-size: 14px;"
        "}"
        "QStatusBar {"
        "  background-color: #F5F5F5;"
        "  border-top: 1px solid #E0E0E0;"
        "}"
        "QMenuBar {"
        "  background-color: #F5F5F5;"
        "  border-bottom: 1px solid #E0E0E0;"
        "}"
        "QMenuBar::item:selected {"
        "  background-color: #E3F2FD;"
        "}");

    MainWindow window;
    window.show();

    return app.exec();
}
