# 现代Qt开发教程（新手篇）4.6——QtSerialPort 串口通信基础

## 1. 前言：为什么还要学串口通信

说实话，在 2026 年还在讲串口（Serial Port）多少有点穿越的味道——这玩意儿的起源可以追溯到上世纪 60 年代的 RS-232 标准。但现实情况是，串口在嵌入式开发和工业控制领域依然是绝对的刚需。你买的每一块开发板（STM32、ESP32、树莓派 Pico）出厂调试靠串口，各种传感器模块（GPS、蓝牙、LoRa）和上位机通信靠串口，工厂里的 PLC 和工控机之间通信还是靠串口。如果你做的是和硬件打交道的工作，串口通信是你绕不过去的基本功。

串口通信的核心特征非常简单：它是点对点的、全双工的、逐字节传输的。通信双方各有一条发送线和一条接收线，各自独立工作。传输参数包括波特率（每秒传输的比特数）、数据位（通常是 8 位）、校验位（None/Even/Odd）和停止位（1 或 2 位），双方必须配置完全一致才能正确通信。

Qt 提供了 QtSerialPort 模块来处理串口通信，核心类是 QSerialPort 和 QSerialPortInfo。QSerialPort 封装了串口的打开、配置、读写操作，接口设计和 QTcpSocket 保持一致——同样是信号槽驱动的异步模型，同样通过 `readyRead` 信号通知数据到达。QSerialPortInfo 则提供了枚举系统中所有可用串口的能力，可以获取端口号、厂商信息、产品描述等元数据。

这一篇我们把串口的配置与打开、异步读写、系统串口枚举这些核心操作全部过一遍。

## 2. 环境说明

本教程基于 Qt 6.9.1，适用于 Windows 10/11（MSVC 2019+ 或 MinGW 11.2+）、Linux（GCC 11+）、WSL2 + WSLg（GUI 支持）。所有示例代码均使用 C++17 标准和 CMake 3.26+ 构建系统。本篇需要链接 Qt6::SerialPort 模块，CMake 中通过 `find_package(Qt6 REQUIRED COMPONENTS Core SerialPort)` 引入。

关于平台差异：Windows 上串口设备的名称是 `COM1`、`COM2` 等；Linux 上是 `/dev/ttyS0`（硬件串口）、`/dev/ttyUSB0`（USB 转串口）或 `/dev/ttyACM0`（USB CDC 虚拟串口）；macOS 上是 `/dev/tty.usbserial-xxx`。另外，在 Linux 上访问串口设备需要当前用户属于 `dialout` 用户组，否则会提示权限不足。你可以通过 `sudo usermod -aG dialout $USER` 把自己加入该组，然后注销重新登录生效。

## 3. 核心概念讲解

### 3.1 串口通信的基本参数

在打开串口之前，你需要理解四个核心参数，它们决定了通信双方如何编码和解码每一个字节。

波特率（Baud Rate）表示每秒传输的比特数，常见值有 9600、19200、38400、57600、115200。波特率越高，传输速度越快，但对线路质量和时钟同步精度的要求也越高。在实际项目中，115200 是最常用的默认值——速度够快，又不会太挑剔线路质量。两端设备的波特率必须完全一致，否则接收到的数据就是乱码。

数据位（Data Bits）表示每个字节实际传输的有效数据位数，可选 5、6、7、8 位。现代应用几乎全部使用 8 位数据位，因为一个字节就是 8 比特。如果你看到 7 位数据位，那通常是很老的协议或者需要在校验位上多留一位空间的场景。

校验位（Parity）用于检测传输过程中的单比特错误。可选 None（无校验）、Even（偶校验）、Odd（奇校验）、Space（校验位始终为 0）、Mark（校验位始终为 1）。绝大多数现代应用使用 None——因为校验位的检错能力非常有限，如果你真的需要可靠传输，应该在应用层使用 CRC 或者校验和。如果使用 None 校验，那一个完整的字符帧就是：起始位（1 位）+ 数据位（8 位）+ 停止位（1 位）= 10 位。

停止位（Stop Bits）标志一个字符帧的结束，可选 1 或 2 位。几乎所有场景都用 1 位停止位。2 位停止位是给老式低速设备预留的，现在基本见不到了。

### 3.2 QSerialPortInfo 枚举系统串口

在连接串口设备之前，我们通常需要先知道系统里有哪些可用的串口。QSerialPortInfo 提供了静态方法 `availablePorts()` 来枚举系统中所有被识别到的串口设备。

