/// @file    gatt_client.h
/// @brief   GATT 客户端演示类，展示 QLowEnergyController 完整操作流程。
///
/// 对应教程：进阶层 05-其他模块/10-QtBluetooth。
/// 由于无需真实 BLE 硬件，本类以 API 演示和流程打印为主，
/// 帮助理解 GATT 服务发现、特征值读写/通知的编程模式。

#pragma once

#include <QObject>

#include <QLowEnergyController>
#include <QLowEnergyService>

#include <QBluetoothDeviceInfo>
#include <QBluetoothUuid>

#include <QByteArray>
#include <QString>

/// @brief GATT 客户端演示类，展示完整的 BLE GATT 操作链。
///
/// 本类不连接真实设备，而是通过 printGattWorkflow() 展示
/// QLowEnergyController 的标准使用流程。同时提供
/// printCharacteristicOps() 展示特征值读写和通知订阅的 API 用法。
class GattClient : public QObject
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父对象指针。
    explicit GattClient(QObject* parent = nullptr);

    /// @brief 析构函数。
    ~GattClient() override;

    /// @brief 打印完整的 GATT 客户端工作流到标准输出。
    ///
    /// 依次展示：创建控制器 → 连接设备 → 发现服务 →
    /// 发现特征值 → 读写/订阅 → 断开连接的完整链路。
    static void printGattWorkflow();

    /// @brief 打印 GATT 特征值操作的 API 示例。
    ///
    /// 展示 Read/Write/Notify 三种特征值操作的代码模式，
    /// 以及 QLowEnergyService::ServiceState 状态机的转换。
    static void printCharacteristicOps();
};
