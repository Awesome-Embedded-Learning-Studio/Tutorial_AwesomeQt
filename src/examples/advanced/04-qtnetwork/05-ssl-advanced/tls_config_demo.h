/// @file    tls_config_demo.h
/// @brief   TLS 配置演示，对比默认与自定义 QSslConfiguration 的差异。
///
/// 对应教程：进阶层 04-QtNetwork/05-SSL/TLS 高级用法。
/// 演示 QSslConfiguration 的协议版本、加密套件过滤和证书验证开关。

#pragma once

#include <QObject>
#include <QSslConfiguration>
#include <QSslSocket>

/// @brief TLS 配置演示类，展示 QSslConfiguration 的自定义能力。
///
/// 打印系统默认配置与自定义配置的对比，并通过自定义配置连接
/// 公共 HTTPS 站点验证实际生效效果。
class TlsConfigDemo : public QObject
{
    Q_OBJECT

public:
    /// @brief 构造函数。
    /// @param[in] parent 父对象指针，Qt 对象树自动管理生命周期。
    explicit TlsConfigDemo(QObject* parent = nullptr);

    /// @brief 打印默认 TLS 配置与自定义 TLS 1.3 配置的对比信息。
    ///
    /// 对比内容包括协议版本、加密套件列表和证书验证开关。
    void demonstrate();

    /// @brief 使用自定义 TLS 配置连接指定主机。
    /// @param[in] host 目标主机名，如 "www.qt.io"。
    /// @note 仅使用 TLS 1.3 协议连接；若服务端不支持 TLS 1.3 则握手失败。
    void connectWithConfig(const QString& host);

signals:
    /// @brief TLS 配置演示全部完成后发射。
    void done();

private slots:
    /// @brief 自定义配置连接加密成功后的回调。
    void onConfigEncrypted();

    /// @brief 自定义配置连接遇到 SSL 错误的回调。
    /// @param[in] errors 握手过程中的 SSL 错误列表。
    void onConfigSslErrors(const QList<QSslError>& errors);

private:
    /// @brief 构建一个自定义 QSslConfiguration（仅允许 TLS 1.3）。
    /// @return 配置好的 QSslConfiguration 对象。
    /// @note 过滤加密套件列表，仅保留 AES-GCM 类套件以展示过滤能力。
    QSslConfiguration buildCustomConfig() const;

    /// @brief 打印单条配置的关键参数。
    /// @param[in] label 配置标签（如 "Default" 或 "Custom TLS 1.3"）。
    /// @param[in] config 待打印的配置。
    void printConfigSummary(const QString& label,
                            const QSslConfiguration& config) const;

    QSslSocket* m_socket;       ///< 用于自定义配置连接测试的套接字
    QString     m_pendingHost;  ///< 当前待连接的主机名
};
