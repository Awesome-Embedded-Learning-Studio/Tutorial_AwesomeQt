#include "sslconfigurationdemo.h"

#include <QDebug>
#include <QSslCertificate>
#include <QSslCipher>
#include <QSslConfiguration>

void demoSslConfiguration()
{
    qDebug() << "\n=== Demo 4: SSL Configuration ===";

    QSslConfiguration defaultConfig =
        QSslConfiguration::defaultConfiguration();

    qDebug() << "  Default protocol:" << defaultConfig.protocol();

    // 列出系统支持的 CA 证书数量
    QList<QSslCertificate> caCerts = defaultConfig.caCertificates();
    qDebug() << "  System CA certificates:" << caCerts.size();

    // 列出支持的加密套件
    QList<QSslCipher> ciphers = defaultConfig.ciphers();
    qDebug() << "  Supported ciphers:" << ciphers.size();
    if (!ciphers.isEmpty()) {
        qDebug() << "  First cipher:" << ciphers.first().name()
                 << "(" << ciphers.first().usedBits() << "bits)";
        qDebug() << "  Last cipher:" << ciphers.last().name()
                 << "(" << ciphers.last().usedBits() << "bits)";
    }

    // 自定义配置示例
    QSslConfiguration customConfig = defaultConfig;
    customConfig.setProtocol(QSsl::TlsV1_2OrLater);

    qDebug() << "\n  Custom config protocol:"
             << customConfig.protocol();
}
