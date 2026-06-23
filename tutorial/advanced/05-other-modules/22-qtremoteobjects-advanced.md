---
title: "5.22 Remote Objects 进阶：自定义序列化与网络传输"
description: "入门篇我们用 Qt Remote Objects 做了进程间通信——一个进程暴露对象，另一个进程获取副本。进阶篇要深入底层机制：.rep 文件怎么定义远程接口、Host 和 Node 的网络传输配置、Source 和 Replica 的角色区别、自定义数据类型怎么序列化。"
---

# 现代Qt开发教程（进阶篇）5.22——Remote Objects 进阶：自定义序列化与网络传输

## 1. 前言

入门篇我们让 Qt Remote Objects 跑通了——两个进程之间共享对象状态，Source 端修改属性，Replica 端自动同步。但入门篇用的是本地连接（`QLocalServer`），所有通信都在同一台机器上。实际项目中，我们经常需要跨网络的进程间通信——比如一个嵌入式设备上的传感器进程把数据推送到 PC 端的监控界面。

而且入门篇只用了 `int`、`QString` 这些 Qt 内置类型。真正做项目的时候，我们需要传递自定义的数据结构——传感器数据包、配置对象、状态报告。这些自定义类型能不能跨进程传递？怎么告诉 Remote Objects 怎么序列化它们？

这篇就拆这两个问题：网络传输配置和自定义类型序列化。同时我们也会更深入地理解 Source 和 Replica 的角色区别——这不只是「服务端/客户端」那么简单。

## 2. 环境说明

本文档基于 Qt 6.2+ 编写，需要 Qt6::RemoteObjects 模块。CMake 中通过 `find_package(Qt6 REQUIRED COMPONENTS RemoteObjects)` 引入。Qt Remote Objects 使用 `repc` 编译器处理 `.rep` 文件，需要在 CMake 中启用 `CMAKE_AUTOMOC` 或手动调用 `qt_add_repc_sources()` / `qt_add_repc_replicas()`。

## 3. 核心概念讲解

### 3.1 .rep 文件定义远程接口

`.rep` 文件是 Qt Remote Objects 的接口定义语言。它声明了远程对象的属性、信号和方法——类似于 IDL（Interface Definition Language）或 gRPC 的 `.proto` 文件。`repc` 编译器根据 `.rep` 文件生成 C++ 头文件，包含 Source 端的基类和 Replica 端的代理类。

```cpp
// sensor.rep — 传感器数据接口定义
class SensorInterface
{
    /// @brief 当前温度值（摄氏度），由 Source 端推送。
    PROP(float temperature = 0.0);

    /// @brief 当前湿度百分比，由 Source 端推送。
    PROP(float humidity = 0.0);

    /// @brief 设备在线状态。
    PROP(bool online = false);

    /// @brief 温度超过阈值时触发。
    /// @param value 超标的温度值。
    /// @param threshold 阈值。
    SIGNAL(temperatureAlarm(float value, float threshold));

    /// @brief 请求传感器重新校准。
    /// @param offset 校准偏移量。
    /// @return 是否成功。
    SLOT(bool recalibrate(float offset));
}
```

`.rep` 文件的语法比较简单：`PROP` 声明属性（带默认值），`SIGNAL` 声明信号，`SLOT` 声明可远程调用的方法。这些声明会被 `repc` 编译器翻译成 C++ 代码。

在 CMake 中集成 `.rep` 文件：

```cmake
find_package(Qt6 REQUIRED COMPONENTS RemoteObjects)

qt_add_repc_sources(sensor_server
    sensor.rep          # .rep 文件路径
)

qt_add_repc_replicas(sensor_client
    sensor.rep          # 同一个 .rep 文件，但生成 Replica 代码
)
```

`qt_add_repc_sources` 为 Source 端生成基类（`SensorInterfaceSimpleSource`），你需要继承它并实现属性和 SLOT。`qt_add_repc_replicas` 为 Replica 端生成代理类（`SensorInterfaceReplica`），Replica 端通过它访问远程对象的属性和调用方法。

### 3.2 Source 和 Replica 的角色区别

Source 和 Replica 的概念不仅仅是「服务端/客户端」。Source 是数据的拥有者和权威来源，Replica 是数据的副本和消费者。一个 Source 可以有多个 Replica，Source 端的属性变化会自动推送到所有 Replica。但 Replica 端对属性的修改不会自动同步回 Source——Replica 是单向同步的副本。

如果需要双向同步（Replica 端修改属性后 Source 端也能看到），需要用 `SLOT` 方法：Replica 端调用 Source 端的 SLOT，SLOT 内部修改 Source 的属性，然后属性变化再自动推送到所有 Replica。这就是一个完整的「Replica 发请求 -> Source 处理 -> 推送给所有 Replica」的循环。

