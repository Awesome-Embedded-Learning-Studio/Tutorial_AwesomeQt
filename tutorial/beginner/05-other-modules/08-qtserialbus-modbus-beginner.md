# 现代Qt开发教程（新手篇）5.8——Qt Serial Bus Modbus 通信基础

## 1. 前言：工业通信的"普通话"

如果你做的是工业自动化、能源管理、楼宇控制这类领域的软件开发，Modbus 是一个绕不开的协议。它诞生于 1979 年，最初由 Modicon 公司（现在的施耐德电气）为 PLC 通信设计，至今仍然是工业现场最广泛使用的通信协议之一。说它是工业通信的"普通话"一点都不为过——不管你用的是什么厂家的设备，大概率都支持 Modbus。

Modbus 之所以生命力这么强，原因很简单：协议规约极其精简，实现成本低，功能却覆盖了最基本的读写需求。它本质上就是一个"读寄存器、写寄存器"的协议，支持线圈（Coil，布尔量）和寄存器（Register，16 位整型）两种数据类型，提供读和写两种操作方向。就这么点东西，足够覆盖 90% 的工业数据采集场景。

Qt 的 QtSerialBus 模块提供了 Modbus RTU（串口）和 Modbus TCP（以太网）两种传输层的客户端/服务端实现。我们这篇只关注客户端侧——也就是你的 Qt 程序作为 Modbus 主站去读写远端从站设备的数据。服务端侧（Qt 程序模拟从站）属于进阶内容，这里不展开。

## 2. 环境说明

本篇基于 Qt 6.9.1，需要引入 `Qt6::SerialBus` 和 `Qt6::Widgets` 两个模块。CMake 配置如下：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core SerialBus Widgets)
```

QtSerialBus 在 Qt 6 中是一个独立的附加模块（Qt Add-On），需要单独安装。在 Qt Installer 里勾选 "Qt Serial Bus" 组件即可，Linux 发行版的包管理器中通常叫 `qt6-serialbus` 或类似名称。

Modbus RTU 模式需要系统有可用的串口（或者 USB 转串口适配器），底层依赖 Qt 的 QtSerialPort 模块。如果你只用 Modbus TCP 模式，则不需要串口硬件，只需要网络连通就行。本篇的示例程序同时支持 RTU 和 TCP 两种连接方式，但没有真实硬件也能跑——程序内置了一个模拟从站来演示数据交互。

编译工具链方面，MSVC 2019+、GCC 11+ 均可，C++17 标准，CMake 3.26+ 构建系统。

## 3. 核心概念讲解

### 3.1 Modbus RTU vs TCP——传输层不同，协议层相同

Modbus 协议栈可以分成两层：上面是 Modbus 应用层协议（定义了功能码、数据帧格式），下面是传输层。RTU 和 TCP 的区别仅仅在传输层。

Modbus RTU（Remote Terminal Unit）运行在串口上，通常使用 RS-485 总线。物理层是两线制差分信号，抗干扰能力强，适合工业现场长距离传输。通信参数需要配置串口号（如 COM3 或 /dev/ttyUSB0）、波特率（常见 9600、19200、115200）、数据位（8）、校验位（None/Even/Odd）、停止位（1/2）。每个从站有一个 1 字节的站地址（1-247），主站发起请求时携带目标地址，从站响应时也携带自己的地址。

Modbus TCP 运行在 TCP/IP 网络上，默认端口是 502。因为 TCP 本身提供了可靠的传输和寻址机制，Modbus TCP 帧比 RTU 帧少了一些字段（不需要 CRC 校验，不需要站地址——用 IP 地址替代）。连接参数只需要 IP 地址和端口号。

从 Qt 的 API 角度看，两种模式的差异被封装在连接参数里，读写操作的代码完全一样。这正是 QtSerialBus 的设计优势——你的业务代码不需要关心底层是串口还是网络。

### 3.2 QModbusClient 与连接建立

QtSerialBus 提供了 QModbusClient 作为 Modbus 主站的抽象基类。你不能直接实例化 QModbusClient，而是通过 QModbusRtuSerialClient 或 QModbusTcpClient 创建具体传输层的客户端对象：

```cpp
// Modbus TCP 客户端
auto *client = new QModbusTcpClient(this);

