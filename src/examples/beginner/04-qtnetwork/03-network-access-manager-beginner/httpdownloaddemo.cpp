#include "httpdownloaddemo.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QElapsedTimer>
#include <QDebug>

void demoDownloadWithProgress(QNetworkAccessManager *manager)
{
    qDebug() << "\n=== Demo 3: Download with Progress ===";

    // 下载一个 ~100KB 的测试文件
    QUrl url("https://httpbin.org/image/png");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      "QtNetworkTutorial/1.0");

    QNetworkReply *reply = manager->get(request);
    QElapsedTimer timer;
    timer.start();

    qint64 lastBytes = 0;

    // 追踪下载进度
    QObject::connect(reply, &QNetworkReply::downloadProgress,
            [=, &lastBytes, &timer](qint64 received, qint64 total) mutable {
        double elapsed = timer.elapsed() / 1000.0;
        if (elapsed > 0) {
            double speedKBps = (received / 1024.0) / elapsed;
            if (total > 0) {
                double percent = static_cast<double>(received)
                                 / total * 100.0;
                qDebug() << "  [Download]"
                         << QString("%1% (%2/%3 KB) @ %4 KB/s")
                                .arg(percent, 0, 'f', 0)
                                .arg(received / 1024.0, 0, 'f', 1)
                                .arg(total / 1024.0, 0, 'f', 1)
                                .arg(speedKBps, 0, 'f', 1);
            } else {
                qDebug() << "  [Download]"
                         << QString("%1 KB received @ %2 KB/s")
                                .arg(received / 1024.0, 0, 'f', 1)
                                .arg(speedKBps, 0, 'f', 1);
            }
        }
        lastBytes = received;
    });

    QObject::connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "  [Download] Error:" << reply->errorString();
            reply->deleteLater();
            return;
        }

        QByteArray data = reply->readAll();
        int statusCode = reply->attribute(
            QNetworkRequest::HttpStatusCodeAttribute).toInt();

        qDebug() << "  [Download] HTTP Status:" << statusCode;
        qDebug() << "  [Download] Total size:"
                 << data.size() << "bytes"
                 << "(" << data.size() / 1024.0 << "KB)";

        // 检查 Content-Type
        QVariant contentType = reply->header(
            QNetworkRequest::ContentTypeHeader);
        qDebug() << "  [Download] Content-Type:" << contentType.toString();

        reply->deleteLater();
    });
}
