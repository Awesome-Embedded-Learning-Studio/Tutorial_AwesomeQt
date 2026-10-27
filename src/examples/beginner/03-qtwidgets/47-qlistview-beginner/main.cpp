// QtWidgets 入门示例 47: QListView Model 驱动列表视图
// 演示：与 QStringListModel 配合
//       setViewMode 列表/图标模式
//       setSpacing / setGridSize 图标布局
//       自定义 ItemDelegate 改变显示样式

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
        "QListView {"
        "  border: 1px solid #DDD;"
        "  border-radius: 4px;"
        "}"
        "QSpinBox {"
        "  padding: 3px 6px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 3px;"
        "}");

    MainWindow window;
    window.show();

    return app.exec();
}
