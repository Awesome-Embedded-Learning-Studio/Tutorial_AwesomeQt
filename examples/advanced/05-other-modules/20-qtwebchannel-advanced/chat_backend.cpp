/// @file    chat_backend.cpp
/// @brief   Implementation of ChatBackend — the QWebChannel-published object.
///
/// Corresponds to tutorial: advanced 05-other-modules/20-qtwebchannel.

#include "chat_backend.h"

#include <QDebug>

ChatBackend::ChatBackend(QObject* parent)
    : QObject(parent)
{
}

QString ChatBackend::lastMessage() const
{
    return m_lastMessage;
}

int ChatBackend::messageCount() const
{
    return m_messageCount;
}

void ChatBackend::sendMessage(const QString& text)
{
    if (text.isEmpty()) {
        return;
    }

    ++m_messageCount;
    m_lastMessage = text;
    m_history.push_back(QString("Client: %1").arg(text));

    emit messageReceived("Client", text);
    emit lastMessageChanged();
    emit messageCountChanged();

    // Auto-respond after every client message so the demo is self-contained.
    generateAutoResponse(text);
}

QStringList ChatBackend::getMessageHistory()
{
    QStringList result;
    for (const auto& entry : m_history) {
        result.append(entry);
    }
    return result;
}

void ChatBackend::generateAutoResponse(const QString& userText)
{
    // Simple echo-style response for demonstration purposes.
    QString response = QString("Echo: you said \"%1\" (msg #%2)")
                           .arg(userText)
                           .arg(m_messageCount);

    ++m_messageCount;
    m_lastMessage = response;
    m_history.push_back(QString("Server: %1").arg(response));

    emit messageReceived("Server", response);
    emit lastMessageChanged();
    emit messageCountChanged();

    qDebug() << "[ChatBackend] auto-response:" << response;
}
