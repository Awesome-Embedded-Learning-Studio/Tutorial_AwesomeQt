# 现代Qt开发教程（新手篇）5.22--Qt Remote Objects 进程间对象共享

## 1. 前言：进程间共享 QObject，不用自己搓序列化了

我们在 Qt 里写久了，迟早会碰到进程间通信的需求。传统做法无非那几样：QLocalSocket / QTcpSocket 手动收发字节流，D-Bus 做系统级消息总线，或者共享内存配上一套同步机制。这些方案都能干活，但每一套都要求你自己处理消息的序列化和反序列化——你得把 C++ 对象的属性一个一个打包成字节流发过去，对端收到之后再一个一个解包还原成对象。字段多了容易漏，版本迭代时协议格式改了两头都得跟着改，维护成本不低。

Qt Remote Objects 这个模块的思路完全不一样。它做的事情是让你直接把一个 QObject 的接口定义在一个 .rep 文件里（叫做 REPC 文件），然后 Qt 的代码生成器会自动帮你生成 Source 端（服务提供方）和 Replica 端（客户端）的 C++ 类。Source 端持有真实的 QObject 实例，Replica 端拿到的是一个"远程副本"——这个副本和真实对象拥有完全相同的属性和信号，你读写属性就像操作本地对象一样，底层 Qt 自动完成跨进程的同步。更妙的是，如果两个进程不在同一台机器上，Qt Remote Objects 还支持通过 TCP 把对象共享出去，局域网内的另一台设备直接连上来就能操作你的对象。

这篇我们要做的是定义一个 REPC 接口文件，在 Source 端发布一个 QObject，在 Replica 端获取远程副本并监听属性变化，然后配置 TCP 传输实现跨设备共享。

## 2. 环境说明

本篇基于 Qt 6.9.1，需要引入 RemoteObjects 模块。CMake 配置如下：

```cmake
find_package(Qt6 REQUIRED COMPONENTS RemoteObjects)
```

Qt Remote Objects 从 Qt 5.10 开始提供，在 Qt 6 中属于附加模块。它的核心依赖是 Qt 的元对象系统（MOC）和信号槽机制——通过 REPC 文件定义接口，repc 编译器在构建时生成对应的 C++ 头文件和源文件，生成的代码利用 Q_PROPERTY 和信号槽在 Source 和 Replica 之间自动同步状态。传输层支持两种：QIODevice（用于本地进程间通信，比如 QLocalSocket）和 QTcpSocket（用于网络通信）。

工具链方面没有特殊要求：MSVC 2019+、GCC 11+、Clang 14+，C++17 标准，CMake 3.26+。repc 编译器随 Qt 安装包一起提供，不需要额外安装。在 CMake 中通过 `qt_add_repc_sources()` 和 `qt_add_repc_replicas()` 两个宏来调用 repc，它们会自动处理生成文件的路径和依赖关系。

## 3. 核心概念讲解

### 3.1 REPC 文件定义远程对象接口

REPC（Remote Objects Protocol Compiler）文件是 Qt Remote Objects 的核心——它用一种简洁的 DSL 描述远程对象的接口，包括属性（PROP）、信号（SIGNAL）和槽（SLOT）。你可以把它类比为 Web 开发里的 IDL（接口定义语言）或者 gRPC 的 .proto 文件，只不过它是专门为 Qt 的信号槽系统设计的。

下面是一个简单的 REPC 文件，定义了一个"传感器数据"远程对象：

```
class SensorData
{
    PROP(double temperature READONLY);
    PROP(double humidity READONLY);
    PROP(QString status);
    PROP(int updateInterval = 1000);

    SIGNAL(statusChanged(QString newStatus));
    SLOT(void reset());
};
```

把这段内容保存为 `sensordata.rep` 文件放在项目目录下。REPC 文件的语法有几个要点要搞清楚。PROP 声明属性，格式是 `PROP(类型 名称 修饰符)`——`READONLY` 表示这个属性只能由 Source 端修改，Replica 端只能读取；不加修饰符的属性是双向可读写的，Replica 端修改后自动同步回 Source 端；`= 默认值` 给属性指定初始值。SIGNAL 声明信号，语法和 Qt 的信号声明基本一致。SLOT 声明可以在 Replica 端远程调用的方法。

