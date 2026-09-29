// QtWidgets 入门示例 72: QPrinter 打印机抽象类
// 演示：页面配置 (setPageSize/setOrientation/setPageMargins)
//       QPainter + QPrinter 自定义打印内容
//       QPrintPreviewDialog 打印预览
//       导出 PDF (QPrinter::PdfFormat)

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