```cpp
/// @brief Source 端实现——传感器服务。
class SensorSource : public SensorInterfaceSimpleSource
{
    Q_OBJECT

public:
    explicit SensorSource(QObject* parent = nullptr)
        : SensorInterfaceSimpleSource(parent)
    {
        // 定时更新传感器数据
        auto* timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &SensorSource::update_readings);
        timer->start(2000);  // 每 2 秒更新
    }

public slots:
    /// @brief 校准传感器（覆盖 .rep 中定义的 SLOT）。
    /// @param offset 校准偏移量。
    /// @return 是否成功。
    bool recalibrate(float offset) override
    {
        m_calibration_offset = offset;
        return true;
    }

private slots:
    /// @brief 更新传感器读数。
    void update_readings()
    {
        // 模拟传感器数据
        float kTemp = read_temperature_hardware() + m_calibration_offset;
        float kHumidity = read_humidity_hardware();

        // 设置属性——变化会自动推送到所有 Replica
        setTemperature(kTemp);
        setHumidity(kHumidity);
        setOnline(true);

        // 检查温度阈值
        if (kTemp > kAlarmThreshold) {
            emit temperatureAlarm(kTemp, kAlarmThreshold);
        }
    }

private:
    float m_calibration_offset = 0.0f;
    static constexpr float kAlarmThreshold = 40.0f;

    // 模拟硬件读取
    float read_temperature_hardware() { return 25.0f + QRandomGenerator::global()->bounded(-5.0f, 15.0f); }
    float read_humidity_hardware() { return 50.0f + QRandomGenerator::global()->bounded(-10.0f, 20.0f); }
};
```

```cpp
/// @brief Replica 端使用——监控界面。
void setup_replica_client()
{
    auto* node = new QRemoteObjectNode(this);
    node->connectToNode(QUrl("tcp://192.168.1.100:5555"));

    // 获取 Replica
    auto* replica = node->acquire<SensorInterfaceReplica>();

    // 监听属性变化
    connect(replica, &SensorInterfaceReplica::temperatureChanged, this,
        [](float value) {
            qDebug() << "温度更新:" << value;
        });

    // 监听信号
    connect(replica, &SensorInterfaceReplica::temperatureAlarm, this,
        [](float value, float threshold) {
            qWarning() << "温度警报! " << value << "超过阈值" << threshold;
        });

    // 远程调用 SLOT
    auto* reply = replica->recalibrate(0.5f);
    connect(reply, &QRemoteObjectPendingReply<bool>::finished, this,
        [](QRemoteObjectPendingReply<bool> reply) {
            qDebug() << "校准结果:" << reply.returnValue();
        });
}
```

注意 Replica 端的远程调用是异步的——`recalibrate()` 返回一个 `QRemoteObjectPendingReply`，真正的返回值通过网络传回来后才可用。这是因为 Remote Objects 底层是基于消息传递的，不能阻塞等待远程响应。

### 3.3 网络传输配置——QRemoteObjectHost 和 QRemoteObjectNode

入门篇用的是本地传输（`QUrl("local:sensor")`），走 `QLocalServer`/`QLocalSocket`。进阶篇我们用 TCP 传输，走 `QTcpServer`/`QTcpSocket`。

```cpp
/// @brief Source 端——启动 Host 并注册远程对象。
void setup_source_host()
{
    auto* host = new QRemoteObjectHost(this);
    host->setHostUrl(QUrl("tcp://0.0.0.0:5555"));  // 监听所有网卡

    auto* sensor = new SensorSource(this);
    host->enableRemoting(sensor);  // 注册到 Host，允许远程访问
}

/// @brief Replica 端——连接到远程 Host。
void setup_replica_node()
{
    auto* node = new QRemoteObjectNode(this);
    node->connectToNode(QUrl("tcp://192.168.1.100:5555"));

    auto* replica = node->acquire<SensorInterfaceReplica>();
    // ... 使用 replica
}
```

`QRemoteObjectHost` 是 Source 端的入口——它既是一个网络服务器（监听连接），又管理 Source 对象的注册。`QRemoteObjectNode` 是 Replica 端的入口——它连接到 Host，获取 Replica 对象。一个 Host 可以注册多个 Source 对象，一个 Node 可以连接多个 Host（但每个 Node 只能有一个 `connectToNode`，要连多个 Host 需要多个 Node）。

网络传输的底层是 `QIODevice`。Remote Objects 框架把 Source 的属性变化序列化成二进制消息，通过 TCP 发送给所有连接的 Replica。消息格式是 Qt 私有的二进制协议，不需要关心细节。

### 3.4 自定义数据类型的序列化

如果我们想在 `.rep` 文件中使用自定义的 C++ 类型（比如 `SensorData` 结构体），需要注册这个类型并告诉 Remote Objects 怎么序列化它。

```cpp
/// @brief 自定义传感器数据包。
struct SensorData
{
    float temperature;
    float humidity;
    qint64 timestamp;  // 毫秒级时间戳
};

// 注册到 Qt 元类型系统
Q_DECLARE_METATYPE(SensorData)

/// @brief SensorData 的流序列化操作符。
/// @param[out] stream 数据流。
/// @param[in] data 要序列化的数据。
/// @return 数据流引用。
QDataStream& operator<<(QDataStream& stream, const SensorData& data)
{
    stream << data.temperature << data.humidity << data.timestamp;
    return stream;
}

/// @brief SensorData 的流反序列化操作符。
/// @param[in] stream 数据流。
/// @param[out] data 反序列化的目标。
/// @return 数据流引用。
QDataStream& operator>>(QDataStream& stream, SensorData& data)
{
    stream >> data.temperature >> data.humidity >> data.timestamp;
    return stream;
}
```

