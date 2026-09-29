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

// ========================================
// 内存中的数据存储（演示用，生产环境请用数据库）
// ========================================

QJsonArray kItems = {};

/// @brief 初始化演示数据
void initSampleData()
{
    QJsonObject item1;
    item1[QStringLiteral("id")] = 1;
    item1[QStringLiteral("name")] = QStringLiteral("QtQuick3D");
    item1[QStringLiteral("category")] = QStringLiteral("3D");
    kItems.append(item1);

    QJsonObject item2;
    item2[QStringLiteral("id")] = 2;
    item2[QStringLiteral("name")] = QStringLiteral("QtPdf");
    item2[QStringLiteral("category")] = QStringLiteral("Document");
    kItems.append(item2);

    QJsonObject item3;
    item3[QStringLiteral("id")] = 3;
    item3[QStringLiteral("name")] = QStringLiteral("QtHttpServer");
    item3[QStringLiteral("category")] = QStringLiteral("Network");
    kItems.append(item3);
}

/// @brief 构造带时间戳的 JSON 响应
QHttpServerResponse makeJsonResponse(
    const QJsonObject &data,
    QHttpServerResponse::StatusCode code)
{
    return QHttpServerResponse(data, code);
}
