/// @file    reconnect_client.cpp
/// @brief   WebSocket 重连客户端的实现。
///
/// @details 实现指数退避重连策略和 Ping/Pong 心跳保活逻辑。
///          对应教程：进阶层 04-QtNetwork/04-WebSocket。

#include "reconnect_client.h"

#include <QAbstractSocket>
#include <QTimer>
#include <QWebSocket>

ReconnectClient::ReconnectClient(QObject* parent)
    : QObject(parent)
    , m_socket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this))
    , m_state(State::kDisconnected)
    , m_heartbeatTimer(new QTimer(this))
    , m_pongTimeoutTimer(new QTimer(this))
    , m_waitingForPong(false)
    , m_reconnectDelayMs(kInitialReconnectDelayMs)
    , m_intentionalDisconnect(false)
{
    // 心跳定时器设置为单次触发，每次收到 Pong 后手动重启
    m_heartbeatTimer->setSingleShot(true);
    m_pongTimeoutTimer->setSingleShot(true);

    connect(m_socket, &QWebSocket::connected,
            this, &ReconnectClient::onConnected);
    connect(m_socket, &QWebSocket::disconnected,
            this, &ReconnectClient::onDisconnected);
    connect(m_socket, &QWebSocket::errorOccurred,
            this, &ReconnectClient::onError);
    connect(m_socket, &QWebSocket::pong,
            this, &ReconnectClient::onPong);
    connect(m_socket, &QWebSocket::textMessageReceived,
            this, &ReconnectClient::onTextMessageReceived);

    connect(m_heartbeatTimer, &QTimer::timeout,
            this, &ReconnectClient::onHeartbeatTimeout);
    connect(m_pongTimeoutTimer, &QTimer::timeout,
            this, &ReconnectClient::onPongTimeout);
}

ReconnectClient::~ReconnectClient()
{
    // @note 析构时必须主动断开，防止信号槽在对象销毁过程中触发
    disconnect();
}

void ReconnectClient::connectToServer(const QUrl& url)
{
    m_serverUrl = url;
    m_intentionalDisconnect = false;
    m_reconnectDelayMs = kInitialReconnectDelayMs;

    if (m_state == State::kConnected || m_state == State::kConnecting)
    {
        // @note 先关闭旧连接，等待 onDisconnected 再自动重连
        m_intentionalDisconnect = false;
        m_socket->close(QWebSocketProtocol::CloseCodeNormal, QLatin1String("Reconnecting"));
        setState(State::kReconnecting);
        return;
    }

    attemptConnect();
}

void ReconnectClient::disconnect()
{
    m_intentionalDisconnect = true;
    stopHeartbeat();
    m_socket->close(QWebSocketProtocol::CloseCodeNormal, QLatin1String("Client disconnect"));
    setState(State::kDisconnected);
}

void ReconnectClient::sendMessage(const QString& message)
{
    if (m_state != State::kConnected)
    {
        return;
    }

    m_socket->sendTextMessage(message);
}

ReconnectClient::State ReconnectClient::currentState() const
{
    return m_state;
}

void ReconnectClient::onConnected()
{
    setState(State::kConnected);
    m_reconnectDelayMs = kInitialReconnectDelayMs;  // 连接成功，重置退避延迟
    startHeartbeat();
}

void ReconnectClient::onDisconnected()
{

    stopHeartbeat();

    if (m_intentionalDisconnect)
    {
        setState(State::kDisconnected);
        return;
    }

    // 非主动断开，触发自动重连
    setState(State::kReconnecting);
    scheduleReconnect();
}

void ReconnectClient::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)

    // @note 错误发生后 QWebSocket 会随之触发 disconnected 信号，
    //       因此这里只打印信息，重连逻辑放在 onDisconnected 中处理
    qDebug("ReconnectClient: socket error occurred - %s",
           qPrintable(m_socket->errorString()));
}

void ReconnectClient::onPong(quint64 elapsedTime, const QByteArray& payload)
{
    Q_UNUSED(elapsedTime)
    Q_UNUSED(payload)

    m_waitingForPong = false;
    m_pongTimeoutTimer->stop();

    qDebug("ReconnectClient: pong received, connection alive");

    // 收到 Pong 后，重新启动下一轮心跳
    m_heartbeatTimer->start(kHeartbeatIntervalMs);
}

void ReconnectClient::onTextMessageReceived(const QString& message)
{
    emit messageReceived(message);
}

void ReconnectClient::onHeartbeatTimeout()
{
    if (m_state != State::kConnected)
    {
        return;
    }

    if (m_waitingForPong)
    {
        // @note 上一轮 Ping 尚未收到 Pong，判定连接已死
        qDebug("ReconnectClient: pong timeout detected, connection is dead");
        stopHeartbeat();
        m_socket->close(QWebSocketProtocol::CloseCodeNormal,
                        QLatin1String("Heartbeat timeout"));
        return;
    }

    // 发送 Ping，启动 Pong 超时检测
    m_waitingForPong = true;
    m_socket->ping();
    m_pongTimeoutTimer->start(kPongTimeoutMs);

    qDebug("ReconnectClient: ping sent, waiting for pong");
}

void ReconnectClient::onPongTimeout()
{
    if (m_state != State::kConnected)
    {
        return;
    }

    // @note Pong 超时意味着连接已经无响应，主动关闭触发重连
    qDebug("ReconnectClient: pong timeout, closing connection to trigger reconnect");
    stopHeartbeat();
    m_socket->close(QWebSocketProtocol::CloseCodeNormal,
                    QLatin1String("Pong timeout"));
}

void ReconnectClient::attemptConnect()
{
    setState(State::kConnecting);
    m_socket->open(m_serverUrl);
}

void ReconnectClient::scheduleReconnect()
{
    qDebug("ReconnectClient: scheduling reconnect in %d ms", m_reconnectDelayMs);

    // @note QTimer::singleShot 保证不会阻塞事件循环，到时触发 attemptConnect
    QTimer::singleShot(m_reconnectDelayMs, this, [this]() {
        if (m_intentionalDisconnect)
        {
            return;
        }
        attemptConnect();
    });

    // 指数退避：延迟翻倍，但不超过上限
    m_reconnectDelayMs = qMin(m_reconnectDelayMs * kReconnectMultiplier,
                              kMaxReconnectDelayMs);
}

void ReconnectClient::setState(State newState)
{
    if (m_state == newState)
    {
        return;
    }

    m_state = newState;
    emit stateChanged(newState);
}

void ReconnectClient::startHeartbeat()
{
    m_waitingForPong = false;
    m_heartbeatTimer->start(kHeartbeatIntervalMs);
}

void ReconnectClient::stopHeartbeat()
{
    m_heartbeatTimer->stop();
    m_pongTimeoutTimer->stop();
    m_waitingForPong = false;
}
