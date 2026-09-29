// QtWidgets 入门示例 27: QComboBox 下拉选择框
// 演示：addItem / addItems / insertItem 添加选项
//       currentIndex() / currentText() / currentData() 获取当前值
//       setEditable(true) 可编辑组合框
//       setModel() 用自定义 Model 填充选项

#include <QApplication>

#include "CitySelectorPanel.h"

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
        "QComboBox {"
        "  padding: 5px 10px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 4px;"
        "  background-color: #FFF;"
        "  font-size: 13px;"
        "}"
        "QComboBox:hover {"
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

    CitySelectorPanel panel;
    panel.show();

    return app.exec();
}
