/// @file    test_client.cpp
/// @brief   Implementation of TestClient — the demo WebSocket client.
///
/// Corresponds to tutorial: advanced 05-other-modules/20-qtwebchannel.
///
/// Implements the QWebChannel handshake protocol:
///   1. Connect WebSocket
///   2. Send {"type":"hello"} — older protocol greeting
///   3. Wait for {"type":"hello"} response confirming the channel is ready
///   4. Send {"type":"init"} to request published object information
///   5. Receive {"type":"init"} response with the object map
///   6. Invoke methods via {"type":"invokeMethod", ...}
///   7. Auto-quit after demo is complete

#include "test_client.h"

#include <QCoreApplication>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QWebSocket>

TestClient::TestClient(const QUrl& url, QObject* parent)
    : QObject(parent)
    , m_socket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this))
{
    connect(m_socket, &QWebSocket::connected,
            this, &TestClient::onConnected);

    connect(m_socket, &QWebSocket::textMessageReceived,
            this, &TestClient::onTextMessageReceived);

    connect(m_socket, &QWebSocket::disconnected,
            this, &TestClient::onDisconnected);

    connect(m_socket, &QWebSocket::errorOccurred,
            this, [this](QAbstractSocket::SocketError error) {
                qWarning() << "[TestClient] WebSocket error:" << error
                           << m_socket->errorString();
            });

    m_socket->open(url);
    qDebug() << "[TestClient] connecting to" << url.toString();
}

void TestClient::onConnected()
{
    qDebug() << "[TestClient] WebSocket connected, will send init after short delay";

    // Delay the init handshake slightly to ensure the server-side
    // QWebChannel has fully registered the transport before we send.
    QTimer::singleShot(200, this, [this]() {
        // QWebChannel uses numeric type IDs, not strings:
        //   TypeInit = 3, TypeInvokeMethod = 6, TypeIdle = 4
        QJsonObject initMsg;
        initMsg["type"] = 3;  // TypeInit
        initMsg["id"] = QString::number(++m_msgId);
        qDebug() << "[TestClient] sending init handshake";
        sendJson(initMsg);
    });
}

void TestClient::onTextMessageReceived(const QString& message)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &error);

    if (error.error || !doc.isObject()) {
        qDebug() << "[TestClient] non-JSON message:" << message;
        return;
    }

    QJsonObject msg = doc.object();

    // QWebChannel uses numeric types: TypeInit=3, TypeResponse=10, etc.
    int typeNum = msg.value("type").toInt(-1);

    if (typeNum == 3) {  // TypeInit response
        // The channel sent the object map — we can now invoke methods.
        qDebug() << "[TestClient] received init response, channel ready";
        qDebug() << "  registered objects:" << msg.value("data").toObject().keys();

        // Schedule the first method invocation after a short delay so the
        // event loop can process any remaining init messages first.
        QTimer::singleShot(100, this, &TestClient::sendTestMessage);

    } else if (typeNum == 10) {  // TypeResponse
        // Response to a method invocation.
        qDebug() << "[TestClient] response received:"
                 << QJsonDocument(msg).toJson(QJsonDocument::Compact);

        if (m_step == 0) {
            // After sendMessage completes, request the message history.
            m_step = 1;
            QTimer::singleShot(100, this, &TestClient::requestHistory);
        } else if (m_step == 1) {
            // After getMessageHistory completes, wrap up the demo.
            m_step = 2;
            QTimer::singleShot(100, this, &TestClient::quitApp);
        }

    } else if (typeNum == 1) {  // TypePropertyUpdate
        // Property update pushed from the channel (lastMessage, messageCount).
        qDebug() << "[TestClient] property update:"
                 << QJsonDocument(msg).toJson(QJsonDocument::Compact);

    } else if (typeNum == 2) {  // TypeSignal
        // Signal emission pushed from the channel (messageReceived).
        qDebug() << "[TestClient] signal:"
                 << QJsonDocument(msg).toJson(QJsonDocument::Compact);

    } else {
        qDebug() << "[TestClient] unknown message type:" << typeNum;
    }
}

void TestClient::onDisconnected()
{
    qDebug() << "[TestClient] disconnected from server";
}

void TestClient::sendJson(const QJsonObject& obj)
{
    QJsonDocument doc(obj);
    m_socket->sendTextMessage(
        QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
}

void TestClient::sendTestMessage()
{
    qDebug() << "[TestClient] invoking backend.sendMessage("
             << "\"Hello from WebChannel!\")";

    // TypeInvokeMethod = 6
    QJsonObject msg;
    msg["type"] = 6;
    msg["object"] = "backend";
    msg["method"] = "sendMessage";
    QJsonArray args;
    args.append("Hello from WebChannel!");
    msg["args"] = args;
    msg["id"] = QString::number(++m_msgId);
    sendJson(msg);
}

void TestClient::requestHistory()
{
    qDebug() << "[TestClient] invoking backend.getMessageHistory()";

    // TypeInvokeMethod = 6
    QJsonObject msg;
    msg["type"] = 6;
    msg["object"] = "backend";
    msg["method"] = "getMessageHistory";
    msg["id"] = QString::number(++m_msgId);
    sendJson(msg);
}

void TestClient::quitApp()
{
    qDebug() << "";
    qDebug() << "=== Demo complete ===";
    qDebug() << "The WebChannel successfully bridged C++ backend methods and"
             << "signals to a WebSocket client without QtWebEngine.";
    m_socket->close();
    QCoreApplication::quit();
}
