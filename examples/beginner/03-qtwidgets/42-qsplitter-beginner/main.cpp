// QtWidgets 入门示例 42: QSplitter 可拖动分割容器
// 演示：水平/垂直分割与嵌套
//       setSizes / sizes 程序化控制各区域宽度
//       setCollapsible 禁止折叠特定区域
//       saveState / restoreState 持久化分割比例

#include <QApplication>

#include "mainwindow.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(
        "QSplitter::handle {"
        "  background-color: #C0C0C0;"
        "}"
        "QSplitter::handle:horizontal {"
        "  width: 3px;"
        "}"
        "QSplitter::handle:vertical {"
        "  height: 3px;"
        "}"
        "QSplitter::handle:hover {"
        "  background-color: #4A90D9;"
        "}"
        "QToolBar {"
        "  spacing: 8px;"
        "  padding: 4px;"
        "  background-color: #F5F5F5;"
        "  border-bottom: 1px solid #DDD;"
        "}"
        "QPushButton {"
        "  padding: 4px 12px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 4px;"
        "  background-color: white;"
        "}"
        "QPushButton:hover {"
        "  background-color: #E8E8E8;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #D0D0D0;"
        "}"
    );

    MainWindow window;
    window.show();

    return app.exec();
}
