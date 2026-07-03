/**
 * @file network_tool_window.cpp
 * @brief NetworkToolWindow 实现——协议装配 + 收发 + 多客户端管理 + 状态栏
 * @copyright Copyright (c) 2026 AwesomeQt
 */

#include "network_tool_window.h"

#include <QAbstractSocket>
#include <QByteArray>
#include <QComboBox>
#include <QDateTime>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHostAddress>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkDatagram>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStatusBar>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTextCursor>
#include <QUdpSocket>
#include <QVBoxLayout>
#include <QWidget>

// 协议在 QComboBox 中的索引（与 protocol_combo_ addItems 顺序严格对应）
static constexpr int kProtoTcpServer = 0;
static constexpr int kProtoTcpClient = 1;
static constexpr int kProtoUdp = 2;

NetworkToolWindow::NetworkToolWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Network Tool");
    resize(820, 560);

    setupCentral();
    setupStatusBar();
    setupConnections();

    refreshConfigVisibility();     // 按默认协议显示/隐藏配置控件
    refreshRunningControls(false); // 初始停止态
    updateStatus();
}

NetworkToolWindow::~NetworkToolWindow() {
    teardownAll();
}

// ============================================================================
// 装配：顶部协议+配置 | 中接收区 | 下发送区
// ============================================================================
void NetworkToolWindow::setupCentral() {
    // ---- 顶部：协议选择 + 配置 ----
    protocol_combo_ = new QComboBox(this);
    protocol_combo_->addItems({"TCP Server", "TCP Client", "UDP"});

    local_port_edit_ = new QLineEdit(this);
    local_port_edit_->setPlaceholderText("e.g. 9000  (0 = system-assigned)");
    local_port_edit_->setMaximumWidth(200);

    target_ip_edit_ = new QLineEdit(this);
    target_ip_edit_->setPlaceholderText("e.g. 127.0.0.1");
    target_ip_edit_->setMaximumWidth(160);
    target_port_edit_ = new QLineEdit(this);
    target_port_edit_->setPlaceholderText("e.g. 9000");
    target_port_edit_->setMaximumWidth(100);

    local_port_label_ = new QLabel("Local port:", this);
    target_label_ = new QLabel("Target IP / port:", this);

    auto* config_form = new QFormLayout;
    config_form->addRow("Protocol:", protocol_combo_);
    config_form->addRow(local_port_label_, local_port_edit_);
    config_form->addRow(target_label_, target_ip_edit_);
    {
        // 目标端口单独成行（与 target_ip 不强对齐成一行，留白更可读）
        auto* tp_row = new QHBoxLayout;
        tp_row->addWidget(target_port_edit_);
        tp_row->addStretch(1);
        config_form->addRow("Target port:", tp_row);
    }

    start_stop_button_ = new QPushButton("Start", this);

    auto* config_layout = new QVBoxLayout;
    config_layout->addLayout(config_form);
    config_layout->addWidget(start_stop_button_);
    config_layout->addStretch(1);

    auto* config_group = new QGroupBox("Configuration", this);
    config_group->setLayout(config_layout);

    // ---- 中：接收区（只读日志，时间戳 + 方向 ←→）----
    receive_edit_ = new QPlainTextEdit(this);
    receive_edit_->setReadOnly(true);
    receive_edit_->setPlaceholderText("Traffic log appears here (timestamps + direction)…");
    auto* recv_group = new QGroupBox("Receive", this);
    auto* recv_layout = new QVBoxLayout;
    recv_layout->addWidget(receive_edit_);
    recv_group->setLayout(recv_layout);

    // ---- 下：发送区 ----
    send_edit_ = new QLineEdit(this);
    send_edit_->setPlaceholderText("Type text to send…");
    send_button_ = new QPushButton("Send", this);

    auto* send_row = new QHBoxLayout;
    send_row->addWidget(send_edit_, 1);
    send_row->addWidget(send_button_);
    auto* send_group = new QGroupBox("Send", this);
    auto* send_layout = new QVBoxLayout;
    send_layout->addLayout(send_row);
    send_group->setLayout(send_layout);

    // 右侧：接收区占大头 + 发送区
    auto* right_layout = new QVBoxLayout;
    right_layout->addWidget(recv_group, 3);
    right_layout->addWidget(send_group, 1);

    auto* central_layout = new QHBoxLayout;
    central_layout->addWidget(config_group);
    central_layout->addLayout(right_layout, 1);

    auto* central = new QWidget(this);
    central->setLayout(central_layout);
    setCentralWidget(central);
}

