// QtWidgets 入门示例 38: QGroupBox 分组框
// 演示：带标题边框的控件分组容器
//       setCheckable(true) 可勾选分组框（整组启用/禁用）
//       setAlignment 标题对齐
//       嵌套布局的正确姿势

#include <QApplication>

#include "GroupBoxDemoWidget.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(
        "QGroupBox {"
        "  font-weight: bold;"
        "  border: 1px solid #BDBDBD;"
        "  border-radius: 6px;"
        "  margin-top: 10px;"
        "  padding-top: 18px;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  subcontrol-position: top left;"
        "  left: 12px;"
        "  padding: 0 6px;"
        "  color: #1976D2;"
        "}"
        "QGroupBox::indicator {"
        "  width: 16px;"
        "  height: 16px;"
        "}"
        "QGroupBox::indicator:unchecked {"
        "  border: 2px solid #BDBDBD;"
        "  border-radius: 3px;"
        "  background-color: white;"
        "}"
        "QGroupBox::indicator:checked {"
        "  border: 2px solid #1976D2;"
        "  border-radius: 3px;"
        "  background-color: #1976D2;"
        "}"
    );

    GroupBoxDemoWidget demo;
    demo.show();

    return app.exec();
}
