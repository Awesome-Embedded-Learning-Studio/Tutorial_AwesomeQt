/**
 * @file serial_tool_window.h
 * @brief 串口调试助手主窗口——配置面板 / 收发区 / 状态栏 / Hex ↔ ASCII
 * @copyright Copyright (c) 2026 AwesomeQt
 */
#pragma once

#include <QMainWindow>
#include <QSerialPort>

class QComboBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QRadioButton;
class QByteArray;

/// @brief 串口调试助手（app 栏整机范式）。
///
/// QMainWindow：左配置面板（端口 / 波特率 / 数据位 / 停止位 / 校验 / 流控 + 打开/关闭按钮），
/// 右上只读接收区（QPlainTextEdit，Hex/ASCII 单选切换显示），右下发送区（QLineEdit + Send），
/// 状态栏串口开/关 + 收发字节计数。打开按钮在开/关两态互斥切换（同一按钮变文案）。
///
/// 关键设计：①QSerialPortInfo::availablePorts 在无硬件环境下返回空——端口 QComboBox 容忍空列表
/// 并提供「刷新」按钮；②readyRead 是异步信号，一次数据可能分多个 readyRead 到达——readAll 累积
/// 到缓冲再显示，不假设一次到齐；③Hex ↔ ASCII：接收用 QByteArray::toHex() 渲染、发送用
/// QByteArray::fromHex() 解析（先正向校验防 fromHex 静默截断）；ASCII 渲染走 Latin-1 保底；
/// ④切换端口/参数前先 close 旧端口；⑤errorOccurred 对不可恢复错误主动 close 收敛。
class SerialToolWindow : public QMainWindow {
    Q_OBJECT
  public:
    explicit SerialToolWindow(QWidget* parent = nullptr);
    ~SerialToolWindow() override;

  private slots:
    void onOpenCloseToggled(); // 打开/关闭按钮：同一按钮两态切换
    void onRefreshPorts();     // 重新列 availablePorts（热插拔后补设备）
    void onReadyRead();        // readyRead → readAll 累积 → 按显示模式渲染
    void onSend();             // 发送区 → QSerialPort::write（Hex/ASCII 切换）
    void onPortError(QSerialPort::SerialPortError error); // errorOccurred → 状态栏报错

  private:
    void setupCentral();
    void setupStatusBar();
    void wireSerial();

    void refreshPorts();
    void refreshOpenControls(bool open);
    void applyConfigToPort();     // 把当前 QComboBox 选中的配置写到 port_（开之前调）
    void refreshReceiveDisplay(); // 切 Hex/ASCII 时把已收数据按新模式重渲
    void updateStatus();
    void appendReceive(const QByteArray& chunk); // 累积 + 渲染新数据

    /// 把整段字节按当前显示模式渲染成文本（Hex 每字节两 hex + 空格分隔；ASCII 走 Latin-1
    /// 保守显示）。
    QString renderChunk(const QByteArray& bytes, bool hexMode) const;
    QString errorToString(
        QSerialPort::SerialPortError error) const; // errorOccurred 自带映射，避免 errorString stale

    // 配置面板控件
    QComboBox* port_combo_{nullptr};
    QComboBox* baud_combo_{nullptr};
    QComboBox* data_bits_combo_{nullptr};
    QComboBox* stop_bits_combo_{nullptr};
    QComboBox* parity_combo_{nullptr};
    QComboBox* flow_control_combo_{nullptr};
    QPushButton* open_close_button_{nullptr};
    QPushButton* refresh_button_{nullptr};

    // 接收区 / 发送区
    QPlainTextEdit* receive_edit_{nullptr};
    QRadioButton* recv_hex_radio_{nullptr};
    QRadioButton* recv_ascii_radio_{nullptr};
    QLineEdit* send_edit_{nullptr};
    QPushButton* send_button_{nullptr};
    QRadioButton* send_hex_radio_{nullptr};
    QRadioButton* send_ascii_radio_{nullptr};

    // 状态栏
    QLabel* status_label_{nullptr};

    QSerialPort* port_{nullptr}; // 关闭后保留实例，避免反复 new/delete（只 close 不 delete）

    QByteArray receive_buffer_;   // 累积收到的原始字节——切 Hex/ASCII 用整段重渲
    qint64 rx_bytes_ = 0;         // 累计接收字节数
    qint64 tx_bytes_ = 0;         // 累计发送字节数
    bool suppress_error_ = false; // 主动 close 期间屏蔽自身触发的 errorOccurred
};
