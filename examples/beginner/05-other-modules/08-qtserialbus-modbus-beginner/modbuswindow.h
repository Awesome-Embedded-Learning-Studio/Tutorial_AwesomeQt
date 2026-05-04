#ifndef MODBUSWINDOW_H
#define MODBUSWINDOW_H

#include <QMainWindow>

#include <QModbusDevice>

class QComboBox;
class QSpinBox;
class QRadioButton;
class QPushButton;
class QLabel;
class QPlainTextEdit;
class QModbusClient;
class QModbusReply;

class ModbusWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ModbusWindow(QWidget *parent = nullptr);
    ~ModbusWindow() override;

private:
    void updateConnectionUi();
    void appendLog(const QString &msg);
    void onConnect();
    void onDisconnect();
    void onStateChanged(QModbusDevice::State state);
    void onErrorOccurred(QModbusDevice::Error error);
    void onReadHoldingRegisters();
    void onReadCoils();
    void onWriteSingleRegister();
    void onWriteSingleCoil();
    void handleReadReply(QModbusReply *reply, const QString &type);
    void handleWriteReply(QModbusReply *reply, const QString &desc);
    bool checkConnected() const;

    // ---- UI 控件 ----
    QRadioButton *tcp_radio_;
    QRadioButton *rtu_radio_;
    QComboBox *host_edit_;
    QSpinBox *port_spin_;
    QComboBox *serial_edit_;
    QComboBox *baud_combo_;
    QPushButton *connect_btn_;
    QPushButton *disconnect_btn_;
    QLabel *status_label_;
    QSpinBox *server_spin_;
    QSpinBox *addr_spin_;
    QSpinBox *count_spin_;
    QSpinBox *value_spin_;
    QPlainTextEdit *log_edit_;

    // ---- Modbus 客户端 ----
    QModbusClient *client_ = nullptr;
};

#endif // MODBUSWINDOW_H
