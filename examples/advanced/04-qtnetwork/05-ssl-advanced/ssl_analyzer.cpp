/// @file    ssl_analyzer.cpp
/// @brief   SslAnalyzer 类实现，SSL 证书链分析与验证。
///
/// 对应教程：进阶层 04-QtNetwork/05-SSL/TLS 高级用法。

#include "ssl_analyzer.h"

#include <QDebug>
#include <QSslCertificate>
#include <QSslCipher>
#include <QSslConfiguration>
#include <QSslSocket>

SslAnalyzer::SslAnalyzer(QObject* parent)
    : QObject(parent)
    , m_socket(new QSslSocket(this))   // Qt 对象树管理生命周期
    , m_hostName()
{
    // 连接加密成功信号，在握手完成后提取证书链
    connect(m_socket, &QSslSocket::encrypted,
            this,     &SslAnalyzer::onEncrypted);

    // 忽略 SSL 错误以保证即使证书过期或自签名也能提取信息
    connect(m_socket, &QSslSocket::sslErrors,
            this,     &SslAnalyzer::onSslErrors);
}

void SslAnalyzer::analyzeHost(const QString& hostName, quint16 port)
{
    m_hostName = hostName;
    qDebug() << "=== SslAnalyzer: connecting to" << hostName << "on port" << port
             << "===";

    // ignoreSslErrors 允许在证书无效时继续握手，方便教学观察
    m_socket->connectToHostEncrypted(hostName, port);
}

void SslAnalyzer::onEncrypted()
{
    qDebug() << "\n[Encrypted] SSL handshake with" << m_hostName << "succeeded.\n";

    // 提取对端证书链，链首为终端实体证书（Leaf Certificate）
    const QList<QSslCertificate> chain = m_socket->peerCertificateChain();

    if (chain.isEmpty()) {
        qDebug() << "  (No peer certificate chain received)";
    } else {
        qDebug() << "Certificate chain length:" << chain.size();
        qDebug() << "";

        for (int i = 0; i < chain.size(); ++i) {
            // 链首是终端实体证书（即服务器证书），其余为中间/根 CA
            printCertificateInfo(chain.at(i), i, (i == 0));
        }
    }

    // 打印对端证书（等同于 chain[0]，单独获取以确保一致性）
    const QSslCertificate peerCert = m_socket->peerCertificate();
    qDebug() << "\n--- Peer (Leaf) Certificate Summary ---";
    qDebug() << "  Subject CN:" << peerCert.subjectDisplayName();
    qDebug() << "  Issuer CN: " << peerCert.issuerDisplayName();
    qDebug() << "  Valid from:" << peerCert.effectiveDate().toString(Qt::ISODate);
    qDebug() << "  Valid to:  " << peerCert.expiryDate().toString(Qt::ISODate);
    qDebug() << "";

    // 打印当前会话使用的加密套件
    QSslCipher cipher = m_socket->sessionCipher();
    qDebug() << "Session cipher:" << cipher.name()
             << "| Protocol:" << cipher.protocolString()
             << "| Bits:" << cipher.usedBits();

    // 验证整条证书链
    verifyCertificateChain(chain);

    emit analysisComplete();
}

void SslAnalyzer::onSslErrors(const QList<QSslError>& errors)
{
    // 教学示例：打印遇到的 SSL 错误但不中止连接
    for (const QSslError& err : errors) {
        qDebug() << "  [SSL Warning]" << err.errorString();
    }

    // 忽略所有 SSL 错误，以便在证书无效时仍可提取证书信息
    m_socket->ignoreSslErrors();
}

void SslAnalyzer::printCertificateInfo(const QSslCertificate& cert, int index,
                                       bool isLeaf)
{
    const QString role = isLeaf ? "Leaf (Server)" : "CA (Intermediate/Root)";

    qDebug() << "--- Certificate #" << index << "|" << role << "---";

    // Subject 信息：证书颁发给谁
    // subjectInfo 返回 QStringList，多数 CN 只有一个值
    const QStringList subjectCnList =
        cert.subjectInfo(QSslCertificate::CommonName);
    const QStringList subjectOrgList =
        cert.subjectInfo(QSslCertificate::Organization);

    qDebug() << "  Subject CN:  " << subjectCnList.join(", ");
    qDebug() << "  Subject Org: " << subjectOrgList.join(", ");

    // Issuer 信息：谁签发了这张证书
    const QStringList issuerCnList =
        cert.issuerInfo(QSslCertificate::CommonName);
    const QStringList issuerOrgList =
        cert.issuerInfo(QSslCertificate::Organization);

    qDebug() << "  Issuer CN:   " << issuerCnList.join(", ");
    qDebug() << "  Issuer Org:  " << issuerOrgList.join(", ");

    // 有效期
    qDebug() << "  Valid from:  " << cert.effectiveDate().toString(Qt::ISODate);
    qDebug() << "  Valid to:    " << cert.expiryDate().toString(Qt::ISODate);

    // 序列号（十六进制）
    qDebug() << "  Serial:      " << cert.serialNumber();

    // SHA256 指纹用于唯一标识证书
    qDebug() << "  SHA256 Fingerprint:" << cert.digest(QCryptographicHash::Sha256).toHex();
    qDebug() << "";
}

void SslAnalyzer::verifyCertificateChain(const QList<QSslCertificate>& chain)
{
    qDebug() << "\n=== Certificate Chain Verification ===";

    if (chain.isEmpty()) {
        qDebug() << "  Chain is empty, nothing to verify.";
        return;
    }

    // QSslCertificate::verify() 返回验证过程中的错误列表
    // 空列表表示证书链验证通过
    const QList<QSslError> errors = QSslCertificate::verify(chain);

    if (errors.isEmpty()) {
        qDebug() << "  Result: PASSED - certificate chain is valid.";
    } else {
        qDebug() << "  Result: FAILED -" << errors.size() << "error(s) found:";
        for (const QSslError& err : errors) {
            qDebug() << "   " << err.errorString();
        }
    }
}
