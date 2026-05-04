#ifndef MQTTWINDOW_H
#define MQTTWINDOW_H

#include <QMainWindow>
#include <QStringList>

#include <QMqttClient>

class QComboBox;
class QSpinBox;
class QTextEdit;
class QPushButton;
class QLabel;
class QPlainTextEdit;
class QMqttSubscription;
class QMqttTopicFilter;
class QMqttTopicName;

class MqttWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MqttWindow(QWidget *parent = nullptr);

private:
    void appendLog(const QString &msg);
    void onConnect();
    void onDisconnect();
    void onStateChanged(QMqttClient::ClientState state);
    void onSubscribe();
    void onUnsubscribe();
    void onPublish();
    void onMessageReceived(const QByteArray &message,
                           const QMqttTopicName &topic);

    // ---- UI 控件 ----
    QComboBox *host_edit_;
    QSpinBox *port_spin_;
    QTextEdit *client_id_edit_;
    QPushButton *connect_btn_;
    QPushButton *disconnect_btn_;
    QLabel *status_label_;
    QTextEdit *sub_topic_edit_;
    QComboBox *sub_qos_combo_;
    QPlainTextEdit *subscribed_list_;
    QTextEdit *pub_topic_edit_;
    QTextEdit *pub_payload_edit_;
    QComboBox *pub_qos_combo_;
    QPlainTextEdit *log_edit_;

    // ---- 数据 ----
    QStringList subscribed_topics_;

    // ---- MQTT 客户端 ----
    QMqttClient *client_;
};

#endif // MQTTWINDOW_H