// Modbus RTU 串口客户端
auto *client = new QModbusRtuSerialClient(this);
```

建立连接需要设置连接参数然后调用 connectDevice()。对于 TCP 模式：

```cpp
client->setConnectionParameter(
    QModbusDevice::NetworkAddressParameter, "192.168.1.100");
client->setConnectionParameter(
    QModbusDevice::NetworkPortParameter, 502);

// 可选：设置响应超时和重试次数
client->setTimeout(1000);     // 1 秒超时
client->setNumberOfRetries(3);

client->connectDevice();
```

对于 RTU 模式：

```cpp
client->setConnectionParameter(
    QModbusDevice::SerialPortNameParameter, "/dev/ttyUSB0");
client->setConnectionParameter(
    QModbusDevice::SerialBaudRateParameter, 9600);
client->setConnectionParameter(
    QModbusDevice::SerialDataBitsParameter, 8);
client->setConnectionParameter(
    QModbusDevice::SerialParityParameter, QSerialPort::NoParity);
client->setConnectionParameter(
    QModbusDevice::SerialStopBitsParameter, 1);

client->connectDevice();
```

connectDevice() 是异步的——调用后立即返回，不会阻塞。连接成功或失败通过 stateChanged 信号通知：

```cpp
connect(client, &QModbusClient::stateChanged, this,
        [](QModbusDevice::State state) {
    switch (state) {
    case QModbusDevice::ConnectedState:
        qDebug() << "已连接";
        break;
    case QModbusDevice::UnconnectedState:
        qDebug() << "已断开";
        break;
    case QModbusDevice::ConnectingState:
        qDebug() << "正在连接...";
        break;
    default:
        break;
    }
});
```

这里有一个常见的坑：connectDevice() 调用后不会立刻变为 ConnectedState，需要等事件循环处理。如果你在 connectDevice() 后面紧接着发读写请求，大概率会失败——因为连接还没建立完成。正确做法是在 stateChanged 信号的槽函数里判断状态，变成 ConnectedState 之后再开始读写操作。

errorOccurred 信号用于捕获连接和通信过程中的错误，建议始终监听：

```cpp
connect(client, &QModbusClient::errorOccurred, this,
        [client](QModbusDevice::Error error) {
    qWarning() << "Modbus 错误:" << error
               << client->errorString();
});
```

### 3.3 读写线圈与保持寄存器

Modbus 协议定义了四种数据对象：线圈（Coil，地址 0x0000 起，可读写布尔量）、离散输入（Discrete Input，地址 0x1000 起，只读布尔量）、保持寄存器（Holding Register，地址 0x0000 起，可读写 16 位整型）、输入寄存器（Input Register，地址 0x1000 起，只读 16 位整型）。对应的 Modbus 功能码分别是 01/05/15（线圈读/写单个/写多个）、02（离散输入读）、03/06/16（保持寄存器读/写单个/写多个）、04（输入寄存器读）。

在 QtSerialBus 的 API 中，读写操作通过 sendReadRequest() 和 sendWriteRequest() 发起。这两个函数返回一个 QModbusReply 指针，用于接收异步结果。

读取保持寄存器（功能码 03）：

```cpp
// 从站地址 1，读取起始地址 0，读取数量 10 个寄存器
QModbusDataUnit readUnit(QModbusDataUnit::HoldingRegisters, 0, 10);

QModbusReply *reply = client->sendReadRequest(readUnit, 1);
```

读取线圈（功能码 01）：

```cpp
QModbusDataUnit readUnit(QModbusDataUnit::Coils, 0, 8);
QModbusReply *reply = client->sendReadRequest(readUnit, 1);
```

写入保持寄存器（功能码 16，写多个）：

```cpp
QModbusDataUnit writeUnit(QModbusDataUnit::HoldingRegisters, 0, 3);
QVector<quint16> values = {100, 200, 300};
writeUnit.setValues(values);

QModbusReply *reply = client->sendWriteRequest(writeUnit, 1);
```

写入线圈（功能码 15，写多个）：

```cpp
QModbusDataUnit writeUnit(QModbusDataUnit::Coils, 0, 4);
QVector<quint16> values = {1, 0, 1, 1};  // 1=ON, 0=OFF
writeUnit.setValues(values);

