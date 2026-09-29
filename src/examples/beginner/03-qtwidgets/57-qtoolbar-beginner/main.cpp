// QtWidgets 入门示例 57: QToolBar 工具栏
// 演示：addAction / addWidget / addSeparator
//       setMovable / setFloatable / setAllowedAreas
//       setIconSize / setToolButtonStyle
//       溢出菜单 / toggleViewAction

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
