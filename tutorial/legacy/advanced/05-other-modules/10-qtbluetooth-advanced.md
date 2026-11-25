---
title: "5.10 蓝牙进阶：GATT Profile 读写 Characteristic"
description: "入门篇我们把 QBluetoothDeviceDiscoveryAgent 的设备扫描跑通了——发现附近的蓝牙设备、显示名称和地址。做设备列表展示确实够用了。但物联网场景中最常见的蓝牙操作不是「扫描设备」，而是「连接 BLE 设备、读写它的 Characteristic」——比如从心率传感器读取心率数据、给智能灯泡发送颜色指令。"
---

# 现代Qt开发教程（进阶篇）5.10——蓝牙进阶：GATT Profile 读写 Characteristic

## 1. 前言 / 从「扫描」到「通信」

入门篇我们把 QBluetoothDeviceDiscoveryAgent 的设备扫描跑通了——发现附近的蓝牙设备、显示名称和地址。做设备列表展示确实够用了。但物联网场景中最常见的蓝牙操作不是「扫描设备」，而是「连接 BLE 设备、读写它的 Characteristic」——比如从心率传感器读取心率数据、给智能灯泡发送颜色指令。

BLE（Bluetooth Low Energy）的数据模型是 GATT（Generic Attribute Profile）。GATT 把设备上的数据组织成「服务 → 特征值」的层级结构。每个 Service 包含若干 Characteristic，每个 Characteristic 有一个 UUID、一组属性（可读/可写/可通知）和一个值。你的应用通过读写 Characteristic 来和设备交互。

这篇我们把 GATT Service 的发现、Characteristic 的读写和通知订阅这三个核心 BLE 操作拆干净。

## 2. 环境说明

本文档基于 Qt 6.2+ 编写，使用 C++17 标准和 CMake 3.26+ 构建系统。本篇依赖 Qt6::Bluetooth 模块，CMake 中通过 `find_package(Qt6 REQUIRED COMPONENTS Bluetooth)` 引入。Linux 上使用 BlueZ 协议栈（确保 `bluetoothd` 服务运行），Windows 使用 WinRT BLE API，macOS 使用 CoreBluetooth。Android 使用 platform BLE API。

## 3. 核心概念讲解

### 3.1 GATT 连接与服务发现

BLE 通信的第一步是连接设备并发现它的 GATT Service。Qt 提供了 `QLowEnergyController` 来管理 BLE 连接。

```cpp
class BleDevice : public QObject
{
    Q_OBJECT
public:
    BleDevice(const QBluetoothDeviceInfo &deviceInfo,
              QObject *parent = nullptr)
        : QObject(parent), deviceInfo_(deviceInfo)
    {
        controller_ = QLowEnergyController::createCentral(deviceInfo);
        controller_->setParent(this);

        connect(controller_, &QLowEnergyController::connected,
                this, &BleDevice::onConnected);
        connect(controller_, &QLowEnergyController::disconnected,
                this, &BleDevice::onDisconnected);
        connect(controller_, &QLowEnergyController::serviceDiscovered,
                this, &BleDevice::onServiceDiscovered);
        connect(controller_,
                &QLowEnergyController::discoveryFinished,
                this, &BleDevice::onDiscoveryFinished);
        connect(controller_,
                &QLowEnergyController::errorOccurred,
                this, &BleDevice::onError);
    }

    void connectToDevice()
    {
        controller_->connectToDevice();
    }

signals:
    void servicesReady();
    void characteristicChanged(const QString &serviceUuid,
                               const QString &charUuid,
                               const QByteArray &value);
    void connectionError(const QString &error);

private:
    void onConnected()
    {
        qDebug() << "BLE connected, discovering services...";
        controller_->discoverServices();
    }

    void onServiceDiscovered(const QBluetoothUuid &serviceUuid)
    {
        qDebug() << "Service found:" << serviceUuid.toString();
    }

    void onDiscoveryFinished()
    {
        qDebug() << "Service discovery complete.";
        emit servicesReady();
    }

    void onDisconnected()
    {
        qDebug() << "BLE disconnected.";
    }

    void onError(QLowEnergyController::Error error)
    {
        qDebug() << "BLE error:" << error
                 << controller_->errorString();
        emit connectionError(controller_->errorString());
    }

    QBluetoothDeviceInfo deviceInfo_;
    QLowEnergyController *controller_;
};
```

连接流程是：`connectToDevice()` → `connected` 信号 → `discoverServices()` → `serviceDiscovered` 信号（每个 Service 一次）→ `discoveryFinished` 信号。服务发现可能需要几秒钟——BLE 设备的 GATT 表可能包含十几个 Service。

### 3.2 读写 Characteristic

服务发现完成后，你可以为每个感兴趣的 Service 创建一个 `QLowEnergyService` 对象，然后读写它的 Characteristic。

