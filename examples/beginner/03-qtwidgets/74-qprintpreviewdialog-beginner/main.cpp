// QtWidgets 入门示例 74: QPrintPreviewDialog 打印预览
// 演示：paintRequested 信号中执行绘制
//       翻页与缩放操作（内置）
//       自定义预览工具栏（添加页面设置快捷按钮）
//       QPageSetupDialog 页面参数配置

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
