// QtWidgets 入门示例 20: QCheckBox 复选框
// 演示：三态复选框：setTristate(true) 与 Qt::PartiallyChecked
//       checkStateChanged(Qt::CheckState) vs toggled(bool) 信号区别
//       复选框组实现"全选/全不选"逻辑
//       与 QTreeWidget 结合的层级复选

#include <QApplication>

#include "check_box_demo.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(
        "QGroupBox {"
        "  font-weight: bold;"
        "  border: 1px solid #DDD;"
        "  border-radius: 6px;"
        "  margin-top: 8px;"
        "  padding-top: 16px;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  left: 12px;"
        "  padding: 0 4px;"
        "}"
        "QCheckBox {"
        "  spacing: 6px;"
        "  padding: 3px 2px;"
        "}"
        "QCheckBox::indicator {"
        "  width: 16px;"
        "  height: 16px;"
        "  border-radius: 3px;"
        "  border: 2px solid #BDBDBD;"
        "  background-color: white;"
        "}"
        "QCheckBox::indicator:checked {"
        "  background-color: #1976D2;"
        "  border-color: #1976D2;"
        "}"
        "QCheckBox::indicator:indeterminate {"
        "  background-color: #90CAF9;"
        "  border-color: #1976D2;"
        "}"
        "QCheckBox::indicator:hover {"
        "  border-color: #64B5F6;"
        "}"
        "QTreeWidget {"
        "  border: 1px solid #DDD;"
        "  border-radius: 4px;"
        "  padding: 4px;"
        "}"
    );

    CheckBoxDemo demo;
    demo.show();

    return app.exec();
}
