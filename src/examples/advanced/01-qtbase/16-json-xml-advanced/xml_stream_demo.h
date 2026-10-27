/// @file    xml_stream_demo.h
/// @brief   演示 Qt XML 流式 API 的高级用法。
///
/// 对应教程：进阶层 01-QtBase/16-JSON 与 XML 解析。

#pragma once

#include <QByteArray>
#include <QBuffer>
#include <QDebug>
#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

/// @brief XML 流式演示类，展示 QXmlStreamWriter 写入、QXmlStreamReader 解析及错误处理。
class XmlStreamDemo
{
public:
    /// @brief 执行全部 XML 演示：写入、解析、错误处理。
    static void runAll();

private:
    /// @brief 使用 QXmlStreamWriter 生成带命名空间的格式化 XML 文档。
    /// @return 生成的 XML 二进制数据。
    static QByteArray demoXmlWriting();

    /// @brief 使用 QXmlStreamReader 状态机方式解析 XML 并逐 token 输出。
    /// @param[in] xmlData 待解析的 XML 原始数据。
    static void demoXmlParsing(const QByteArray& xmlData);

    /// @brief 演示 QXmlStreamReader 对格式错误 XML 的检测与报告。
    static void demoXmlErrorHandling();
};
