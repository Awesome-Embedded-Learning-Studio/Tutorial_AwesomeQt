// QtWidgets 入门示例 63: QInputDialog 输入对话框
// 演示：getText / getInt / getDouble / getItem 四种静态方法
//       自定义 QDialog 多字段输入
//       重写 accept() 做校验，阻止无效提交

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