```cpp
void readHeartRate(BleDevice *device)
{
    // 心率服务的标准 UUID
    const QString kHrServiceUuid =
        "{0000180d-0000-1000-8000-00805f9b34fb}";
    const QString kHrMeasurementUuid =
        "{00002a37-0000-1000-8000-00805f9b34fb}";

    QLowEnergyService *service =
        device->controller()->createServiceObject(
            QBluetoothUuid(kHrServiceUuid));
    if (!service) {
        qDebug() << "Heart rate service not found";
        return;
    }

    connect(service, &QLowEnergyService::stateChanged,
            [=](QLowEnergyService::ServiceState state) {
        if (state == QLowEnergyService::RemoteServiceDiscovered) {
            // 找到心率测量 Characteristic
            QLowEnergyCharacteristic hrChar =
                service->characteristic(
                    QBluetoothUuid(kHrMeasurementUuid));
            if (hrChar.isValid()) {
                // 订阅通知（心率传感器通过通知推送数据）
                QLowEnergyDescriptor notification =
                    hrChar.descriptor(
                        QBluetoothUuid::ClientCharacteristicConfiguration);
                if (notification.isValid()) {
                    service->writeDescriptor(notification,
                        QByteArray::fromHex("0100"));  // 启用通知
                }
            }
        }
    });

    connect(service, &QLowEnergyService::characteristicChanged,
            [](const QLowEnergyCharacteristic &characteristic,
               const QByteArray &newValue) {
        // 心率数据解析（第一个字节是 Flags，第二个字节是心率值）
        if (newValue.size() >= 2) {
            quint8 flags = newValue[0];
            quint8 hr = newValue[1];
            qDebug() << "Heart rate:" << hr << "bpm";
        }
    });

    service->discoverDetails();
}
```

BLE 的通知机制是设备主动推送数据的方式。你写 `0100` 到 Client Characteristic Configuration Descriptor（CCCD）来启用通知，之后设备每次有新数据就会推送过来，触发 `characteristicChanged` 信号。写 `0000` 可以关闭通知。

写 Characteristic 用于向设备发送控制命令：

```cpp
void writeColor(QLowEnergyService *service,
                 const QString &charUuid,
                 const QColor &color)
{
    QLowEnergyCharacteristic charObj =
        service->characteristic(QBluetoothUuid(charUuid));
    if (!charObj.isValid()) return;

    // 检查写权限
    if (!(charObj.properties() & QLowEnergyCharacteristic::Write)) {
        qDebug() << "Characteristic not writable";
        return;
    }

    QByteArray value;
    value.append(static_cast<char>(color.red()));
    value.append(static_cast<char>(color.green()));
    value.append(static_cast<char>(color.blue()));
    service->writeCharacteristic(charObj, value);
}
```

### 3.3 处理 BLE 连接的不稳定性

BLE 连接比 WiFi 和有线网络脆弱得多——设备可能会因为省电模式主动断开、距离远了信号变弱、操作系统的 BLE 管理器可能杀掉后台的 BLE 连接。你的代码必须能处理这些情况。

```cpp
// 自动重连
connect(controller_, &QLowEnergyController::errorOccurred,
        this, [this](QLowEnergyController::Error error) {
    if (error == QLowEnergyController::RemoteHostClosedError ||
        error == QLowEnergyController::NetworkError) {
        qDebug() << "BLE connection lost, reconnecting...";
        QTimer::singleShot(3000, this, [this]() {
            controller_->connectToDevice();
        });
    }
});
```

注意 BLE 重连不需要重新扫描设备——你已经有 `QBluetoothDeviceInfo` 了，直接调 `connectToDevice()` 即可。但如果设备换了广播地址（某些 BLE 设备的隐私功能会定期更换 MAC 地址），旧的 `QBluetoothDeviceInfo` 可能就无效了，需要重新扫描。

## 4. 踩坑预防

第一个坑是 Android 上的 BLE 权限。Android 12+ 需要在 AndroidManifest.xml 中声明 `BLUETOOTH_SCAN`、`BLUETOOTH_CONNECT` 和 `BLUETOOTH_ADVERTISE` 权限（旧的 `BLUETOOTH` 和 `BLUETOOTH_ADMIN` 不够用了）。缺少任何一个都会导致 `QLowEnergyController` 创建失败。

第二个坑是 Characteristic 的「写无响应」模式。某些 BLE 设备的 Characteristic 只支持 `WriteWithoutResponse`——你调用 `writeCharacteristic()` 后不会收到写确认，也不知道数据是否到达。如果你的命令需要确认，检查 Characteristic 的 properties 是否包含 `Write`（带响应）而不是只有 `WriteNoResponse`。

## 5. 练习项目

练习项目：BLE 环境传感器读取器。连接一个 BLE 温湿度传感器（如 nRF52840 开发板），读取温度和湿度 Characteristic 的值。支持通知模式实时更新。UI 显示温度、湿度和更新时间。断线后自动重连。

完成标准：能发现并连接 BLE 设备、正确读取温度和湿度值、通知模式下实时更新、断线自动重连后数据恢复。

## 6. 官方文档参考链接

[Qt 文档 · QLowEnergyController](https://doc.qt.io/qt-6/qlowenergycontroller.html) -- BLE 连接管理，包含连接、服务发现和错误处理

[Qt 文档 · QLowEnergyService](https://doc.qt.io/qt-6/qlowenergyservice.html) -- BLE 服务操作，包含 Characteristic 读写和通知订阅

[Qt 文档 · QLowEnergyCharacteristic](https://doc.qt.io/qt-6/qlowenergycharacteristic.html) -- Characteristic 数据模型，包含属性检查和值操作

*（链接已验证，2026-06-11 可访问）*

---

到这里就大功告成了。GATT 连接、Service 发现、Characteristic 读写和通知订阅——搞定了这些，你就能和任何 BLE 设备通信了。
