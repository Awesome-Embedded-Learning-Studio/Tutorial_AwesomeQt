/**
 * Qt Remote Objects 传感器数据共享示例
 *
 * 本示例演示 Qt RemoteObjects 模块的核心功能：
 * - REPC 文件定义远程对象接口（sensordata.rep）
 * - QRemoteObjectHost 发布 Source 端对象
 * - QRemoteObjectNode 获取 Replica 端远程副本
 * - 属性自动同步与远程槽函数调用
 *
 * 启动后 Source 端每秒模拟更新传感器数据，
 * Replica 端实时监听属性变化，3 秒后发送 reset 请求。
 */

#include <QDebug>
#include <QCoreApplication>
#include <QRandomGenerator>
#include <QRemoteObjectHost>
#include <QRemoteObjectNode>
#include <QTimer>

#include "rep_sensordata_replica.h"
#include "sensorsource.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "Qt Remote Objects 传感器数据共享示例";
    qDebug() << "本示例演示 REPC 接口定义 + Source 发布 + Replica 同步";

    // ========================================
    // Source 端：模拟传感器数据采集
    // ========================================

    QRemoteObjectHost hostNode;
    // 使用本地传输，适合同机进程间通信
    // 跨设备场景改为 "tcp://0.0.0.0:5555"
    hostNode.setHostUrl(QUrl(QStringLiteral("local:sensordata")));

    SensorSource sensorSource;
    sensorSource.setTemperature(22.0);
    sensorSource.setHumidity(50.0);
    sensorSource.setStatus(QStringLiteral("Normal"));
    sensorSource.setUpdateCount(0);

    // 发布到远程对象节点，名称为 "sensor"
    hostNode.enableRemoting(&sensorSource, QStringLiteral("sensor"));

    // 模拟传感器数据更新——每秒产生一次新数据
    QTimer updateTimer;
    int count = 0;
    QObject::connect(&updateTimer, &QTimer::timeout, [&]() {
        double temp = 20.0 + QRandomGenerator::global()->bounded(10.0);
        double hum = 40.0 + QRandomGenerator::global()->bounded(40.0);
        sensorSource.setTemperature(temp);
        sensorSource.setHumidity(hum);
        sensorSource.setUpdateCount(++count);

        // 超过阈值自动更新状态
        if (temp > 28.0) {
            sensorSource.setStatus(QStringLiteral("HighTemp"));
        } else {
            sensorSource.setStatus(QStringLiteral("Normal"));
        }

        qDebug().noquote()
            << QStringLiteral("[Source] 更新: temp=%1 hum=%2 count=%3")
                   .arg(temp, 0, 'f', 1)
                   .arg(hum, 0, 'f', 1)
                   .arg(count);
    });
    updateTimer.start(1000);

    // ========================================
    // Replica 端：连接并监听远程数据
    // ========================================

    QRemoteObjectNode replicaNode;
    replicaNode.connectToNode(QUrl(QStringLiteral("local:sensordata")));

    SensorDataReplica *replica
        = replicaNode.acquire<SensorDataReplica>(QStringLiteral("sensor"));

    // 等待 Replica 初始化完成
    if (replica->waitForSource(3000)) {
        qDebug() << "[Replica] 已连接到远程传感器";
        qDebug().noquote()
            << QStringLiteral("[Replica] 初始数据: temp=%1 hum=%2 status=%3")
                   .arg(replica->temperature(), 0, 'f', 1)
                   .arg(replica->humidity(), 0, 'f', 1)
                   .arg(replica->status());

        // 监听温度变化
        QObject::connect(replica,
            &SensorDataReplica::temperatureChanged,
            [](double temp) {
                qDebug().noquote()
                    << QStringLiteral("[Replica] 温度变化: %1")
                           .arg(temp, 0, 'f', 1);
            });

        // 监听状态变化信号
        QObject::connect(replica,
            &SensorDataReplica::statusChanged,
            [](const QString &newStatus) {
                qDebug() << "[Replica] 状态信号:" << newStatus;
            });

        // 3 秒后从 Replica 端触发 reset
        QTimer::singleShot(3000, [replica]() {
            qDebug() << "[Replica] 发送 reset 请求...";
            replica->reset();
        });

    } else {
        qWarning() << "[Replica] 等待 Source 超时";
    }

    return app.exec();
}
