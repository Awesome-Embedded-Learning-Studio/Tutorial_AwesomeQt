#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QElapsedTimer>
#include <QThread>
#include <QDebug>

// ========== 声明日志类别 ==========
// 注意：避免使用 "debug"、"info"、"warning"、"critical" 等保留名
// 注意：避免使用 "qt" 前缀，这是为 Qt 内部保留的

#include <QLoggingCategory>

// 网络模块日志类别
Q_DECLARE_LOGGING_CATEGORY(networkLog)

// 性能分析日志类别
Q_DECLARE_LOGGING_CATEGORY(perfLog)

/**
 * 网络管理器类：演示分类日志的使用
 */
class NetworkManager : public QObject {
    Q_OBJECT

public:
    NetworkManager(QObject *parent = nullptr) : QObject(parent), m_retryCount(0) {}

    /**
     * 模拟连接服务器
     * @param url 服务器地址
     */
    void connectToServer(const QString &url) {
        qCDebug(networkLog) << "开始连接服务器:" << url;

        // 模拟连接过程
        bool success = doConnect(url);

        if (success) {
            qCInfo(networkLog) << "连接成功";
        } else {
            m_retryCount++;
            qCWarning(networkLog) << "连接失败，这是第" << m_retryCount << "次重试";

            if (m_retryCount >= 3) {
                qCCritical(networkLog) << "连接失败超过最大重试次数";
            }
        }
    }

    /**
     * 模拟下载数据
     * @param dataSize 数据大小（字节）
     */
    void downloadData(int dataSize) {
        QElapsedTimer timer;
        timer.start();

        qCDebug(networkLog) << "开始下载数据，大小:" << dataSize << "字节";

        // 模拟下载
        QThread::msleep(100);

        qint64 elapsed = timer.elapsed();
        qCInfo(networkLog) << "下载完成，耗时:" << elapsed << "毫秒";

        // 性能日志：记录下载速度
        qCDebug(perfLog) << "下载速度:"
                         << (dataSize / 1024.0 / (elapsed / 1000.0))
                         << "KB/s";
    }

private:
    bool doConnect(const QString &url) {
        Q_UNUSED(url)
        // 模拟连接失败（前两次失败，第三次成功）
        return m_retryCount >= 2;
    }

    int m_retryCount;
};

#endif // NETWORKMANAGER_H
