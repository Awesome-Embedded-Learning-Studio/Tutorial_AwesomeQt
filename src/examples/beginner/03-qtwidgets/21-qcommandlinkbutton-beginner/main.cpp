// QtWidgets 入门示例 21: QCommandLinkButton 命令链接按钮
// 演示：setDescription() 设置副标题描述文字
//       向导对话框中的功能选项场景
//       图标按钮 + clicked 信号响应
//       QSS 统一跨平台外观

#include <QApplication>

#include "project_wizard.h"

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
        "}");

    ProjectWizard wizard;
    wizard.show();

    return app.exec();
}