```cpp
const QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

for (const QSerialPortInfo &info : ports) {
    qDebug() << "Port:" << info.portName();
    qDebug() << "  Description:" << info.description();
    qDebug() << "  Manufacturer:" << info.manufacturer();
    qDebug() << "  System location:" << info.systemLocation();
    qDebug() << "  Vendor ID:" << Qt::hex << info.vendorIdentifier();
    qDebug() << "  Product ID:" << Qt::hex << info.productIdentifier();
    qDebug() << "  Has product identifier:" << info.hasProductIdentifier();
    qDebug() << "";
}
```

`portName()` 返回的是平台的串口名称（比如 `COM3` 或 `ttyUSB0`），这个名称就是后续传给 `QSerialPort::setPortName()` 的参数。`description()` 和 `manufacturer()` 是设备在系统中注册的描述信息和厂商名称，对于 USB 转串口设备，这两个字段通常能帮你识别出设备的型号和品牌。`vendorIdentifier()` 和 `productIdentifier()` 是 USB 设备的 VID 和 PID，如果你需要自动识别特定型号的设备，可以匹配这两个值。

在 Linux 上，如果你插拔了 USB 转串口设备，`availablePorts()` 的返回值会实时变化。不过在某些情况下，你可能需要等待几秒钟让 udev 规则处理完毕后再查询。在 Windows 上，设备管理器里的变化会立即反映到这个列表中。

### 3.3 QSerialPort 配置与打开

获取到串口名称之后，就可以创建 QSerialPort 实例、配置参数、打开串口了。

```cpp
QSerialPort *serial = new QSerialPort(this);

// 设置串口名称
serial->setPortName("COM3");       // Windows
// serial->setPortName("ttyUSB0"); // Linux

// 配置通信参数
serial->setBaudRate(QSerialPort::Baud115200);
serial->setDataBits(QSerialPort::Data8);
serial->setParity(QSerialPort::NoParity);
serial->setStopBits(QSerialPort::OneStop);
serial->setFlowControl(QSerialPort::NoFlowControl);

// 以读写模式打开串口
if (!serial->open(QIODevice::ReadWrite)) {
    qDebug() << "Failed to open serial port:" << serial->errorString();
    return;
}

qDebug() << "Serial port opened successfully!";
```

`setBaudRate()` 除了接受预定义的枚举值，也可以直接传整数（比如 `setBaudRate(256000)`）来设置非标准波特率——前提是你的硬件和驱动支持这个速率。

`setFlowControl()` 设置流控模式。可选 `NoFlowControl`（无流控）、`HardwareControl`（RTS/CTS 硬件流控）和 `SoftwareControl`（XON/XOFF 软件流控）。硬件流控需要串口线上有额外的 RTS 和 CTS 信号线，在短距离、低波特率的场景下通常不需要。软件流控通过在数据流中插入特殊的控制字符（XON = 0x11，XOFF = 0x13）来控制传输节奏，如果你的数据中包含这些字节值，软件流控会出问题。

`open()` 的参数指定打开模式——`ReadOnly`、`WriteOnly` 或 `ReadWrite`。大多数场景使用 `ReadWrite`。打开失败时 `errorString()` 会给出具体的错误原因，比如「设备不存在」「权限不足」「设备已被占用」等。

### 3.4 readyRead 异步读取与 write 发送

QSerialPort 的读写接口和 QTcpSocket 完全一致——通过 `readyRead` 信号异步接收数据，通过 `write()` 方法发送数据。

```cpp
// 异步读取：有新数据到达时触发
connect(serial, &QSerialPort::readyRead, [=]() {
    QByteArray data = serial->readAll();
    qDebug() << "Received" << data.size() << "bytes:" << data.toHex(' ');
});

// 发送数据
QByteArray sendData = "Hello, Serial Device!";
qint64 bytesWritten = serial->write(sendData);
if (bytesWritten == -1) {
    qDebug() << "Write failed:" << serial->errorString();
} else {
    qDebug() << "Written" << bytesWritten << "bytes";
}
```

`write()` 方法是非阻塞的，它把数据放到 Qt 的发送缓冲区后立刻返回。返回值是实际放入缓冲区的字节数。如果你想确保数据已经从串口硬件发出去了（而不只是在缓冲区里），可以调用 `waitForBytesWritten()` 进行同步等待，或者连接 `bytesWritten(qint64 bytes)` 信号来异步获知发送进度。在实际项目中，由于串口波特率通常不高（115200 波特率下实际吞吐量大约 11.5 KB/s），大数据量发送时需要考虑分块和流控。

`readyRead` 信号的触发行为和 QTcpSocket 类似：当串口接收缓冲区有新数据时触发。它不保证每次触发对应一个完整的「消息」或者「命令帧」——如果你在下位机端每次发送 10 字节，上位机可能会在一次 `readyRead` 中收到 10 字节，也可能收到 5 字节然后在下一次收到 5 字节。所以你的读取逻辑必须能够处理数据分片的情况。

