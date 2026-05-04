# 现代Qt开发教程（新手篇）5.9——Qt MQTT 客户端基础

## 1. 前言：万物互联时代的消息总线

MQTT（Message Queuing Telemetry Transport）最初是 IBM 在 1999 年为油管远程监控设计的协议，设计目标是"在不可靠网络上用最少的带宽传输最关键的数据"。二十年过去了，它成了物联网领域的事实标准——智能家居、车联网、工业遥测、移动推送，几乎一切"小设备往云端报数据"的场景都在用 MQTT。原因和 Modbus 类似：协议极简、带宽占用小、实现成本低，但它在消息路由和可靠性方面的设计比 Modbus 现代得多。

MQTT 的核心模型是发布/订阅（Publish/Subscribe）。和传统的请求/响应模式不同，发布者不需要知道谁在接收消息——它只管往一个"话题"（Topic）上发消息，订阅了这个话题的所有客户端都会收到。中间有一个 Broker（消息代理）负责路由分发。这种解耦模型在物联网场景里非常实用：传感器只管发数据，后台服务只管收数据，两者通过 Broker 中转，互不依赖。

Qt 的 QtMqtt 模块提供了完整的 MQTT 5.0 和 3.1.1 客户端实现。这篇我们要做的是用 QMqttClient 连接 Broker、订阅话题、发布消息，把 MQTT 的核心交互流程全部走通。QoS 等级的区别和选择策略也会在实操中讲清楚——这三个等级（0/1/2）看起来简单，但选错了会导致消息丢失或者性能浪费，值得认真理解。

## 2. 环境说明

本篇基于 Qt 6.9.1，需要引入 `Qt6::Mqtt` 和 `Qt6::Widgets` 两个模块。CMake 配置如下：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core Mqtt Widgets)
```

QtMqtt 在 Qt 6 中是一个附加模块（Qt Add-On），在 Qt Installer 里需要单独勾选安装。某些发行版的包管理器中可能叫 `qt6-mqtt` 或类似名称。如果你从源码编译 Qt，QtMqtt 在 `qt5compat` 或者单独的 `qtmqtt` 仓库中。

你需要一个 MQTT Broker 来测试。最简单的选择是用 Mosquitto（开源 Broker），Linux 上一条命令就能装好：`sudo apt install mosquitto`，装完默认监听 `localhost:1883`。Windows 和 macOS 上也有对应的安装包。如果你不想在本地安装，也可以用一些公共测试 Broker（比如 `test.mosquitto.org`），不过公共 Broker 的稳定性和安全性无法保证，仅适合开发测试。

编译工具链方面，MSVC 2019+、GCC 11+ 均可，C++17 标准，CMake 3.26+ 构建系统。

## 3. 核心概念讲解

### 3.1 QMqttClient——连接 Broker 的入口

QMqttClient 是 QtMqtt 的核心类，封装了 MQTT 客户端的完整生命周期：建立连接、订阅话题、发布消息、接收消息、断开连接。创建 QMqttClient 时需要指定 Broker 的地址和端口：

```cpp
#include <QMqttClient>

auto *client = new QMqttClient(this);
client->setHostname("localhost");
client->setPort(1883);
```

QMqttClient 默认使用 MQTT 3.1.1 协议版本。如果你的 Broker 支持 MQTT 5.0 并且你想使用 5.0 的新特性（比如消息过期时间、原因码等），可以通过 setProtocolVersion() 切换：

```cpp
client->setProtocolVersion(QMqttClient::MQTT_5_0);
```

建立连接同样是异步的——调用 connectToHost() 后立即返回，连接结果通过 stateChanged 信号通知：

```cpp
connect(client, &QMqttClient::stateChanged, this,
        [](QMqttClient::ClientState state) {
    switch (state) {
    case QMqttClient::Connected:
        qDebug() << "已连接到 Broker";
        break;
    case QMqttClient::Disconnected:
        qDebug() << "已断开";
        break;
    case QMqttClient::Connecting:
        qDebug() << "正在连接...";
        break;
    default:
        break;
    }
});

client->connectToHost();
```

除了地址和端口，QMqttClient 还支持设置 ClientId（客户端标识符，Broker 用于区分不同客户端）、用户名/密码（如果 Broker 开启了认证）、WebSocket 传输模式等。ClientId 如果不设置，Qt 会自动生成一个 UUID。在某些 Broker 配置下，重复的 ClientId 会导致旧连接被踢掉——这在调试的时候可能会遇到。

断开连接用 disconnectFromHost()，同样是异步的：

```cpp
client->disconnectFromHost();
```

QMqttClient 还提供了一个 brokerSessionSubscribed() 信号，用于检测 Clean Session 为 false 时重连后 Broker 是否恢复了之前的订阅状态。这在需要持久会话的场景中很有用。

### 3.2 subscribe——订阅话题与 messageReceived 信号

连接成功之后，下一步是订阅感兴趣的话题。MQTT 的话题是一个用 `/` 分隔的层级路径，比如 `home/livingroom/temperature` 或者 `device/sensor01/status`。订阅时可以使用通配符：`+` 匹配单层，`#` 匹配多层。比如 `home/+/temperature` 会匹配 `home/livingroom/temperature` 和 `home/bedroom/temperature`，但不会匹配 `home/livingroom/humidity`；`home/#` 会匹配所有以 `home/` 开头的话题。

