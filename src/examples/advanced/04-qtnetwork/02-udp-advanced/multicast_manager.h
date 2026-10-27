/// @file    multicast_manager.h
/// @brief   UDP 组播管理器，封装组播组的加入/离开与数据收发。
///
/// @details 对应教程：进阶层 04-QtNetwork/02-UDP 高级用法。
///          演示 QUdpSocket 的组播（multicast）机制，包括 joinMulticastGroup、
///          leaveMulticastGroup 以及组播数据的发送与接收。

#pragma once

#include <QHostAddress>
#include <QObject>
#include <QUdpSocket>

/// @brief UDP 组播管理器，负责加入指定组播组并收发数据报。
///
/// 使用 QUdpSocket 绑定本地端口后加入组播地址，通过 readyRead 信号驱动
/// 异步接收，收到完整数据报后转发给上层。
class MulticastManager : public QObject
{
    Q_OBJECT

public:
    /// @brief 构造函数，创建 socket 并绑定到指定端口。
    /// @param[in] bindPort       本地绑定端口。
    /// @param[in] multicastPort  组播发送目标端口（接收方应绑定此端口）。
    /// @param[in] parent         父对象指针，Qt 对象树管理生命周期。
    /// @note  bindPort 与 multicastPort 可以不同：发送方绑定任意可用端口，
    ///        但发送到接收方正在监听的 multicastPort。ShareAddress 模式允许多个
    ///        socket 绑定同一端口，因此同一主机可同时运行收发端。
    explicit MulticastManager(quint16 bindPort, quint16 multicastPort,
                              QObject* parent = nullptr);

    /// @brief 析构函数，自动离开组播组并关闭 socket。
    ~MulticastManager() override;

    /// @brief 加入指定的组播地址。
    /// @param[in] groupAddress 组播组地址，须在 224.0.0.0 ~ 239.255.255.255 范围内。
    /// @return 成功返回 true；失败时可通过 socket 的 error() 查看原因。
    /// @note   必须在 socket 绑定成功后调用，否则 join 会失败。
    bool joinGroup(const QHostAddress& groupAddress);

    /// @brief 离开当前已加入的组播组。
    /// @note  析构时会自动调用，也可手动提前离开。
    void leaveGroup();

    /// @brief 向组播组发送数据报。
    /// @param[in] data  要发送的原始字节序列。
    /// @return 实际写入的字节数；-1 表示发送失败。
    /// @note  发送目标为构造时设定的组播地址和端口，数据报经底层路由分发给组内所有成员。
    qint64 sendMessage(const QByteArray& data);

    /// @brief 获取当前绑定的本地端口。
    /// @return 端口号，未绑定时返回 0。
    quint16 boundPort() const;

signals:
    /// @brief 收到组播数据报时发射。
    /// @param sender  发送方的 IP 地址。
    /// @param data    完整的数据报负载（不含 UDP 头）。
    void messageReceived(const QHostAddress& sender, const QByteArray& data);

private slots:
    /// @brief readyRead 信号的槽函数，读取到达的数据报。
    void onReadyRead();

private:
    QUdpSocket* m_socket;          ///< UDP socket，由本对象持有
    QHostAddress m_groupAddress;   ///< 当前加入的组播组地址
    quint16 m_bindPort;            ///< 本地绑定端口
    quint16 m_multicastPort;       ///< 组播发送目标端口
    bool m_joined;                 ///< 是否已成功加入组播组
};
