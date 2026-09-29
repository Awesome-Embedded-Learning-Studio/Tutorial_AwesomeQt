#include "httpsrequest.h"

#include <QDebug>
#include <QSslError>
#include <QSslSocket>

void demoHttpsRequest(const QString &hostname, const QString &path)
{
    qDebug() << "\n=== Demo 3: HTTPS Request to"
             << hostname << "===";

    QSslSocket *socket = new QSslSocket();

    QObject::connect(socket, &QSslSocket::encrypted, [=]() {
        qDebug() << "  [OK] TLS connected, sending HTTP request...";

        // 构造一个简单的 HTTP/1.1 GET 请求
        QString request = QString("GET %1 HTTP/1.1\r\n"
                                  "Host: %2\r\n"
                                  "User-Agent: QtSslTutorial/1.0\r\n"
                                  "Connection: close\r\n"
                                  "\r\n")
                              .arg(path, hostname);

        socket->write(request.toUtf8());
        socket->flush();
    });

    // 读取响应
    QObject::connect(socket, &QSslSocket::readyRead, [=]() {
        QByteArray data = socket->readAll();

        // 只打印响应头部分（到第一个 \r\n\r\n）
        int headerEnd = data.indexOf("\r\n\r\n");
        if (headerEnd > 0) {
            QByteArray header = data.left(headerEnd);
            qDebug() << "  --- HTTP Response Headers ---";
            qDebug().noquote() << "  " << header;
        }
    });

    // SSL 错误处理
    QObject::connect(socket, &QSslSocket::sslErrors,
            [=](const QList<QSslError> &errors) {
        qDebug() << "  [SSL Errors]:" << errors.size();
        socket->ignoreSslErrors();
    });

    // 断开连接
    QObject::connect(socket, &QSslSocket::disconnected, [=]() {
        qDebug() << "  Connection closed.";
        socket->deleteLater();
    });

    // 错误处理
    QObject::connect(socket, &QAbstractSocket::errorOccurred,
            [=](QAbstractSocket::SocketError error) {
        qDebug() << "  [Error]:" << error << socket->errorString();
        socket->deleteLater();
    });

    socket->connectToHostEncrypted(hostname, 443);
}
