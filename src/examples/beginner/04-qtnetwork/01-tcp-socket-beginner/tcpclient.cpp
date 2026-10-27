#include "tcpclient.h"

#include <QCoreApplication>
#include <QTimer>
#include <QDebug>

TcpClient::TcpClient(QObject *parent)
    : QObject(parent),
      m_socket(new QTcpSocket(this)),
      m_retryCount(0),
      m_maxRetries(3)
{
    // 连接成功
    connect(m_socket, &QTcpSocket::connected, this, [this]() {
        qDebug() << "[Client] Connected to server!";
        m_retryCount = 0;

        // 发送测试消息
        m_socket->write("Hello from Qt TCP Client!");
        m_socket->flush();
        qDebug() << "[Client] Sent: Hello from Qt TCP Client!";
    });

    // 收到服务器回复
    connect(m_socket, &QTcpSocket::readyRead, this, [this]() {
        QByteArray data = m_socket->readAll();
        qDebug() << "[Client] Server replied:" << data;
    });

    // 连接断开
    connect(m_socket, &QTcpSocket::disconnected, this, [this]() {
        qDebug() << "[Client] Disconnected from server.";
    });

    // 错误处理
    connect(m_socket, &QAbstractSocket::errorOccurred, this,
            [this](QAbstractSocket::SocketError error) {
        qDebug() << "[Client] Error:" << error
                 << m_socket->errorString();

        // 如果还没超过最大重试次数，延迟重连
        if (m_retryCount < m_maxRetries) {
            attemptReconnect();
        } else {
            qDebug() << "[Client] Max retries reached, giving up.";
            QCoreApplication::quit();
        }
    });
}

void TcpClient::connectToServer(const QHostAddress &host, quint16 port)
{
    m_host = host;
    m_port = port;
    qDebug() << "[Client] Connecting to"
             << host.toString() << ":" << port << "...";
    m_socket->connectToHost(host, port);
}

void TcpClient::sendMessage(const QByteArray &data)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->write(data);
        m_socket->flush();
        qDebug() << "[Client] Sent:" << data;
    } else {
        qDebug() << "[Client] Not connected, cannot send.";
    }
}

void TcpClient::disconnectFromServer()
{
    m_socket->disconnectFromHost();
}

void TcpClient::attemptReconnect()
{
    ++m_retryCount;
    int delayMs = 1000 * (1 << (m_retryCount - 1));

    qDebug() << "[Client] Retrying in"
             << delayMs / 1000 << "second(s)..."
             << "(attempt" << m_retryCount << "/" << m_maxRetries << ")";

    QTimer::singleShot(delayMs, this, [this]() {
        qDebug() << "[Client] Reconnecting...";
        m_socket->connectToHost(m_host, m_port);
    });
}
