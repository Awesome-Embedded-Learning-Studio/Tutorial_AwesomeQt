/// UDP 发送器：支持单播和广播发送
#pragma once

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>

class UdpSender : public QObject
{
    Q_OBJECT

public:
    explicit UdpSender(QObject *parent = nullptr);

    /// 向指定地址发送数据报（单播）
    void sendUnicast(const QByteArray &data,
                     const QHostAddress &addr,
                     quint16 port);

    /// 向整个子网广播数据报
    void sendBroadcast(const QByteArray &data, quint16 port);

private:
    QUdpSocket *m_socket;
};