```cpp
// 订阅一个具体话题，QoS 级别 1
auto *subscription = client->subscribe(
    QMqttTopicFilter("home/livingroom/temperature"), 1);

if (!subscription) {
    qWarning() << "订阅失败";
}
```

subscribe() 返回一个 QMqttSubscription 指针。你可以通过这个指针获取订阅的状态（是否被 Broker 确认），也可以连接它的 messageReceived 信号来接收匹配该话题的消息。不过更常见的做法是直接用 QMqttClient 的 messageReceived 信号——它会转发所有已订阅话题的消息：

```cpp
connect(client, &QMqttClient::messageReceived, this,
        [](const QByteArray &message,
           const QMqttTopicName &topic) {
    qDebug() << "收到消息:" << topic.name() << message;
});
```

messageReceived 的两个参数分别是消息内容和话题名称。话题名称是 Broker 转发时附带的实际话题（不是订阅时的通配符），你可以根据话题名称来区分消息来源。

取消订阅用 unsubscribe()：

```cpp
client->unsubscribe(QMqttTopicFilter("home/livingroom/temperature"));
```

### 3.3 publish——发布消息

发布消息用 publish() 方法，需要指定话题、消息内容和 QoS 等级：

```cpp
// 发布消息，QoS 0
client->publish(
    QMqttTopicName("home/livingroom/temperature"),
    QByteArray("23.5"), 0);

// 发布消息，QoS 1
client->publish(
    QMqttTopicName("device/sensor01/status"),
    QByteArray("{\"online\": true}"), 1);
```

publish() 返回一个 message ID。如果 QoS 等级大于 0，这个 ID 可以用来追踪消息的确认状态。QMqttClient 提供了 messageSent() 和 messageStatusChanged() 信号来通知消息的发送状态——QoS 1 下 messageSent 表示 Broker 已确认收到，QoS 2 下还有额外的投递完成确认。

消息内容是 QByteArray，也就是说你可以发任何二进制数据——纯文本、JSON、Protobuf、甚至图片都行。MQTT 协议本身不关心消息体的格式，格式由你的应用层约定。

### 3.4 QoS 等级 0/1/2——可靠性与性能的权衡

MQTT 定义了三个 QoS（Quality of Service）等级，控制消息从发布者到 Broker 再到订阅者的投递可靠性。选择合适的 QoS 等级是 MQTT 使用中最关键的决策之一。

QoS 0（最多一次，At most once）：消息发出就不管了。发布者把消息发给 Broker，不等待确认，Broker 把消息转发给订阅者，也不等待确认。如果网络丢包，消息就丢了。优点是最低的带宽占用和延迟，适合高频且允许偶尔丢失的数据——比如传感器每秒上报一次温度，丢一两个采样点完全不影响业务。

QoS 1（至少一次，At least once）：消息保证至少到达一次。发布者发送消息后等待 Broker 的 PUBACK 确认，如果在超时时间内没有收到确认就重发。这意味着在网络不稳定的情况下，订阅者可能收到重复的消息。你的业务逻辑需要自己处理去重。适合不能丢但能容忍偶尔重复的场景——比如设备状态变更通知、告警消息。

QoS 2（恰好一次，Exactly once）：消息保证恰好到达一次。通过四步握手（PUBLISH -> PUBREC -> PUBREL -> PUBCOMP）实现，确保既不丢也不重复。缺点是开销最大——每条消息需要四次报文交换，延迟和带宽占用都显著增加。适合不能丢也不能重复的关键数据——比如计费数据、指令确认。

实际项目中，大部分物联网场景用 QoS 0 或 QoS 1 就够了。QoS 2 很少使用——如果你真的需要"恰好一次"语义，通常在应用层用唯一 ID 做幂等处理比依赖 QoS 2 更可靠。

需要注意的是，MQTT 的 QoS 是分段的：发布者到 Broker 一段，Broker 到订阅者另一段，两段独立协商。比如发布者用 QoS 2 发送，订阅者用 QoS 1 订阅，Broker 收到后会以 QoS 1 转发给订阅者——消息的最终 QoS 取两段中的较低值。

## 4. 综合示例：MQTT 消息调试工具

把前面学的串起来，我们写一个 MQTT 消息调试工具。程序提供连接 Broker 的配置界面，支持订阅话题和发布消息，所有收到的消息显示在日志区。你可以用它来调试任何 MQTT 系统——只需要运行一个 Mosquitto Broker 就能实际测试发布和订阅的完整流程。

