#include "httppostdemo.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

void demoPostRequest(QNetworkAccessManager *manager)
{
    qDebug() << "\n=== Demo 2: HTTP POST Request ===";

    QNetworkRequest request(QUrl("https://httpbin.org/post"));

    // 设置 Content-Type 为 JSON
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      "application/json");
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      "QtNetworkTutorial/1.0");

    // 构造 JSON 请求体
    QJsonObject json;
    json["name"] = "Qt Learner";
    json["topic"] = "QNetworkAccessManager";
    json["version"] = "6.9.1";
    QJsonDocument doc(json);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    qDebug() << "  [POST] Sending JSON:" << jsonData;

    QNetworkReply *reply = manager->post(request, jsonData);

    QObject::connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "  [POST] Network error:" << reply->errorString();
            reply->deleteLater();
            return;
        }

        int statusCode = reply->attribute(
            QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << "  [POST] HTTP Status:" << statusCode;

        QByteArray data = reply->readAll();

        // 解析服务器回显的 JSON
        QJsonDocument responseDoc = QJsonDocument::fromJson(data);
        if (!responseDoc.isNull()) {
            QJsonObject response = responseDoc.object();
            qDebug() << "  [POST] Server received our data:";
            if (response.contains("json")) {
                QJsonObject receivedJson = response["json"].toObject();
                qDebug() << "    name:" << receivedJson["name"].toString();
                qDebug() << "    topic:" << receivedJson["topic"].toString();
                qDebug() << "    version:" << receivedJson["version"].toString();
            }
            if (response.contains("headers")) {
                QJsonObject headers = response["headers"].toObject();
                qDebug() << "  [POST] Content-Type sent:"
                         << headers["Content-Type"].toString();
            }
        }

        reply->deleteLater();
    });
}
