#include <QApplication>

#include "mqttwindow.h"

// ============================================================================
// 主函数
// ============================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MqttWindow window;
    window.show();

    return app.exec();
}
