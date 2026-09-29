/// @file    ble_scanner.h
/// @brief   BLE 设备扫描器，演示 QBluetoothDeviceDiscoveryAgent 的进阶用法。
///
/// 对应教程：进阶层 05-其他模块/10-QtBluetooth。
/// 展示 BLE 设备发现、信号强度读取、服务 UUID 枚举等 API。

#pragma once

#include <QObject>

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>

#include <QList>
#include <QString>

/// @brief BLE 设备扫描器，封装 QBluetoothDeviceDiscoveryAgent。
///
/// 提供 startScan() 启动低功耗蓝牙设备扫描，
/// 自动统计发现设备数量并通过信号通知扫描完成。
class BleScanner : public QObject
{
    Q_OBJECT

public:
    /// @brief 构造函数，初始化发现代理。
    /// @param[in] parent 父对象指针，Qt 对象树自动管理生命周期。
    explicit BleScanner(QObject* parent = nullptr);

    /// @brief 析构函数，确保扫描停止后释放资源。
    ~BleScanner() override;

    /// @brief 启动 BLE 设备扫描。
    /// @note 调用前会先停止上一次扫描，避免重复触发。
    void startScan();

    /// @brief 获取已发现的设备数量。
    /// @return 当前已发现的 BLE 设备数。
    [[nodiscard]] int discoveredCount() const;

signals:
    /// @brief 扫描完成时发射。
    /// @param[in] count 本次扫描发现的设备总数。
    void scanFinished(int count);

private slots:
    /// @brief 设备发现回调，每发现一个设备触发一次。
    /// @param[in] info 新发现的蓝牙设备信息。
    /// @note Qt 在不同平台可能对同一设备多次触发，内部按地址去重。
    void onDeviceDiscovered(const QBluetoothDeviceInfo& info);

    /// @brief 扫描结束回调。
    void onScanFinished();

private:
    /// @brief 打印单个设备的详细信息到标准输出。
    /// @param[in] info 设备信息引用。
    void displayDeviceInfo(const QBluetoothDeviceInfo& info) const;

    QBluetoothDeviceDiscoveryAgent* m_discoveryAgent;  ///< 设备发现代理
    QList<QBluetoothDeviceInfo> m_devices;              ///< 已发现的设备列表
    int m_count;                                        ///< 已发现设备计数
};
