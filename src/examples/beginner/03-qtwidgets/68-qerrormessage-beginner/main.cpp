// QtWidgets 入门示例 68: QErrorMessage 错误消息对话框
// 演示：showMessage 显示错误消息
//       "不再显示"复选框的记忆机制
//       与 qInstallMessageHandler 结合
//       可抑制的非致命错误通知

#include <QApplication>

#include "mainwindow.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
