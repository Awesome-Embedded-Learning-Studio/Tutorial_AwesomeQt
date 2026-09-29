// QtWidgets 入门示例 73: QPrintDialog 打印对话框
// 演示：exec() 弹出系统打印对话框
//       获取用户配置（份数、打印范围、单双面）
//       与 QPainter 联动的完整打印流程
//       无打印机时的 PDF 回退方案

#include <QApplication>

#include "MainWindow.h"

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
