/// @file    main.cpp
/// @brief   Qt Bluetooth 进阶示例的入口程序。
///
/// 对应教程：进阶层 05-其他模块/10-QtBluetooth。
/// 演示三大功能：
///   1. BLE 设备扫描（BleScanner）
///   2. GATT 客户端工作流打印（GattClient::printGattWorkflow）
///   3. 特征值操作示例打印（GattClient::printCharacteristicOps）

#include "ble_scanner.h"
#include "gatt_client.h"

#include <QCoreApplication>
#include <QTimer>

#include <QDebug>

/// @brief 程序入口，运行 BLE 扫描和 GATT 工作流演示。
/// @param[in] argc 命令行参数计数。
/// @param[in] argv 命令行参数数组。
/// @return 应用程序退出码。
int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("10-qtbluetooth-advanced"));

    qDebug() << "========================================";
    qDebug() << "  Qt Bluetooth Advanced Demo";
    qDebug() << "========================================";
    qDebug() << "";

    // ── Demo 1: GATT 客户端工作流（无需硬件） ──
    qDebug() << "[Demo 1] GATT Client Workflow";
    GattClient::printGattWorkflow();

    // ── Demo 2: 特征值操作示例（无需硬件） ──
    qDebug() << "[Demo 2] Characteristic Operations";
    GattClient::printCharacteristicOps();

    // ── Demo 3: BLE 设备扫描（可能需要蓝牙硬件） ──
    qDebug() << "[Demo 3] BLE Device Discovery";
    qDebug() << "Note: This requires a Bluetooth adapter.";
    qDebug() << "      In WSL2 or VM environments, no devices may be found.";
    qDebug() << "";

    auto* scanner = new BleScanner(&app);  // 由 app 对象树管理生命周期

    // 扫描完成后，打印总结并安排退出
    QObject::connect(scanner, &BleScanner::scanFinished, &app,
                     [](int count) {
                         qDebug() << "";
                         qDebug() << "Scan complete." << count
                                  << "BLE device(s) discovered.";
                     });

    // 安全超时：5 秒后无论扫描状态如何，强制退出应用
    // @note 在 WSL2/无蓝牙适配器环境中，discoveryAgent 可能不会
    //       正常 finished，因此必须设置超时兜底
    QTimer::singleShot(5000, &app, []() {
        qDebug() << "";
        qDebug() << "Timeout reached (5s). Exiting demo.";
        QCoreApplication::quit();
    });

    // 启动扫描
    scanner->startScan();

    return app.exec();
}
