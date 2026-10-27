// QtWidgets 入门示例 49: QTreeView Model 驱动树视图
// 演示：与 QStandardItemModel 配合树结构展示
//       QFileSystemModel + QTreeView 文件树示例
//       setRootIndex 设置显示根节点
//       expand / collapse / expandAll 节点展开控制

#include <QApplication>
#include "MainWindow.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(
        "QLineEdit {"
        "  padding: 4px 8px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 3px;"
        "}"
        "QLineEdit:focus {"
        "  border-color: #4A90D9;"
        "}"
        "QPushButton {"
        "  padding: 5px 10px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 3px;"
        "  background-color: white;"
        "}"
        "QPushButton:hover {"
        "  background-color: #E8E8E8;"
        "}"
        "QTreeView {"
        "  border: 1px solid #DDD;"
        "  border-radius: 4px;"
        "}"
        "QTreeView::item:selected {"
        "  background-color: #E3F2FD;"
        "  color: #1565C0;"
        "}");

    MainWindow window;
    window.show();

    return app.exec();
}
