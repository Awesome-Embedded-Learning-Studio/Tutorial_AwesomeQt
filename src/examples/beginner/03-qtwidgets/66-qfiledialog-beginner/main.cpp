// QtWidgets 入门示例 66: QFileDialog 文件选择对话框
// 演示：getOpenFileName 打开单文件
//       getSaveFileName 保存文件
//       getOpenFileNames 批量选择
//       getExistingDirectory 选择目录
//       setNameFilter 文件类型过滤
//       QStandardPaths 默认目录

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
