#include "serialportlist.h"

#include <QDebug>
#include <QSerialPortInfo>

void listAvailablePorts()
{
    qDebug() << "\n=== Demo 1: Available Serial Ports ===";

    const QList<QSerialPortInfo> ports =
        QSerialPortInfo::availablePorts();

    if (ports.isEmpty()) {
        qDebug() << "  No serial ports found on this system.";
        qDebug() << "  - Linux: check /dev/ttyUSB* or /dev/ttyACM*";
        qDebug() << "  - Windows: check Device Manager -> Ports";
        qDebug() << "  - Ensure user is in 'dialout' group (Linux)";
        return;
    }

    qDebug() << "  Found" << ports.size() << "serial port(s):\n";

    for (const QSerialPortInfo &info : ports) {
        qDebug() << "  Port:" << info.portName();
        qDebug() << "    System location:" << info.systemLocation();
        qDebug() << "    Description:" << info.description();
        qDebug() << "    Manufacturer:" << info.manufacturer();

        if (info.hasVendorIdentifier()) {
            qDebug() << "    Vendor ID: 0x"
                     << Qt::hex << info.vendorIdentifier() << Qt::dec;
        }
        if (info.hasProductIdentifier()) {
            qDebug() << "    Product ID: 0x"
                     << Qt::hex << info.productIdentifier() << Qt::dec;
        }

        // 获取该端口支持的标准波特率
        QList<qint32> baudRates = info.standardBaudRates();
        if (!baudRates.isEmpty()) {
            qDebug() << "    Standard baud rates:"
                     << baudRates.size() << "supported"
                     << "(e.g." << baudRates.first() << "-"
                     << baudRates.last() << ")";
        }

        qDebug() << "";
    }
}
