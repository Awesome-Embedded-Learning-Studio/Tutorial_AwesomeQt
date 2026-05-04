#include "tlsprobe.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QSslCertificate>
#include <QSslCipher>
#include <QSslConfiguration>
#include <QSslError>
#include <QSslSocket>

void probeTlsInfo(const QString &hostname, quint16 port)
{
    qDebug() << "\n=== Demo 2: TLS Probe for"
             << hostname << "===";

    QSslSocket *socket = new QSslSocket();

    // 配置 SSL 参数：要求 TLS 1.2 或更高版本
    QSslConfiguration sslConfig =
        QSslConfiguration::defaultConfiguration();
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    socket->setSslConfiguration(sslConfig);

    QElapsedTimer timer;
    timer.start();

    // TLS 加密连接成功
    QObject::connect(socket, &QSslSocket::encrypted, [=, &timer]() {
        qint64 elapsed = timer.elapsed();
        qDebug() << "  [OK] Encrypted connection established in"
                 << elapsed << "ms";

        // 获取 TLS 会话信息
        qDebug() << "  Protocol:"
                 << socket->sessionProtocol();
        QSslCipher cipher = socket->sessionCipher();
        qDebug() << "  Cipher:" << cipher.name()
                 << "(bits:" << cipher.usedBits() << ")";

        // 获取服务端证书信息
        QSslCertificate cert = socket->peerCertificate();
        if (!cert.isNull()) {
            qDebug() << "\n  --- Server Certificate ---";
            qDebug() << "  Subject CN:"
                     << cert.subjectInfo(QSslCertificate::CommonName);
            qDebug() << "  Subject O:"
                     << cert.subjectInfo(QSslCertificate::Organization);
            qDebug() << "  Issuer CN:"
                     << cert.issuerInfo(QSslCertificate::CommonName);
            qDebug() << "  Valid from:"
                     << cert.effectiveDate().toString(Qt::ISODate);
            qDebug() << "  Valid until:"
                     << cert.expiryDate().toString(Qt::ISODate);
            qDebug() << "  Serial number:"
                     << cert.serialNumber();

            // 检查证书是否过期
            if (cert.expiryDate() < QDateTime::currentDateTime()) {
                qDebug() << "  WARNING: Certificate has EXPIRED!";
            }

            // 检查证书是否即将过期（30 天内）
            qint64 daysToExpiry =
                QDateTime::currentDateTime().daysTo(cert.expiryDate());
            if (daysToExpiry >= 0 && daysToExpiry <= 30) {
                qDebug() << "  WARNING: Certificate expires in"
                         << daysToExpiry << "days!";
            }
        }

        socket->close();
        socket->deleteLater();
    });

    // SSL 错误处理
    QObject::connect(socket, &QSslSocket::sslErrors,
            [=](const QList<QSslError> &errors) {
        qDebug() << "  [SSL Errors]:" << errors.size()
                 << "error(s) occurred:";
        for (const QSslError &error : errors) {
            qDebug() << "   -" << error.errorString();
        }

        // 仅开发阶段：忽略所有 SSL 错误以继续连接
        // 生产环境应该只忽略已知的、安全的错误
        qDebug() << "  [DEV] Ignoring SSL errors for demonstration.";
        socket->ignoreSslErrors();
    });

    // 连接错误
    QObject::connect(socket, &QAbstractSocket::errorOccurred,
            [=](QAbstractSocket::SocketError error) {
        qDebug() << "  [Socket Error]:" << error
                 << socket->errorString();
        socket->deleteLater();
    });

    // 发起加密连接
    qDebug() << "  Connecting to" << hostname << ":" << port << "...";
    socket->connectToHostEncrypted(hostname, port);
}
