// QtWidgets 入门示例 58: QStatusBar 状态栏
// 演示：showMessage 临时消息
//       addWidget / addPermanentWidget 嵌入控件
//       QProgressBar 嵌入状态栏
//       clearMessage 与消息优先级

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
