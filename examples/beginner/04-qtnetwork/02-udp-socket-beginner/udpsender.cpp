#include "udpsender.h"

#include <QDebug>

UdpSender::UdpSender(QObject *parent)
    : QObject(parent), m_socket(new QUdpSocket(this))
{
    // 绑定一个随机端口用于接收回复
    m_socket->bind(QHostAddress::Any, 0);

    connect(m_socket, &QUdpSocket::readyRead, this, [this]() {
        while (m_socket->hasPendingDatagrams()) {
            QByteArray buffer;
            buffer.resize(
                static_cast<int>(m_socket->pendingDatagramSize()));
            QHostAddress addr;
            quint16 port;
            m_socket->readDatagram(
                buffer.data(), buffer.size(), &addr, &port);
            qDebug() << "[Sender] Reply from"
                     << addr.toString() << ":" << buffer;
        }
    });
}

void UdpSender::sendUnicast(const QByteArray &data,
                             const QHostAddress &addr,
                             quint16 port)
{
    qint64 written = m_socket->writeDatagram(data, addr, port);
    if (written == -1) {
        qDebug() << "[Sender] Unicast failed:"
                 << m_socket->errorString();
    } else {
        qDebug() << "[Sender] Unicast" << written
                 << "bytes to" << addr.toString() << ":" << port
                 << "->" << data;
    }
}

void UdpSender::sendBroadcast(const QByteArray &data, quint16 port)
{
    qint64 written = m_socket->writeDatagram(
        data, QHostAddress::Broadcast, port);
    if (written == -1) {
        qDebug() << "[Sender] Broadcast failed:"
                 << m_socket->errorString();
    } else {
        qDebug() << "[Sender] Broadcast" << written
                 << "bytes on port" << port
                 << "->" << data;
    }
}
