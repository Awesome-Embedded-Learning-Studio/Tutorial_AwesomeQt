#include <QApplication>

#include "modbuswindow.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    ModbusWindow window;
    window.show();

    return app.exec();
}