属性的类型必须是 Qt 元对象系统支持的类型——基本类型（int、double、QString 等）直接使用，自定义类型需要用 Q_DECLARE_METATYPE 注册并且在 REPC 文件中用 `INCLUDE` 指令引入对应的头文件。

### 3.2 QRemoteObjectHost 发布对象

Source 端负责创建真实的 QObject 实例并通过 QRemoteObjectHost 把它发布出去。QRemoteObjectHost 是远程对象的主机节点，它内部启动一个监听服务（可以是本地 socket 或 TCP socket），等待 Replica 端连接上来。

```cpp
#include <QRemoteObjectHost>
#include "rep_sensordata_source.h"

// 创建 Source 端节点，监听 TCP 端口
QRemoteObjectHost hostNode;
hostNode.setHostUrl(QUrl(QStringLiteral("tcp://0.0.0.0:5555")));

// 创建 Source 端对象实例
SensorDataSimpleSource sensorSource;
sensorSource.setTemperature(23.5);
sensorSource.setHumidity(65.0);
sensorSource.setStatus(QStringLiteral("Normal"));

// 将对象发布到节点上，标识符为 "sensor"
hostNode.enableRemoting(&sensorSource, QStringLiteral("sensor"));
```

这里有几个地方要注意。`rep_sensordata_source.h` 是 repc 编译器根据 sensordata.rep 自动生成的头文件，里面包含了 SensorDataSimpleSource 类——这个类继承自 QObject，已经把 REPC 文件里定义的所有 PROP、SIGNAL、SLOT 都声明好了。你只需要实例化它、设置属性值、连接信号槽就行。enableRemoting() 的第一个参数是 Source 对象指针，第二个参数是"远程对象名称"——Replica 端通过这个名称来查找和连接对象。这个名称在整个网络中必须唯一。

setHostUrl() 决定了传输层协议。`tcp://host:port` 使用 TCP socket，适合跨设备通信。`local://name` 使用 QLocalSocket，适合同一台机器上的进程间通信，延迟更低且不需要端口管理。如果你只是在本机做进程间通信，优先用 local。

### 3.3 QRemoteObjectNode 获取远程副本

Replica 端通过 QRemoteObjectNode 连接到 Source 端节点，然后通过 acquire() 获取远程对象的副本。获取到的 Replica 对象拥有和 Source 端完全相同的属性接口，但它背后不是一个真实的 QObject——它是一个"代理"，属性的读写在底层被自动转发到 Source 端。

```cpp
#include <QRemoteObjectNode>
#include "rep_sensordata_replica.h"

// 创建 Replica 端节点，连接到 Source 端
QRemoteObjectNode replicaNode;
replicaNode.connectToNode(QUrl(QStringLiteral("tcp://127.0.0.1:5555")));

// 获取远程对象的 Replica
QSharedPointer<SensorDataReplica> sensor
    = replicaNode.acquire<SensorDataReplica>(QStringLiteral("sensor"));

// 等待 Replica 初始化完成
sensor->waitForSource();
// 或者用异步方式：
// QObject::connect(sensor.data(), &SensorDataReplica::initialized, [&]() { ... });
```

acquire() 返回的是一个 QSharedPointer，这个 Replica 对象由 QRemoteObjectNode 管理，你不需要手动删除。waitForSource() 是同步等待——它会阻塞当前线程直到 Replica 从 Source 端获取到初始状态或者超时。如果你在 GUI 线程中调用，建议用异步方式，通过 initialized 信号来通知 Replica 就绪。

Replica 就绪后，你就可以像操作本地 QObject 一样使用它了：

