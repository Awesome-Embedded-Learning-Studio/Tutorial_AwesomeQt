/// @file    chat_server.h
/// @brief   WebSocket chat server with room management and message broadcasting.
///
/// 对应教程：进阶层 05-其他模块/19-QtWebSockets Server。
/// 演示 QWebSocketServer 的房间管理、客户端追踪与定向广播。

#pragma once

#include <QHash>
#include <QList>
#include <QObject>
#include <QWebSocket>
#include <QWebSocketServer>

#include <memory>

/// @brief 单个客户端的上下文信息。
struct ClientInfo
{
    QWebSocket* socket;   ///< 底层 WebSocket 连接（服务器拥有所有权）
    QString username;     ///< 客户端显示名
    QString room;         ///< 当前所在房间名，空串表示未加入
};

/// @brief 基于 QWebSocketServer 的聊天服务器。
///
/// 支持 JSON 文本协议：
///   - {"type":"join","room":"general"}   加入/切换房间
///   - {"type":"msg","text":"hello"}      向同房间广播消息
///   - {"type":"pm","to":"user","text":""} 房间内私信
///   - {"type":"leave"}                   离开当前房间
class ChatServer : public QObject
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父对象，用于 Qt 对象树生命周期管理。
    explicit ChatServer(QObject* parent = nullptr);

    /// @brief 析构函数，关闭所有客户端连接并释放服务器资源。
    ~ChatServer() override;

    // 禁止拷贝和移动
    ChatServer(const ChatServer&) = delete;
    ChatServer& operator=(const ChatServer&) = delete;
    ChatServer(ChatServer&&) = delete;
    ChatServer& operator=(ChatServer&&) = delete;

    /// @brief 在指定端口启动 WebSocket 服务器。
    /// @param[in] port 监听端口号。
    /// @return true 表示成功开始监听，false 表示端口被占用或其他错误。
    bool start(quint16 port);

    /// @brief 获取当前所有房间的名称列表。
    /// @return 房间名列表（去重后）。
    QStringList rooms() const;

    /// @brief 获取指定房间内的客户端数量。
    /// @param[in] room 房间名称。
    /// @return 该房间内的客户端数量。
    int clientCount(const QString& room) const;

signals:
    /// @brief 服务器运行日志信号，用于外部打印或记录。
    /// @param[in] msg 日志消息文本。
    void serverInfo(const QString& msg);

private slots:
    /// @brief 有新客户端连接时由 QWebSocketServer 触发。
    void onNewConnection();

    /// @brief 处理客户端发来的文本消息（JSON 协议）。
    /// @param[in] message 原始文本帧内容。
    void onTextMessageReceived(const QString& message);

    /// @brief 客户端断开连接时清理资源。
    void onClientDisconnected();

private:
    /// @brief 根据 WebSocket 指针查找对应的 ClientInfo。
    /// @param[in] socket 客户端 WebSocket 连接。
    /// @return 指向 ClientInfo 的指针，未找到则返回 nullptr。
    ClientInfo* findClient(QWebSocket* socket);

    /// @brief 向同一房间内的其他客户端广播消息。
    /// @param[in] sender 发送者的 WebSocket 连接。
    /// @param[in] text 要广播的消息文本。
    void broadcastToRoom(QWebSocket* sender, const QString& text);

    /// @brief 向指定用户名发送私信。
    /// @param[in] sender 发送者的 WebSocket。
    /// @param[in] targetName 目标用户名。
    /// @param[in] text 私信内容。
    /// @return true 表示找到目标用户并已发送。
    bool sendPrivateMessage(QWebSocket* sender, const QString& targetName,
                            const QString& text);

    /// @brief 向单个客户端发送 JSON 消息。
    /// @param[in] socket 目标 WebSocket 连接。
    /// @param[in] type   消息类型字段。
    /// @param[in] data   附加数据键值对。
    void sendJson(QWebSocket* socket, const QString& type,
                  const QHash<QString, QString>& data);

    /// @brief 重新计算并广播指定房间的客户端计数。
    /// @param[in] room 房间名称。
    void broadcastRoomCount(const QString& room);

    /// @brief 服务器级别的唯一 WebSocket 监听器。
    std::unique_ptr<QWebSocketServer> m_server;

    /// @brief 所有已连接客户端的信息列表。
    /// @note 使用 QList<ClientInfo*> 存储；析构和断开时手动删除。
    QList<ClientInfo*> m_clients;
};
