/// @file    multicast_manager.cpp
/// @brief   UDP 组播管理器的实现。
///
/// @details 对应教程：进阶层 04-QtNetwork/02-UDP 高级用法。
///          实现组播组加入/离开、数据报收发的完整逻辑。

#include "multicast_manager.h"

#include <QNetworkDatagram>
#include <QDebug>

MulticastManager::MulticastManager(quint16 bindPort, quint16 multicastPort,
                                   QObject* parent)
    : QObject(parent)
    , m_socket(new QUdpSocket(this))    // 父对象管理生命周期，无需手动 delete
    , m_bindPort(bindPort)
    , m_multicastPort(multicastPort)
    , m_joined(false)
{
    // ShareAddress 允许多个 socket 同时绑定同一端口，组播场景需要此选项
    bool ok = m_socket->bind(QHostAddress::AnyIPv4, m_bindPort,
                             QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    if (!ok) {
        qWarning() << "MulticastManager: bind failed on port" << m_bindPort
                    << "-" << m_socket->errorString();
        return;
    }

    // 使用 lambda 连接 readyRead，将数据报读取委托给槽函数
    connect(m_socket, &QUdpSocket::readyRead,
            this, &MulticastManager::onReadyRead);
}

MulticastManager::~MulticastManager()
{
    if (m_joined) {
        leaveGroup();
    }
}

bool MulticastManager::joinGroup(const QHostAddress& groupAddress)
{
    if (m_joined) {
        qWarning() << "MulticastManager: already joined a group, leaving first";
        leaveGroup();
    }

    m_groupAddress = groupAddress;

    // 加入组播组前确认地址处于组播范围
    if (!groupAddress.isMulticast()) {
        qWarning() << "MulticastManager:" << groupAddress.toString()
                    << "is not a multicast address";
        return false;
    }

    if (!m_socket->joinMulticastGroup(groupAddress)) {
        qWarning() << "MulticastManager: joinMulticastGroup failed -"
                    << m_socket->errorString();
        return false;
    }

    m_joined = true;
    qDebug() << "MulticastManager: joined group" << groupAddress.toString()
             << "on port" << m_bindPort;
    return true;
}

void MulticastManager::leaveGroup()
{
    if (!m_joined) {
        return;
    }

    if (!m_socket->leaveMulticastGroup(m_groupAddress)) {
        qWarning() << "MulticastManager: leaveMulticastGroup failed -"
                    << m_socket->errorString();
    } else {
        qDebug() << "MulticastManager: left group" << m_groupAddress.toString();
    }

    m_joined = false;
}

qint64 MulticastManager::sendMessage(const QByteArray& data)
{
    // 向组播组地址发送数据报，组内所有成员都会收到
    qint64 written = m_socket->writeDatagram(data, m_groupAddress, m_multicastPort);
    if (written < 0) {
        qWarning() << "MulticastManager: send failed -" << m_socket->errorString();
    }
    return written;
}

quint16 MulticastManager::boundPort() const
{
    return m_socket->localPort();
}

void MulticastManager::onReadyRead()
{
    // 逐个读取到达的数据报，防止缓冲区积压丢失
    while (m_socket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_socket->receiveDatagram();
        if (datagram.isValid()) {
            emit messageReceived(datagram.senderAddress(), datagram.data());
        }
    }
}