串口数据的完整帧解析通常需要在应用层实现。最常见的方案是定义帧格式：帧头（固定的 1-2 字节标识）+ 长度字段 + 数据体 + 校验和（CRC）。你在 `readyRead` 的槽函数里把数据追加到一个缓冲区中，然后检查缓冲区里是否包含完整的帧，如果有就提取出来处理，不完整就等下一次 `readyRead` 继续拼接。

```cpp
QByteArray m_rxBuffer;  // 接收缓冲区

connect(serial, &QSerialPort::readyRead, [=]() {
    m_rxBuffer.append(serial->readAll());

    // 循环解析完整的帧（假设帧格式：0xAA 0x55 + 1字节长度 + 数据 + 1字节校验）
    while (m_rxBuffer.size() >= 4) {  // 最小帧长度：帧头(2) + 长度(1) + 校验(1)
        if (static_cast<quint8>(m_rxBuffer[0]) != 0xAA
            || static_cast<quint8>(m_rxBuffer[1]) != 0x55) {
            // 帧头不匹配，丢弃一个字节继续找
            m_rxBuffer.remove(0, 1);
            continue;
        }

        quint8 dataLen = static_cast<quint8>(m_rxBuffer[2]);
        int frameLen = 2 + 1 + dataLen + 1;  // 帧头 + 长度 + 数据 + 校验

        if (m_rxBuffer.size() < frameLen) {
            break;  // 数据不完整，等下一次 readyRead
        }

        // 提取一帧数据
        QByteArray frame = m_rxBuffer.left(frameLen);
        m_rxBuffer.remove(0, frameLen);

        // 验证校验和并处理帧数据
        processFrame(frame);
    }
});
```

这种「缓冲区追加 + 循环解析」的模式是串口通信中最常见的数据接收范式。实际的帧格式会因协议不同而不同，但核心思路都是一样的。

### 3.5 错误处理与串口状态监控

QSerialPort 继承自 QIODevice，和 QTcpSocket 一样有 `errorOccurred` 信号来报告错误。

```cpp
connect(serial, &QSerialPort::errorOccurred,
        [](QSerialPort::SerialPortError error) {
    if (error != QSerialPort::NoError) {
        qDebug() << "Serial port error:" << error;
    }
});
```

常见的错误类型包括 `DeviceNotFoundError`（设备不存在）、`PermissionError`（权限不足）、`OpenError`（打开失败）、`ResourceError`（设备意外断开，比如 USB 转串口线被拔了）。其中 `ResourceError` 在实际使用中非常常见——用户不小心碰到了 USB 线，设备就断开了。你需要在检测到这个错误后关闭串口，清理缓冲区，然后可以选择定期尝试重新打开。

另外，对于 USB 转串口设备的热插拔监控，Qt 没有直接提供信号通知。如果你需要实时监控设备的插入和拔出，在 Linux 上可以监听 udev 事件（通过 QSocketNotifier 监听 netlink socket），在 Windows 上可以使用 WM_DEVICECHANGE 消息。这属于进阶话题，这里就不展开了。

## 4. 踩坑预防清单

关于 Linux 权限的问题：如果你的程序在 Linux 上打开串口时提示权限不足，99% 的原因是当前用户不在 `dialout` 用户组里。解决方法前面已经提过了：`sudo usermod -aG dialout $USER`，然后注销重新登录。如果你不想重启会话，也可以临时用 `sudo chmod 666 /dev/ttyUSB0` 修改设备权限（不推荐用于生产环境）。

关于串口独占的问题：串口设备在同一时间只能被一个进程打开。如果你已经有一个串口调试工具（比如 minicom、PuTTY、Arduino IDE 的串口监视器）占用了这个端口，你的 Qt 程序调用 `open()` 会失败。确保在使用前关闭其他正在占用串口的程序。

关于波特率的匹配问题：两端的波特率必须完全一致，这是串口通信最基本的前提。如果你收到的是乱码（可打印字符但内容不对），首先检查波特率是否匹配。如果你收到的是不可读的二进制数据或者什么都没收到，还应该检查数据位、校验位、停止位是否一致。

关于 `readyRead` 的触发时机：在某些操作系统上，串口驱动会在接收到一定量的数据后才通知上层。这意味着即使下位机只发了 1 个字节，你的 `readyRead` 也可能不会立刻触发。如果你需要尽快响应每一个字节（比如响应某个按键命令），可以在打开串口后设置底层缓冲区策略。在某些 Linux 驱动中可以通过 `termios` 的 `VMIN` 和 `VTIME` 参数控制，不过 Qt 没有直接暴露这个接口。

现在我们来做一个小的代码填空练习，检验一下前面的内容。补全以下代码，实现一个串口的配置、打开和基本收发：

