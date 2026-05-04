// QtWidgets 入门示例 45: QFrame 作为分隔线的用法
// 演示：水平分隔线 HLine+Sunken
//       垂直分隔线在工具栏中使用
//       QFrame 作为有边框容器配置
//       QFrame vs addSpacing 区别

#include <QApplication>

#include "MainWindow.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 全局 QSS：统一输入框和标签的基础外观
    app.setStyleSheet(
        "QLineEdit {"
        "  padding: 4px 8px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 3px;"
        "}"
        "QLineEdit:focus {"
        "  border-color: #4A90D9;"
        "}"
        "QToolBar {"
        "  spacing: 4px;"
        "  padding: 2px;"
        "}"
        "QToolBar QToolButton {"
        "  padding: 4px 8px;"
        "}");

    MainWindow window;
    window.show();

    return app.exec();
}
