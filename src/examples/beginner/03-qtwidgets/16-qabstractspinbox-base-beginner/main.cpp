// QtWidgets 入门示例 16: QAbstractSpinBox 数字输入基类
// 演示：setReadOnly / setButtonSymbols 控件外观
//       editingFinished 信号与输入验证
//       stepBy() 步进值控制
//       validate() 输入合法性检验机制

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
        "QSpinBox, QDoubleSpinBox {"
        "  padding: 4px 8px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 3px;"
        "}"
    );

    DemoWindow window;
    window.show();

    return app.exec();
}