void NetworkToolWindow::setupStatusBar() {
    status_label_ = new QLabel("Stopped", this);
    statusBar()->addWidget(status_label_);
}

void NetworkToolWindow::setupConnections() {
    connect(protocol_combo_, qOverload<int>(&QComboBox::currentIndexChanged), this,
            &NetworkToolWindow::onProtocolChanged);
    connect(start_stop_button_, &QPushButton::clicked, this,
            &NetworkToolWindow::onStartStopToggled);
    connect(send_button_, &QPushButton::clicked, this, &NetworkToolWindow::onSend);
    connect(send_edit_, &QLineEdit::returnPressed, this, &NetworkToolWindow::onSend);
}

// ============================================================================
// 协议切换：仅改配置可见性；运行中切换会被 refreshRunningControls 拒（见 onStartStopToggled）
// ============================================================================
void NetworkToolWindow::onProtocolChanged() {
    refreshConfigVisibility();
    updateStatus();
}

void NetworkToolWindow::refreshConfigVisibility() {
    const int proto = protocol_combo_->currentIndex();
    // TCP Server / UDP：用「本机端口」（listen/bind）；TCP Client：用「目标
    // IP+端口」（connectToHost）
    const bool need_local = (proto == kProtoTcpServer || proto == kProtoUdp);
    // TCP Client 要目标（connectToHost）；UDP 也要目标（writeDatagram）——UDP 同时显示本机端口
    const bool need_target = (proto == kProtoTcpClient || proto == kProtoUdp);
    local_port_label_->setVisible(need_local);
    local_port_edit_->setVisible(need_local);
    target_label_->setVisible(need_target);
    target_ip_edit_->setVisible(need_target);
    // target_port 单独行：连同 target_ip 一起显隐
    auto* tp_parent = target_port_edit_->parentWidget();
    if (tp_parent != nullptr) {
        tp_parent->setVisible(need_target);
    }
}

void NetworkToolWindow::refreshRunningControls(bool running) {
    start_stop_button_->setText(running ? "Stop" : "Start");
    // 运行中锁住协议切换和配置输入（防止改了端口却没生效的错觉）
    protocol_combo_->setEnabled(!running);
    local_port_edit_->setReadOnly(running);
    target_ip_edit_->setReadOnly(running);
    target_port_edit_->setReadOnly(running);
    // 发送按钮：运行态才可发；且 TCP Client 必须已 connected（onTcpConnected 再校验）
    send_button_->setEnabled(running);
    send_edit_->setEnabled(running);
}

