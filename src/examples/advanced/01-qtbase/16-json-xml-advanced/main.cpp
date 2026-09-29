/// @file    main.cpp
/// @brief   程序入口，运行 JSON/CBOR 与 XML 流式处理的高级演示。
///
/// 对应教程：进阶层 01-QtBase/16-JSON 与 XML 解析。

#include "json_stream_demo.h"
#include "xml_stream_demo.h"

#include <QCoreApplication>
#include <QDebug>

auto main(int argc, char* argv[]) -> int
{
    QCoreApplication app(argc, argv);

    QCoreApplication::setApplicationName("json-xml-advanced-demo");
    QCoreApplication::setApplicationVersion("1.0");

    qDebug() << "========================================";
    qDebug() << "  Qt6 XML/JSON" << QString::fromUtf8("高级处理示例");
    qDebug() << "========================================";
    qDebug() << "  Qt" << QString::fromUtf8("版本:") << QT_VERSION_STR;
    qDebug() << "";

    // 演示 1-2：XML 流式写入与解析
    qDebug() << "\n[XML]" << QString::fromUtf8("QXmlStreamWriter 生成 XML");
    qDebug() << "========================================";
    XmlStreamDemo::runAll();

    // 演示 3-5：JSON 解析、内存分析与 CBOR
    qDebug() << "\n[JSON] QJsonDocument" << QString::fromUtf8("与 QCborValue");
    qDebug() << "========================================";
    JsonStreamDemo::runAll();

    qDebug() << "";
    qDebug() << "========================================";
    qDebug() << QString::fromUtf8("  所有演示执行完毕");
    qDebug() << "========================================";

    return 0;
}
