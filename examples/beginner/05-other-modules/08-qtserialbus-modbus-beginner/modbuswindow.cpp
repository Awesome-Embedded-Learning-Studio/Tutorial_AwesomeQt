/**
 * Qt Serial Bus Modbus 通信基础示例
 *
 * 本示例演示 QtSerialBus 模块的 Modbus 客户端用法：
 * 1. QModbusTcpClient / QModbusRtuSerialClient 连接建立
 * 2. sendReadRequest() 读取保持寄存器和线圈
 * 3. sendWriteRequest() 写入保持寄存器和线圈
 * 4. QModbusReply 异步结果处理与错误捕获
 *
 * 注意：本程序需要 Modbus 从站设备或模拟器才能实际测试。
 *       如果没有硬件，程序会正确报告连接失败并展示完整 API 调用流程。
 */

#include <QApplication>
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QModbusClient>
#include <QModbusDataUnit>
#include <QModbusRtuSerialClient>
#include <QModbusTcpClient>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSerialPort>
#include <QSpinBox>
#include <QTime>
#include <QVBoxLayout>

#include "modbuswindow.h"

// ============================================================================
// 主窗口：Modbus 客户端调试工具
// ============================================================================
ModbusWindow::ModbusWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Qt Serial Bus Modbus 通信示例");
    resize(650, 550);

    auto *central = new QWidget(this);
    auto *main_layout = new QVBoxLayout(central);

    // ---- 连接配置区 ----
    auto *conn_group = new QWidget(this);
    auto *conn_layout = new QHBoxLayout(conn_group);

    tcp_radio_ = new QRadioButton("Modbus TCP", this);
    rtu_radio_ = new QRadioButton("Modbus RTU", this);
    tcp_radio_->setChecked(true);

    host_edit_ = new QComboBox(this);
    host_edit_->setEditable(true);
    host_edit_->addItems({"127.0.0.1", "192.168.1.100", "localhost"});

    port_spin_ = new QSpinBox(this);
    port_spin_->setRange(1, 65535);
    port_spin_->setValue(502);

    serial_edit_ = new QComboBox(this);
    serial_edit_->setEditable(true);
    serial_edit_->addItems({"/dev/ttyUSB0", "/dev/ttyS0", "COM3"});

    baud_combo_ = new QComboBox(this);
    baud_combo_->addItems({"9600", "19200", "38400", "115200"});

    connect_btn_ = new QPushButton("连接", this);
    disconnect_btn_ = new QPushButton("断开", this);
    disconnect_btn_->setEnabled(false);

    conn_layout->addWidget(tcp_radio_);
    conn_layout->addWidget(new QLabel("主机:", this));
    conn_layout->addWidget(host_edit_, 1);
    conn_layout->addWidget(new QLabel("端口:", this));
    conn_layout->addWidget(port_spin_);

    conn_layout->addWidget(rtu_radio_);
    conn_layout->addWidget(new QLabel("串口:", this));
    conn_layout->addWidget(serial_edit_, 1);
    conn_layout->addWidget(new QLabel("波特率:", this));
    conn_layout->addWidget(baud_combo_);

    conn_layout->addWidget(connect_btn_);
    conn_layout->addWidget(disconnect_btn_);

    main_layout->addWidget(conn_group);

    // ---- 状态栏 ----
    status_label_ = new QLabel("状态：未连接", this);
    status_label_->setStyleSheet("font-weight: bold;");
    main_layout->addWidget(status_label_);

    // ---- 读写操作区 ----
    auto *ops_group = new QWidget(this);
    auto *ops_layout = new QHBoxLayout(ops_group);

    // 通用参数
    auto *param_layout = new QFormLayout();
    server_spin_ = new QSpinBox(this);
    server_spin_->setRange(1, 247);
    server_spin_->setValue(1);
    addr_spin_ = new QSpinBox(this);
    addr_spin_->setRange(0, 65535);
    addr_spin_->setValue(0);
    count_spin_ = new QSpinBox(this);
    count_spin_->setRange(1, 125);
    count_spin_->setValue(10);
    value_spin_ = new QSpinBox(this);
    value_spin_->setRange(0, 65535);
    value_spin_->setValue(0);

    param_layout->addRow("从站地址:", server_spin_);
    param_layout->addRow("起始地址:", addr_spin_);
    param_layout->addRow("数量:", count_spin_);
    param_layout->addRow("写入值:", value_spin_);
    ops_layout->addLayout(param_layout);

    // 操作按钮
    auto *btn_layout = new QVBoxLayout();
    auto *read_regs_btn = new QPushButton("读取保持寄存器", this);
    auto *read_coils_btn = new QPushButton("读取线圈", this);
    auto *write_reg_btn = new QPushButton("写入单个寄存器", this);
    auto *write_coil_btn = new QPushButton("写入单个线圈", this);

    for (auto *btn : {read_regs_btn, read_coils_btn,
                      write_reg_btn, write_coil_btn}) {
        btn->setMinimumHeight(32);
        btn_layout->addWidget(btn);
    }
    ops_layout->addLayout(btn_layout);

    // 日志清空
    auto *clear_btn = new QPushButton("清空日志", this);
    btn_layout->addWidget(clear_btn);

    main_layout->addWidget(ops_group);

    // ---- 日志输出区 ----
    log_edit_ = new QPlainTextEdit(this);
    log_edit_->setReadOnly(true);
    log_edit_->setMaximumBlockCount(200);
    main_layout->addWidget(log_edit_, 1);

    setCentralWidget(central);

    // ---- 初始化 UI 状态 ----
    updateConnectionUi();

    // ---- 信号槽 ----

    // TCP / RTU 切换
    connect(tcp_radio_, &QRadioButton::toggled, this,
            &ModbusWindow::updateConnectionUi);

    // 连接
    connect(connect_btn_, &QPushButton::clicked, this,
            &ModbusWindow::onConnect);

    // 断开
    connect(disconnect_btn_, &QPushButton::clicked, this,
            &ModbusWindow::onDisconnect);

    // 读取保持寄存器
    connect(read_regs_btn, &QPushButton::clicked, this,
            &ModbusWindow::onReadHoldingRegisters);

    // 读取线圈
    connect(read_coils_btn, &QPushButton::clicked, this,
            &ModbusWindow::onReadCoils);

    // 写入单个寄存器
    connect(write_reg_btn, &QPushButton::clicked, this,
            &ModbusWindow::onWriteSingleRegister);

    // 写入单个线圈
    connect(write_coil_btn, &QPushButton::clicked, this,
            &ModbusWindow::onWriteSingleCoil);

    // 清空日志
    connect(clear_btn, &QPushButton::clicked, this,
            [this]() { log_edit_->clear(); });
}