// ============================================================================
// Start / Stop（同一按钮两态切换）
// ============================================================================
void NetworkToolWindow::onStartStopToggled() {
    // 用 server/socket 是否存在判断运行态（而非按钮文案，避免状态脱钩）
    const bool running =
        (tcp_server_ != nullptr) || (tcp_client_ != nullptr) || (udp_socket_ != nullptr);
    if (running) {
        teardownAll();
        refreshRunningControls(false);
        updateStatus();
        return;
    }

    const int proto = protocol_combo_->currentIndex();
    switch (proto) {
        case kProtoTcpServer: {
            bool ok = false;
            const quint16 port = local_port_edit_->text().trimmed().toUShort(&ok);
            // 端口 0 = 系统分配（合法），非 0 范围外的算非法
            if (local_port_edit_->text().trimmed().isEmpty() ||
                (!ok && !local_port_edit_->text().trimmed().isEmpty())) {
                QMessageBox::warning(this, "Start",
                                     "Invalid local port. Enter 0..65535 (0 = system-assigned).");
                return;
            }
            tcp_server_ = new QTcpServer(this);
            connect(tcp_server_, &QTcpServer::newConnection, this,
                    &NetworkToolWindow::onTcpNewConnection);
            connect(tcp_server_, &QTcpServer::acceptError, this,
                    &NetworkToolWindow::onServerAcceptError);
            // 端口 0 让系统分配，serverPort() 取实际端口——绝不假设用户填的就是 listen 成功的
            if (!tcp_server_->listen(QHostAddress::Any, port)) {
                QMessageBox::warning(this, "Start",
                                     "Failed to listen:\n" +
                                         tcp_server_->errorString()); // 端口占用/权限不够在此暴露
                delete tcp_server_;
                tcp_server_ = nullptr;
                return;
            }
            appendLog("i", QString("TCP Server listening on port %1 (0 → %2 assigned)")
                               .arg(port)
                               .arg(tcp_server_->serverPort()));
            break;
        }
        case kProtoTcpClient: {
            const QString ip = target_ip_edit_->text().trimmed();
            bool ok = false;
            const quint16 port = target_port_edit_->text().trimmed().toUShort(&ok);
            if (ip.isEmpty() || !ok) {
                QMessageBox::warning(this, "Start", "Enter a valid target IP and port.");
                return;
            }
            tcp_client_ = new QTcpSocket(this);
            connect(tcp_client_, &QTcpSocket::connected, this, &NetworkToolWindow::onTcpConnected);
            connect(tcp_client_, &QTcpSocket::disconnected, this,
                    [this]() { onTcpDisconnected(nullptr); }); // 自身断开：socket=nullptr 表示自身
            connect(tcp_client_, &QTcpSocket::readyRead, this, &NetworkToolWindow::onTcpReadyRead);
            connect(tcp_client_, &QTcpSocket::errorOccurred, this,
                    &NetworkToolWindow::onSocketErrorOccurred);
            appendLog("i", QString("TCP Client connecting to %1:%2…").arg(ip).arg(port));
            tcp_client_->connectToHost(ip, port); // 异步：connected/disconnected 信号驱动后续态
            break;
        }
        case kProtoUdp: {
            bool ok = false;
            const quint16 port = local_port_edit_->text().trimmed().toUShort(&ok);
            if (local_port_edit_->text().trimmed().isEmpty() ||
                (!ok && !local_port_edit_->text().trimmed().isEmpty())) {
                QMessageBox::warning(this, "Start",
                                     "Invalid local port. Enter 0..65535 (0 = system-assigned).");
                return;
            }
            udp_socket_ = new QUdpSocket(this);
            connect(udp_socket_, &QUdpSocket::readyRead, this, &NetworkToolWindow::onUdpReadyRead);
            connect(udp_socket_, &QUdpSocket::errorOccurred, this,
                    &NetworkToolWindow::onSocketErrorOccurred);
            if (!udp_socket_->bind(QHostAddress::Any, port)) {
                QMessageBox::warning(this, "Start",
                                     "Failed to bind:\n" + udp_socket_->errorString());
                delete udp_socket_;
                udp_socket_ = nullptr;
                return;
            }
            appendLog("i", QString("UDP bound on port %1").arg(udp_socket_->localPort()));
            break;
        }
        default:
            break;
    }

    refreshRunningControls(true);
    updateStatus();
}

// ============================================================================
// Stop：按当前协议 close 并 deleteLater 全部资源（new 出来的都有 this 父，但显式清理更稳）
// ============================================================================
void NetworkToolWindow::teardownAll() {
    if (tcp_server_ != nullptr) {
        // 先 close server，再逐个 close 客户端——server close 不会主动断客户端 socket
        tcp_server_->close();
        tcp_server_->deleteLater();
        tcp_server_ = nullptr;
    }
    for (auto* client : tcp_clients_) {
        disconnect(client, &QTcpSocket::disconnected, this, nullptr); // 避免断开信号回调已删对象
        client->disconnectFromHost();
        client->deleteLater();
    }
    tcp_clients_.clear();
    if (tcp_client_ != nullptr) {
        disconnect(tcp_client_, &QTcpSocket::disconnected, this, nullptr); // 与客户端分支对称屏蔽
        tcp_client_->disconnectFromHost();
        tcp_client_->deleteLater();
        tcp_client_ = nullptr;
    }
    if (udp_socket_ != nullptr) {
        udp_socket_->close();
        udp_socket_->deleteLater();
        udp_socket_ = nullptr;
    }
    appendLog("i", "Stopped.");
}