```cpp
QSerialPort *serial = new QSerialPort(this);

serial->setPortName("COM3");
serial->__________(QSerialPort::Baud115200);
serial->__________(QSerialPort::Data8);
serial->setParity(QSerialPort::NoParity);
serial->setStopBits(QSerialPort::OneStop);

if (serial->__________(QIODevice::ReadWrite)) {
    qDebug() << "Serial port opened!";

    connect(serial, &QSerialPort::__________, [=]() {
        QByteArray data = serial->readAll();
        qDebug() << "Received:" << data.toHex(' ');
    });

    serial->__________("Hello, device!");
}
```

提示：需要填入的分别是 `setBaudRate`、`setDataBits`、`open`、`readyRead`、`write`。

参考答案如下：

```cpp
QSerialPort *serial = new QSerialPort(this);

serial->setPortName("COM3");
serial->setBaudRate(QSerialPort::Baud115200);
serial->setDataBits(QSerialPort::Data8);
serial->setParity(QSerialPort::NoParity);
serial->setStopBits(QSerialPort::OneStop);

if (serial->open(QIODevice::ReadWrite)) {
    qDebug() << "Serial port opened!";

    connect(serial, &QSerialPort::readyRead, [=]() {
        QByteArray data = serial->readAll();
        qDebug() << "Received:" << data.toHex(' ');
    });

    serial->write("Hello, device!");
}
```

## 5. 练习项目

我们来做一个串口信息查询和回声测试工具。程序首先列出系统中所有可用的串口设备及其详细信息，然后尝试打开指定的串口（通过命令行参数传入），进入回声模式：从串口读取的所有数据原样回发。

完成标准：程序无参数运行时列出所有可用串口信息（名称、描述、厂商、VID/PID）；带参数运行时打开指定串口（默认 115200/8N1），显示连接信息后进入回声模式；支持通过命令行参数自定义波特率；串口断开时打印错误信息并自动退出；所有收发数据以十六进制和 ASCII 两种格式同时显示。

提示几个方向：用 `QSerialPortInfo::availablePorts()` 枚举串口，在 `readyRead` 的槽函数里读取数据并原样 `write()` 回去。十六进制显示用 `QByteArray::toHex(' ')`，ASCII 显示可以直接用 `qDebug() << QString::fromUtf8(data)` 但要注意非打印字符的过滤。错误处理连接 `errorOccurred` 信号，检测到 `ResourceError` 时退出。

再看一个调试挑战：以下串口读取代码有什么问题？

```cpp
QSerialPort serial;
serial.setPortName("ttyUSB0");
serial.setBaudRate(QSerialPort::Baud115200);
serial.open(QIODevice::ReadWrite);

connect(&serial, &QSerialPort::readyRead, [&]() {
    QByteArray data = serial.readAll();
    if (data.size() == 10) {
        processCommand(data);
    }
});
```

这里面有几个隐患。首先，没有检查 `open()` 的返回值，如果串口不存在或被占用，后续操作全部无效。其次，假设每次 `readyRead` 恰好返回 10 字节——这是错误的，串口数据到达是随机的，`readyRead` 可能触发时缓冲区里有 3 字节，也可能有 15 字节。必须使用缓冲区累积 + 帧解析的方式。另外，没有设置数据位、校验位、停止位，使用的是系统默认值，不同平台默认值可能不同。还缺少对 `errorOccurred` 信号的处理，串口断开时无法感知。

## 6. 完整示例代码

本篇的完整示例代码在 `examples/beginner/04-qtnetwork/06-serial-port-beginner/` 目录下，包含一个控制台程序，演示了串口枚举、配置打开、异步收发、错误处理等完整流程。由于串口需要硬件设备配合，示例代码包含了模拟模式，可以在没有真实串口设备的情况下验证逻辑正确性。

## 7. 官方文档参考

- [Qt 文档 · QSerialPort 类](https://doc.qt.io/qt-6/qserialport.html) -- 串口读写和配置的完整 API
- [Qt 文档 · QSerialPortInfo 类](https://doc.qt.io/qt-6/qserialportinfo.html) -- 串口设备枚举和信息查询
- [Qt 文档 · QtSerialPort 模块概述](https://doc.qt.io/qt-6/qtserialport-index.html) -- 模块总览和编程指南
- [Qt 示例 · Terminal](https://doc.qt.io/qt-6/qtserialport-terminal-example.html) -- Qt 官方的串口终端示例

*（链接已验证，2026-04-23 可访问）*

---

到这里就大功告成了。串口通信虽然看起来很「老」，但在嵌入式和工业领域它是不可替代的基础设施。QSerialPort 的接口设计非常干净，如果你已经熟悉了 QTcpSocket 的异步读写模式，上手串口编程也就是配置几个参数的事。真正的难点不在于 Qt 的 API，而在于理解各种硬件协议的帧格式——这部分就需要你在实际项目中积累了。
