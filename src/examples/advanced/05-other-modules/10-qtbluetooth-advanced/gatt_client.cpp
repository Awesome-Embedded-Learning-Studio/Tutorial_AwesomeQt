/// @file    gatt_client.cpp
/// @brief   GATT 客户端演示类的实现。
///
/// 对应教程：进阶层 05-其他模块/10-QtBluetooth。
/// 以流程打印方式展示 QLowEnergyController 的完整 API 链路，
/// 无需真实 BLE 硬件即可理解 GATT 操作模式。

#include "gatt_client.h"

#include <QDebug>

// ────────────────────────────────────────────────────────────
// 一些常见的 BLE 标准 UUID，用于演示
// ────────────────────────────────────────────────────────────
namespace {
const QString kHeartRateServiceUuid =
    QStringLiteral("0000180d-0000-1000-8000-00805f9b34fb");
const QString kHeartRateMeasurementUuid =
    QStringLiteral("00002a37-0000-1000-8000-00805f9b34fb");
const QString kBodySensorLocationUuid =
    QStringLiteral("00002a38-0000-1000-8000-00805f9b34fb");
const QString kBatteryServiceUuid =
    QStringLiteral("0000180f-0000-1000-8000-00805f9b34fb");
const QString kBatteryLevelUuid =
    QStringLiteral("00002a19-0000-1000-8000-00805f9b34fb");
}  // namespace

// ────────────────────────────────────────────────────────────
// 构造 / 析构
// ────────────────────────────────────────────────────────────

GattClient::GattClient(QObject* parent)
    : QObject(parent)
{
}

GattClient::~GattClient() = default;

// ────────────────────────────────────────────────────────────
// 静态方法：打印 GATT 工作流
// ────────────────────────────────────────────────────────────

void GattClient::printGattWorkflow()
{
    qDebug() << "";
    qDebug() << "========================================";
    qDebug() << "  GATT Client Workflow Demo";
    qDebug() << "========================================";
    qDebug() << "";

    // ── Step 1: 创建控制器 ──
    qDebug() << "[Step 1] Create QLowEnergyController";
    qDebug() << "  Code:";
    qDebug() << "    auto* controller = QLowEnergyController::createCentral(device);";
    qDebug() << "";
    qDebug() << "  Note: device is a QBluetoothDeviceInfo obtained from BleScanner.";
    qDebug() << "  QLowEnergyController::createCentral() is the factory method for";
    qDebug() << "  central role (the one that connects to peripherals).";
    qDebug() << "";

    // ── Step 2: 连接信号 ──
    qDebug() << "[Step 2] Connect controller signals";
    qDebug() << "  Signals to connect:";
    qDebug() << "    - connected()              -> trigger discoverServices()";
    qDebug() << "    - discoveryFinished()      -> iterate discovered services";
    qDebug() << "    - errorOccurred()          -> handle connection failures";
    qDebug() << "    - stateChanged()           -> track connection state machine";
    qDebug() << "";

    // ── Step 3: 连接设备 ──
    qDebug() << "[Step 3] Connect to BLE device";
    qDebug() << "  Code:";
    qDebug() << "    controller->connectToDevice();";
    qDebug() << "";
    qDebug() << "  State machine: UnconnectedState -> ConnectingState ->";
    qDebug() << "                 ConnectedState -> DiscoveringState ->";
    qDebug() << "                 DiscoveredState";
    qDebug() << "";

    // ── Step 4: 发现服务 ──
    qDebug() << "[Step 4] Discover GATT services";
    qDebug() << "  Code:";
    qDebug() << "    // Automatic after connected(), or manual:";
    qDebug() << "    controller->discoverServices();";
    qDebug() << "";
    qDebug() << "    // When discoveryFinished() is emitted:";
    qDebug() << "    const QList<QBluetoothUuid> services = controller->services();";
    qDebug() << "    for (const auto& uuid : services) {";
    qDebug() << "        qDebug() << \"Service:\" << uuid.toString();";
    qDebug() << "    }";
    qDebug() << "";

    // ── Step 5: 发现服务详情 ──
    qDebug() << "[Step 5] Discover service details (characteristics & descriptors)";
    qDebug() << "  Code:";
    qDebug() << "    QLowEnergyService* service = controller->createServiceObject(";
    qDebug() << "        targetUuid);";
    qDebug() << "    service->discoverDetails();";
    qDebug() << "";
    qDebug() << "    // When state changes to RemoteServiceDiscovered:";
    qDebug() << "    const QList<QLowEnergyCharacteristic> chars =";
    qDebug() << "        service->characteristics();";
    qDebug() << "";
    qDebug() << "  Note: createServiceObject() returns nullptr if UUID not found.";
    qDebug() << "  Must check return value before calling discoverDetails().";
    qDebug() << "";

    // ── Step 6: 断开连接 ──
    qDebug() << "[Step 6] Disconnect and cleanup";
    qDebug() << "  Code:";
    qDebug() << "    controller->disconnectFromDevice();";
    qDebug() << "    delete service;     // created by createServiceObject()";
    qDebug() << "    delete controller;  // or use parent ownership";
    qDebug() << "";

    // ── 常见标准服务 UUID 参考 ──
    qDebug() << "=== Common Standard Service UUIDs ===";
    qDebug() << "  Heart Rate:      " << kHeartRateServiceUuid;
    qDebug() << "  Battery:         " << kBatteryServiceUuid;
    qDebug() << "========================================";
    qDebug() << "";
}

// ────────────────────────────────────────────────────────────
// 静态方法：打印特征值操作
// ────────────────────────────────────────────────────────────

