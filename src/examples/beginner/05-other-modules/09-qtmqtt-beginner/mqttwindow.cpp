/**
 * Qt MQTT 客户端基础示例
 *
 * 本示例演示 QtMqtt 模块的核心用法：
 * 1. QMqttClient 连接 Broker（hostname / port / clientId）
 * 2. subscribe() 订阅话题（支持通配符 + 和 #）
 * 3. publish() 发布消息（指定话题、载荷、QoS）
 * 4. messageReceived 信号接收订阅消息
 *
 * 测试方式：运行本地 Mosquitto Broker (localhost:1883)，
 * 或连接公共测试 Broker（如 test.mosquitto.org）。
 */

#include <QApplication>
#include <QByteArray>
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMqttClient>
#include <QMqttSubscription>
#include <QMqttTopicFilter>
#include <QMqttTopicName>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QSplitter>
#include <QTextEdit>
#include <QTime>
#include <QVBoxLayout>

#include "mqttwindow.h"

// ============================================================================
// 主窗口：MQTT 消息调试工具
// ============================================================================
MqttWindow::MqttWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Qt MQTT 客户端基础示例");
    resize(700, 550);

    auto *central = new QWidget(this);
    auto *main_layout = new QVBoxLayout(central);

    // ---- 连接配置区 ----
    auto *conn_widget = new QWidget(this);
    auto *conn_layout = new QHBoxLayout(conn_widget);

    conn_layout->addWidget(new QLabel("Broker:", this));
    host_edit_ = new QComboBox(this);
    host_edit_->setEditable(true);
    host_edit_->addItems({"localhost", "127.0.0.1",
                          "test.mosquitto.org"});
    conn_layout->addWidget(host_edit_, 1);

    conn_layout->addWidget(new QLabel("端口:", this));
    port_spin_ = new QSpinBox(this);
    port_spin_->setRange(1, 65535);
    port_spin_->setValue(1883);
    conn_layout->addWidget(port_spin_);

    conn_layout->addWidget(new QLabel("ClientId:", this));
    client_id_edit_ = new QTextEdit(this);
    client_id_edit_->setMaximumHeight(30);
    client_id_edit_->setPlaceholderText("留空自动生成");
    conn_layout->addWidget(client_id_edit_, 1);

    connect_btn_ = new QPushButton("连接", this);
    disconnect_btn_ = new QPushButton("断开", this);
    disconnect_btn_->setEnabled(false);
    conn_layout->addWidget(connect_btn_);
    conn_layout->addWidget(disconnect_btn_);

    main_layout->addWidget(conn_widget);

    // ---- 状态栏 ----
    status_label_ = new QLabel("状态：未连接", this);
    status_label_->setStyleSheet("font-weight: bold;");
    main_layout->addWidget(status_label_);

    // ---- 中间区域：订阅 + 发布 并排 ----
    auto *mid_splitter = new QSplitter(Qt::Horizontal, this);

    // -- 订阅区 --
    auto *sub_widget = new QWidget(this);
    auto *sub_layout = new QVBoxLayout(sub_widget);
    sub_layout->addWidget(new QLabel("订阅话题:", this));

    sub_topic_edit_ = new QTextEdit(this);
    sub_topic_edit_->setMaximumHeight(30);
    sub_topic_edit_->setPlaceholderText(
        "例如: test/topic 或 device/+/telemetry");
    sub_layout->addWidget(sub_topic_edit_);

    auto *sub_ctrl = new QHBoxLayout();
    sub_ctrl->addWidget(new QLabel("QoS:", this));
    sub_qos_combo_ = new QComboBox(this);
    sub_qos_combo_->addItems({"0", "1", "2"});
    sub_qos_combo_->setCurrentIndex(0);
    sub_ctrl->addWidget(sub_qos_combo_);

    auto *sub_btn = new QPushButton("订阅", this);
    auto *unsub_btn = new QPushButton("取消订阅", this);
    sub_ctrl->addWidget(sub_btn);
    sub_ctrl->addWidget(unsub_btn);
    sub_layout->addLayout(sub_ctrl);

    sub_layout->addWidget(new QLabel("已订阅话题:", this));
    subscribed_list_ = new QPlainTextEdit(this);
    subscribed_list_->setReadOnly(true);
    subscribed_list_->setMaximumHeight(60);
    sub_layout->addWidget(subscribed_list_);

    mid_splitter->addWidget(sub_widget);

    // -- 发布区 --
    auto *pub_widget = new QWidget(this);
    auto *pub_layout = new QVBoxLayout(pub_widget);
    pub_layout->addWidget(new QLabel("发布消息:", this));

    pub_topic_edit_ = new QTextEdit(this);
    pub_topic_edit_->setMaximumHeight(30);
    pub_topic_edit_->setPlaceholderText("目标话题");
    pub_layout->addWidget(pub_topic_edit_);

    pub_payload_edit_ = new QTextEdit(this);
    pub_payload_edit_->setMaximumHeight(80);
    pub_payload_edit_->setPlaceholderText("消息内容");
    pub_layout->addWidget(pub_payload_edit_, 1);

    auto *pub_ctrl = new QHBoxLayout();
    pub_ctrl->addWidget(new QLabel("QoS:", this));
    pub_qos_combo_ = new QComboBox(this);
    pub_qos_combo_->addItems({"0", "1", "2"});
    pub_qos_combo_->setCurrentIndex(0);
    pub_ctrl->addWidget(pub_qos_combo_);

    auto *pub_btn = new QPushButton("发布", this);
    pub_ctrl->addWidget(pub_btn);
    pub_layout->addLayout(pub_ctrl);

    mid_splitter->addWidget(pub_widget);

    mid_splitter->setSizes({350, 350});
    main_layout->addWidget(mid_splitter, 1);

    // ---- 日志输出区 ----
    main_layout->addWidget(new QLabel("消息日志:", this));
    log_edit_ = new QPlainTextEdit(this);
    log_edit_->setReadOnly(true);
    log_edit_->setMaximumBlockCount(300);
    main_layout->addWidget(log_edit_, 2);

    // 日志清空按钮
    auto *bottom_layout = new QHBoxLayout();
    auto *clear_btn = new QPushButton("清空日志", this);
    bottom_layout->addStretch();
    bottom_layout->addWidget(clear_btn);
    main_layout->addLayout(bottom_layout);

    setCentralWidget(central);

    // ---- 创建 QMqttClient ----
    client_ = new QMqttClient(this);

    // 监听状态变化
    connect(client_, &QMqttClient::stateChanged, this,
            &MqttWindow::onStateChanged);

    // 监听收到的消息
    connect(client_, &QMqttClient::messageReceived, this,
            &MqttWindow::onMessageReceived);

    // 监听连接错误
    connect(client_, &QMqttClient::errorChanged, this,
            [this](QMqttClient::ClientError err) {
        appendLog("错误码: " + QString::number(err));
    });

    // ---- 按钮信号槽 ----
    connect(connect_btn_, &QPushButton::clicked, this,
            &MqttWindow::onConnect);
    connect(disconnect_btn_, &QPushButton::clicked, this,
            &MqttWindow::onDisconnect);
    connect(sub_btn, &QPushButton::clicked, this,
            &MqttWindow::onSubscribe);
    connect(unsub_btn, &QPushButton::clicked, this,
            &MqttWindow::onUnsubscribe);
    connect(pub_btn, &QPushButton::clicked, this,
            &MqttWindow::onPublish);
    connect(clear_btn, &QPushButton::clicked, this,
            [this]() { log_edit_->clear(); });
}

