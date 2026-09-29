#include "udpreceiver.h"

#include <QHostInfo>
#include <QNetworkInterface>
#include <QDebug>

UdpReceiver::UdpReceiver(quint16 port, QObject *parent)
    : QObject(parent), m_socket(new QUdpSocket(this)), m_port(port)
{
    // 绑定端口，ShareAddress 允许多个 Socket 绑同一端口
    if (!m_socket->bind(QHostAddress::Any, port,
                        QAbstractSocket::ShareAddress)) {
        qDebug() << "[Receiver] Bind failed on port"
                 << port << ":" << m_socket->errorString();
        return;
    }

    qDebug() << "[Receiver] Listening on port" << port;

    // 有数据报到达时触发
    connect(m_socket, &QUdpSocket::readyRead, this, [this]() {
        readPendingDatagrams();
    });
}

void UdpReceiver::readPendingDatagrams()
{
    while (m_socket->hasPendingDatagrams()) {
        // 先查询数据报大小，再分配缓冲区
        qint64 datagramSize = m_socket->pendingDatagramSize();
        QByteArray buffer;
        buffer.resize(static_cast<int>(datagramSize));

        QHostAddress senderAddr;
        quint16 senderPort;

        // 读取数据报，同时获取发送方信息
        qint64 bytesRead = m_socket->readDatagram(
            buffer.data(), buffer.size(),
            &senderAddr, &senderPort);

        if (bytesRead == -1) {
            qDebug() << "[Receiver] Read error:"
                     << m_socket->errorString();
            continue;
        }

        qDebug() << "[Receiver] Got" << bytesRead << "bytes from"
                 << senderAddr.toString() << ":" << senderPort
                 << "->" << buffer;

        // 如果是发现消息，回复本机信息
        if (buffer == "DISCOVER") {
            QByteArray reply = QString("HERE:%1:%2")
                .arg(QHostInfo::localHostName())
                .arg(getLocalIP())
                .toUtf8();
            m_socket->writeDatagram(reply, senderAddr, senderPort);
            qDebug() << "[Receiver] Replied with:" << reply;
        }
    }
}

QString UdpReceiver::getLocalIP()
{
    const auto interfaces = QNetworkInterface::allInterfaces();
    for (const auto &iface : interfaces) {
        // 跳过回环和未启用的接口
        if (iface.flags() & QNetworkInterface::IsLoopBack) continue;
        if (!(iface.flags() & QNetworkInterface::IsUp)) continue;
        if (!(iface.flags() & QNetworkInterface::IsRunning)) continue;

        const auto entries = iface.addressEntries();
        for (const auto &entry : entries) {
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol
                && entry.ip() != QHostAddress::LocalHost) {
                return entry.ip().toString();
            }
        }
    }
    return "127.0.0.1";
}