完整代码见 `examples/beginner/05-other-modules/09-qtmqtt-beginner/`，下面是关键部分的讲解。

CMake 配置：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core Mqtt Widgets)
# ...
target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Mqtt Qt6::Widgets)
```

程序界面上方是连接配置和订阅区，中部是发布消息区，下方是消息日志。

连接 Broker 的核心代码：

```cpp
client_ = new QMqttClient(this);
client_->setHostname(host_edit_->text());
client_->setPort(port_spin_->value());

// 可选：设置 ClientId
client_->setClientId("qt-mqtt-example");

connect(client_, &QMqttClient::stateChanged, this,
        &MqttWindow::onStateChanged);
connect(client_, &QMqttClient::messageReceived, this,
        &MqttWindow::onMessageReceived);

client_->connectToHost();
```

订阅话题：

```cpp
void onSubscribe()
{
    QString topic = sub_topic_edit_->text();
    int qos = sub_qos_combo_->currentIndex();
    auto *sub = client_->subscribe(
        QMqttTopicFilter(topic), qos);
    if (sub) {
        appendLog("已订阅: " + topic
                  + " (QoS " + QString::number(qos) + ")");
    } else {
        appendLog("订阅失败: " + topic);
    }
}
```

发布消息：

```cpp
void onPublish()
{
    QString topic = pub_topic_edit_->text();
    QByteArray payload = pub_payload_edit_->toPlainText().toUtf8();
    int qos = pub_qos_combo_->currentIndex();

    client_->publish(QMqttTopicName(topic), payload, qos);
    appendLog("已发布 -> " + topic
              + " [" + QString::number(payload.size()) + " bytes]"
              + " (QoS " + QString::number(qos) + ")");
}
```

消息接收：

```cpp
void onMessageReceived(const QByteArray &message,
                       const QMqttTopicName &topic)
{
    appendLog("收到 <- " + topic.name()
              + ": " + QString::fromUtf8(message));
}
```

运行程序后，先连接到本地 Mosquitto Broker（localhost:1883），然后在订阅区输入一个话题（比如 `test/topic`）点订阅，在发布区输入同一个话题和一条消息文本点发布。你会看到消息出现在日志区——消息从你的发布端发出，经过 Broker 路由，再回到你的订阅端接收。如果你打开两个程序实例，分别用不同的话题订阅，可以模拟多客户端的发布/订阅交互。

## 5. 练习项目

练习项目：MQTT 设备遥测面板。

我们要做一个简单的设备遥测数据展示面板。假设有多个传感器通过 MQTT 上报温度和湿度数据，消息格式为 JSON（如 `{"temperature": 23.5, "humidity": 65.2}`），话题格式为 `device/{deviceId}/telemetry`。

完成标准是这样的：程序订阅 `device/+/telemetry` 通配话题，收到消息后解析 JSON，用 QLabel 显示每个设备的最新温湿度数据；设备列表动态更新——发现新设备就添加到界面，已有设备就更新数值；如果某个设备超过 30 秒没有新数据，对应的显示区域变为灰色表示离线；使用 QJsonDocument 解析 JSON，使用 QTimer 检查设备超时。

几个实现提示：用 QMap 或 QHash 以 deviceId 为键存储设备数据和时间戳，消息到达时更新对应条目并刷新显示；QTimer 每 5 秒检查一次所有设备的最后活跃时间，超过 30 秒的标记为离线；话题名用 split('/') 分割提取 deviceId；QVBoxLayout 动态添加/更新每个设备的 QLabel 组。

## 6. 官方文档参考

[Qt 文档 · QtMqtt 模块](https://doc.qt.io/qt-6/qtmqtt-index.html) -- MQTT 模块总览

[Qt 文档 · QMqttClient](https://doc.qt.io/qt-6/qmqttclient.html) -- MQTT 客户端核心类

[Qt 文档 · QMqttSubscription](https://doc.qt.io/qt-6/qmqttsubscription.html) -- 订阅对象

[Qt 文档 · QMqttMessage](https://doc.qt.io/qt-6/qmqttmessage.html) -- MQTT 消息封装

*(链接已验证，2026-04-23 可访问)*

---

到这里就大功告成了。MQTT 的核心交互流程其实很简单：QMqttClient 连接 Broker，subscribe 订阅话题，publish 发布消息，messageReceived 信号接收消息。四个步骤走完，你就掌握了 MQTT 客户端开发的基础。QoS 等级的选择是实际项目中最需要认真考虑的部分——大部分物联网场景用 QoS 0 或 1 就够了，别为了"可靠性"无脑上 QoS 2，那个四步握手的开销在大量消息的场景下会显著影响吞吐。把发布/订阅模型和 QoS 等级搞清楚之后，对接任何 MQTT 系统都不是问题。

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
