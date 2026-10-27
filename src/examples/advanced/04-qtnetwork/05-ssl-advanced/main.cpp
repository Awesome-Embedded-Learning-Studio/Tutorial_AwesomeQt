/// @file    main.cpp
/// @brief   程序入口，串联 SSL 证书分析与 TLS 配置演示。
///
/// 对应教程：进阶层 04-QtNetwork/05-SSL/TLS 高级用法。
/// 演示流程：先分析 www.qt.io 的证书链，再展示 TLS 配置对比，
/// 最后用自定义 TLS 1.3 配置连接公共站点。

#include "ssl_analyzer.h"
#include "tls_config_demo.h"

#include <QCoreApplication>
#include <QTimer>

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    // 设置应用程序元信息，供 QSslSocket 等模块读取
    QCoreApplication::setApplicationName("ssl-advanced-demo");
    QCoreApplication::setApplicationVersion("1.0");

    // --- Phase 1: SSL 证书链分析 ---
    // 使用 new + &app 确保 SslAnalyzer 在 app 析构前被销毁
    SslAnalyzer* analyzer = new SslAnalyzer(&app);

    // --- Phase 2: TLS 配置演示 ---
    TlsConfigDemo* configDemo = new TlsConfigDemo(&app);

    // 证书分析完成后，启动 TLS 配置演示
    QObject::connect(analyzer, &SslAnalyzer::analysisComplete,
                     configDemo, [configDemo]() {
                         configDemo->demonstrate();
                         configDemo->connectWithConfig("www.qt.io");
                     });

    // TLS 配置演示完成后退出应用
    QObject::connect(configDemo, &TlsConfigDemo::done,
                     &app, &QCoreApplication::quit);

    // 安全网：如果网络不通或信号未触发，15 秒后自动退出
    // @note 教学示例中必须设置超时兜底，避免事件循环永久挂起
    QTimer::singleShot(15000, &app, []() {
        qDebug() << "\n[Timeout] Auto-quit after 15 seconds (network may be unreachable).";
        QCoreApplication::exit(1);
    });

    // 启动第一步：分析 www.qt.io 的证书链
    analyzer->analyzeHost("www.qt.io");

    return app.exec();
}
