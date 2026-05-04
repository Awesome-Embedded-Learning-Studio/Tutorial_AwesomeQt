# 现代Qt开发教程（新手篇）5.10——QtBluetooth 蓝牙基础

## 1. 前言：蓝牙开发的现实骨感

蓝牙技术在消费电子领域已经无处不在——无线耳机、智能手环、车载系统、IoT 传感器，几乎所有带"无线"字样的外设都走蓝牙。听起来很美好，但实际做蓝牙开发的时候你会发现一个尴尬的现实：经典蓝牙和低功耗蓝牙（BLE）虽然都叫蓝牙，但协议栈、API 设计、使用姿势完全不同，基本上是两套独立的系统。

Qt 的 QtBluetooth 模块同时封装了经典蓝牙和 BLE 的操作接口。经典蓝牙方面，Qt 提供了设备发现、服务浏览、RFCOMM 串口通信等完整的 SPP（Serial Port Profile）支持；BLE 方面，Qt 封装了 GATT（Generic Attribute Profile）的完整操作链——设备扫描、服务发现、特征值读写和通知订阅。这篇我们要做的是把这两条线路的核心流程全部走通，同时把蓝牙开发中最容易踩的权限配置坑讲清楚。

说实话，蓝牙开发最大的痛苦不是 API 本身——Qt 的封装做得相当好——而是平台权限和硬件适配。Android 上需要运行时权限申请，Linux 上 BlueZ 的版本和权限配置经常出幺蛾子，macOS/iOS 有自己的沙盒限制。这些环境问题处理不好，代码写得再漂亮也跑不起来。所以这篇我们不光讲 API，也会把各平台的权限配置和常见报错逐个拆解。

## 2. 环境说明

本篇基于 Qt 6.9.1，需要引入 QtBluetooth 模块。CMake 配置如下：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core Bluetooth Widgets)
```

硬件方面，你的开发机必须有蓝牙适配器。大部分笔记本都自带蓝牙芯片；台式机可能需要外接 USB 蓝牙适配器。Linux 上可以用 `hciconfig` 或 `bluetoothctl` 确认蓝牙适配器是否被系统识别。Android 设备通常自带蓝牙，但 Android 6.0 以上需要在 AndroidManifest.xml 中声明权限并且在运行时动态申请。

平台差异方面：Linux 依赖 BlueZ 5.x 蓝牙协议栈（大部分现代发行版默认安装），Windows 10/11 使用系统自带蓝牙栈，macOS/iOS 使用 CoreBluetooth 框架，Android 使用系统蓝牙 API。编译工具链方面，MSVC 2019+、GCC 11+、Clang 14+ 均可，C++17 标准，CMake 3.26+ 构建系统。

如果你在 WSL2 环境下开发，蓝牙适配器的直通配置比较麻烦，建议直接在物理机或虚拟机中测试蓝牙功能。

## 3. 核心概念讲解

### 3.1 QBluetoothDeviceDiscoveryAgent——扫描蓝牙设备

所有蓝牙操作的第一步都是发现周围的设备。Qt 提供了 QBluetoothDeviceDiscoveryAgent 来完成这个任务，它封装了平台蓝牙栈的扫描逻辑，通过信号通知发现的设备。

```cpp
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>

auto *discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);

connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
        this, [](const QBluetoothDeviceInfo &info) {
    qDebug() << "发现设备:" << info.name()
             << "地址:" << info.address().toString()
             << "RSSI:" << info.rssi();
});

connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
        this, []() {
    qDebug() << "扫描结束";
});

discoveryAgent->start();
```

deviceDiscovered 信号每发现一个设备就触发一次，携带的 QBluetoothDeviceInfo 包含设备名称、蓝牙地址、信号强度（RSSI）、设备类型等信息。finished 信号在扫描完成时触发。QBluetoothDeviceInfo 还有一个 coreConfigurations() 方法可以判断设备支持经典蓝牙还是 BLE——返回值包含 QBluetoothDeviceInfo::LowEnergyCoreConfiguration 表示支持 BLE。

扫描方法分两种：start() 默认扫描所有类型的蓝牙设备（经典蓝牙 + BLE），start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod) 只扫描 BLE 设备。BLE 扫描通常比经典蓝牙扫描快，因为 BLE 的广播间隔短、协议开销小。

一个需要注意的地方：扫描过程中 deviceDiscovered 可能对同一个设备触发多次，因为设备的信号强度在变化。如果你需要去重，用设备的蓝牙地址（QBluetoothAddress）作为唯一键。

### 3.2 QBluetoothSocket——经典蓝牙 SPP 串口通信

经典蓝牙在嵌入式和工业场景中最常用的协议是 SPP（Serial Port Profile），它把蓝牙连接虚拟成串口，双方通过 RFCOMM 协议收发数据。Qt 的 QBluetoothSocket 封装了 RFCOMM 连接的完整生命周期。

使用 QBluetoothSocket 之前，你需要知道目标设备的蓝牙地址和它提供的 SPP 服务。服务信息可以通过 QBluetoothServiceDiscoveryAgent 获取，它会扫描目标设备上所有可用的蓝牙服务：

```cpp
#include <QBluetoothServiceDiscoveryAgent>
#include <QBluetoothSocket>

