// QtWidgets 入门示例 12: QAbstractButton 按钮基类机制
// 演示：setCheckable / setChecked / setAutoRepeat 核心属性
//       clicked / toggled / pressed / released 四个信号
//       QButtonGroup 单选互斥管理
//       继承 QAbstractButton 自定义圆形按钮

#include <QApplication>

#include "DemoWindow.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(
        "QPushButton {"
        "  padding: 6px 14px;"
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
        "QPushButton:checked {"
        "  background-color: #BBDEFB;"
        "  border-color: #64B5F6;"
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
        "QRadioButton {"
        "  spacing: 4px;"
        "  font-size: 12px;"
        "}"
    );

    DemoWindow window;
    window.show();

    return app.exec();
}
