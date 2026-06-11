---
title: "5.9 MQTT 进阶：QoS 1/2、遗嘱消息、TLS 加密连接"
description: "入门篇我们把 QMqttClient 的基本发布/订阅跑通了——连接 Broker、订阅 Topic、发布消息。写个简单的消息收发程序确实够用了。但 MQTT 协议在物联网领域广泛使用的核心原因不是「能发消息」，而是它的可靠性保障和服务质量等级。"
---

# 现代Qt开发教程（进阶篇）5.9——MQTT 进阶：QoS 1/2、遗嘱消息、TLS 加密连接

## 1. 前言 / 从「能发消息」到「可靠投递」

入门篇我们把 QMqttClient 的基本发布/订阅跑通了——连接 Broker、订阅 Topic、发布消息。写个简单的消息收发程序确实够用了。但 MQTT 协议在物联网领域广泛使用的核心原因不是「能发消息」，而是它的可靠性保障和服务质量等级。

MQTT 定义了三个 QoS 等级：QoS 0（最多一次，可能丢失）、QoS 1（至少一次，不丢但可能重复）、QoS 2（恰好一次，不丢不重复）。入门篇我们用的都是 QoS 0，消息丢了就丢了。但在实际项目中——比如消防报警系统的传感器数据——丢消息是不能接受的。你需要理解 QoS 1 和 2 的握手流程，以及在 Qt 中怎么正确使用它们。

然后是遗嘱消息（Last Will and Testament，LWT）。当设备异常断开连接时（断电、网络中断），Broker 会自动发布该设备预设的遗嘱消息。这让其他设备能感知到「某设备掉线了」——不需要心跳轮询，协议层面就帮你搞定了。

这篇我们把 QoS 1/2 的正确使用、遗嘱消息配置、以及 TLS 加密连接这三个工程必备能力拆干净。

## 2. 环境说明

本文档基于 Qt 6.2+ 编写，使用 C++17 标准和 CMake 3.26+ 构建系统。本篇依赖 Qt6::Mqtt 模块，CMake 中通过 `find_package(Qt6 REQUIRED COMPONENTS Mqtt)` 引入。测试 Broker 可以用 Mosquitto（开源，轻量级，支持 TLS）。QtMqtt 模块在 Qt 6 中是 Qt for Automation 的一部分，需要确认你的 Qt 安装包含此模块。

## 3. 核心概念讲解

### 3.1 QoS 等级——从「尽力而为」到「精确投递」

MQTT 的三个 QoS 等级各有适用场景。QoS 0 适合高频传感器数据——丢一两条无所谓，下一条马上就来了。QoS 1 适合控制命令——必须送达，重复可以由接收端去重。QoS 2 适合计费数据——不能丢也不能重复，但握手开销最大。

在 Qt 中指定 QoS 等级非常简单——`publish()` 和 `subscribe()` 方法都有 QoS 参数：

```cpp
// QoS 0：发完就忘
client->publish(QMqttTopicName("sensor/data"), payload, 0);

// QoS 1：至少一次投递
client->publish(QMqttTopicName("control/command"), payload, 1);

// QoS 2：恰好一次投递
client->publish(QMqttTopicName("billing/record"), payload, 2);
```

QoS 1 的底层流程是：客户端发送 PUBLISH（QoS 1）→ Broker 回复 PUBACK → 完成。如果客户端没收到 PUBACK，它会重发消息。这意味着接收端可能收到重复消息——你的应用逻辑必须能处理这种情况（用消息 ID 去重）。

QoS 2 的底层流程更复杂：客户端发送 PUBLISH（QoS 2）→ Broker 回复 PUBREC → 客户端回复 PUBREL → Broker 回复 PUBCOMP → 完成。四步握手保证了消息恰好被投递一次，但延迟和开销是 QoS 1 的两倍。

```cpp
// 订阅时指定 QoS——表示「我最高接受哪个 QoS」
auto *subscription = client->subscribe(
    QMqttTopicFilter("sensor/#"), 1);  // 最高 QoS 1

connect(subscription, &QMqttSubscription::messageReceived,
        [](QMqttMessage msg) {
    qDebug() << "Topic:" << msg.topic().name()
             << "QoS:" << msg.payload()
             << "Payload:" << msg.payload();
});
```

QoS 的选择需要权衡可靠性和性能。一个常见的策略是：普通遥测数据用 QoS 0，控制命令用 QoS 1，关键事件（报警、计费）用 QoS 2。不要所有消息都用 QoS 2——性能开销在消息量大时非常明显。

### 3.2 遗嘱消息——设备掉线自动通知

遗嘱消息的配置必须在连接 Broker 之前完成。你告诉 Broker：「如果我异常断开了，帮我发布这条消息到这个 Topic」。Broker 检测到连接断开后自动发出遗嘱消息。