// ============================================================================
// TCP Server：接客户端
// ============================================================================
void NetworkToolWindow::onTcpNewConnection() {
    // hasPendingConnections 循环接（高并发可能一次积压多个）
    while (tcp_server_ != nullptr && tcp_server_->hasPendingConnections()) {
        auto* socket = tcp_server_->nextPendingConnection();
        socket->setParent(this); // 显式挂父——析构时对象树兜底，否则退出时 deleteLater 不执行会泄漏
        tcp_clients_.append(socket);
        appendLog("i", QString("Client connected: %1:%2")
                           .arg(socket->peerAddress().toString())
                           .arg(socket->peerPort()));
        // 每个客户端接自己的 readyRead/disconnected（用 sender() 区分谁的数据）
        connect(socket, &QTcpSocket::readyRead, this, &NetworkToolWindow::onTcpReadyRead);
        connect(socket, &QTcpSocket::disconnected, this,
                [this, socket]() { onTcpDisconnected(socket); });
    }
    updateStatus();
}

// ============================================================================
// TCP 收数据：Server 客户端与 Client 自身共用此 slot，用 sender() 区分来源
// ============================================================================
void NetworkToolWindow::onTcpReadyRead() {
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket == nullptr) {
        return;
    }
    // readyRead 异步，本次只保证「至少一字节可读」。readAll 一次清空读缓冲，while 主要是
    // 防御性（slot 执行期间又有新数据到达的边缘情况），正常只迭代一次。
    while (socket->bytesAvailable() > 0) {
        const QByteArray chunk = socket->readAll();
        if (chunk.isEmpty()) {
            break;
        }
        rx_bytes_ += chunk.size();
        appendLog("←", QString::fromUtf8(chunk));
    }
    updateStatus();
}

void NetworkToolWindow::onTcpConnected() {
    send_button_->setEnabled(true); // 重连成功恢复发送（断开时已禁用）
    appendLog("i", "TCP Client connected.");
    updateStatus();
}

