// QtWidgets 入门示例 06: 对话框体系基础
// 演示：QDialog exec() 模态 vs show() 非模态
//       QDialogButtonBox 标准按钮配置
//       自定义对话框布局与数据返回
//       accept() / reject() 关闭对话框
//       个人信息编辑对话框

#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(
        "QPushButton {"
        "  border: 1px solid #CCC;"
        "  border-radius: 4px;"
        "  background-color: #FAFAFA;"
        "}"
        "QPushButton:hover { background-color: #E8E8E8; }"
        "QPushButton:pressed { background-color: #DDD; }"
        "QLineEdit, QSpinBox, QTextEdit {"
        "  border: 1px solid #CCC;"
        "  border-radius: 4px;"
        "  padding: 4px 8px;"
        "}"
        "QLineEdit:focus, QSpinBox:focus, QTextEdit:focus {"
        "  border-color: #3498DB;"
        "}"
    );

    MainWindow window;
    window.show();

    return app.exec();
}
