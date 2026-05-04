// QtWidgets 入门示例 56: QMenuBar / QMenu / QAction 菜单系统
// 演示：menuBar()->addMenu() 添加顶级菜单
//       QAction 创建菜单项、图标、快捷键
//       setCheckable / QActionGroup / addSeparator
//       contextMenuEvent 右键上下文菜单
//       菜单与工具栏共享 QAction

#include <QApplication>

#include "TextEditor.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(
        "QToolBar { spacing: 4px; padding: 2px; }"
        "QPlainTextEdit {"
        "  border: 1px solid #DDD;"
        "  border-radius: 4px;"
        "  padding: 8px;"
        "  font-family: monospace;"
        "  font-size: 13px;"
        "}"
        "QMenuBar {"
        "  background-color: #F5F5F5;"
        "  border-bottom: 1px solid #E0E0E0;"
        "}"
        "QMenuBar::item:selected {"
        "  background-color: #E3F2FD;"
        "}"
        "QMenu {"
        "  border: 1px solid #CCC;"
        "  padding: 4px 0px;"
        "}"
        "QMenu::item {"
        "  padding: 4px 24px;"
        "}"
        "QMenu::item:selected {"
        "  background-color: #E3F2FD;"
        "}"
        "QMenu::separator {"
        "  height: 1px;"
        "  background: #E0E0E0;"
        "  margin: 4px 8px;"
        "}"
        "QStatusBar {"
        "  background-color: #F5F5F5;"
        "  border-top: 1px solid #E0E0E0;"
        "}");

    TextEditor editor;
    editor.show();

    return app.exec();
}
