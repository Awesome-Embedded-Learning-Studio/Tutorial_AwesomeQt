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

#include <QHttpServerResponse>
#include <QJsonArray>
#include <QJsonObject>

// ========================================
// 内存中的数据存储（演示用，生产环境请用数据库）
// ========================================

extern QJsonArray kItems;

/// @brief 初始化演示数据
void initSampleData();

/// @brief 构造带时间戳的 JSON 响应
QHttpServerResponse makeJsonResponse(
    const QJsonObject &data,
    QHttpServerResponse::StatusCode code
    = QHttpServerResponse::StatusCode::Ok);