ModbusWindow::~ModbusWindow()
{
    if (client_) {
        client_->disconnectDevice();
    }
}

/// 切换 TCP / RTU 界面元素可用性
void ModbusWindow::updateConnectionUi()
{
    bool isTcp = tcp_radio_->isChecked();
    host_edit_->setEnabled(isTcp);
    port_spin_->setEnabled(isTcp);
    serial_edit_->setEnabled(!isTcp);
    baud_combo_->setEnabled(!isTcp);
}

/// 追加日志
void ModbusWindow::appendLog(const QString &msg)
{
    log_edit_->appendPlainText(
        QTime::currentTime().toString("[HH:mm:ss] ") + msg);
}

/// 建立连接
void ModbusWindow::onConnect()
{
    // 如果已有客户端，先断开
    if (client_) {
        client_->disconnectDevice();
        client_->deleteLater();
        client_ = nullptr;
    }

    if (tcp_radio_->isChecked()) {
        client_ = new QModbusTcpClient(this);
        client_->setConnectionParameter(
            QModbusDevice::NetworkAddressParameter,
            host_edit_->currentText());
        client_->setConnectionParameter(
            QModbusDevice::NetworkPortParameter,
            port_spin_->value());

        appendLog(QString("正在连接 TCP: %1:%2")
                      .arg(host_edit_->currentText())
                      .arg(port_spin_->value()));
    } else {
        client_ = new QModbusRtuSerialClient(this);
        client_->setConnectionParameter(
            QModbusDevice::SerialPortNameParameter,
            serial_edit_->currentText());
        client_->setConnectionParameter(
            QModbusDevice::SerialBaudRateParameter,
            baud_combo_->currentText().toInt());
        client_->setConnectionParameter(
            QModbusDevice::SerialDataBitsParameter, 8);
        client_->setConnectionParameter(
            QModbusDevice::SerialParityParameter,
            QSerialPort::NoParity);
        client_->setConnectionParameter(
            QModbusDevice::SerialStopBitsParameter, 1);

        appendLog(QString("正在连接 RTU: %1 @ %2 baud")
                      .arg(serial_edit_->currentText())
                      .arg(baud_combo_->currentText()));
    }

    client_->setTimeout(1000);
    client_->setNumberOfRetries(3);

    // 监听状态变化
    connect(client_, &QModbusClient::stateChanged, this,
            &ModbusWindow::onStateChanged);
    connect(client_, &QModbusClient::errorOccurred, this,
            &ModbusWindow::onErrorOccurred);

    client_->connectDevice();
}

/// 断开连接
void ModbusWindow::onDisconnect()
{
    if (client_) {
        client_->disconnectDevice();
        appendLog("已断开连接");
    }
}

