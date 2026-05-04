// QtWidgets 入门示例 65: QFontDialog 字体选择对话框
// 演示：getFont 模态选择
//       setCurrentFont 初始预选
//       currentFontChanged 实时预览
//       QFontDatabase 过滤等宽字体

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
