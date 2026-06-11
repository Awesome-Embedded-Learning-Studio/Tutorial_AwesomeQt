/// @file    mqtt_client_demo.cpp
/// @brief   MqttClientDemo 实现，演示 QMqttClient 的 API 调用流程。
///
/// 对应教程：进阶层 05-其他模块/09-MQTT 高级。
/// 本文件中的所有 MQTT 操作通过控制台输出展示 API 用法，
/// 不依赖实际运行的 Broker。

#include "mqtt_client_demo.h"

#include <QMqttClient>
#include <QMqttTopicName>
#include <QMqttTopicFilter>
#include <QMqttSubscription>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QDebug>

// ─── 构造与析构 ────────────────────────────────────────────────────────────────

MqttClientDemo::MqttClientDemo(QObject* parent)
    : QObject(parent)
    , m_client(nullptr)
    , m_messageCount(0)
{
    // QMqttClient 默认使用 TCP 传输，构造时无需额外参数
    m_client = new QMqttClient(this);

    // 连接核心信号 —— 必须在 connectToHost() 之前完成，否则可能丢失首帧事件
    QObject::connect(m_client, &QMqttClient::connected,
                     this, &MqttClientDemo::onConnected);
    QObject::connect(m_client, &QMqttClient::disconnected,
                     this, &MqttClientDemo::onDisconnected);
    QObject::connect(m_client, &QMqttClient::stateChanged,
                     this, &MqttClientDemo::onStateChanged);
    QObject::connect(m_client, &QMqttClient::messageReceived,
                     this, &MqttClientDemo::onMessageReceived);
    QObject::connect(m_client, &QMqttClient::messageSent,
                     this, &MqttClientDemo::onMessageSent);
    QObject::connect(m_client, &QMqttClient::pingResponseReceived,
                     this, []() {
        qDebug() << "  [Ping] 收到 Broker 心跳响应，连接仍然活跃";
    });

    qDebug() << "[MqttClientDemo] 客户端对象已创建，等待配置...";
}

MqttClientDemo::~MqttClientDemo() = default;

// ─── 公有方法 ──────────────────────────────────────────────────────────────────

void MqttClientDemo::setupClient()
{
    printSection(QStringLiteral("Demo 1: 客户端配置"));

    // 基础连接参数 —— hostname 和 port 是连接 Broker 的最小必要信息
    m_client->setHostname(QStringLiteral("broker.example.com"));
    m_client->setPort(1883);

    // 客户端标识符 —— Broker 用此区分不同连接；为空时 Broker 会自动分配
    m_client->setClientId(QStringLiteral("QtMqttDemo_001"));

    // MQTT 协议版本 —— 使用 v3.1.1，兼容性最广泛
    m_client->setProtocolVersion(QMqttClient::MQTT_3_1_1);

    // 认证凭据 —— 如果 Broker 开启了 ACL，用户名/密码用于权限校验
    m_client->setUsername(QStringLiteral("device_user"));
    m_client->setPassword(QStringLiteral("secure_password"));

    // Keep Alive —— 客户端在此间隔内无数据传输时发送 PINGREQ，
    // 防止 Broker 因超时断开连接
    m_client->setKeepAlive(60);

    // Clean Session —— 为 true 时断开后 Broker 清除该客户端的所有订阅和离线消息
    m_client->setCleanSession(true);

    qDebug() << "  Hostname:        " << m_client->hostname();
    qDebug() << "  Port:            " << m_client->port();
    qDebug() << "  Client ID:       " << m_client->clientId();
    qDebug() << "  Protocol:        MQTT v3.1.1";
    qDebug() << "  Keep Alive:      " << m_client->keepAlive() << "s";
    qDebug() << "  Clean Session:   " << (m_client->cleanSession() ? "true" : "false");
    qDebug() << "";
}

