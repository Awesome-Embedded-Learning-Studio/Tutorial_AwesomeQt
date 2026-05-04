// QtWidgets 入门示例 44: QScrollArea 滚动区域容器
// 演示：setWidget 设置被滚动的内容控件
//       setWidgetResizable(true) 内容自适应宽度
//       动态添加内容后自动滚动到底部
//       自定义滚动条样式 QSS

#include <QApplication>

#include "mainwindow.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(
        // ---- 自定义垂直滚动条 ----
        "QScrollBar:vertical {"
        "  background: #F5F5F5;"
        "  width: 8px;"
        "  margin: 0;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: #C0C0C0;"
        "  min-height: 30px;"
        "  border-radius: 4px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "  background: #A0A0A0;"
        "}"
        "QScrollBar::handle:vertical:pressed {"
        "  background: #808080;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "  height: 0px;"
        "}"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
        "  background: none;"
        "}"

        // ---- QScrollArea 外观 ----
        "QScrollArea {"
        "  border: 1px solid #DDD;"
        "  border-radius: 6px;"
        "  background-color: white;"
        "}"

        // ---- 输入框 ----
        "QLineEdit {"
        "  padding: 6px 10px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 4px;"
        "}"
        "QLineEdit:focus {"
        "  border-color: #4A90D9;"
        "}"

        // ---- 按钮 ----
        "QPushButton {"
        "  padding: 6px 14px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 4px;"
        "  background-color: white;"
        "}"
        "QPushButton:hover {"
        "  background-color: #E8E8E8;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #D0D0D0;"
        "}"
    );

    MainWindow window;
    window.show();

    return app.exec();
}
