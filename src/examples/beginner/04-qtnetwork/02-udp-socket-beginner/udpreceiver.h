/// UDP 接收器：绑定指定端口并打印收到的所有数据报
#pragma once

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>

class UdpReceiver : public QObject
{
    Q_OBJECT

public:
    explicit UdpReceiver(quint16 port, QObject *parent = nullptr);

private:
    void readPendingDatagrams();

    /// 获取本机首选局域网 IP（非 loopback）
    static QString getLocalIP();

    QUdpSocket *m_socket;
    quint16 m_port;
};
