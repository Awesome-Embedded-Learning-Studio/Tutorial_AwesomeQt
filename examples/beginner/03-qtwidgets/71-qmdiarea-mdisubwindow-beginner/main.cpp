// QtWidgets 入门示例 71: QMdiArea / QMdiSubWindow 多文档界面
// 演示：addSubWindow 添加子窗口
//       tileSubWindows / cascadeSubWindows 排列
//       subWindowActivated 信号追踪活动窗口
//       子窗口菜单项自动更新

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