void GattClient::printCharacteristicOps()
{
    qDebug() << "";
    qDebug() << "========================================";
    qDebug() << "  GATT Characteristic Operations Demo";
    qDebug() << "========================================";
    qDebug() << "";

    // ── Read 操作 ──
    qDebug() << "[Op 1] READ a characteristic";
    qDebug() << "  Example: read battery level";
    qDebug() << "";
    qDebug() << "  Code:";
    qDebug() << "    QLowEnergyService* service = controller->createServiceObject(";
    qDebug() << "        QBluetoothUuid(\"" << kBatteryServiceUuid << "\"));";
    qDebug() << "    service->discoverDetails();";
    qDebug() << "";
    qDebug() << "    // After discovery completes:";
    qDebug() << "    const QLowEnergyCharacteristic batteryChar =";
    qDebug() << "        service->characteristic(";
    qDebug() << "            QBluetoothUuid(\"" << kBatteryLevelUuid << "\"));";
    qDebug() << "    if (batteryChar.isValid()) {";
    qDebug() << "        service->readCharacteristic(batteryChar);";
    qDebug() << "    }";
    qDebug() << "";
    qDebug() << "    // Handle result in characteristicRead() signal:";
    qDebug() << "    connect(service, &QLowEnergyService::characteristicRead,";
    qDebug() << "            [](const QLowEnergyCharacteristic& c, const QByteArray& v) {";
    qDebug() << "        qDebug() << \"Value:\" << v.toHex() << \"(\" << v.size() << \"bytes)\";";
    qDebug() << "    });";
    qDebug() << "";

    // ── Write 操作 ──
    qDebug() << "[Op 2] WRITE to a characteristic";
    qDebug() << "  Example: write body sensor location";
    qDebug() << "";
    qDebug() << "  Code:";
    qDebug() << "    const QLowEnergyCharacteristic locChar =";
    qDebug() << "        service->characteristic(";
    qDebug() << "            QBluetoothUuid(\"" << kBodySensorLocationUuid << "\"));";
    qDebug() << "    if (locChar.properties() & QLowEnergyCharacteristic::Write) {";
    qDebug() << "        QByteArray value;";
    qDebug() << "        value.append(static_cast<char>(0x01));  // Chest location";
    qDebug() << "        service->writeCharacteristic(locChar, value);";
    qDebug() << "    }";
    qDebug() << "";
    qDebug() << "  Note: Check characteristic.properties() for Write/WriteNoResponse/";
    qDebug() << "        WriteSigned flags before attempting write.";
    qDebug() << "";
    qDebug() << "  Write modes (QLowEnergyService::WriteMode):";
    qDebug() << "    - WriteWithResponse  (default): waits for ACK from peripheral";
    qDebug() << "    - WriteWithoutResponse: fire-and-forget, no confirmation";
    qDebug() << "";

    // ── Notify / Indicate 操作 ──
    qDebug() << "[Op 3] SUBSCRIBE to notifications (Heart Rate Measurement)";
    qDebug() << "  Example: enable heart rate notifications";
    qDebug() << "";
    qDebug() << "  Code:";
    qDebug() << "    const QLowEnergyCharacteristic hrChar =";
    qDebug() << "        service->characteristic(";
    qDebug() << "            QBluetoothUuid(\"" << kHeartRateMeasurementUuid << "\"));";
    qDebug() << "    if (hrChar.properties() & QLowEnergyCharacteristic::Notify) {";
    qDebug() << "        // Enable notification via CCC descriptor (0x2902)";
    qDebug() << "        const QLowEnergyDescriptor ccc = hrChar.descriptor(";
    qDebug() << "            QBluetoothUuid::ClientCharacteristicConfiguration);";
    qDebug() << "        service->writeDescriptor(ccc, QByteArray::fromHex(\"0100\"));";
    qDebug() << "    }";
    qDebug() << "";
    qDebug() << "    // Receive notifications via characteristicChanged() signal:";
    qDebug() << "    connect(service, &QLowEnergyService::characteristicChanged,";
    qDebug() << "            [](const QLowEnergyCharacteristic& c, const QByteArray& v) {";
    qDebug() << "        // Heart Rate Measurement format:";
    qDebug() << "        // Byte 0: Flags (bit 0 = HR format: 0=UINT8, 1=UINT16)";
    qDebug() << "        // Byte 1 (or 1-2): Heart Rate value";
    qDebug() << "        const bool isUint16 = (v[0] & 0x01) != 0;";
    qDebug() << "        int hr = isUint16";
    qDebug() << "            ? (static_cast<int>(v[2]) << 8) | static_cast<int>(v[1])";
    qDebug() << "            : static_cast<int>(v[1]);";
    qDebug() << "        qDebug() << \"Heart Rate:\" << hr << \"bpm\";";
    qDebug() << "    });";
    qDebug() << "";
    qDebug() << "  Note: CCC descriptor value 0x0101 = Indicate, 0x0100 = Notify.";
    qDebug() << "  To disable: write 0x0000 back to the CCC descriptor.";
    qDebug() << "";

    // ── 状态机总结 ──
    qDebug() << "=== QLowEnergyService State Machine ===";
    qDebug() << "  Invalid                  -> service not yet created";
    qDebug() << "  RemoteService            -> created, awaiting discovery";
    qDebug() << "  RemoteServiceDiscovering -> details in progress";
    qDebug() << "  RemoteServiceDiscovered  -> ready for read/write/notify";
    qDebug() << "  LocalService             -> for peripheral role only";
    qDebug() << "========================================";
    qDebug() << "";
}