void MqttClientDemo::setupWill()
{
    printSection(QStringLiteral("Demo 2: Last Will and Testament (LWT)"));

    const QString willTopic   = QStringLiteral("device/status");
    const QString willMessage = QStringLiteral("offline");
    const quint8  willQos     = 1;
    const bool    willRetain  = true;

    // LWT 配置 —— 必须在 connectToHost() 之前调用，因为 LWT 是 CONNECT 报文的一部分
    m_client->setWillTopic(willTopic);
    m_client->setWillMessage(willMessage.toUtf8());
    m_client->setWillQoS(willQos);
    m_client->setWillRetain(willRetain);

    qDebug() << "  Will Topic:      " << willTopic;
    qDebug() << "  Will Message:    " << willMessage;
    qDebug() << "  Will QoS:        " << willQos;
    qDebug() << "  Will Retain:     "
             << (willRetain ? "true (Broker 保存最后一条)" : "false");
    qDebug() << "";
    qDebug() << "  [说明] 当客户端异常断开时，Broker 将自动向";
    qDebug() << "         \"" << willTopic << "\" 发布 \"" << willMessage << "\"";
    qDebug() << "         订阅该主题的其他设备即可感知此设备已离线。";
    qDebug() << "";
}

void MqttClientDemo::demonstrateSubscription()
{
    printSection(QStringLiteral("Demo 3: 订阅操作（QoS 0 / 1 / 2）"));

    // 定义三组订阅主题及其 QoS 级别，展示不同保证语义
    // subscribe() 使用 QMqttTopicFilter（支持通配符 + 和 #）
    struct SubscriptionInfo {
        QString filter;
        quint8  qos;
        QString description;
    };

    const SubscriptionInfo subscriptions[] = {
        {QStringLiteral("sensor/temperature"), 0,
         QStringLiteral("QoS 0 -- 最多一次，允许丢失（如高频遥测数据）")},
        {QStringLiteral("device/command"),     1,
         QStringLiteral("QoS 1 -- 至少一次，不丢但可能重复（如控制指令）")},
        {QStringLiteral("system/critical"),    2,
         QStringLiteral("QoS 2 -- 恰好一次，既不丢也不重复（如固件升级通知）")}
    };

    for (const auto& sub : subscriptions) {
        qDebug() << "  [SUB] Filter:" << sub.filter
                 << " | QoS:" << sub.qos;
        qDebug() << "        " << sub.description;
    }

    qDebug() << "";
    qDebug() << "  [说明] QMqttClient::subscribe() 接受 QMqttTopicFilter，";
    qDebug() << "         返回 QMqttSubscription* 指针。连接其 messageReceived";
    qDebug() << "         信号即可处理该主题的消息。";
    qDebug() << "         通配符 \"+\" 匹配单层，\"#\" 匹配多层。";
    qDebug() << "         例：\"sensor/#\" 匹配 sensor/temperature、sensor/humidity 等。";
    qDebug() << "";
    qDebug() << "  [代码示例]";
    qDebug() << "    auto* sub = m_client->subscribe(";
    qDebug() << "        QMqttTopicFilter(\"sensor/#\"), 1);";
    qDebug() << "    connect(sub, &QMqttSubscription::messageReceived,";
    qDebug() << "            [](QMqttMessage msg) { ... });";
    qDebug() << "";
}

