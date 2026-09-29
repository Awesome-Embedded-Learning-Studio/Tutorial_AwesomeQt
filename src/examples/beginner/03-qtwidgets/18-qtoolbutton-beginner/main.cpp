// QtWidgets 入门示例 18: QToolButton 工具栏专用按钮
// 演示：setToolButtonStyle（图标/文字/两者）
//       setPopupMode（菜单显示模式：延迟/即时/只有箭头）
//       与 QAction 关联：setDefaultAction()
//       在工具栏中自动调整样式的机制

#include "mainwindow.h"

#include <QApplication>

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(
        "QToolBar {"
        "  spacing: 6px;"
        "  padding: 4px;"
        "  background-color: #F5F5F5;"
        "  border-bottom: 1px solid #DDD;"
        "}"
        "QToolButton {"
        "  padding: 4px 8px;"
        "  border: 1px solid transparent;"
        "  border-radius: 4px;"
        "}"
        "QToolButton:hover {"
        "  background-color: #E0E0E0;"
        "  border: 1px solid #CCC;"
        "}"
        "QToolButton:pressed {"
        "  background-color: #D0D0D0;"
        "}"
        "QToolButton::menu-button {"
        "  width: 16px;"
        "  border-left: 1px solid #CCC;"
        "}"
        "QToolButton::menu-button:hover {"
        "  background-color: #D0D0D0;"
        "}"
    );

    MainWindow window;
    window.show();

    return app.exec();
}