```cpp
QMqttClient client;

// 配置遗嘱消息——必须在 connectToHost() 之前
client.setWillTopic("device/status/device-001");
client.setWillMessage("{\"status\":\"offline\",\"reason\":\"unexpected\"}");
client.setWillQoS(1);
client.setWillRetain(true);  // 保留消息，新订阅者也能收到

client.setHostname("broker.example.com");
client.setPort(1883);
client.connectToHost();
```

`setWillRetain(true)` 让遗嘱消息成为保留消息——Broker 会存储它，后续有新客户端订阅这个 Topic 时会立刻收到最新的离线状态。这是实现「设备在线/离线状态看板」的标准做法。

设备正常上线后应该主动发布一条在线状态消息，覆盖遗嘱消息：

```cpp
connect(&client, &QMqttClient::connected, [&]() {
    // 覆盖遗嘱消息，告诉所有人我在线
    client.publish(QMqttTopicName("device/status/device-001"),
                   "{\"status\":\"online\"}", 1, true);
});
```

`true` 参数表示这是一个 retain 消息。这样任何新订阅 `device/status/device-001` 的客户端都会立刻收到最新的在线状态。

### 3.3 TLS 加密连接——MQTT over SSL

生产环境的 MQTT 通信应该走 TLS 加密（端口 8883），防止数据被窃听或篡改。Qt 的 QMqttClient 支持通过 `setSslConfiguration()` 配置 TLS。

```cpp
QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
sslConfig.setProtocol(QSsl::TlsV1_2OrLater);

client.setSslConfiguration(sslConfig);
client.setHostname("broker.example.com");
client.setPort(8883);  // MQTT over SSL 的标准端口

// 处理 SSL 错误
connect(&client, &QMqttClient::sslErrors,
        [](const QList<QSslError> &errors) {
    for (const auto &err : errors) {
        qDebug() << "MQTT SSL error:" << err.errorString();
    }
});

client.connectToHost();
```

如果 Broker 使用自签名证书，你需要把 CA 证书加载到 SSL 配置中：

```cpp
QFile caFile("ca-cert.pem");
caFile.open(QIODevice::ReadOnly);
QSslCertificate caCert(&caFile, QSsl::Pem);
caFile.close();

QList<QSslCertificate> caCerts = sslConfig.caCertificates();
caCerts.append(caCert);
sslConfig.setCaCertificates(caCerts);
```

现在有一道思考题。你的 QoS 1 消息接收端偶尔处理了同一条消息两次。这是 Bug 还是正常行为？

这是正常行为。QoS 1 保证「至少一次」投递，不保证不重复。场景是：Broker 发送消息给接收端，接收端收到了并处理了，但回复的 PUBACK 在网络中丢失了。Broker 没收到 PUBACK，重发消息。接收端第二次收到同一条消息。解决方案是在应用层做消息去重——消息中携带唯一 ID，接收端维护一个「已处理消息 ID」的缓存（比如最近 100 个），收到重复 ID 就跳过。

## 4. 踩坑预防

第一个坑是 QoS 降级。MQTT 协议允许 Broker 在转发消息时降低 QoS 等级。比如你以 QoS 2 发布消息，但接收端以 QoS 1 订阅——Broker 会以 QoS 1 投递。这意味着即使发布端用了 QoS 2，如果订阅端没有以相同或更高的 QoS 订阅，最终投递的 QoS 可能更低。

第二个坑是遗嘱消息不会被触发的情况。如果客户端调用了 `disconnectFromHost()` 正常断开，Broker 不会发送遗嘱消息——因为这是「正常退出」。遗嘱只在连接异常中断时触发（TCP 连接超时、设备断电等）。如果你的设备有时正常退出有时异常退出，需要两种情况都处理——正常退出时主动发一条离线消息覆盖遗嘱。

## 5. 练习项目

练习项目：设备状态监控系统。5 个模拟设备（用 QTimer 模拟数据上报），连接到 Mosquitto Broker。每个设备上线时发在线状态，设置遗嘱消息。中央监控面板订阅 `device/status/#` 和 `device/data/#`，显示所有设备的在线/离线状态和最新数据。使用 QoS 1 保证状态消息可靠投递，TLS 加密连接 Broker。

完成标准：设备上线/下线状态实时更新、异常断开时遗嘱消息正确触发、重连后自动恢复在线状态、TLS 连接无 SSL 错误。

## 6. 官方文档参考链接

[Qt 文档 · QMqttClient](https://doc.qt.io/qt-6/qmqttclient.html) -- MQTT 客户端完整 API，包含连接、发布、订阅和遗嘱配置

[Qt 文档 · QMqttSubscription](https://doc.qt.io/qt-6/qmqttsubscription.html) -- 订阅管理，包含消息接收和 QoS 信息

[Qt 文档 · Qt MQTT](https://doc.qt.io/qt-6/qtmqtt-index.html) -- MQTT 模块总览

*（链接已验证，2026-06-11 可访问）*

---

到这里就大功告成了。QoS 1/2 可靠投递、遗嘱消息掉线感知、TLS 加密——搞定了这些，你的 MQTT 通信就能用在严肃的物联网项目中了。