void MqttClientDemo::demonstratePublish()
{
    printSection(QStringLiteral("Demo 4: 发布操作（QoS 0 / 1 / 2）"));

    struct PublishInfo {
        QString topic;
        QString payload;
        quint8  qos;
        bool    retain;
        QString note;
    };

    const PublishInfo messages[] = {
        {QStringLiteral("sensor/temperature"),
         QStringLiteral("{\"temp\":23.5,\"unit\":\"C\"}"),
         0, false,
         QStringLiteral("QoS 0 火后不管，适合高频推送")},
        {QStringLiteral("device/command"),
         QStringLiteral("{\"action\":\"reboot\",\"delay\":5}"),
         1, false,
         QStringLiteral("QoS 1 确保送达，适合控制命令")},
        {QStringLiteral("system/critical"),
         QStringLiteral("{\"event\":\"firmware_update\",\"ver\":\"2.1.0\"}"),
         2, true,
         QStringLiteral("QoS 2 恰好一次 + Retain，新订阅者也会收到")}
    };

    for (const auto& msg : messages) {
        ++m_messageCount;

        qDebug() << "  [PUB #" << m_messageCount << "]"
                 << " Topic:" << msg.topic
                 << " | QoS:" << msg.qos
                 << " | Retain:" << (msg.retain ? "true" : "false");
        qDebug() << "           Payload:" << msg.payload;
        qDebug() << "           " << msg.note;
        qDebug() << "";

        // publish() 使用 QMqttTopicName（不支持通配符），返回 message ID
        // 实际发布代码（此处仅注释展示，因无 Broker）：
        // qint32 msgId = m_client->publish(
        //     QMqttTopicName(msg.topic),
        //     msg.payload.toUtf8(),
        //     msg.qos,
        //     msg.retain
        // );
        // m_pendingMessages.insert(msgId, msg.topic);
    }

    qDebug() << "  [说明] publish() 返回 message ID：";
    qDebug() << "         QoS 0 返回 0（无需确认），";
    qDebug() << "         QoS 1 返回非零 ID，Broker 回 PUBACK 时触发 messageSent，";
    qDebug() << "         QoS 2 返回非零 ID，两阶段握手（PUBREC + PUBCOMP）。";
    qDebug() << "";
}

void MqttClientDemo::demonstrateTlsSetup()
{
    printSection(QStringLiteral("Demo 5: TLS/SSL 安全连接"));

    // 默认 MQTT 端口 1883 为明文，TLS 加密使用 8883 端口
    const quint16 kTlsPort = 8883;

    // 准备 SSL 配置
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();

    // 要求验证 Broker 证书 —— 生产环境必须开启，测试环境可临时关闭
    sslConfig.setPeerVerifyMode(QSslSocket::AutoVerifyPeer);

    // 添加自定义 CA 证书（如企业内部 PKI）
    // sslConfig.addCaCertificates(QStringLiteral("/path/to/ca-cert.pem"));

    qDebug() << "  TLS Port:         " << kTlsPort;
    qDebug() << "  SSL Protocol:     TLS 1.2+";
    qDebug() << "  Peer Verify:      AutoVerifyPeer";
    qDebug() << "  CA Certificate:   System default";
    qDebug() << "";
    qDebug() << "  [说明] QMqttClient 的 TLS 连接方式：";
    qDebug() << "         1. 配置 QSslConfiguration（证书、协议版本等）";
    qDebug() << "         2. 调用 setPort(8883) 设置 TLS 端口";
    qDebug() << "         3. 调用 connectToHostEncrypted(sslConfig) 发起加密连接";
    qDebug() << "           或通过 setTransport() 注入自定义 QSslSocket";
    qDebug() << "";
    qDebug() << "  [代码示例]";
    qDebug() << "    m_client->setHostname(\"broker.example.com\");";
    qDebug() << "    m_client->setPort(8883);";
    qDebug() << "    QSslConfiguration ssl = QSslConfiguration::defaultConfiguration();";
    qDebug() << "    ssl.setPeerVerifyMode(QSslSocket::AutoVerifyPeer);";
    qDebug() << "    m_client->connectToHostEncrypted(ssl);";
    qDebug() << "";
    qDebug() << "  [替代方案] 手动创建传输层：";
    qDebug() << "    auto* sslSock = new QSslSocket(this);";
    qDebug() << "    sslSock->setSslConfiguration(sslConfig);";
    qDebug() << "    m_client->setTransport(sslSock, QMqttClient::SecureSocket);";
    qDebug() << "    m_client->connectToHost();";
    qDebug() << "";
    qDebug() << "  [警告] 生产环境必须使用 TLS，否则凭据和载荷以明文传输！";
    qDebug() << "";

    // 临时恢复端口（避免影响后续状态显示）
    Q_UNUSED(sslConfig)
}

