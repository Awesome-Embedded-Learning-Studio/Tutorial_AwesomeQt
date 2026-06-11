/// @file    ssl_analyzer.h
/// @brief   SSL 证书链分析器，连接远程主机并提取/验证证书链。
///
/// 对应教程：进阶层 04-QtNetwork/05-SSL/TLS 高级用法。
/// 演示 QSslSocket 连接、证书链提取、QSslCertificate 信息读取与链验证。

#pragma once

#include <QObject>
#include <QSslCertificate>
#include <QSslSocket>

/// @brief SSL 证书链分析器，连接指定主机并展示其证书链信息。
///
/// 使用 QSslSocket 建立加密连接后，提取对端证书链并逐张打印
/// 主题、颁发者、有效期和序列号，最后调用 QSslCertificate::verify()
/// 进行证书链合法性校验。
class SslAnalyzer : public QObject
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父对象指针，Qt 对象树自动管理生命周期。
    explicit SslAnalyzer(QObject* parent = nullptr);

    /// @brief 发起对指定主机的 SSL 连接分析。
    /// @param[in] hostName 目标主机名，如 "www.qt.io"。
    /// @param[in] port     目标端口，默认 443。
    /// @note 内部会忽略 SSL 错误以保证即使证书无效也能提取信息进行分析。
    void analyzeHost(const QString& hostName, quint16 port = 443);

signals:
    /// @brief 证书链分析完成后发射。
    /// @note 无论验证通过与否，只要提取完成就会发射。
    void analysisComplete();

private slots:
    /// @brief 加密连接建立后的回调，提取并分析证书链。
    void onEncrypted();

    /// @brief SSL 错误回调，忽略错误以继续证书提取。
    /// @param[in] errors SSL 握手过程中遇到的错误列表。
    void onSslErrors(const QList<QSslError>& errors);

private:
    /// @brief 打印单张证书的详细信息。
    /// @param[in] cert    待打印的证书。
    /// @param[in] index   证书在链中的序号（从 0 开始）。
    /// @param[in] isLeaf  是否为终端实体证书（链首）。
    void printCertificateInfo(const QSslCertificate& cert, int index, bool isLeaf);

    /// @brief 对证书链执行验证并打印结果。
    /// @param[in] chain 证书链，从终端实体到根 CA。
    void verifyCertificateChain(const QList<QSslCertificate>& chain);

    QSslSocket* m_socket;   ///< 用于建立 SSL 连接的套接字
    QString     m_hostName; ///< 当前正在分析的主机名
};