```cpp
// 读取属性
qDebug() << "Temperature:" << sensor->temperature();
qDebug() << "Humidity:" << sensor->humidity();

// 写入可读写属性（会自动同步到 Source 端）
sensor->pushStatus(QStringLiteral("Alert"));

// 监听属性变化
QObject::connect(sensor.data(), &SensorDataReplica::temperatureChanged,
    [](double value) {
        qDebug() << "温度更新:" << value;
    });

// 调用远程槽函数
sensor->pushReset();
```

这里有一个细节需要理解：READONLY 属性只能从 Source 端推送到 Replica 端，Replica 端调用 setter 不会有任何效果。非 READONLY 的属性是双向同步的——Replica 端调用 setter 后，值会通过传输层发送到 Source 端更新真实对象的属性，然后 Source 端再把更新后的值广播给所有连接的 Replica。这就是为什么双向属性的同步延迟会比只读属性稍高一点。

### 3.4 局域网跨设备共享

如果你的 Source 端和 Replica 端运行在同一台机器的不同进程里，用 `local://` 协议就足够了。但如果你的场景是真正的跨设备通信——比如一台工控机上的 Qt 程序采集传感器数据发布出来，另一台开发机上运行的 Qt 程序实时显示这些数据——就需要走 TCP。配置上只需要改 setHostUrl() 和 connectToNode() 的 URL 参数，其余代码完全不变。

Source 端监听所有网卡：

```cpp
QRemoteObjectHost hostNode;
hostNode.setHostUrl(QUrl(QStringLiteral("tcp://0.0.0.0:5555")));
```

Replica 端连接到 Source 端的 IP 地址：

```cpp
QRemoteObjectNode replicaNode;
replicaNode.connectToNode(QUrl(QStringLiteral("tcp://192.168.1.100:5555")));
```

跨设备场景下有几点要格外注意。首先是网络延迟——局域网环境下 RTT 通常在 0.1-1ms 量级，属性同步的延迟可以忽略不计；但如果链路上有 Wi-Fi 或者跨了交换机，延迟可能到 5-10ms，对于实时控制场景就需要考虑延迟补偿。其次是连接断开重连——QRemoteObjectNode 在 TCP 连接中断后不会自动重连，你需要监听 error 信号并在检测到断开后重新调用 connectToNode()。最后是安全性——Qt Remote Objects 的 TCP 传输是明文的，不提供加密和认证。如果你的网络环境不可信，需要在前面加一层 VPN 或者 SSH 隧道。

## 4. 综合示例：传感器数据远程共享

我们把前面的内容整合成一个完整的示例。它包含一个 Source 端和一个 Replica 端：Source 端模拟传感器数据采集，每隔一秒更新温度和湿度值并通过 QRemoteObjectHost 发布出去；Replica 端连接到 Source 端获取实时数据并在控制台输出。为了简化演示，我们把两端写在同一个程序里，用 QTimer 模拟数据更新。

这个示例使用本地传输（`local://`），不需要启动两个独立的进程。如果你需要测试跨进程或者跨设备，只需要把 hostNode 和 replicaNode 拆到两个程序中，改 URL 为 `tcp://` 即可。

CMake 配置：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core RemoteObjects)

qt_add_executable(${PROJECT_NAME} main.cpp)

# 注册 REPC 文件的 Source 端代码生成
qt_add_repc_sources(${PROJECT_NAME} sensordata.rep)
# 注册 REPC 文件的 Replica 端代码生成
qt_add_repc_replicas(${PROJECT_NAME} sensordata.rep)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::RemoteObjects)
```

sensordata.rep 文件：

```
class SensorData
{
    PROP(double temperature READONLY);
    PROP(double humidity READONLY);
    PROP(QString status);
    PROP(int updateCount = 0);

    SIGNAL(statusChanged(QString newStatus));
    SLOT(void reset());
};
```

main.cpp 的完整代码：

```cpp
#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QRandomGenerator>
#include <QRemoteObjectHost>
#include <QRemoteObjectNode>

