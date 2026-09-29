#include <QApplication>

#include "printexamplewindow.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    PrintExampleWindow window;
    window.show();

    return app.exec();
}