auto *serviceAgent = new QBluetoothServiceDiscoveryAgent(targetDevice, this);

connect(serviceAgent, &QBluetoothServiceDiscoveryAgent::serviceDiscovered,
        this, [](const QBluetoothServiceInfo &service) {
    qDebug() << "发现服务:" << service.serviceName()
             << "描述:" << service.serviceDescription();
});

serviceAgent->start();
```

找到 SPP 服务后，就可以建立连接了。QBluetoothSocket 的使用方式和 QTcpSocket 非常类似——调用 connectToService() 发起连接，connected 信号表示连接成功，readyRead 信号表示有数据可读，write() 发送数据：

```cpp
auto *socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);

connect(socket, &QBluetoothSocket::connected, this, [socket]() {
    qDebug() << "蓝牙连接成功";
    socket->write("Hello from Qt!\r\n");
});

connect(socket, &QBluetoothSocket::readyRead, this, [socket]() {
    QByteArray data = socket->readAll();
    qDebug() << "收到数据:" << data;
});

connect(socket, &QBluetoothSocket::errorOccurred, this,
        [](QBluetoothSocket::SocketError error) {
    qWarning() << "蓝牙连接错误:" << error;
});

// 连接到目标设备的 SPP 服务
socket->connectToService(targetAddress, targetPort);
```

你会发现，这段代码和 QTcpSocket 的用法几乎一模一样——这是因为 Qt 故意把蓝牙 socket 和 TCP socket 的 API 设计成一致的，降低学习成本。QBluetoothSocket 同样是异步的，基于事件循环工作，不会阻塞主线程。

断开连接用 disconnectFromService()，错误处理通过 errorOccurred 信号。一个常见的坑是连接超时——蓝牙配对过程中如果设备不在范围内或者没有响应，connectToService() 不会自动超时，你可能需要自己用 QTimer 设一个超时检测。

### 3.3 QLowEnergyController——BLE 连接流程

BLE 的架构和经典蓝牙完全不同。BLE 设备通过 GATT（Generic Attribute Profile）组织数据，GATT 的结构是"服务-特征值"的树形层次——一个 BLE 设备包含多个 Service，每个 Service 包含多个 Characteristic，每个 Characteristic 有一个值（value）和一组属性（read/write/notify）。

Qt 使用 QLowEnergyController 管理 BLE 连接。连接流程分四步：创建控制器、建立连接、发现服务、读写特征值。

```cpp
#include <QLowEnergyController>
#include <QLowEnergyService>

// 第一步：创建控制器，指定目标 BLE 设备
auto *controller = QLowEnergyController::createCentral(deviceInfo, this);

connect(controller, &QLowEnergyController::connected, this,
        [controller]() {
    qDebug() << "BLE 连接成功，开始发现服务";
    controller->discoverServices();
});

connect(controller, &QLowEnergyController::serviceDiscovered, this,
        [](const QBluetoothUuid &uuid) {
    qDebug() << "发现服务:" << uuid.toString();
});

connect(controller, &QLowEnergyController::discoveryFinished, this,
        [controller]() {
    qDebug() << "服务发现完成";
    // 在这里创建 service 对象，读写特征值
});

connect(controller, &QLowEnergyController::errorOccurred, this,
        [](QLowEnergyController::Error error) {
    qWarning() << "BLE 错误:" << error;
});

// 第二步：建立连接
controller->connectToDevice();
```

连接成功后调用 discoverServices() 开始 GATT 服务发现。服务发现完成后，你可以用 controller->createServiceObject() 获取特定的服务对象，然后通过 QLowEnergyService 读写特征值：

```cpp
// 获取特定服务
auto *service = controller->createServiceObject(targetServiceUuid, this);

connect(service, &QLowEnergyService::stateChanged, this,
        [service](QLowEnergyService::ServiceState state) {
    if (state == QLowEnergyService::RemoteServiceDiscovered) {
        qDebug() << "服务详情发现完成";

        // 读取特征值
        QLowEnergyCharacteristic chars =
            service->characteristic(targetCharUuid);
        if (chars.isValid()) {
            service->readCharacteristic(chars);
        }
    }
});