QModbusReply *reply = client->sendWriteRequest(writeUnit, 1);
```

QModbusDataUnit 的第一个参数是寄存器类型，第二个是起始地址，第三个是数量。注意这里的地址是 Modbus 协议中的"协议地址"（从 0 开始），不是 PLC 面板上显示的"Modbus 地址"（通常从 1 开始）。如果你的 PLC 文档说"保持寄存器地址 40001"，对应的协议地址是 0；如果文档说"40010"，对应的协议地址是 9。这个偏移差 1 是新手最常踩的坑之一。

### 3.4 QModbusReply 异步结果处理

所有读写请求都是异步的。sendReadRequest() 和 sendWriteRequest() 调用后立刻返回一个 QModbusReply 指针。真正的结果通过 QModbusReply 的 finished 信号回调获取。

读取结果的处理模板：

```cpp
if (reply) {
    // 防止内存泄漏——reply 用完必须删除
    QObject::connect(reply, &QModbusReply::finished, [reply]() {
        if (reply->error() == QModbusDevice::NoError) {
            // 读取成功，获取数据
            const QModbusDataUnit unit = reply->result();
            for (int i = 0; i < unit.valueCount(); ++i) {
                qDebug() << "地址" << unit.startAddress() + i
                         << ":" << unit.value(i);
            }
        } else {
            // 通信错误
            qWarning() << "读取失败:" << reply->errorString();
        }
        reply->deleteLater();
    });
} else {
    // 请求发送失败（比如参数错误）
    qWarning() << "发送请求失败";
}
```

写入结果的处理类似，区别在于写入成功后 unit 里没有有效数据——只需要检查 error() 即可：

```cpp
if (reply) {
    QObject::connect(reply, &QModbusReply::finished, [reply]() {
        if (reply->error() == QModbusDevice::NoError) {
            qDebug() << "写入成功";
        } else {
            qWarning() << "写入失败:" << reply->errorString();
        }
        reply->deleteLater();
    });
}
```

有几个细节值得注意。第一，reply->deleteLater() 不能省。QModbusReply 对象不会自动销毁，如果不手动删除就会内存泄漏。第二，在 finished 信号的槽函数内部访问 reply 是安全的——因为 deleteLater() 会把删除推迟到下一次事件循环迭代。第三，如果你需要在槽函数外引用 reply（比如取消请求），需要自己管理 reply 的生命周期——调用 reply->deleteLater() 后不能再访问它。

另一个实用函数是 sendReadWriteRequest()，它在一个事务中同时发送读和写请求（功能码 23，Read/Write Multiple Registers）。不是所有从站设备都支持这个功能码，使用前需要确认设备手册。

## 4. 综合示例：Modbus 客户端调试工具

把前面学的串起来，我们写一个 Modbus 客户端调试工具。程序支持 TCP 和 RTU 两种连接方式（通过界面切换），提供读写保持寄存器和线圈的功能，并将结果显示在日志区。由于实际硬件可能不可用，程序会检测连接失败并给出清晰的错误提示——你可以在没有真实设备的情况下了解整个 API 的使用流程。

完整代码见 `examples/beginner/05-other-modules/08-qtserialbus-modbus-beginner/`，下面是关键部分的讲解。

CMake 配置：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core SerialBus Widgets)
# ...
target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::SerialBus Qt6::Widgets)
```

程序界面上方是连接配置区，中部是读写操作区，下方是日志输出区。

连接建立的核心代码。我们根据用户选择的模式创建不同的客户端：

```cpp
// 创建客户端
if (tcp_radio_->isChecked()) {
    client_ = new QModbusTcpClient(this);
    client_->setConnectionParameter(
        QModbusDevice::NetworkAddressParameter,
        host_edit_->text());
    client_->setConnectionParameter(
        QModbusDevice::NetworkPortParameter,
        port_spin_->value());
} else {
    client_ = new QModbusRtuSerialClient(this);
    client_->setConnectionParameter(
        QModbusDevice::SerialPortNameParameter,
        serial_edit_->text());
    client_->setConnectionParameter(
        QModbusDevice::SerialBaudRateParameter,
        baud_combo_->currentText().toInt());
    client_->setConnectionParameter(
        QModbusDevice::SerialDataBitsParameter, 8);
    client_->setConnectionParameter(
        QModbusDevice::SerialParityParameter,
        QSerialPort::NoParity);
    client_->setConnectionParameter(
        QModbusDevice::SerialStopBitsParameter, 1);
}

client_->setTimeout(1000);
client_->setNumberOfRetries(3);

// 监听状态变化
connect(client_, &QModbusClient::stateChanged, this,
        &ModbusWindow::onStateChanged);
connect(client_, &QModbusClient::errorOccurred, this,
        &ModbusWindow::onError);

client_->connectDevice();
```