void MqttClientDemo::printStatus() const
{
    printSection(QStringLiteral("客户端状态汇总"));

    const auto stateToString = [](QMqttClient::ClientState s) -> QString {
        switch (s) {
        case QMqttClient::Disconnected:
            return QStringLiteral("Disconnected");
        case QMqttClient::Connecting:
            return QStringLiteral("Connecting");
        case QMqttClient::Connected:
            return QStringLiteral("Connected");
        }
        return QStringLiteral("Unknown");
    };

    const auto errorToString = [](QMqttClient::ClientError e) -> QString {
        switch (e) {
        case QMqttClient::NoError:
            return QStringLiteral("NoError");
        case QMqttClient::InvalidProtocolVersion:
            return QStringLiteral("InvalidProtocolVersion");
        case QMqttClient::IdRejected:
            return QStringLiteral("IdRejected");
        case QMqttClient::ServerUnavailable:
            return QStringLiteral("ServerUnavailable");
        case QMqttClient::BadUsernameOrPassword:
            return QStringLiteral("BadUsernameOrPassword");
        case QMqttClient::NotAuthorized:
            return QStringLiteral("NotAuthorized");
        case QMqttClient::TransportInvalid:
            return QStringLiteral("TransportInvalid");
        case QMqttClient::ProtocolViolation:
            return QStringLiteral("ProtocolViolation");
        case QMqttClient::UnknownError:
            return QStringLiteral("UnknownError");
        case QMqttClient::Mqtt5SpecificError:
            return QStringLiteral("Mqtt5SpecificError");
        }
        return QStringLiteral("UnmappedError");
    };

    qDebug() << "  Client State:    " << stateToString(m_client->state());
    qDebug() << "  Last Error:      " << errorToString(m_client->error());
    qDebug() << "  Messages Sent:   " << m_messageCount;
    qDebug() << "  Pending Acks:    " << m_pendingMessages.size();
    qDebug() << "";
}

// ─── 私有槽函数 ────────────────────────────────────────────────────────────────

void MqttClientDemo::onConnected()
{
    qDebug() << "  [Signal] connected -- 已成功连接到 Broker";
    qDebug() << "  此处通常执行：订阅主题、发布上线状态消息";
}

void MqttClientDemo::onDisconnected()
{
    qDebug() << "  [Signal] disconnected -- 已与 Broker 断开连接";
    qDebug() << "  此处通常执行：清理资源、启动重连定时器";
}

void MqttClientDemo::onStateChanged(QMqttClient::ClientState state)
{
    // 状态转换日志有助于排查连接问题
    const char* stateName = "Unknown";
    switch (state) {
    case QMqttClient::Disconnected: stateName = "Disconnected"; break;
    case QMqttClient::Connecting:   stateName = "Connecting";   break;
    case QMqttClient::Connected:    stateName = "Connected";    break;
    }
    qDebug() << "  [Signal] stateChanged -- 新状态:" << stateName;
}

void MqttClientDemo::onMessageReceived(const QByteArray& message,
                                        const QMqttTopicName& topic)
{
    qDebug() << "  [Signal] messageReceived"
             << " | Topic:" << topic.name()
             << " | Payload:" << message;
}

void MqttClientDemo::onMessageSent(qint32 id)
{
    // messageSent 信号：Broker 确认已收到该消息（QoS 1 PUBACK / QoS 2 PUBCOMP）
    const QString topic = m_pendingMessages.value(id, QStringLiteral("<unknown>"));
    m_pendingMessages.remove(id);
    qDebug() << "  [Signal] messageSent -- msgId:" << id
             << " | topic:" << topic << " 已确认送达";
}

// ─── 私有辅助 ──────────────────────────────────────────────────────────────────

void MqttClientDemo::printSection(const QString& title)
{
    qDebug() << "";
    qDebug() << "=======================================================";
    qDebug() << " " << title;
    qDebug() << "=======================================================";
}
