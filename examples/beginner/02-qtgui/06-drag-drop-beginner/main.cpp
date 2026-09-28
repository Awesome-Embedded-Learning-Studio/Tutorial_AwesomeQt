// QtGui 入门示例 06: 拖放系统基础
// 演示：拖放源发起拖拽、拖放目标接收数据、QMimeData 文本携带、文件拖入接收

#include <QApplication>

#include "widget.h"

// ============================================================================
// 主函数：创建主窗口并进入事件循环
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Widget window;
    window.show();

    return app.exec();
}