// repc 生成的头文件，放在构建目录中
#include "rep_sensordata_source.h"
#include "rep_sensordata_replica.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "Qt Remote Objects 传感器数据共享示例";
    qDebug() << "本示例演示 REPC 接口定义 + Source 发布 + Replica 同步";

    // ========================================
    // Source 端：模拟传感器数据采集
    // ========================================

    QRemoteObjectHost hostNode;
    // 使用本地传输，适合同机进程间通信
    // 跨设备场景改为 "tcp://0.0.0.0:5555"
    hostNode.setHostUrl(QUrl(QStringLiteral("local:sensordata")));

    SensorDataSimpleSource sensorSource;
    sensorSource.setTemperature(22.0);
    sensorSource.setHumidity(50.0);
    sensorSource.setStatus(QStringLiteral("Normal"));
    sensorSource.setUpdateCount(0);

    // 发布到远程对象节点，名称为 "sensor"
    hostNode.enableRemoting(&sensorSource, QStringLiteral("sensor"));

    // 连接 reset 槽——Replica 端调用 pushReset() 时触发
    QObject::connect(&sensorSource, &SensorDataSimpleSource::reset,
        [&sensorSource]() {
            qDebug() << "[Source] 收到远程 reset 请求，重置数据";
            sensorSource.setTemperature(22.0);
            sensorSource.setHumidity(50.0);
            sensorSource.setUpdateCount(0);
            sensorSource.setStatus(QStringLiteral("Reset"));
        });

    // 模拟传感器数据更新——每秒产生一次新数据
    QTimer updateTimer;
    int count = 0;
    QObject::connect(&updateTimer, &QTimer::timeout, [&]() {
        double temp = 20.0 + QRandomGenerator::global()->bounded(10.0);
        double hum = 40.0 + QRandomGenerator::global()->bounded(40.0);
        sensorSource.setTemperature(temp);
        sensorSource.setHumidity(hum);
        sensorSource.setUpdateCount(++count);

        // 超过阈值自动更新状态
        if (temp > 28.0) {
            sensorSource.setStatus(QStringLiteral("HighTemp"));
            emit sensorSource.statusChanged(
                QStringLiteral("HighTemp"));
        } else {
            sensorSource.setStatus(QStringLiteral("Normal"));
        }

        qDebug().noquote()
            << QStringLiteral("[Source] 更新: temp=%1 hum=%2 count=%3")
                   .arg(temp, 0, 'f', 1)
                   .arg(hum, 0, 'f', 1)
                   .arg(count);
    });
    updateTimer.start(1000);

    // ========================================
    // Replica 端：连接并监听远程数据
    // ========================================

    QRemoteObjectNode replicaNode;
    replicaNode.connectToNode(QUrl(QStringLiteral("local:sensordata")));

    QSharedPointer<SensorDataReplica> replica
        = replicaNode.acquire<SensorDataReplica>(
            QStringLiteral("sensor"));

    // 等待 Replica 初始化完成
    if (replica->waitForSource(3000)) {
        qDebug() << "[Replica] 已连接到远程传感器";
        qDebug().noquote()
            << QStringLiteral("[Replica] 初始数据: temp=%1 hum=%2 status=%3")
                   .arg(replica->temperature(), 0, 'f', 1)
                   .arg(replica->humidity(), 0, 'f', 1)
                   .arg(replica->status());

        // 监听温度变化
        QObject::connect(replica.data(),
            &SensorDataReplica::temperatureChanged,
            [](double temp) {
                qDebug().noquote()
                    << QStringLiteral("[Replica] 温度变化: %1").arg(temp, 0, 'f', 1);
            });

        // 监听状态变化信号（REPC 中定义的 SIGNAL）
        QObject::connect(replica.data(),
            &SensorDataReplica::statusChanged,
            [](const QString &newStatus) {
                qDebug() << "[Replica] 状态信号:" << newStatus;
            });

        // 3 秒后从 Replica 端触发 reset
        QTimer::singleShot(3000, [replica]() {
            qDebug() << "[Replica] 发送 reset 请求...";
            replica->pushReset();
        });

    } else {
        qWarning() << "[Replica] 等待 Source 超时";
    }

    return app.exec();
}
```

运行程序后你会看到 Source 端每秒更新一次温度和湿度数据，Replica 端实时接收到温度变化的通知。大约 3 秒后 Replica 端发送 reset 请求，Source 端收到后重置所有数据。控制台输出大致如下：

```
[Source] 更新: temp=24.3 hum=62.1 count=1
[Replica] 温度变化: 24.3
[Source] 更新: temp=29.1 hum=55.8 count=2
[Replica] 温度变化: 29.1
[Replica] 状态信号: "HighTemp"
...
[Replica] 发送 reset 请求...
[Source] 收到远程 reset 请求，重置数据
```

几个实现细节要解释一下。Source 端和 Replica 端写在同一个 main() 里只是为了演示方便——实际项目中它们一定是在不同的进程甚至不同的设备上。Source 端的 enableRemoting() 调用是幂等的——如果远程对象名称已经被注册，重复调用不会报错。replica->pushReset() 这个命名是 repc 生成的——对于 REPC 文件中定义的 SLOT(void reset())，Source 端的类有 reset() 信号，Replica 端的类有 pushReset() 方法（内部会触发 Source 端的 reset 信号）。这个 push 前缀是 repc 的命名约定，不用担心记不住，看一眼生成的头文件就知道了。

如果你要把这个示例改成跨进程运行，只需要把 Source 端和 Replica 端的代码分别放到两个 main.cpp 中。Source 端的 URL 改为 `tcp://0.0.0.0:5555`，Replica 端的 URL 改为 `tcp://Source端IP:5555`。每个程序只需要引用自己那一侧的生成头文件——Source 端 include `rep_sensordata_source.h`，Replica 端 include `rep_sensordata_replica.h`，REPC 文件在两端都需要。

