/// @file    main.cpp
/// @brief   MQTT 高级示例程序入口，演示 QoS / LWT / TLS 等 API。
///
/// 对应教程：进阶层 05-其他模块/09-MQTT 高级。
/// 使用 QCoreApplication（控制台应用），依次调用各演示函数，
/// 最后通过 QTimer 自动退出事件循环。

#include "mqtt_client_demo.h"

#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <cstdio>

/// @brief 自定义消息处理器，确保日志输出到 stderr。
/// @note 某些 Qt 发行版的默认处理器不输出 qDebug/qInfo，
///       安装此处理器保证所有日志可见。
/// @param[in] type    消息类型（调试、信息、警告等）。
/// @param[in] context 消息的源文件位置信息。
/// @param[in] msg     消息内容。
static void messageHandler(QtMsgType type,
                           const QMessageLogContext& context,
                           const QString& msg)
{
    const char* prefix = "";
    switch (type) {
    case QtDebugMsg:    prefix = "DEBUG";    break;
    case QtInfoMsg:     prefix = "INFO";     break;
    case QtWarningMsg:  prefix = "WARNING";  break;
    case QtCriticalMsg: prefix = "CRITICAL"; break;
    case QtFatalMsg:    prefix = "FATAL";    break;
    }

    // 简化输出：仅显示前缀和消息，不显示文件名/行号
    fprintf(stderr, "[%s] %s\n", prefix, msg.toLocal8Bit().constData());

    // QMessageLogContext 仅在 debug 构建中有文件/行信息
    Q_UNUSED(context)
}

/// @brief 程序入口，创建 MQTT 客户端演示对象并依次执行各 Demo。
/// @param[in] argc 命令行参数个数。
/// @param[in] argv 命令行参数数组。
/// @return 应用程序退出码。
int main(int argc, char* argv[])
{
    // 安装消息处理器，确保控制台可见所有日志输出
    qInstallMessageHandler(messageHandler);

    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("09-mqtt-advanced"));
    app.setApplicationVersion(QStringLiteral("1.0.0"));

    qDebug() << "========================================";
    qDebug() << " MQTT 高级示例 — QoS / LWT / TLS";
    qDebug() << " 本示例展示 API 用法，无需实际 Broker";
    qDebug() << "========================================";

    // 创建演示对象，利用 Qt 对象树自动管理生命周期
    MqttClientDemo demo;

    // Demo 1：配置客户端连接参数
    demo.setupClient();

    // Demo 2：配置 Last Will（遗嘱消息）
    demo.setupWill();

    // Demo 3：展示三种 QoS 级别的订阅操作
    demo.demonstrateSubscription();

    // Demo 4：展示三种 QoS 级别的发布操作
    demo.demonstratePublish();

    // Demo 5：展示 TLS/SSL 安全连接配置
    demo.demonstrateTlsSetup();

    // 打印最终状态汇总
    demo.printStatus();

    qDebug() << "";
    qDebug() << "========================================";
    qDebug() << " 所有 Demo 演示完毕";
    qDebug() << "========================================";

    // 无 Broker 场景下不需要进入事件循环，延迟退出即可
    QTimer::singleShot(0, &app, &QCoreApplication::quit);

    return app.exec();
}
