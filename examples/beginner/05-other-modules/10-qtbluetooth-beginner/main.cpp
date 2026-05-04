#include <QApplication>

#include "bluetoothwindow.h"

// ========================================
// 主函数
// ========================================

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qDebug() << "Qt Bluetooth 设备扫描工具";
    qDebug() << "本示例演示设备扫描和 BLE GATT 服务发现";

    BluetoothWindow window;
    window.show();

    return app.exec();
}
