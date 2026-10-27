// QtWidgets 入门示例 02: 事件处理与传播基础
// 演示：mousePressEvent/mouseMoveEvent/keyPressEvent/resizeEvent 重写
//       accept/ignore 事件传播、installEventFilter 事件过滤器拦截
//       sendEvent vs postEvent 的区别

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
