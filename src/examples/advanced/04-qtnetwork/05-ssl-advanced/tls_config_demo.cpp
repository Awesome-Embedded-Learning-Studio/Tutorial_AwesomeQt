/// @file    tls_config_demo.cpp
/// @brief   TlsConfigDemo 类实现，TLS 配置对比与自定义连接演示。
///
/// 对应教程：进阶层 04-QtNetwork/05-SSL/TLS 高级用法。

#include "tls_config_demo.h"

#include <algorithm>

#include <QDebug>
#include <QSslCipher>
#include <QSslConfiguration>
#include <QSslSocket>

TlsConfigDemo::TlsConfigDemo(QObject* parent)
    : QObject(parent)
    , m_socket(new QSslSocket(this))
    , m_pendingHost()
{
    connect(m_socket, &QSslSocket::encrypted,
            this,     &TlsConfigDemo::onConfigEncrypted);

    connect(m_socket, &QSslSocket::sslErrors,
            this,     &TlsConfigDemo::onConfigSslErrors);
}

void TlsConfigDemo::demonstrate()
{
    qDebug() << "\n========================================";
    qDebug() << "  TLS Configuration Comparison Demo";
    qDebug() << "========================================\n";

    // 获取系统默认配置
    const QSslConfiguration defaultConfig = QSslConfiguration::defaultConfiguration();

    // 构建自定义配置（TLS 1.3 + 过滤加密套件）
    const QSslConfiguration customConfig = buildCustomConfig();

    // 打印两者的对比信息
    printConfigSummary("Default", defaultConfig);
    printConfigSummary("Custom (TLS 1.3)", customConfig);

    // 对比加密套件数量
    qDebug() << "--- Cipher Count Comparison ---";
    qDebug() << "  Default ciphers:" << defaultConfig.ciphers().size();
    qDebug() << "  Custom ciphers:  " << customConfig.ciphers().size();
    qDebug() << "";

    // 对比证书验证开关
    qDebug() << "--- Certificate Verification ---";
    qDebug() << "  Default peerVerifyMode:"
             << static_cast<int>(defaultConfig.peerVerifyMode());
    qDebug() << "  Custom peerVerifyMode: "
             << static_cast<int>(customConfig.peerVerifyMode());
    qDebug() << "";
}

void TlsConfigDemo::connectWithConfig(const QString& host)
{
    m_pendingHost = host;

    qDebug() << "=== TlsConfigDemo: connecting to" << host
             << "with custom TLS 1.3 config ===";

    const QSslConfiguration customConfig = buildCustomConfig();
    m_socket->setSslConfiguration(customConfig);

    // 手动忽略 SSL 错误以避免因本地信任库不完整导致连接中断
    m_socket->ignoreSslErrors();

    m_socket->connectToHostEncrypted(host, 443);
}

void TlsConfigDemo::onConfigEncrypted()
{
    qDebug() << "\n[TLS 1.3 Config] Encrypted connection to" << m_pendingHost
             << "established.";

    // 打印实际协商的协议和加密套件
    QSslCipher cipher = m_socket->sessionCipher();
    qDebug() << "  Negotiated cipher:  " << cipher.name();
    qDebug() << "  Negotiated protocol:" << cipher.protocolString();
    qDebug() << "  Key bits used:      " << cipher.usedBits();

    // 打印实际生效的 QSslConfiguration 协议
    QSslConfiguration activeConfig = m_socket->sslConfiguration();
    qDebug() << "  Active protocol:    "
             << static_cast<int>(activeConfig.protocol());
    qDebug() << "";

    emit done();
}

void TlsConfigDemo::onConfigSslErrors(const QList<QSslError>& errors)
{
    for (const QSslError& err : errors) {
        qDebug() << "  [TLS Config SSL Warning]" << err.errorString();
    }

    // 忽略错误以便教学演示能够继续
    m_socket->ignoreSslErrors();
}

QSslConfiguration TlsConfigDemo::buildCustomConfig() const
{
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();

    // 强制使用 TLS 1.3 协议，拒绝回退到旧版本
    // @note TLS 1.3 与 TLS 1.2 握手流程不同（0-RTT 等），部分服务端可能尚未支持
    config.setProtocol(QSsl::TlsV1_3);

    // 启用证书验证（生产环境必须开启）
    config.setPeerVerifyMode(QSslSocket::VerifyPeer);

    // 过滤加密套件：仅保留包含 "AES" 或 "CHACHA" 的套件
    // @note 此处仅演示过滤能力，实际部署应根据安全策略选择套件
    const QList<QSslCipher> allCiphers = QSslConfiguration::supportedCiphers();
    QList<QSslCipher> filteredCiphers;

    for (const QSslCipher& cipher : allCiphers) {
        const QString name = cipher.name().toUpper();
        if (name.contains("AES") || name.contains("CHACHA")) {
            filteredCiphers.append(cipher);
        }
    }

    if (!filteredCiphers.isEmpty()) {
        config.setCiphers(filteredCiphers);
    }
    // 若过滤结果为空则保留默认套件列表，避免无法握手

    return config;
}

void TlsConfigDemo::printConfigSummary(const QString& label,
                                       const QSslConfiguration& config) const
{
    qDebug() << "---" << label << "Configuration ---";

    // 协议版本
    qDebug() << "  Protocol:     " << static_cast<int>(config.protocol());

    // 加密套件列表（最多显示前 5 个，避免输出过长）
    const QList<QSslCipher> ciphers = config.ciphers();
    const auto showCount = std::min<qsizetype>(ciphers.size(), 5);

    qDebug() << "  Ciphers (" << ciphers.size() << " total, showing first"
             << showCount << "):";

    for (int i = 0; i < showCount; ++i) {
        qDebug() << "   " << ciphers.at(i).name()
                 << "|" << ciphers.at(i).protocolString();
    }

    // 证书验证模式
    const QString verifyModeStr =
        (config.peerVerifyMode() == QSslSocket::VerifyPeer)
            ? "VerifyPeer (strict)"
            : (config.peerVerifyMode() == QSslSocket::VerifyNone)
                  ? "VerifyNone (no verification)"
                  : "AutoVerifyPeer / QueryPeer";

    qDebug() << "  Peer verify:  " << verifyModeStr;
    qDebug() << "";
}
