// QtWidgets 入门示例 22: QLineEdit 单行文本输入
// 演示：setPlaceholderText / setMaxLength / setReadOnly
//       setEchoMode（密码框、无回显）
//       setValidator + QIntValidator / QRegularExpressionValidator
//       textChanged vs textEdited vs editingFinished 信号区别

#include <QApplication>

#include "register_form.h"

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
        "QLineEdit {"
        "  padding: 6px 10px;"
        "  border: 1px solid #CCC;"
        "  border-radius: 4px;"
        "  font-size: 13px;"
        "}"
        "QLineEdit:focus {"
        "  border-color: #1976D2;"
        "}");

    RegisterForm form;
    form.show();

    return app.exec();
}