// 监听特征值变化（通知模式）
connect(service, &QLowEnergyService::characteristicChanged, this,
        [](const QLowEnergyCharacteristic &chars,
           const QByteArray &newValue) {
    qDebug() << "特征值变化:" << chars.uuid()
             << "新值:" << newValue.toHex();
});

// 必须先调用 discoverDetails 才能读写特征值
service->discoverDetails();
```

BLE 的读写操作有两个需要特别注意的点。第一，所有操作都是异步的——readCharacteristic() 发出读取请求后，结果通过 characteristicRead 信号返回；writeCharacteristic() 发出写入请求后，结果通过 characteristicWritten 信号确认。第二，BLE 的 notify 机制（特征值变化时主动推送）需要先调用 service->writeDescriptor() 写入 Client Characteristic Configuration Descriptor（CCCD）来启用通知：

```cpp
QLowEnergyCharacteristic chars = service->characteristic(targetCharUuid);

// 启用 notify 通知
QLowEnergyDescriptor cccd = chars.descriptor(
    QBluetoothUuid::ClientCharacteristicConfiguration);

if (cccd.isValid()) {
    service->writeDescriptor(cccd,
        QByteArray::fromHex("0100"));  // 0x0100 = enable notification
}
```

这段代码是 BLE 开发中最容易遗漏的一步——不写 CCCD 就不会收到 notify 回调，很多人的 BLE 代码"读可以但收不到推送"就是这个原因。

### 3.4 蓝牙权限配置——最容易卡住的一环

蓝牙 API 写对了，权限没配好，程序照样跑不起来。各平台的权限配置差异很大，我们逐个过一下。

Linux 桌面环境需要 BlueZ 5.x 协议栈和 D-Bus 权限。如果你的程序以普通用户运行，通常需要将用户加入 `bluetooth` 用户组，或者配置 D-Bus 的策略允许你的应用访问 BlueZ 的接口。你可以通过 `bluetoothctl` 命令行工具先确认系统蓝牙是否正常工作——如果 bluetoothctl 能扫描到设备，说明 BlueZ 没问题，Qt 程序也应该能正常调用。

Android 平台需要在 AndroidManifest.xml 中声明蓝牙权限，而且在 Android 6.0（API 23）以上版本还需要在运行时动态申请位置权限（是的，蓝牙扫描被认为涉及位置隐私）。Qt 6 中可以通过 QtAndroid::requestPermissions() 或 QPermission 相关 API 处理。需要的权限声明包括 BLUETOOTH、BLUETOOTH_ADMIN、ACCESS_FINE_LOCATION（经典蓝牙扫描需要），Android 12+ 还需要 BLUETOOTH_SCAN、BLUETOOTH_CONNECT、BLUETOOTH_ADVERTISE。

macOS 和 iOS 使用 CoreBluetooth 框架，需要在 Info.plist 中添加 NSBluetoothAlwaysUsageDescription 键，描述为什么你的应用需要蓝牙。iOS 还要求应用必须在 foreground 状态才能进行蓝牙扫描。

## 4. 综合示例：蓝牙设备扫描与通信工具

把前面学的串起来，我们写一个蓝牙设备扫描工具。程序启动后扫描周围的蓝牙设备，显示设备名称、地址、类型和信号强度。对于经典蓝牙设备，展示其基本信息；对于 BLE 设备，额外显示其 GATT 服务列表。完整代码见 `examples/beginner/05-other-modules/10-qtbluetooth-beginner/`，下面是关键部分的讲解。

CMake 配置：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core Bluetooth Widgets)
# ...
target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Bluetooth Qt6::Widgets)
```

程序界面分为上下两区：上方是设备扫描列表，显示所有发现的蓝牙设备；下方是选中设备的详情——如果是 BLE 设备，会展示其 GATT 服务列表。

设备扫描的核心代码：

```cpp
void startScan()
{
    discovery_agent_ = new QBluetoothDeviceDiscoveryAgent(this);

    connect(discovery_agent_,
            &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &BluetoothWindow::onDeviceDiscovered);

    connect(discovery_agent_,
            &QBluetoothDeviceDiscoveryAgent::finished,
            this, [this]() {
        status_label_->setText("扫描完成，共发现 "
                              + QString::number(device_list_.size())
                              + " 个设备");
    });

    status_label_->setText("正在扫描...");
    discovery_agent_->start();
}

void onDeviceDiscovered(const QBluetoothDeviceInfo &info)
{
    // 用地址去重
    QString addr = info.address().toString();
    if (device_map_.contains(addr)) {
        return;
    }

    device_map_.insert(addr, info);

    QString type = "未知";
    if (info.coreConfigurations()
        & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
        type = "BLE";
    } else {
        type = "经典蓝牙";
    }

    QString entry = info.name()
                  + " [" + addr + "]"
                  + " " + type
                  + " RSSI:" + QString::number(info.rssi());

    device_list_->addItem(entry);
    device_list_.push_back(info);
}
```

