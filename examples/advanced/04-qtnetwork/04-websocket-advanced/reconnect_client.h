/// @file    reconnect_client.h
/// @brief   WebSocket client with exponential backoff reconnection and heartbeat.
///
/// @details 对应教程：进阶层 04-QtNetwork/04-WebSocket。
///          演示 QWebSocket 的断线重连（指数退避）和 Ping/Pong 心跳保活机制。

#pragma once

#include <QElapsedTimer>
#include <QObject>
#include <QTimer>
#include <QUrl>
#include <QWebSocket>

/// @brief WebSocket 客户端，支持自动重连和心跳保活。
class ReconnectClient : public QObject
{
    Q_OBJECT

public:
    /// @brief 连接状态枚举。
    enum class State
    {
        kDisconnected,  ///< 已断开，未尝试连接
        kConnecting,    ///< 正在建立连接
        kConnected,     ///< 已连接，可收发消息
        kReconnecting   ///< 连接中断，正在重连
    };
    Q_ENUM(State)

    /// @brief 构造函数。
    /// @param[in] parent 父对象指针，Qt 对象树管理生命周期。
    explicit ReconnectClient(QObject* parent = nullptr);

    /// @brief 析构函数，确保资源清理。
    ~ReconnectClient() override;

    /// @brief 发起 WebSocket 连接。
    /// @param[in] url 目标 WebSocket 服务器地址（如 ws://localhost:12345）。
    /// @note 调用后进入 kConnecting 状态；若当前已连接则先断开再重连。
    void connectToServer(const QUrl& url);

    /// @brief 主动断开连接，不触发自动重连。
    /// @note 将状态置为 kDisconnected，取消所有定时器。
    void disconnect();

    /// @brief 向服务器发送文本消息。
    /// @param[in] message 要发送的文本内容。
    /// @note 仅在 kConnected 状态下才实际发送，否则丢弃。
    void sendMessage(const QString& message);

    /// @brief 获取当前连接状态。
    /// @return 当前 State 枚举值。
    State currentState() const;

signals:
    /// @brief 连接状态发生变化。
    /// @param[in] state 新的状态值。
    void stateChanged(State state);

    /// @brief 收到服务器发来的文本消息。
    /// @param[in] message 接收到的文本内容。
    void messageReceived(const QString& message);

private slots:
    /// @brief WebSocket 连接成功回调。
    void onConnected();

    /// @brief WebSocket 连接断开回调。
    /// @note 非主动断开时，触发指数退避重连。
    void onDisconnected();

    /// @brief WebSocket 错误回调。
    /// @param[in] error 错误类型。
    void onError(QAbstractSocket::SocketError error);

    /// @brief 收到 Pong 回复回调。
    /// @param[in] elapsedTime 服务端回复耗时（毫秒）。
    /// @param[in] payload Pong 附带的数据。
    /// @note 重置心跳计时器，标记连接存活。
    void onPong(quint64 elapsedTime, const QByteArray& payload);

    /// @brief 收到文本消息回调。
    /// @param[in] message 接收到的文本内容。
    void onTextMessageReceived(const QString& message);

    /// @brief 心跳定时器超时回调，发送 Ping 并检测 Pong 超时。
    /// @note 每次发送 Ping 后启动 Pong 超时检测；
    ///       若上一轮 Ping 尚未收到 Pong，判定连接已死，触发重连。
    void onHeartbeatTimeout();

    /// @brief Pong 超时定时器回调，判定连接已死。
    void onPongTimeout();

private:
    /// @brief 尝试建立一次连接（内部使用）。
    void attemptConnect();

    /// @brief 调度下一次重连，计算指数退避延迟。
    void scheduleReconnect();

    /// @brief 更新状态并发射信号。
    /// @param[in] newState 目标状态。
    void setState(State newState);

    /// @brief 启动心跳定时器。
    /// @note 连接成功后调用，每隔 kHeartbeatIntervalMs 发一次 Ping。
    void startHeartbeat();

    /// @brief 停止心跳定时器和 Pong 超时检测。
    void stopHeartbeat();

    QWebSocket* m_socket;           ///< WebSocket 连接实例
    QUrl m_serverUrl;               ///< 目标服务器地址
    State m_state;                  ///< 当前连接状态

    QTimer* m_heartbeatTimer;       ///< 心跳 Ping 定时器
    QTimer* m_pongTimeoutTimer;     ///< Pong 超时检测定时器
    bool m_waitingForPong;          ///< 是否正在等待 Pong 回复

    int m_reconnectDelayMs;         ///< 当前重连延迟（毫秒）
    bool m_intentionalDisconnect;   ///< 是否为主动断开（不重连）

    // 常量配置
    static constexpr int kInitialReconnectDelayMs = 1000;   ///< 初始重连延迟 1 秒
    static constexpr int kMaxReconnectDelayMs = 30000;      ///< 最大重连延迟 30 秒
    static constexpr int kReconnectMultiplier = 2;          ///< 退避倍数
    static constexpr int kHeartbeatIntervalMs = 10000;      ///< 心跳间隔 10 秒
    static constexpr int kPongTimeoutMs = 5000;             ///< Pong 超时 5 秒
};
