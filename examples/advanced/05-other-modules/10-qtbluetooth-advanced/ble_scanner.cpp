/// @file    ble_scanner.cpp
/// @brief   BLE 设备扫描器的实现。
///
/// 对应教程：进阶层 05-其他模块/10-QtBluetooth。
/// 演示 QBluetoothDeviceDiscoveryAgent 的信号槽用法和设备信息提取。

#include "ble_scanner.h"

#include <QBluetoothAddress>
#include <QBluetoothUuid>

#include <QDebug>

// ────────────────────────────────────────────────────────────
// 构造 / 析构
// ────────────────────────────────────────────────────────────

BleScanner::BleScanner(QObject* parent)
    : QObject(parent)
    , m_discoveryAgent(new QBluetoothDeviceDiscoveryAgent(this))
    , m_count(0)
{
    // 使用低功耗蓝牙方法扫描，过滤掉经典蓝牙设备
    m_discoveryAgent->setLowEnergyDiscoveryTimeout(3000);  // 3 秒 BLE 扫描窗口

    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &BleScanner::onDeviceDiscovered);

    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
            this, &BleScanner::onScanFinished);

    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred,
            this, [](QBluetoothDeviceDiscoveryAgent::Error error) {
                qWarning() << "[BleScanner] Discovery error:" << error;
            });
}

BleScanner::~BleScanner()
{
    // 确保扫描停止；代理由 Qt 对象树自动释放
    if (m_discoveryAgent->isActive()) {
        m_discoveryAgent->stop();
    }
}

// ────────────────────────────────────────────────────────────
// 公有方法
// ────────────────────────────────────────────────────────────

void BleScanner::startScan()
{
    if (m_discoveryAgent->isActive()) {
        m_discoveryAgent->stop();
    }

    m_devices.clear();
    m_count = 0;

    qDebug() << "[BleScanner] Starting BLE device discovery...";
    qDebug() << "[BleScanner] Scan timeout:"
             << m_discoveryAgent->lowEnergyDiscoveryTimeout() << "ms";

    m_discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

[[nodiscard]] int BleScanner::discoveredCount() const
{
    return m_count;
}

// ────────────────────────────────────────────────────────────
// 私有槽
// ────────────────────────────────────────────────────────────

void BleScanner::onDeviceDiscovered(const QBluetoothDeviceInfo& info)
{
    // 过滤：只保留低功耗蓝牙设备，忽略经典蓝牙
    if (info.coreConfigurations()
        != QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
        return;
    }

    // 按设备地址去重，防止同一设备被多次报告
    for (const auto& existing : m_devices) {
        if (existing.address() == info.address()) {
            return;
        }
    }

    m_devices.append(info);
    ++m_count;

    displayDeviceInfo(info);
}

void BleScanner::onScanFinished()
{
    qDebug() << "[BleScanner] Discovery finished. Total BLE devices found:"
             << m_count;
    emit scanFinished(m_count);
}

// ────────────────────────────────────────────────────────────
// 私有方法
// ────────────────────────────────────────────────────────────

void BleScanner::displayDeviceInfo(const QBluetoothDeviceInfo& info) const
{
    qDebug() << "---- BLE Device #" << m_count << "----";
    qDebug() << "  Name:    "
             << (info.name().isEmpty() ? QStringLiteral("(unnamed)") : info.name());
    qDebug() << "  Address: " << info.address().toString();

    // RSSI（Received Signal Strength Indicator）表示信号强度，单位 dBm
    // 值越接近 0 信号越强；典型范围 -30（极近）到 -100（极远）
    if (info.rssi() != 0) {
        qDebug() << "  RSSI:    " << info.rssi() << "dBm";
    }

    // 列出设备广播的服务 UUID（用于后续 GATT 服务匹配）
    const auto serviceUuids = info.serviceUuids();
    if (!serviceUuids.isEmpty()) {
        qDebug() << "  Service UUIDs (" << serviceUuids.size() << "):";
        for (const auto& uuid : serviceUuids) {
            qDebug() << "    -" << uuid.toString(QUuid::WithoutBraces);
        }
    }

    qDebug() << "";
}
