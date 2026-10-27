// QtWidgets 入门示例 29: QSpinBox / QDoubleSpinBox 数字步进框
// 演示：setRange / setSingleStep / setPrefix / setSuffix
//       setValue / value() 取值与设值
//       QDoubleSpinBox::setDecimals() 控制小数位
//       valueChanged(int/double) 信号响应

#include <QApplication>

#include "ImageExportPanel.h"

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
        "  padding: 5px 8px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 4px;"
        "  background-color: #FFF;"
        "  font-size: 13px;"
        "}"
        "QSpinBox:hover, QDoubleSpinBox:hover {"
        "  border-color: #1976D2;"
        "}"
        "QPlainTextEdit {"
        "  border: 1px solid #DDD;"
        "  border-radius: 4px;"
        "  padding: 8px;"
        "  font-family: monospace;"
        "  font-size: 12px;"
        "}"
    );

    ImageExportPanel panel;
    panel.show();

    return app.exec();
}