/// 状态变化回调
void ModbusWindow::onStateChanged(QModbusDevice::State state)
{
    bool connected = (state == QModbusDevice::ConnectedState);
    connect_btn_->setEnabled(!connected);
    disconnect_btn_->setEnabled(connected);
    status_label_->setText(
        connected ? "状态：已连接" : "状态：未连接");

    if (connected) {
        appendLog("连接成功");
    }
}

/// 错误回调
void ModbusWindow::onErrorOccurred(QModbusDevice::Error error)
{
    Q_UNUSED(error)
    if (client_) {
        appendLog("错误: " + client_->errorString());
    }
}

/// 读取保持寄存器（功能码 03）
void ModbusWindow::onReadHoldingRegisters()
{
    if (!checkConnected()) return;

    int server = server_spin_->value();
    int addr = addr_spin_->value();
    int count = count_spin_->value();

    QModbusDataUnit unit(QModbusDataUnit::HoldingRegisters,
                         addr, count);
    QModbusReply *reply = client_->sendReadRequest(unit, server);
    handleReadReply(reply, "保持寄存器");
}

/// 读取线圈（功能码 01）
void ModbusWindow::onReadCoils()
{
    if (!checkConnected()) return;

    int server = server_spin_->value();
    int addr = addr_spin_->value();
    int count = count_spin_->value();

    QModbusDataUnit unit(QModbusDataUnit::Coils, addr, count);
    QModbusReply *reply = client_->sendReadRequest(unit, server);
    handleReadReply(reply, "线圈");
}

/// 写入单个保持寄存器（功能码 06）
void ModbusWindow::onWriteSingleRegister()
{
    if (!checkConnected()) return;

    int server = server_spin_->value();
    int addr = addr_spin_->value();
    quint16 value = static_cast<quint16>(value_spin_->value());

    QModbusDataUnit unit(QModbusDataUnit::HoldingRegisters, addr, 1);
    unit.setValue(0, value);

    QModbusReply *reply = client_->sendWriteRequest(unit, server);
    handleWriteReply(reply,
                     QString("写入寄存器 地址=%1 值=%2")
                         .arg(addr).arg(value));
}

/// 写入单个线圈（功能码 05）
void ModbusWindow::onWriteSingleCoil()
{
    if (!checkConnected()) return;

    int server = server_spin_->value();
    int addr = addr_spin_->value();
    bool on = (value_spin_->value() != 0);

    QModbusDataUnit unit(QModbusDataUnit::Coils, addr, 1);
    unit.setValue(0, on ? 1 : 0);

    QModbusReply *reply = client_->sendWriteRequest(unit, server);
    handleWriteReply(reply,
                     QString("写入线圈 地址=%1 值=%2")
                         .arg(addr).arg(on ? "ON" : "OFF"));
}

/// 处理读取请求的异步应答
void ModbusWindow::handleReadReply(QModbusReply *reply, const QString &type)
{
    if (!reply) {
        appendLog("发送" + type + "读取请求失败: "
                  + (client_ ? client_->errorString() : "未知错误"));
        return;
    }

    connect(reply, &QModbusReply::finished, this,
            [this, reply, type]() {
                if (reply->error() == QModbusDevice::NoError) {
                    const QModbusDataUnit data = reply->result();
                    appendLog(QString("--- %1 读取成功 (从站 %2) ---")
                                  .arg(type)
                                  .arg(server_spin_->value()));
                    for (int i = 0; i < data.valueCount(); ++i) {
                        appendLog(QString("  [%1] = %2")
                            .arg(data.startAddress() + i)
                            .arg(data.value(i)));
                    }
                } else {
                    appendLog(type + " 读取失败: "
                              + reply->errorString());
                }
                reply->deleteLater();
            });
}

/// 处理写入请求的异步应答
void ModbusWindow::handleWriteReply(QModbusReply *reply, const QString &desc)
{
    if (!reply) {
        appendLog("发送写入请求失败: "
                  + (client_ ? client_->errorString() : "未知错误"));
        return;
    }

    connect(reply, &QModbusReply::finished, this,
            [this, reply, desc]() {
                if (reply->error() == QModbusDevice::NoError) {
                    appendLog(desc + " -> 成功");
                } else {
                    appendLog(desc + " -> 失败: "
                              + reply->errorString());
                }
                reply->deleteLater();
            });
}

/// 检查连接状态
bool ModbusWindow::checkConnected() const
{
    if (!client_
        || client_->state() != QModbusDevice::ConnectedState) {
        const_cast<ModbusWindow*>(this)->appendLog(
            "错误：未连接，请先建立连接");
        return false;
    }
    return true;
}