// socket=nullptr 表示 TCP Client 自身断开；非空表示 Server 模式某客户端断开
void NetworkToolWindow::onTcpDisconnected(QTcpSocket* socket) {
    if (socket == nullptr) {
        // TCP Client 自身断开：禁用发送按钮（UI 与连接态对齐，避免点了才报 Not connected）
        send_button_->setEnabled(false);
        appendLog("i", "TCP Client disconnected.");
        updateStatus();
        return;
    }
    // Server 模式：从客户端集合移除并 deleteLater（防止悬空指针 + 内存泄漏）
    const QString tag =
        QString("%1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort());
    tcp_clients_.removeOne(socket);
    socket->deleteLater();
    appendLog("i", "Client disconnected: " + tag);
    updateStatus();
}

// ============================================================================
// UDP 收：hasPendingDatagrams + receiveDatagram（不是 readAll）
// ============================================================================
void NetworkToolWindow::onUdpReadyRead() {
    // 一次 readyRead 可能含多个数据报——循环消费到没有 pending 为止
    while (udp_socket_ != nullptr && udp_socket_->hasPendingDatagrams()) {
        const QNetworkDatagram dgram = udp_socket_->receiveDatagram();
        if (!dgram.isValid()) {
            break;
        }
        const QByteArray payload = dgram.data();
        rx_bytes_ += payload.size();
        appendLog("←", QString::fromUtf8(payload) + "   [from " + dgram.senderAddress().toString() +
                           ":" + QString::number(dgram.senderPort()) + "]");
    }
    updateStatus();
}

// ============================================================================
// 发送：按当前协议写入；TCP Client 须 connected 才发
// ============================================================================
void NetworkToolWindow::onSend() {
    const QString text = send_edit_->text();
    if (text.isEmpty()) {
        return;
    }
    const QByteArray payload = text.toUtf8();
    qint64 written = -1;

    const int proto = protocol_combo_->currentIndex();
    switch (proto) {
        case kProtoTcpServer: {
            // Server 模式：广播到所有已连接客户端（调试助手典型用法）
            if (tcp_clients_.isEmpty()) {
                appendLog("!", "No connected client to send to.");
                return;
            }
            // 累加每个客户端实发字节（非只计一份 payload）；个别失败不抹掉已成功的
            written = 0;
            bool any_failed = false;
            for (auto* client : tcp_clients_) {
                const qint64 n = client->write(payload);
                if (n < 0) {
                    any_failed = true; // 错误由 errorOccurred 上报
                } else {
                    written += n;
                }
            }
            if (any_failed) {
                if (written == 0) {
                    written = -1; // 全部失败
                } else {
                    appendLog("!", "Partial send: some clients failed (see socket error).");
                }
            }
            break;
        }
        case kProtoTcpClient: {
            if (tcp_client_ == nullptr || tcp_client_->state() != QAbstractSocket::ConnectedState) {
                // connected 信号态为准——断开后 write 会被 Qt 丢弃/报错，这里前置拒绝
                appendLog("!", "Not connected. Wait for the connected state before sending.");
                return;
            }
            written = tcp_client_->write(payload);
            break;
        }
        case kProtoUdp: {
            if (udp_socket_ == nullptr) {
                appendLog("!", "UDP not bound.");
                return;
            }
            // UDP 发：要明确目标（UDP 无连接）。从配置区取 target；若留空则按「发给自己
            // 已 bind 端口」的 loopback 调试用法（localPort 即目标）
            const QString ip = target_ip_edit_->text().trimmed();
            bool ok = false;
            quint16 port = target_port_edit_->text().trimmed().toUShort(&ok);
            QHostAddress target;
            if (!ip.isEmpty() && ok) {
                target.setAddress(ip);
            } else if (ok == false && ip.isEmpty()) {
                // 目标留空：默认 loopback 回环（自己 bind 的端口），便于自测
                target = QHostAddress(QHostAddress::LocalHost);
                port = udp_socket_->localPort();
            } else {
                appendLog("!", "Enter a valid target IP and port for UDP send.");
                return;
            }
            written = udp_socket_->writeDatagram(payload, target, port);
            break;
        }
        default:
            break;
    }

    if (written < 0) {
        appendLog("!", "Send failed (see socket error in status bar).");
        return;
    }
    tx_bytes_ += written;
    appendLog("→", text);
    updateStatus();
}

// ============================================================================
// 错误上报
// ============================================================================
void NetworkToolWindow::onSocketErrorOccurred() {
    auto* socket = qobject_cast<QAbstractSocket*>(sender());
    if (socket == nullptr) {
        return;
    }
    // errorString 此时是新鲜的（slot 同步派发）；空字符串也算有用信息（不阻塞 UI）
    appendLog("!", "Socket error: " + socket->errorString());
}

void NetworkToolWindow::onServerAcceptError() {
    if (tcp_server_ == nullptr) {
        return;
    }
    appendLog("!", "Server accept error: " + tcp_server_->errorString());
}

// ============================================================================
// 日志 + 状态栏
// ============================================================================
void NetworkToolWindow::appendLog(const QString& direction, const QString& text) {
    const QString stamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    const QString line = QString("[%1] %2 %3").arg(stamp, direction, text);
    // 追加到末尾（insertPlainText 不强制换行，保持逐片到达的流式显示）
    receive_edit_->moveCursor(QTextCursor::End);
    receive_edit_->insertPlainText(line + "\n");
    receive_edit_->moveCursor(QTextCursor::End);
}

void NetworkToolWindow::updateStatus() {
    const QString proto_name = protocol_combo_->currentText();
    QString state;
    QString detail;

    const int proto = protocol_combo_->currentIndex();
    switch (proto) {
        case kProtoTcpServer:
            if (tcp_server_ != nullptr) {
                state = "Listening";
                detail = QString("port %1 · %2 client(s)")
                             .arg(tcp_server_->serverPort())
                             .arg(tcp_clients_.size());
            } else {
                state = "Stopped";
            }
            break;
        case kProtoTcpClient:
            if (tcp_client_ != nullptr) {
                const auto s = tcp_client_->state();
                state = (s == QAbstractSocket::ConnectedState) ? "Connected" : "Connecting";
                detail = QString("%1:%2")
                             .arg(tcp_client_->peerAddress().toString())
                             .arg(tcp_client_->peerPort());
            } else {
                state = "Stopped";
            }
            break;
        case kProtoUdp:
            if (udp_socket_ != nullptr) {
                state = "Bound";
                detail = QString("port %1").arg(udp_socket_->localPort());
            } else {
                state = "Stopped";
            }
            break;
        default:
            state = "Stopped";
            break;
    }

    status_label_->setText(QString("%1  ·  %2  ·  %3  ·  RX %4 bytes  ·  TX %5 bytes")
                               .arg(proto_name)
                               .arg(state)
                               .arg(detail.isEmpty() ? QStringLiteral("-") : detail)
                               .arg(rx_bytes_)
                               .arg(tx_bytes_));
}
