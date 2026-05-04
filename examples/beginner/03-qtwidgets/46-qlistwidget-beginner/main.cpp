// QtWidgets 入门示例 46: QListWidget 便捷列表控件
// 演示：addItem / addItems / insertItem 添加条目
//       currentItem / selectedItems 获取选中
//       QListWidgetItem 图标/复选框/自定义数据
//       itemDoubleClicked / itemChanged 信号

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
        "  padding: 5px 10px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 4px;"
        "}"
        "QLineEdit:focus {"
        "  border-color: #4A90D9;"
        "}"
        "QPushButton {"
        "  padding: 5px 12px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 4px;"
        "  background-color: white;"
        "}"
        "QPushButton:hover {"
        "  background-color: #E8E8E8;"
        "}"
        "QListWidget {"
        "  border: 1px solid #DDD;"
        "  border-radius: 4px;"
        "  padding: 4px;"
        "}"
        "QListWidget::item {"
        "  padding: 4px 2px;"
        "}"
        "QListWidget::item:selected {"
        "  background-color: #E3F2FD;"
        "  color: #1565C0;"
        "}");

    MainWindow window;
    window.show();

    return app.exec();
}