读取保持寄存器的操作：

```cpp
void onReadRegisters()
{
    if (!client_ || client_->state() != QModbusDevice::ConnectedState) {
        appendLog("错误：未连接");
        return;
    }

    int serverAddr = server_spin_->value();
    int startAddr = addr_spin_->value();
    int count = count_spin_->value();

    QModbusDataUnit unit(QModbusDataUnit::HoldingRegisters,
                         startAddr, count);
    QModbusReply *reply = client_->sendReadRequest(unit, serverAddr);

    if (reply) {
        connect(reply, &QModbusReply::finished, this, [this, reply]() {
            if (reply->error() == QModbusDevice::NoError) {
                const QModbusDataUnit data = reply->result();
                for (int i = 0; i < data.valueCount(); ++i) {
                    appendLog(QString("  地址 %1 = %2")
                        .arg(data.startAddress() + i)
                        .arg(data.value(i)));
                }
            } else {
                appendLog("读取失败: " + reply->errorString());
            }
            reply->deleteLater();
        });
    } else {
        appendLog("发送读取请求失败");
    }
}
```

如果你手边没有 Modbus 设备，程序在连接失败时会显示具体的错误信息（比如 "Connection refused" 或串口不存在），你依然可以看到完整的 API 调用流程。要实际测试读写功能，需要运行一个 Modbus 模拟器（比如 ModRSSim、diagslave 等）或者连接真实的 Modbus 从站设备。

## 5. 练习项目

练习项目：Modbus 寄存器监视器。

我们要做一个持续轮询的寄存器监视工具。程序按照用户设定的时间间隔（比如 1 秒）循环读取指定范围的保持寄存器，将每次读取的结果以表格形式显示，如果某个寄存器的值发生了变化就高亮标记。

完成标准是这样的：使用 QTimer 定时触发读取操作，每次读取完成后刷新表格数据；表格使用 QTableWidget 显示，列为"地址"、"当前值"、"上次值"、"变化时间"；值发生变化时对应行背景色变为浅黄色，持续 3 秒后恢复；支持配置轮询间隔、从站地址、起始地址、读取数量。

几个实现提示：QTimer 的 timeout 信号触发下一次读取请求，但要注意上一次请求可能还没返回——可以在 finished 槽函数里判断是否需要发起下一轮，或者用一个标志位防止重叠请求；QTableWidget 的 QTableWidgetItem 可以通过 setBackground() 设置背景色，配合 QTimer 延迟恢复；建议把读取逻辑封装成一个独立的函数，方便在定时器和手动读取按钮之间复用。

## 6. 官方文档参考

[Qt 文档 · QtSerialBus 模块](https://doc.qt.io/qt-6/qtserialbus-index.html) -- 串行总线模块总览

[Qt 文档 · QModbusClient](https://doc.qt.io/qt-6/qmodbusclient.html) -- Modbus 客户端基类

[Qt 文档 · QModbusTcpClient](https://doc.qt.io/qt-6/qmodbustcpclient.html) -- Modbus TCP 客户端

[Qt 文档 · QModbusRtuSerialClient](https://doc.qt.io/qt-6/qmodbusrtuserialclient.html) -- Modbus RTU 串口客户端

[Qt 文档 · QModbusDataUnit](https://doc.qt.io/qt-6/qmodbusdataunit.html) -- Modbus 数据单元

[Qt 文档 · QModbusReply](https://doc.qt.io/qt-6/qmodbusreply.html) -- Modbus 异步应答

*(链接已验证，2026-04-23 可访问)*

---

到这里就大功告成了。Modbus 是工业领域最基础的通信协议，QtSerialBus 把它的 RTU 和 TCP 两种传输模式封装成了统一的 API——创建客户端、设置连接参数、发起读写请求、处理异步结果，这个流程走通之后，对接任何 Modbus 设备都只是参数配置的事情。记住几个关键点：connectDevice() 是异步的，等 stateChanged 信号确认连接成功后再操作；QModbusReply 必须在 finished 槽函数里 deleteLater()，否则会内存泄漏；协议地址从 0 开始，和 PLC 面板地址差 1。把这些坑避开，Modbus 通信这块就不会出什么问题了。

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
