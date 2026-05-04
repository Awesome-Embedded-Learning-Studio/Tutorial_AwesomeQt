// QtWidgets 入门示例 43: QToolBox 工具箱折叠面板
// 演示：addItem / insertItem 添加面板
//       currentChanged 信号响应当前面板切换
//       setItemEnabled 禁用某个面板
//       侧边栏导航的典型应用场景

#include <QApplication>

#include "mainwindow.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(
        "QToolBox::tab {"
        "  padding: 8px 12px;"
        "  min-height: 20px;"
        "  font-weight: bold;"
        "  border: 1px solid #CCC;"
        "  border-radius: 3px;"
        "  background-color: #F0F0F0;"
        "}"
        "QToolBox::tab:selected {"
        "  background-color: #E0E8F0;"
        "  border-color: #4A90D9;"
        "}"
        "QToolBox::tab:!enabled {"
        "  color: #999;"
        "  background-color: #F8F8F8;"
        "}"
        "QSplitter::handle {"
        "  background-color: #C0C0C0;"
        "}"
        "QSplitter::handle:horizontal {"
        "  width: 3px;"
        "}"
        "QSplitter::handle:hover {"
        "  background-color: #4A90D9;"
        "}"
    );

    MainWindow window;
    window.show();

    return app.exec();
}