注册自定义类型需要在程序启动时调用：

```cpp
/// @brief 在 main() 中注册自定义类型。
void register_custom_types()
{
    qRegisterMetaType<SensorData>("SensorData");
    qRegisterMetaTypeStreamOperators<SensorData>("SensorData");
}
```

然后在 `.rep` 文件中就可以使用这个类型了。不过要注意，`.rep` 文件只能识别 Qt 内置类型和已注册的自定义类型。如果类型没有被正确注册，`repc` 生成的代码会编译失败。

现在有个调试题给大家。你注册了自定义类型并用在 `.rep` 文件的 PROP 中，但 Replica 端收到的属性值总是空的。可能是什么原因？

最可能的原因是忘记调用 `qRegisterMetaTypeStreamOperators`。`qRegisterMetaType` 只注册了类型到元类型系统，但不注册序列化操作符。Remote Objects 需要通过 `QDataStream` 来序列化属性值，如果没有注册流操作符，序列化就会失败，Replica 端收到的是默认构造的空值。另一个可能的原因是 `operator<<` 和 `operator>>` 的字段顺序不一致——序列化和反序列化必须严格按相同顺序读写。

## 4. 踩坑预防

第一个坑是 `.rep` 文件的修改不会自动触发重新编译。有时候你改了 `.rep` 文件（比如加了一个新的 PROP），但 CMake 没有检测到变化，不会重新运行 `repc`。结果是 C++ 代码还在用旧的接口定义，编译通过但新属性不存在。解决方案是在修改 `.rep` 文件后手动触发一次全量重新编译（`cmake --build build --clean-first`），或者 touch 一下生成的头文件。

第二个坑是自定义类型必须在 Source 端和 Replica 端都用完全相同的方式注册。如果 Source 端的 `SensorData` 多了一个字段但没更新 Replica 端的注册代码，反序列化会读到错误的数据（字段偏移错位）。这是因为 `QDataStream` 按顺序读写，没有字段名或版本号的概念。解决方案是在自定义类型中加一个版本号字段，或者在 `operator<<` 和 `operator>>` 中加版本检查。

第三个坑是 Replica 端在连接断开后不会自动重连。如果 Source 端的 Host 临时不可用（比如网络中断），Replica 端的 `QRemoteObjectNode` 不会自动重试连接。需要手动监听连接状态变化并在断开后重新调用 `connectToNode`。`QRemoteObjectNode` 有一个 `connectionToNodeChanged` 信号可以用来检测连接状态。

## 5. 练习项目

练习项目是一个分布式温度监控系统。系统由两个进程组成：传感器进程（Source）和监控进程（Replica）。

传感器进程模拟 3 个传感器，每个传感器有温度、湿度、在线状态三个属性，每秒更新随机数据。温度超过阈值时触发告警信号。传感器进程通过 TCP Host 把这些数据暴露出去。监控进程连接到传感器进程，获取 3 个传感器的 Replica，在控制台打印每个传感器的实时数据。当收到温度告警时用 `qWarning` 输出。监控进程还可以通过远程调用向传感器进程发送校准命令。

完成标准是两个进程能通过 TCP 正确通信、数据实时同步、告警信号正确触发、远程校准命令正确执行。传感器数据使用自定义的 `SensorData` 结构体传递。

提示几个关键点：`.rep` 文件定义传感器的接口（属性用自定义 `SensorData` 类型）；自定义类型需要 `Q_DECLARE_METATYPE` + `qRegisterMetaTypeStreamOperators`；Source 端用 `QRemoteObjectHost`，Replica 端用 `QRemoteObjectNode`。

## 6. 官方文档参考链接

[Qt 文档 · Qt Remote Objects](https://doc.qt.io/qt-6/qtremoteobjects-module.html) -- 模块总览

[Qt 文档 · QRemoteObjectHost](https://doc.qt.io/qt-6/qremoteobjecthost.html) -- Source 端 Host 管理

[Qt 文档 · QRemoteObjectNode](https://doc.qt.io/qt-6/qremoteobjectnode.html) -- Replica 端节点管理

[Qt 文档 · QRemoteObjectPendingCall](https://doc.qt.io/qt-6/qremoteobjectpendingcall.html) -- 异步远程调用结果

[Qt 文档 · repc 编译器](https://doc.qt.io/qt-6/qtremoteobjects-repc.html) -- .rep 文件语法和 repc 工具

---

到这里 Qt Remote Objects 的进阶用法就拆完了。`.rep` 文件定义接口、TCP 网络传输、Source/Replica 角色分离、自定义类型序列化——这套机制虽然不如 gRPC 或 Cap'n Proto 那么强大，但对于 Qt 应用间的通信来说已经足够了。它的优势是与 Qt 的元类型系统和信号槽机制深度集成，用起来非常自然。
