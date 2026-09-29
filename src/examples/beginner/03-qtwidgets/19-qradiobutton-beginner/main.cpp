// QtWidgets 入门示例 19: QRadioButton 单选按钮
// 演示：自动互斥：同一 parent 下单选按钮天然互斥
//       QButtonGroup 跨 parent 实现互斥分组
//       toggled(bool) 信号监听状态变化
//       自定义样式 QSS 圆形按钮美化

#include <QApplication>

#include "settings_widget.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 全局 QSS: 圆形 indicator 美化
    app.setStyleSheet(
        // QGroupBox 样式
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

        // QRadioButton 圆形 indicator
        "QRadioButton::indicator {"
        "  width: 18px;"
        "  height: 18px;"
        "  border-radius: 9px;"
        "  border: 2px solid #BDBDBD;"
        "  background-color: white;"
        "}"
        "QRadioButton::indicator:checked {"
        "  border-color: #1976D2;"
        "  background-color: #1976D2;"
        "}"
        "QRadioButton::indicator:hover {"
        "  border-color: #90CAF9;"
        "}"
        "QRadioButton::indicator:unchecked:hover {"
        "  border-color: #64B5F6;"
        "}"

        // QRadioButton 整体
        "QRadioButton {"
        "  spacing: 8px;"
        "  padding: 6px 4px;"
        "}"
        "QRadioButton:hover {"
        "  background-color: rgba(25, 118, 210, 0.04);"
        "  border-radius: 4px;"
        "}"
    );

    SettingsWidget widget;
    widget.show();

    return app.exec();
}
