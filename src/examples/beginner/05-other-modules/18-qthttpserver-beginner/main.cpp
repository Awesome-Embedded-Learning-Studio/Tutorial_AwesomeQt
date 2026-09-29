/**
 * QtHttpServer 嵌入式 HTTP 服务器示例
 *
 * 本示例演示 QtHttpServer 模块的核心功能：
 * - QHttpServer::route() 注册 GET/POST 路由处理函数
 * - QHttpServerRequest 读取请求体、请求头、查询参数
 * - QHttpServerResponse 构造 JSON 响应
 * - 嵌入式 REST API 服务
 *
 * 启动后监听 8080 端口，提供以下接口：
 *   GET  /api/status              -- 服务状态
 *   GET  /api/items               -- 获取全部数据
 *   POST /api/items               -- 添加新数据项（JSON body）
 *   GET  /api/items/search?keyword=xxx -- 按关键词搜索
 */

#include "apidata.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QHostAddress>
#include <QHttpServer>
#include <QHttpHeaders>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QTcpServer>

#include <QDebug>

// ========================================
// 主函数
// ========================================

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "QtHttpServer 嵌入式 REST API 服务示例";
    qDebug() << "本示例演示 QHttpServer::route() + GET/POST + JSON";

    initSampleData();

    QHttpServer server;

    // GET /api/status —— 返回服务状态信息
    server.route(QStringLiteral("/api/status"),
        QHttpServerRequest::Method::Get,
        [](const QHttpServerRequest &req) {
            qDebug() << "[GET /api/status]"
                     << "from:" << req.remoteAddress().toString();

            QJsonObject status;
            status[QStringLiteral("service")]
                = QStringLiteral("Qt REST API Demo");
            status[QStringLiteral("version")] = QStringLiteral("1.0.0");
            status[QStringLiteral("status")] = QStringLiteral("running");
            status[QStringLiteral("item_count")] = kItems.size();
            status[QStringLiteral("timestamp")]
                = QDateTime::currentDateTime().toString(Qt::ISODate);

            return makeJsonResponse(status);
        });

    // GET /api/items —— 获取全部数据项
    server.route(QStringLiteral("/api/items"),
        QHttpServerRequest::Method::Get,
        [](const QHttpServerRequest &req) {
            qDebug() << "[GET /api/items]"
                     << "from:" << req.remoteAddress().toString();

            QJsonObject result;
            result[QStringLiteral("items")] = kItems;
            result[QStringLiteral("total")] = kItems.size();

            return makeJsonResponse(result);
        });

    // POST /api/items —— 添加新数据项（接收 JSON body）
    server.route(QStringLiteral("/api/items"),
        QHttpServerRequest::Method::Post,
        [](const QHttpServerRequest &req) {
            qDebug() << "[POST /api/items]"
                     << "from:" << req.remoteAddress().toString()
                     << "body size:" << req.body().size();

            // 解析请求体为 JSON
            QJsonParseError parseErr;
            QJsonDocument doc = QJsonDocument::fromJson(req.body(), &parseErr);

            if (parseErr.error != QJsonParseError::NoError) {
                qDebug() << "  JSON 解析失败:" << parseErr.errorString();
                QJsonObject errObj;
                errObj[QStringLiteral("error")]
                    = QStringLiteral("Invalid JSON");
                errObj[QStringLiteral("detail")] = parseErr.errorString();
                return makeJsonResponse(
                    errObj,
                    QHttpServerResponse::StatusCode::BadRequest);
            }

            QJsonObject newItem = doc.object();

            // 校验必需字段
            if (!newItem.contains(QStringLiteral("name"))
                || !newItem.contains(QStringLiteral("category"))) {
                QJsonObject errObj;
                errObj[QStringLiteral("error")]
                    = QStringLiteral(
                        "Missing required fields: name, category");
                return makeJsonResponse(
                    errObj,
                    QHttpServerResponse::StatusCode::BadRequest);
            }

            // 分配 ID 并存入内存
            int newId = kItems.size() + 1;
            newItem[QStringLiteral("id")] = newId;
            kItems.append(newItem);

            qDebug() << "  新增数据项 id:" << newId
                     << "name:"
                     << newItem[QStringLiteral("name")].toString();

            QJsonObject response;
            response[QStringLiteral("message")]
                = QStringLiteral("Item created");
            response[QStringLiteral("item")] = newItem;

            return makeJsonResponse(
                response,
                QHttpServerResponse::StatusCode::Created);
        });

    // GET /api/items/search?keyword=xxx —— 按关键词搜索
    server.route(QStringLiteral("/api/items/search"),
        QHttpServerRequest::Method::Get,
        [](const QHttpServerRequest &req) {
            QString keyword = req.query().queryItemValue(
                QStringLiteral("keyword"));

            qDebug() << "[GET /api/items/search]"
                     << "keyword:" << keyword
                     << "from:" << req.remoteAddress().toString();

            QJsonArray results;
            for (const QJsonValue &val : kItems) {
                QJsonObject item = val.toObject();
                QString name = item[QStringLiteral("name")].toString();
                QString cat = item[QStringLiteral("category")].toString();

                // 在 name 和 category 中搜索（不区分大小写）
                if (name.contains(keyword, Qt::CaseInsensitive)
                    || cat.contains(keyword, Qt::CaseInsensitive)) {
                    results.append(item);
                }
            }

            QJsonObject response;
            response[QStringLiteral("keyword")] = keyword;
            response[QStringLiteral("results")] = results;
            response[QStringLiteral("count")] = results.size();

            return makeJsonResponse(response);
        });

    // 全局响应后处理——添加自定义响应头
    server.addAfterRequestHandler(&server,
        [](const QHttpServerRequest &, QHttpServerResponse &resp) {
            QHttpHeaders headers = resp.headers();
            headers.replaceOrAppend("X-Powered-By",
                                    QStringLiteral("QtHttpServer"));
            resp.setHeaders(std::move(headers));
        });

    // 创建 TCP 服务器并监听 8080 端口
    auto tcpServer = std::make_unique<QTcpServer>();
    if (!tcpServer->listen(QHostAddress::Any, 8080)) {
        qCritical() << "无法监听端口 8080:" << tcpServer->serverError();
        return -1;
    }

    // 将 HTTP 服务器绑定到 TCP 服务器
    server.bind(tcpServer.get());

    // 打印启动信息
    qDebug() << "";
    qDebug() << "HTTP 服务器已启动";
    qDebug() << "监听地址:" << tcpServer->serverAddress().toString();
    qDebug() << "监听端口:" << tcpServer->serverPort();
    qDebug() << "";
    qDebug() << "可用路由:";
    qDebug() << "  GET  /api/status              -- 服务状态";
    qDebug() << "  GET  /api/items               -- 获取全部数据";
    qDebug() << "  POST /api/items               -- 添加新数据项";
    qDebug() << "  GET  /api/items/search?keyword=xxx -- 按关键词搜索";
    qDebug() << "";
    qDebug() << "测试命令:";
    qDebug() << "  curl http://localhost:8080/api/status";
    qDebug() << "  curl http://localhost:8080/api/items";
    qDebug() << "  curl -X POST http://localhost:8080/api/items"
                " \\\n    -H 'Content-Type: application/json' \\\n"
                "    -d '{\"name\":\"QtNetwork\",\"category\":\"Network\"}'";
    qDebug() << "  curl 'http://localhost:8080/api/items/search?keyword=Qt'";

    return app.exec();
}
