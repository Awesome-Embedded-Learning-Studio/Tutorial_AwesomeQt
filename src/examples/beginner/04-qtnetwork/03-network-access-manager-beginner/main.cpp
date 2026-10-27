/**
 * Qt QNetworkAccessManager 基础示例
 *
 * 本示例演示 HTTP 通信的核心操作：
 * 1. GET 请求获取远程资源
 * 2. POST 请求提交 JSON 数据
 * 3. setHeader 设置自定义请求头
 * 4. downloadProgress 信号追踪下载进度
 * 5. QNetworkReply::finished 异步回调处理
 *
 * 核心要点：
 * - QNetworkAccessManager 一个应用只需一个实例
 * - 所有请求都是异步的，通过信号槽接收响应
 * - QNetworkReply 必须手动 deleteLater()
 * - HTTP 状态码 4xx/5xx 不算网络错误，需单独检查
 */

#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QDebug>

#include "httpgetdemo.h"
#include "httppostdemo.h"
#include "httpdownloaddemo.h"

// ========================================
// 主函数
// ========================================

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "=== Qt QNetworkAccessManager Basic Example ===";

    // 整个应用只创建一个 QNetworkAccessManager 实例
    QNetworkAccessManager *manager = new QNetworkAccessManager(&app);

    // 示例 1: GET 请求
    demoGetRequest(manager);

    // 延迟 2000ms 后运行示例 2（等前一个请求完成）
    QTimer::singleShot(2000, [=]() {
        demoPostRequest(manager);
    });

    // 延迟 4000ms 后运行示例 3
    QTimer::singleShot(4000, [=]() {
        demoDownloadWithProgress(manager);
    });

    // 延迟 8000ms 后退出（给所有请求足够时间完成）
    QTimer::singleShot(8000, [&app]() {
        qDebug() << "\n=== Summary ===";
        qDebug() << "QNetworkAccessManager handles HTTP asynchronously.";
        qDebug() << "Always check reply->error() AND HTTP status code.";
        qDebug() << "Always call reply->deleteLater() when done.";
        qDebug() << "Demo finished.";
        QCoreApplication::quit();
    });

    return app.exec();
}
