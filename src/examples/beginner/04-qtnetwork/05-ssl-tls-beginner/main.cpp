/**
 * Qt SSL/TLS 基础示例
 *
 * 本示例演示 SSL/TLS 加密通信的核心操作：
 * 1. QSslSocket::connectToHostEncrypted 建立加密连接
 * 2. 获取并展示服务端证书信息
 * 3. SSL 错误处理（开发阶段 ignoreSslErrors）
 * 4. QSslConfiguration 配置 SSL 参数
 * 5. SSL 支持状态检查
 *
 * 核心要点：
 * - QSslSocket 继承自 QTcpSocket，加密对上层透明
 * - connectToHostEncrypted 先 TCP 再 TLS 握手
 * - 生产环境绝不能无差别 ignoreSslErrors()
 * - Linux 依赖 OpenSSL 运行时库，Windows 使用 Schannel
 */

#include <QCoreApplication>
#include <QDebug>
#include <QSslSocket>
#include <QTimer>

#include "sslsupportcheck.h"
#include "tlsprobe.h"
#include "httpsrequest.h"
#include "sslconfigurationdemo.h"

// ========================================
// 主函数
// ========================================

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "=== Qt SSL/TLS Basic Example ===";

    // 示例 1: 检查 SSL 支持
    checkSslSupport();

    if (!QSslSocket::supportsSsl()) {
        qDebug() << "\nSSL is not available. Cannot proceed with demos.";
        qDebug() << "Please install OpenSSL libraries and try again.";
        return 0;
    }

    // 示例 4: SSL 配置信息
    demoSslConfiguration();

    // 示例 2: TLS 信息探测（连接真实服务器）
    probeTlsInfo("example.com");

    // 延迟 2000ms 后运行示例 3（等前一个连接完成）
    QTimer::singleShot(2000, [&app]() {
        demoHttpsRequest("example.com", "/");
    });

    // 延迟 8000ms 后退出（给所有连接足够时间完成）
    QTimer::singleShot(8000, [&app]() {
        qDebug() << "\n=== Summary ===";
        qDebug() << "QSslSocket wraps QTcpSocket with transparent encryption.";
        qDebug() << "Always check QSslSocket::supportsSsl() first.";
        qDebug() << "Never use ignoreSslErrors() in production!";
        qDebug() << "Demo finished.";
        QCoreApplication::quit();
    });

    return app.exec();
}