选中 BLE 设备后连接并发现服务：

```cpp
void connectBleDevice(const QBluetoothDeviceInfo &device)
{
    auto *controller = QLowEnergyController::createCentral(device, this);

    connect(controller, &QLowEnergyController::connected,
            this, [controller]() {
        controller->discoverServices();
    });

    connect(controller, &QLowEnergyController::serviceDiscovered,
            this, [this](const QBluetoothUuid &uuid) {
        detail_text_->append("  服务: " + uuid.toString());
    });

    connect(controller, &QLowEnergyController::discoveryFinished,
            this, [this]() {
        detail_text_->append("服务发现完成");
    });

    connect(controller, &QLowEnergyController::errorOccurred,
            this, [this](QLowEnergyController::Error error) {
        detail_text_->append("连接错误: "
                           + QString::number(static_cast<int>(error)));
    });

    detail_text_->clear();
    detail_text_->append("正在连接: " + device.name() + "...");
    controller->connectToDevice();
}
```

运行程序后点击"开始扫描"按钮，附近所有可发现的蓝牙设备会逐个出现在列表中。选中一个 BLE 设备后点击"连接"，程序会建立 BLE 连接并列出该设备的所有 GATT 服务。你会发现 BLE 设备的服务数量通常比预期的多——很多 BLE 设备除了业务服务外还包含 Battery Service、Device Information Service 等标准服务。

## 5. 练习项目

练习项目：BLE 心率监测数据接收器。

大部分 BLE 心率传感器都实现了标准的 Heart Rate Service（UUID: 0x180D），其中包含 Heart Rate Measurement 特征值（UUID: 0x2A37），支持 notify 模式。我们要做的就是扫描并连接一个 BLE 心率传感器（如果没有真实设备，可以用手机上的 BLE 模拟器 App 替代），订阅心率特征值的通知，实时显示心率数据。

完成标准是这样的：程序扫描并过滤出支持 Heart Rate Service 的 BLE 设备；连接设备后自动找到 Heart Rate Measurement 特征值并启用 notify；心率数据实时显示在界面上；连接断开后自动重连。

几个实现提示：扫描时通过 QBluetoothDeviceInfo::serviceUuids() 检查设备是否包含 0x180D 这个 UUID；连接后用 createServiceObject(QBluetoothUuid(QStringLiteral("0000180d-0000-1000-8000-00805f9b34fb"))) 获取心率服务；心率特征值的数据格式是第一个字节为标志位，第二个字节起为心率值（如果标志位 bit0 为 0 则心率是 uint8，bit0 为 1 则心率是 uint16）。

## 6. 官方文档参考

[Qt 文档 · QtBluetooth 模块](https://doc.qt.io/qt-6/qtbluetooth-index.html) -- 蓝牙模块总览

[Qt 文档 · QBluetoothDeviceDiscoveryAgent](https://doc.qt.io/qt-6/qbluetoothdevicediscoveryagent.html) -- 设备扫描

[Qt 文档 · QBluetoothSocket](https://doc.qt.io/qt-6/qbluetoothsocket.html) -- 经典蓝牙串口通信

[Qt 文档 · QLowEnergyController](https://doc.qt.io/qt-6/qlowenergycontroller.html) -- BLE 连接管理

[Qt 文档 · QLowEnergyService](https://doc.qt.io/qt-6/qlowenergyservice.html) -- BLE GATT 服务操作

*(链接已验证，2026-04-23 可访问)*

---

到这里就大功告成了。QtBluetooth 的核心脉络其实就两条线路：经典蓝牙走 QBluetoothDeviceDiscoveryAgent + QBluetoothSocket，BLE 走 QLowEnergyController + QLowEnergyService。两条线路的 API 设计风格差异不小，但各自的流程都是线性的——扫描、连接、通信，一步步推进。真正需要花精力的不是 API 调用本身，而是平台权限配置和硬件适配。建议先在 Linux 或 Windows 上把经典蓝牙的 SPP 通信跑通，再来搞 BLE 的 GATT 读写——前者调试手段更丰富，后者涉及的概念更多。BLE 的 CCCD 描述符写入是新手最容易踩的坑，如果收不到 notify 回调，第一反应就是检查 CCCD 有没有写。

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
