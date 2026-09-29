/**
 * @file network_tool_window.h
 * @brief 网络调试助手主窗口——协议选择 / 配置面板 / 收发区 / 状态栏
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include <QHostAddress>
#include <QMainWindow>

class QComboBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QTcpServer;
class QTcpSocket;
class QUdpSocket;

/// @brief 网络调试助手（app 栏整机范式）。
///
/// QMainWindow：顶部协议选择 + 配置面板（本机端口 / 目标 IP+端口）+ Start/Stop 按钮，
/// 中部只读接收区（QPlainTextEdit 日志，带时间戳与方向 ←→），底部发送区（QLineEdit + Send），
/// 状态栏显示协议 + 连接态 + RX/TX 字节计数。三种模式互斥，切换前先 Stop 旧实例。
///
/// 三种协议：
/// - **TCP Server**：QTcpServer::listen(QHostAddress::Any, port)，newConnection 接客户端
///   QTcpSocket，readyRead 收；多客户端用 QList 管理，断开 deleteLater。
/// - **TCP Client**：QTcpSocket::connectToHost(ip, port)，connected/disconnected/readyRead
///   信号驱动，write 发；断开后禁止 write（connected 信号态为准）。
/// - **UDP**：QUdpSocket::bind(Any, port)，receiveDatagram 收、writeDatagram(datagram, target) 发。
///
/// 关键设计：①端口 0 = 系统分配，serverPort() 取实际端口，绝不假设；②readyRead 异步，数据
/// 可能分片到达——逐块 readAll/readDatagram 累积再显示，不假设一次到齐；③UDP 收用
/// hasPendingDatagrams + receiveDatagram（不是 readAll）；④端口占用/绑定失败读 errorString 反馈；
/// ⑤客户端断开处理 connected/disconnected 信号，断开后 write 直接拒绝。
class NetworkToolWindow : public QMainWindow {
    Q_OBJECT
  public:
    explicit NetworkToolWindow(QWidget* parent = nullptr);
    ~NetworkToolWindow() override;

  private slots:
    void onProtocolChanged();  // 切协议：更新配置控件可见性（不清资源）
    void onStartStopToggled(); // Start/Stop：同一按钮两态切换
    void onSend();             // 发送区 → 按当前协议写入
    void onTcpNewConnection(); // TCP Server：接客户端 QTcpSocket
    void onTcpReadyRead();     // TCP（Server 客户端 / Client 自身）：收数据
    void onTcpConnected();     // TCP Client：连上 → 状态栏 + 解锁发送
    void onTcpDisconnected(QTcpSocket* socket = nullptr); // TCP：客户端断开（Server 传 socket）
    void onUdpReadyRead();                                // UDP：hasPendingDatagrams 循环收
    void onSocketErrorOccurred();                         // 各 socket error → 状态栏报错
    void onServerAcceptError();                           // TCP Server accept 失败 → 状态栏报错

  private:
    void setupCentral();
    void setupStatusBar();
    void setupConnections();

    void teardownAll(); // Stop：按当前协议 close 并 deleteLater 全部 socket/server
    void refreshRunningControls(bool running);
    void refreshConfigVisibility(); // 协议决定本机端口 / 目标 IP+端口 哪些可见
    void updateStatus();
    void appendLog(const QString& direction, const QString& text); // ←/→ + 时间戳 + 文本

    // 协议选择 + 配置面板
    QComboBox* protocol_combo_{nullptr};
    QLabel* local_port_label_{nullptr};
    QLineEdit* local_port_edit_{nullptr};
    QLabel* target_label_{nullptr};
    QLineEdit* target_ip_edit_{nullptr};
    QLineEdit* target_port_edit_{nullptr};
    QPushButton* start_stop_button_{nullptr};

    // 收发区
    QPlainTextEdit* receive_edit_{nullptr};
    QLineEdit* send_edit_{nullptr};
    QPushButton* send_button_{nullptr};

    // 状态栏
    QLabel* status_label_{nullptr};

    // 运行态资源（按协议存在其中之一）
    QTcpServer* tcp_server_{nullptr}; // TCP Server 模式
    QList<QTcpSocket*> tcp_clients_;  // TCP Server：已连接的客户端集合
    QTcpSocket* tcp_client_{nullptr}; // TCP Client 模式：自身连接 socket
    QUdpSocket* udp_socket_{nullptr}; // UDP 模式

    qint64 rx_bytes_ = 0; // 累计接收字节数
    qint64 tx_bytes_ = 0; // 累计发送字节数
};
