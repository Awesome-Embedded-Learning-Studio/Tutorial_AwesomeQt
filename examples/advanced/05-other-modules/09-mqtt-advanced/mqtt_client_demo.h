/// @file    mqtt_client_demo.h
/// @brief   MQTT 客户端封装，演示 QoS 级别、LWT 遗嘱消息与发布/订阅流程。
///
/// 对应教程：进阶层 05-其他模块/09-MQTT 高级。
/// 本示例不依赖实际 MQTT Broker，通过控制台输出展示 API 用法与状态流转。

#pragma once

#include <QObject>
#include <QMap>
#include <QTimer>
#include <QMqttClient>
#include <QMqttTopicName>
#include <QMqttTopicFilter>

/// @brief MQTT 客户端演示类，展示 QMqttClient 的核心 API 用法。
///
/// 涵盖：客户端配置、Last Will and Testament (LWT)、
/// 三级 QoS 订阅/发布、TLS/SSL 连接设置。
/// 由于教学环境无 Broker，所有操作通过控制台输出展示 API 调用方式与参数含义。
class MqttClientDemo : public QObject
{
    Q_OBJECT

public:
    /// @brief 构造函数，创建并初始化 QMqttClient 实例。
    /// @param[in] parent 父对象指针，Qt 对象树管理生命周期。
    explicit MqttClientDemo(QObject* parent = nullptr);

    /// @brief 析构函数。
    ~MqttClientDemo() override;

    // 禁止拷贝与赋值
    MqttClientDemo(const MqttClientDemo&) = delete;
    MqttClientDemo& operator=(const MqttClientDemo&) = delete;

    /// @brief 配置 MQTT Broker 连接参数（主机名、端口、凭据）。
    /// @note 实际生产环境中，密码应通过安全途径获取，不应硬编码。
    void setupClient();

    /// @brief 配置 Last Will and Testament（遗嘱消息）。
    ///
    /// 当客户端异常断开时，Broker 会自动向 LWT 主题发布预设消息，
    /// 其他订阅者可以据此感知设备离线。
    /// @note LWT 必须在 connectToHost() 之前设置，连接建立后无法修改。
    void setupWill();

    /// @brief 演示三种 QoS 级别的订阅操作。
    ///
    /// - QoS 0：最多一次（at most once），消息可能丢失。
    /// - QoS 1：至少一次（at least once），消息不会丢失但可能重复。
    /// - QoS 2：恰好一次（exactly once），既不丢失也不重复。
    /// @note 实际 QoS 取订阅者与发布者中较低的那个。
    void demonstrateSubscription();

    /// @brief 演示三种 QoS 级别的消息发布操作。
    ///
    /// 展示如何通过 publish() 发送不同 QoS 等级的消息，
    /// 以及如何处理 publish 返回的 message ID 用于追踪送达状态。
    void demonstratePublish();

    /// @brief 演示 TLS/SSL 安全连接配置。
    ///
    /// 展示如何通过 connectToHostEncrypted() 和 QSslConfiguration
    /// 建立加密的 MQTT 连接，包括证书验证与 SSL 协议版本选择。
    /// @note 生产环境必须使用 TLS 加密，否则凭据和载荷以明文传输。
    void demonstrateTlsSetup();

    /// @brief 打印当前客户端状态汇总。
    void printStatus() const;

private slots:
    /// @brief 处理 Broker 连接成功信号。
    void onConnected();

    /// @brief 处理与 Broker 断开连接的信号。
    void onDisconnected();

    /// @brief 处理状态变更信号。
    /// @param[in] state 新的客户端状态。
    void onStateChanged(QMqttClient::ClientState state);

    /// @brief 处理收到的 MQTT 消息。
    /// @param[in] message 消息载荷。
    /// @param[in] topic   消息主题。
    void onMessageReceived(const QByteArray& message,
                           const QMqttTopicName& topic);

    /// @brief 处理消息发送确认信号（Broker 已接收）。
    /// @param[in] id 已确认的消息 ID。
    void onMessageSent(qint32 id);

private:
    /// @brief 打印分隔线，美化控制台输出。
    /// @param[in] title 分隔线标题文本。
    static void printSection(const QString& title);

    QMqttClient* m_client;                     ///< MQTT 客户端实例
    QMap<qint32, QString> m_pendingMessages;   ///< 等待确认的消息（msgId -> topic）
    int m_messageCount;                        ///< 已发布消息计数
};