/// 追加日志
void MqttWindow::appendLog(const QString &msg)
{
    log_edit_->appendPlainText(
        QTime::currentTime().toString("[HH:mm:ss] ") + msg);
}

/// 连接 Broker
void MqttWindow::onConnect()
{
    client_->setHostname(host_edit_->currentText());
    client_->setPort(static_cast<quint16>(port_spin_->value()));

    QString clientId = client_id_edit_->toPlainText().trimmed();
    if (!clientId.isEmpty()) {
        client_->setClientId(clientId);
    }

    appendLog(QString("正在连接 %1:%2 ...")
                  .arg(host_edit_->currentText())
                  .arg(port_spin_->value()));
    client_->connectToHost();
}

/// 断开连接
void MqttWindow::onDisconnect()
{
    client_->disconnectFromHost();
    subscribed_topics_.clear();
    subscribed_list_->clear();
    appendLog("已断开连接");
}

/// 状态变化回调
void MqttWindow::onStateChanged(QMqttClient::ClientState state)
{
    bool connected = (state == QMqttClient::Connected);
    connect_btn_->setEnabled(!connected);
    disconnect_btn_->setEnabled(connected);
    status_label_->setText(
        connected ? "状态：已连接" : "状态：未连接");

    if (connected) {
        appendLog("连接成功");
    }
}

/// 订阅话题
void MqttWindow::onSubscribe()
{
    if (client_->state() != QMqttClient::Connected) {
        appendLog("错误：未连接到 Broker");
        return;
    }

    QString topic = sub_topic_edit_->toPlainText().trimmed();
    if (topic.isEmpty()) {
        appendLog("错误：话题不能为空");
        return;
    }

    quint8 qos = static_cast<quint8>(sub_qos_combo_->currentIndex());
    auto *sub = client_->subscribe(
        QMqttTopicFilter(topic), qos);

    if (sub) {
        subscribed_topics_.append(topic);
        subscribed_list_->setPlainText(subscribed_topics_.join("\n"));
        appendLog(QString("已订阅: %1 (QoS %2)")
                      .arg(topic).arg(qos));
    } else {
        appendLog("订阅失败: " + topic);
    }
}

/// 取消订阅
void MqttWindow::onUnsubscribe()
{
    if (client_->state() != QMqttClient::Connected) {
        appendLog("错误：未连接到 Broker");
        return;
    }

    QString topic = sub_topic_edit_->toPlainText().trimmed();
    if (topic.isEmpty()) return;

    client_->unsubscribe(QMqttTopicFilter(topic));
    subscribed_topics_.removeAll(topic);
    subscribed_list_->setPlainText(subscribed_topics_.join("\n"));
    appendLog("已取消订阅: " + topic);
}

/// 发布消息
void MqttWindow::onPublish()
{
    if (client_->state() != QMqttClient::Connected) {
        appendLog("错误：未连接到 Broker");
        return;
    }

    QString topic = pub_topic_edit_->toPlainText().trimmed();
    QByteArray payload = pub_payload_edit_->toPlainText().toUtf8();
    if (topic.isEmpty()) {
        appendLog("错误：话题不能为空");
        return;
    }

    quint8 qos = static_cast<quint8>(pub_qos_combo_->currentIndex());
    client_->publish(QMqttTopicName(topic), payload, qos);
    appendLog(QString("已发布 -> %1 [%2 bytes] (QoS %3)")
                  .arg(topic)
                  .arg(payload.size())
                  .arg(qos));
}

/// 收到消息回调
void MqttWindow::onMessageReceived(const QByteArray &message,
                                   const QMqttTopicName &topic)
{
    appendLog(QString("收到 <- %1: %2")
                  .arg(topic.name())
                  .arg(QString::fromUtf8(message)));
}
