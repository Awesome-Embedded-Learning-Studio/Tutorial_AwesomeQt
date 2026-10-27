/// @file    chat_backend.h
/// @brief   Backend object exposed to remote clients via QWebChannel.
///
/// Demonstrates how a QObject subclass with Q_PROPERTY, Q_INVOKABLE, and
/// signals can be published through QWebChannel so that any WebSocket client
/// (browser JS, another Qt app, etc.) can invoke methods and react to
/// signals without QtWebEngine.

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>

/// @brief Chat-like backend published through QWebChannel.
///
/// Remote clients can call sendMessage() and getMessageHistory(), read
/// lastMessage / messageCount properties, and connect to the
/// messageReceived signal — all through the WebChannel bridge.
class ChatBackend : public QObject
{
    Q_OBJECT

    /// Last message text received by the backend.
    Q_PROPERTY(QString lastMessage READ lastMessage NOTIFY lastMessageChanged)
    /// Total number of messages processed so far.
    Q_PROPERTY(int messageCount READ messageCount NOTIFY messageCountChanged)

public:
    /// @brief Constructs the backend with an optional parent for ownership.
    /// @param[in] parent  Parent QObject for Qt object-tree lifecycle.
    explicit ChatBackend(QObject* parent = nullptr);

    /// @brief Returns the text of the most recently received message.
    /// @return The last message string, or empty if none yet.
    QString lastMessage() const;

    /// @brief Returns the total number of messages that have been sent.
    /// @return Message count.
    int messageCount() const;

    /// @brief Sends a chat message and triggers an auto-response for demo.
    /// @param[in] text  The message body from the remote client.
    Q_INVOKABLE void sendMessage(const QString& text);

    /// @brief Returns the full message history accumulated so far.
    /// @return A QStringList containing every message in order.
    Q_INVOKABLE QStringList getMessageHistory();

signals:
    /// @brief Emitted whenever a new message arrives (from client or auto).
    /// @param from  Identifier of the sender ("Client" or "Server").
    /// @param text  The message body.
    void messageReceived(const QString& from, const QString& text);

    /// @brief Notifies that the lastMessage property has changed.
    void lastMessageChanged();

    /// @brief Notifies that the messageCount property has changed.
    void messageCountChanged();

private:
    /// @brief Generates a canned auto-response for demonstration.
    /// @param[in] userText  The user message to base the response on.
    void generateAutoResponse(const QString& userText);

    QVector<QString> m_history;  ///< All messages in chronological order.
    QString m_lastMessage;       ///< Cached last message for Q_PROPERTY read.
    int m_messageCount = 0;      ///< Running counter of total messages.
};
