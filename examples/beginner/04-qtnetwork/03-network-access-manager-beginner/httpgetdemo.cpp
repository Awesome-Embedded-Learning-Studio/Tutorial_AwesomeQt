#include "httpgetdemo.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

void demoGetRequest(QNetworkAccessManager *manager)
{
    qDebug() << "\n=== Demo 1: HTTP GET Request ===";

    // httpbin.org 是一个免费的 HTTP 测试服务
    QNetworkRequest request(QUrl("https://httpbin.org/get"));

    // 设置自定义请求头
    request.setRawHeader("X-Custom-Header", "Qt-Network-Tutorial");
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      "QtNetworkTutorial/1.0");

    QNetworkReply *reply = manager->get(request);

    // 追踪下载进度
    QObject::connect(reply, &QNetworkReply::downloadProgress,
            [](qint64 received, qint64 total) {
        if (total > 0) {
            double percent = static_cast<double>(received) / total * 100.0;
            qDebug() << "  [GET] Progress:"
                     << QString("%1% (%2/%3 bytes)")
                            .arg(percent, 0, 'f', 0)
                            .arg(received)
                            .arg(total);
        } else {
            qDebug() << "  [GET] Downloaded" << received << "bytes";
        }
    });

    // 响应完成
    QObject::connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "  [GET] Network error:" << reply->errorString();
            reply->deleteLater();
            return;
        }

        // 检查 HTTP 状态码
        int statusCode = reply->attribute(
            QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << "  [GET] HTTP Status:" << statusCode;

        // 读取响应体
        QByteArray data = reply->readAll();
        qDebug() << "  [GET] Response size:" << data.size() << "bytes";

        // 尝试解析 JSON
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isNull()) {
            qDebug() << "  [GET] Response is valid JSON";
            // 检查我们的自定义 header 是否被服务器回显
            QJsonObject obj = doc.object();
            if (obj.contains("headers")) {
                QJsonObject headers = obj["headers"].toObject();
                qDebug() << "  [GET] Server echoed User-Agent:"
                         << headers["User-Agent"].toString();
                qDebug() << "  [GET] Server echoed X-Custom-Header:"
                         << headers["X-Custom-Header"].toString();
            }
        }

        reply->deleteLater();
    });
}