## 5. 练习项目

练习项目：双进程远程配置面板。

在示例基础上拆分成两个独立进程：一个"设备端"进程运行 Source 端，模拟一个可配置的设备（包含设备名称、工作模式、采样频率、运行状态等属性，提供 start / stop / reboot 三个槽函数）；一个"控制端"进程运行 Replica 端，提供一个简单的命令行交互界面（输入命令查看属性、修改配置、调用远程方法）。

完成标准是这样的：两个进程分别编译运行后，控制端能实时看到设备端的状态变化（设备端用 QTimer 模拟状态切换），控制端修改采样频率后设备端立即生效，控制端调用 reboot 后设备端的运行计数归零并重置所有属性。

几个实现提示：REPC 文件中需要定义更多的属性和槽，注意区分哪些属性应该是 READONLY（比如运行计数）哪些应该是双向可写的（比如采样频率）。命令行交互可以用 QTextStream 从 stdin 逐行读取命令，用 if-else 或 QMap 映射到对应的 Replica 方法调用。设备端的状态切换可以用 QStateMachine 或者简单的 QTimer + 状态枚举来实现。

## 6. 官方文档参考

[Qt 文档 · Qt Remote Objects 模块](https://doc.qt.io/qt-6/qtremoteobjects-index.html) -- 远程对象模块总览与架构说明

[Qt 文档 · QRemoteObjectHost](https://doc.qt.io/qt-6/qremoteobjecthost.html) -- Source 端主机节点，发布远程对象

[Qt 文档 · QRemoteObjectNode](https://doc.qt.io/qt-6/qremoteobjectnode.html) -- Replica 端节点，连接并获取远程副本

[Qt 文档 · REPC 语法参考](https://doc.qt.io/qt-6/qtremoteobjects-repc.html) -- REPC 文件语法定义与代码生成规则

---

到这里就大功告成了。Qt Remote Objects 把进程间对象共享这件事做到了极致的简洁——定义一个 .rep 文件描述接口，Source 端 enableRemoting() 发布对象，Replica 端 acquire() 获取副本，剩下的属性同步、信号传递、远程方法调用全部由 Qt 框架在底层自动完成。你不需要手搓任何序列化代码，不需要维护消息协议版本，甚至不需要关心传输层是本地 socket 还是 TCP。对于一个需要在多进程或者多设备之间共享状态的 Qt 项目来说，这套方案的侵入性极低，接入成本几乎只有"写一个 .rep 文件"这么一点。
