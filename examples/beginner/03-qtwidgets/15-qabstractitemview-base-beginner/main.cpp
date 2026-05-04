// QtWidgets 入门示例 15: QAbstractItemView 视图基类
// 演示：setModel() / setSelectionModel() 绑定模型与选择模型
//       选择模式：SingleSelection / MultiSelection / ExtendedSelection
//       currentIndex() / selectedIndexes() 获取选中项
//       setItemDelegate() 设置自定义委托

#include <QApplication>

#include "demo_window.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(
        "QPushButton {"
        "  padding: 5px 12px;"
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
        "QTableView {"
        "  gridline-color: #E0E0E0;"
        "  selection-background-color: #BBDEFB;"
        "  selection-color: #000;"
        "}"
        "QListView {"
        "  selection-background-color: #BBDEFB;"
        "  selection-color: #000;"
        "}"
    );

    DemoWindow window;
    window.show();

    return app.exec();
}
