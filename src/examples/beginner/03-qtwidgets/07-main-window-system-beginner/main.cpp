// QtWidgets 入门示例 07: QMainWindow 主窗口体系基础
// 演示：QMenuBar / QMenu / QAction 菜单系统
//       QToolBar 工具栏按钮与分隔线
//       QStatusBar 状态栏（临时消息 + 永久组件）
//       QDockWidget 可停靠面板
//       简易文本编辑器

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
