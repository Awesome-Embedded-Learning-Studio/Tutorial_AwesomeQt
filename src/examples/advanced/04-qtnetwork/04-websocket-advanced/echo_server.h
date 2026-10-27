/// @file    echo_server.h
/// @brief   简易 WebSocket Echo 服务器，用于测试重连和心跳逻辑。
///
/// @details 对应教程：进阶层 04-QtNetwork/04-WebSocket。
///          使用 QWebSocketServer 监听端口，将收到的文本消息原样回传。

#pragma once

#include <QList>
#include <QObject>
#include <QWebSocket>
#include <QWebSocketServer>

/// @brief WebSocket Echo 服务器，将收到的消息原样回传。
class EchoServer : public QObject
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父对象指针，Qt 对象树管理生命周期。
    explicit EchoServer(QObject* parent = nullptr);

    /// @brief 析构函数，关闭服务器并清理所有客户端连接。
    ~EchoServer() override;

    /// @brief 在指定端口启动监听。
    /// @param[in] port 监听端口号。
    /// @return 成功返回 true，端口被占用或其它错误返回 false。
    /// @note 只能在未启动状态下调用。
    bool start(quint16 port);

    /// @brief 停止服务器，断开所有客户端连接。
    /// @note 断开后客户端会触发 disconnected 信号，便于测试重连逻辑。
    void stop();

    /// @brief 获取当前服务器是否正在运行。
    /// @return 正在监听返回 true。
    bool isRunning() const;

    /// @brief 获取当前已连接的客户端数量。
    /// @return 活跃连接数。
    int clientCount() const;

signals:
    /// @brief 有新的客户端连接成功。
    void clientConnected();

    /// @brief 有客户端断开连接。
    /// @param[in] client 断开的客户端指针。
    void clientDisconnected(QWebSocket* client);

    /// @brief 服务器收到并回传了一条消息。
    /// @param[in] message 回传的消息内容。
    void messageEchoed(const QString& message);

private slots:
    /// @brief 有新连接接入回调。
    void onNewConnection();

    /// @brief 收到客户端文本消息回调。
    /// @param[in] message 收到的文本内容。
    void onTextMessageReceived(const QString& message);

    /// @brief 客户端断开连接回调。
    void onClientDisconnected();

private:
    QWebSocketServer* m_server;                 ///< WebSocket 服务器实例
    QList<QWebSocket*> m_clients;               ///< 当前已连接的客户端列表
};
