/// @file    json_stream_demo.h
/// @brief   演示 Qt JSON/CBOR 处理的高级用法。
///
/// 对应教程：进阶层 01-QtBase/16-JSON 与 XML 解析。

#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QCborArray>
#include <QCborMap>
#include <QCborValue>
#include <QString>

/// @brief JSON/CBOR 演示类，展示复杂 JSON 构建、格式对比、解析错误处理和 CBOR 二进制格式。
class JsonStreamDemo
{
public:
    /// @brief 执行全部 JSON/CBOR 演示：构建、内存分析、解析、CBOR 对比。
    static void runAll();

private:
    /// @brief 构建多层嵌套的复杂 JSON 文档。
    /// @return 包含用户信息的完整 JSON 文档。
    static QJsonDocument buildComplexJson();

    /// @brief 对比 JSON Indented 与 Compact 格式的内存开销。
    /// @param[in] doc 待分析的 JSON 文档。
    static void demoJsonMemoryAnalysis(const QJsonDocument& doc);

    /// @brief 演示 JSON 字符串解析与 QJsonParseError 错误定位。
    static void demoJsonParsing();

    /// @brief 演示 CBOR 二进制 JSON 的写入、读取及与 JSON 的体积对比。
    /// @param[in] jsonDoc 用于对比的 JSON 文档。
    static void demoCborBinaryJson(const QJsonDocument& jsonDoc);
};
