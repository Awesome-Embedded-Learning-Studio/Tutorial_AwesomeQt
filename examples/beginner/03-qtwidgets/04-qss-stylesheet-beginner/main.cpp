// QtWidgets 入门示例 04: QSS 样式表基础
// 演示：类选择器 / ID 选择器 / 后代选择器 / 伪状态(:hover :pressed :checked :disabled)
//       外部 QSS 文件加载 / 浅色-深色主题动态切换

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
