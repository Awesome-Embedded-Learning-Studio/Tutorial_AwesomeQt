// QtWidgets 入门示例 67: QProgressDialog 进度对话框
// 演示：setLabelText / setValue / setRange 基础配置
//       canceled 信号响应用户取消
//       setAutoClose / setAutoReset 行为
//       与 QThread 配合的后台长任务进度汇报

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
