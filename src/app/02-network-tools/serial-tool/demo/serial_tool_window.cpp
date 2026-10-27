/**
 * @file serial_tool_window.cpp
 * @brief SerialToolWindow 实现——配置面板装配 + 收发 + Hex/ASCII 切换 + 错误上报
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "serial_tool_window.h"

#include <algorithm>

#include <QByteArray>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSerialPortInfo>
#include <QStatusBar>
#include <QString>
#include <QStringList>
#include <QTextCursor>
#include <QVBoxLayout>
#include <QWidget>

SerialToolWindow::SerialToolWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Serial Tool");
    resize(820, 560);

    port_ = new QSerialPort(this); // 整生命周期持有，open/close 复用
    setupCentral();
    setupStatusBar();
    wireSerial();

    refreshPorts();             // 列首屏可用串口（无硬件可能为空——combo 容忍）
    refreshOpenControls(false); // 初始关闭态
    updateStatus();
}

SerialToolWindow::~SerialToolWindow() {
    if (port_->isOpen()) {
        port_->close();
    }
}

// ============================================================================
// 装配：左配置面板 | 右上下收发区
// ============================================================================
void SerialToolWindow::setupCentral() {
    // ---- 左：配置面板（QFormLayout 表单，标筢单元右对齐）----
    port_combo_ = new QComboBox(this);
    port_combo_->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    // 波特率：常用档位 + QSerialPort 内置默认值，用户可手改 combo 自由输入
    baud_combo_ = new QComboBox(this);
    baud_combo_->setEditable(true);
    baud_combo_->addItems({"1200", "2400", "4800", "9600", "19200", "38400", "57600", "115200"});
    baud_combo_->setCurrentText("115200");

    // 数据位：QSerialPort::DataBits 枚举的 5/6/7/8
    data_bits_combo_ = new QComboBox(this);
    data_bits_combo_->addItems({"8", "7", "6", "5"});
    data_bits_combo_->setCurrentText("8");

    // 停止位：1 / 1.5 / 2（对应 OneStop / OneAndHalfStop / TwoStop）
    stop_bits_combo_ = new QComboBox(this);
    stop_bits_combo_->addItems({"1", "1.5", "2"});

    // 校验：无 / 偶 / 奇（NoParity / EvenParity / OddParity）
    parity_combo_ = new QComboBox(this);
    parity_combo_->addItems({"None", "Even", "Odd"});

    // 流控：无 / 硬件（NoFlowControl / HardwareControl）——SoftwareControl 少用不列
    flow_control_combo_ = new QComboBox(this);
    flow_control_combo_->addItems({"None", "Hardware"});

    auto* config_form = new QFormLayout;
    config_form->addRow("Port:", port_combo_);
    config_form->addRow("Baud rate:", baud_combo_);
    config_form->addRow("Data bits:", data_bits_combo_);
    config_form->addRow("Stop bits:", stop_bits_combo_);
    config_form->addRow("Parity:", parity_combo_);
    config_form->addRow("Flow control:", flow_control_combo_);

    open_close_button_ = new QPushButton("Open", this);
    refresh_button_ = new QPushButton("Refresh ports", this);

    auto* btn_row = new QHBoxLayout;
    btn_row->addWidget(open_close_button_);
    btn_row->addWidget(refresh_button_);

    auto* config_layout = new QVBoxLayout;
    config_layout->addLayout(config_form);
    config_layout->addLayout(btn_row);
    config_layout->addStretch(1);

    auto* config_group = new QGroupBox("Configuration", this);
    config_group->setLayout(config_layout);

    // ---- 右上：接收区（只读 + Hex/ASCII 单选）----
    receive_edit_ = new QPlainTextEdit(this);
    receive_edit_->setReadOnly(true);
    receive_edit_->setPlaceholderText("Received data appears here…");

    recv_hex_radio_ = new QRadioButton("Hex", this);
    recv_ascii_radio_ = new QRadioButton("ASCII", this);
    recv_ascii_radio_->setChecked(true); // 默认 ASCII
    auto* recv_mode_row = new QHBoxLayout;
    recv_mode_row->addWidget(new QLabel("Display:"));
    recv_mode_row->addWidget(recv_ascii_radio_);
    recv_mode_row->addWidget(recv_hex_radio_);
    recv_mode_row->addStretch(1);
    auto* recv_layout = new QVBoxLayout;
    recv_layout->addLayout(recv_mode_row);
    recv_layout->addWidget(receive_edit_);
    auto* recv_group = new QGroupBox("Receive", this);
    recv_group->setLayout(recv_layout);

    // ---- 右下：发送区（QLineEdit + Send，Hex/ASCII 切换）----
    send_edit_ = new QLineEdit(this);
    send_edit_->setPlaceholderText("Type text to send…");
    send_button_ = new QPushButton("Send", this);
    send_hex_radio_ = new QRadioButton("Hex", this);
    send_ascii_radio_ = new QRadioButton("ASCII", this);
    send_ascii_radio_->setChecked(true); // 默认 ASCII

    auto* send_mode_row = new QHBoxLayout;
    send_mode_row->addWidget(new QLabel("Send as:"));
    send_mode_row->addWidget(send_ascii_radio_);
    send_mode_row->addWidget(send_hex_radio_);
    send_mode_row->addStretch(1);

    auto* send_layout = new QVBoxLayout;
    send_layout->addLayout(send_mode_row);
    auto* send_row = new QHBoxLayout;
    send_row->addWidget(send_edit_, 1);
    send_row->addWidget(send_button_);
    send_layout->addLayout(send_row);
    auto* send_group = new QGroupBox("Send", this);
    send_group->setLayout(send_layout);

    auto* right_layout = new QVBoxLayout;
    right_layout->addWidget(recv_group, 3);
    right_layout->addWidget(send_group, 1);

    auto* central_layout = new QHBoxLayout;
    central_layout->addWidget(config_group);
    central_layout->addLayout(right_layout, 1);

    auto* central = new QWidget(this);
    central->setLayout(central_layout);
    setCentralWidget(central);

    // 信号接驳（lambda 信号槽写法）
    connect(open_close_button_, &QPushButton::clicked, this, &SerialToolWindow::onOpenCloseToggled);
    connect(refresh_button_, &QPushButton::clicked, this, &SerialToolWindow::onRefreshPorts);
    connect(send_button_, &QPushButton::clicked, this, &SerialToolWindow::onSend);
    connect(send_edit_, &QLineEdit::returnPressed, this, &SerialToolWindow::onSend);
    // 切 Hex/ASCII：接收区把已收整段按新模式重渲；发送区只是改变下次发送的编码解释
    connect(recv_hex_radio_, &QRadioButton::toggled, this, [this]() { refreshReceiveDisplay(); });
    connect(recv_ascii_radio_, &QRadioButton::toggled, this, [this]() { refreshReceiveDisplay(); });
}

void SerialToolWindow::setupStatusBar() {
    status_label_ = new QLabel("Closed", this);
    statusBar()->addWidget(status_label_);
}

void SerialToolWindow::wireSerial() {
    // readyRead：一次数据可能分多个信号到达——每次 readAll 累积到 buffer_，再渲染新增
    connect(port_, &QSerialPort::readyRead, this, &SerialToolWindow::onReadyRead);
    // errorOccurred：串口硬件层错误（拔线/占用）→ 状态栏报错；主动 close 期间屏蔽
    connect(port_, &QSerialPort::errorOccurred, this, &SerialToolWindow::onPortError);
}

// ============================================================================
// 端口列表：availablePorts 在无硬件环境下返回空——combo 容忍 + 提供刷新
// ============================================================================
void SerialToolWindow::refreshPorts() {
    port_combo_->blockSignals(true);
    port_combo_->clear();
    const auto ports = QSerialPortInfo::availablePorts();
    for (const auto& info : ports) {
        // portName() 返回平台适配名（Windows COMx / Linux ttySx/ttyUSBx / macOS cu.*）
        port_combo_->addItem(info.portName());
    }
    port_combo_->blockSignals(false);
}

void SerialToolWindow::onRefreshPorts() {
    refreshPorts();
    updateStatus();
}

// ============================================================================
// 打开 / 关闭（同一按钮两态切换）
// ============================================================================
void SerialToolWindow::onOpenCloseToggled() {
    if (port_->isOpen()) {
        // 主动 close：errorOccurred 可能被触发（如 ResourceError），用 flag 屏蔽避免噪声
        suppress_error_ = true;
        port_->close();
        suppress_error_ = false;
        refreshOpenControls(false);
        updateStatus();
        return;
    }

    const QString name = port_combo_->currentText().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Open Port", "No port selected. Refresh the port list first.");
        return;
    }

    // 切端口/参数前先确保旧端口已关（此处虽未打开，但保持防御性）
    if (port_->isOpen()) {
        suppress_error_ = true;
        port_->close();
        suppress_error_ = false;
    }

    applyConfigToPort();
    port_->setPortName(name);
    if (!port_->open(QIODevice::ReadWrite)) {
        // 打开失败读 errorString 反馈，不静默
        QMessageBox::warning(this, "Open Port",
                             "Failed to open port " + name + ":\n" + port_->errorString());
        refreshOpenControls(false);
        updateStatus();
        return;
    }

    refreshOpenControls(true);
    updateStatus();
}

void SerialToolWindow::refreshOpenControls(bool open) {
    // 开/关两态：按钮文案 + 配置控件可编辑性互斥（开着时禁止改端口/参数）
    open_close_button_->setText(open ? "Close" : "Open");
    const bool editable = !open;
    port_combo_->setEnabled(editable);
    baud_combo_->setEnabled(editable);
    data_bits_combo_->setEnabled(editable);
    stop_bits_combo_->setEnabled(editable);
    parity_combo_->setEnabled(editable);
    flow_control_combo_->setEnabled(editable);
    refresh_button_->setEnabled(editable);
    send_button_->setEnabled(open);
    send_edit_->setEnabled(open);
}

// ============================================================================
// 把 QComboBox 当前选中项映射到 QSerialPort 配置（open 之前调一次）
// ============================================================================
void SerialToolWindow::applyConfigToPort() {
    bool ok = false;
    const qint32 baud = baud_combo_->currentText().trimmed().toUInt(&ok);
    if (ok && baud > 0) {
        port_->setBaudRate(baud); // 用户可在 editable combo 自由输入任意合法波特率
    }

    // 用 currentIndex 映射（与 stop/parity/flow 一致），避免文案/翻译依赖：
    // combo 顺序 {"8","7","6","5"} → index 0..3
    switch (data_bits_combo_->currentIndex()) {
        case 1:
            port_->setDataBits(QSerialPort::Data7);
            break;
        case 2:
            port_->setDataBits(QSerialPort::Data6);
            break;
        case 3:
            port_->setDataBits(QSerialPort::Data5);
            break;
        default: // 0 → Data8
            port_->setDataBits(QSerialPort::Data8);
            break;
    }

    switch (stop_bits_combo_->currentIndex()) {
        case 1:
            port_->setStopBits(QSerialPort::OneAndHalfStop);
            break;
        case 2:
            port_->setStopBits(QSerialPort::TwoStop);
            break;
        default:
            port_->setStopBits(QSerialPort::OneStop);
            break;
    }

    switch (parity_combo_->currentIndex()) {
        case 1:
            port_->setParity(QSerialPort::EvenParity);
            break;
        case 2:
            port_->setParity(QSerialPort::OddParity);
            break;
        default:
            port_->setParity(QSerialPort::NoParity);
            break;
    }

    switch (flow_control_combo_->currentIndex()) {
        case 1:
            port_->setFlowControl(QSerialPort::HardwareControl);
            break;
        default:
            port_->setFlowControl(QSerialPort::NoFlowControl);
            break;
    }
}

// ============================================================================
// 接收：readyRead 累积 → 按显示模式渲染
// ============================================================================
void SerialToolWindow::onReadyRead() {
    const QByteArray chunk = port_->readAll(); // 本次 readyRead 到达的字节（可能只是片段）
    appendReceive(chunk);
}

void SerialToolWindow::appendReceive(const QByteArray& chunk) {
    receive_buffer_.append(chunk); // 累积原始字节——切 Hex/ASCII 用整段重渲
    rx_bytes_ += chunk.size();

    const bool hex = recv_hex_radio_->isChecked();
    const QString rendered = renderChunk(chunk, hex);
    // 追加文本：移到末尾后 insertPlainText（不加自动换行，保持连续字节流；
    // appendPlainText 会强制换行，不适合逐片到达的流式显示）
    receive_edit_->moveCursor(QTextCursor::End);
    receive_edit_->insertPlainText(rendered);
    updateStatus();
}

void SerialToolWindow::refreshReceiveDisplay() {
    // 切 Hex/ASCII：把已收整段 buffer_ 按新模式整体重渲（避免历史段留在旧模式）
    const bool hex = recv_hex_radio_->isChecked();
    receive_edit_->setPlainText(renderChunk(receive_buffer_, hex));
    receive_edit_->moveCursor(QTextCursor::End);
}

QString SerialToolWindow::renderChunk(const QByteArray& bytes, bool hexMode) const {
    if (hexMode) {
        // toHex() 给每字节两 hex 字符（无分隔），手动加空格更可读
        const QByteArray hex = bytes.toHex(' ');
        return QString::fromLatin1(hex);
    }
    // ASCII 模式：Latin-1 保底逐字节映射（不丢字节、不抛编码异常）。注意这不是严格「字节忠实
    // 显示」——CR(0x0D) 会回车覆盖、NUL/控制字符在 QPlainTextEdit 里显示失真；要逐字节精确看请切
    // Hex。
    return QString::fromLatin1(bytes);
}

// ============================================================================
// 发送：Hex/ASCII 切换 → QSerialPort::write
// ============================================================================
void SerialToolWindow::onSend() {
    if (!port_->isOpen()) {
        status_label_->setText("Cannot send: port is closed");
        return;
    }
    const QString text = send_edit_->text();
    if (text.isEmpty()) {
        return;
    }

    QByteArray payload;
    if (send_hex_radio_->isChecked()) {
        // Hex 模式：用户输入 "48 65 6C 6C 6F"。QByteArray::fromHex 对非 hex 字符是「跳过」
        // 而非报错（实证 qbytearray.cpp:4641），混入非法字符会静默截断——所以先正向校验：
        // 去空白后必须全是 [0-9a-fA-F] 且长度偶数（奇数长度 fromHex 会吞掉最高位半字节）。
        QString compact;
        for (QChar c : text) {
            if (!c.isSpace()) {
                compact.append(c);
            }
        }
        const auto is_hex_digit = [](QChar c) {
            return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
        };
        if (compact.length() % 2 != 0 ||
            !std::all_of(compact.begin(), compact.end(), is_hex_digit)) {
            status_label_->setText("Send failed: invalid hex input (expect pairs of 0-9a-fA-F)");
            return;
        }
        payload = QByteArray::fromHex(compact.toLatin1());
    } else {
        payload = text.toUtf8(); // ASCII/UTF-8 文本
    }

    const qint64 written = port_->write(payload);
    if (written < 0) {
        status_label_->setText("Send failed: " + port_->errorString());
        return;
    }
    // write 只保证入 Qt 内部缓冲，flush 把它推到驱动层——否则 close 时 pending 字节可能丢
    port_->flush();
    tx_bytes_ += written; // 注：TX 统计入队字节；严格物理发出量需用 bytesWritten 信号
    updateStatus();
}

// ============================================================================
// 错误上报：errorOccurred → 状态栏（主动 close 期间屏蔽）
// ============================================================================
void SerialToolWindow::onPortError(QSerialPort::SerialPortError error) {
    if (error == QSerialPort::NoError) {
        return;
    }
    if (suppress_error_) {
        return; // 主动 close 期间不报噪声
    }
    // 用 error 码自带映射，不依赖 errorString（它可能在信号排队派发前被覆盖、stale）
    status_label_->setText(
        QString("Port error [%1]: %2").arg(static_cast<int>(error)).arg(errorToString(error)));

    // 不可恢复错误（拔线/被占用/权限/找不到）：主动 close 收敛到关闭态——否则卡在
    // isOpen()==true 的半开态，后续 Send 静默失败、配置控件锁死无法自救
    if (error == QSerialPort::ResourceError || error == QSerialPort::OpenError ||
        error == QSerialPort::PermissionError || error == QSerialPort::DeviceNotFoundError) {
        suppress_error_ = true;
        if (port_->isOpen()) {
            port_->close();
        }
        suppress_error_ = false;
        refreshOpenControls(false);
        updateStatus();
    }
}

QString SerialToolWindow::errorToString(QSerialPort::SerialPortError error) const {
    switch (error) {
        case QSerialPort::DeviceNotFoundError:
            return "Device not found";
        case QSerialPort::PermissionError:
            return "Permission denied";
        case QSerialPort::OpenError:
            return "Already open or open failed";
        case QSerialPort::ResourceError:
            return "Device unavailable (unplugged?)";
        default:
            return "Serial port error";
    }
}

// ============================================================================
// 状态栏：开/关 + 收发字节计数
// ============================================================================
void SerialToolWindow::updateStatus() {
    const QString state = port_->isOpen() ? "Open" : "Closed";
    status_label_->setText(QString("Port: %1  ·  %2  ·  RX %3 bytes  ·  TX %4 bytes")
                               .arg(port_->isOpen() ? port_->portName() : QStringLiteral("-"))
                               .arg(state)
                               .arg(rx_bytes_)
                               .arg(tx_bytes_));
}
