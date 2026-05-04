#include "chatclient.h"

#include <QDateTime>
#include <QDebug>

ChatClient::ChatClient(QObject *parent)
    : QObject(parent),
      m_ws(new QWebSocket()),
      m_heartbeatTimer(new QTimer(this)),
      m_messageCount(0)
{
    // 连接成功
    connect(m_ws, &QWebSocket::connected, this, [this]() {
        qDebug() << "[Client] Connected to server!";
        m_messageCount = 0;

        // 启动心跳：每 10 秒发送一次 ping
        m_heartbeatTimer->start(10000);
    });

    // 收到文本消息
    connect(m_ws, &QWebSocket::textMessageReceived, this,
            [this](const QString &message) {
        ++m_messageCount;
        qDebug() << "[Client] Text received (#"
                 << m_messageCount << "):" << message;
    });

    // 收到二进制消息
    connect(m_ws, &QWebSocket::binaryMessageReceived, this,
            [](const QByteArray &data) {
        qDebug() << "[Client] Binary received:"
                 << data.size() << "bytes";
    });

    // pong 回复（心跳响应）
    connect(m_ws, &QWebSocket::pong, this,
            [](quint64 elapsedTime, const QByteArray &payload) {
        qDebug() << "[Client] Pong received:" << payload
                 << "(elapsed:" << elapsedTime << "ms)";
    });

    // 连接断开
    connect(m_ws, &QWebSocket::disconnected, this, [this]() {
        qDebug() << "[Client] Disconnected.";
        m_heartbeatTimer->stop();
    });

    // 心跳定时器：定期发送 ping
    connect(m_heartbeatTimer, &QTimer::timeout, this, [this]() {
        if (m_ws->state() == QAbstractSocket::ConnectedState) {
            // payload 携带当前时间戳，用于计算往返延迟
            QByteArray payload =
                QByteArray::number(QDateTime::currentMSecsSinceEpoch());
            m_ws->ping(payload);
        }
    });
}

ChatClient::~ChatClient()
{
    m_ws->close();
    m_ws->deleteLater();
}

void ChatClient::connectToServer(const QUrl &url)
{
    qDebug() << "[Client] Connecting to" << url.toString() << "...";
    m_ws->open(url);
}

void ChatClient::sendTextMessage(const QString &message)
{
    if (m_ws->state() == QAbstractSocket::ConnectedState) {
        m_ws->sendTextMessage(message);
        qDebug() << "[Client] Sent:" << message;
    } else {
        qDebug() << "[Client] Not connected, cannot send.";
    }
}

void ChatClient::sendBinaryMessage(const QByteArray &data)
{
    if (m_ws->state() == QAbstractSocket::ConnectedState) {
        m_ws->sendBinaryMessage(data);
        qDebug() << "[Client] Sent binary:" << data.size() << "bytes";
    }
}

void ChatClient::disconnectFromServer()
{
    m_heartbeatTimer->stop();
    m_ws->close();
}
