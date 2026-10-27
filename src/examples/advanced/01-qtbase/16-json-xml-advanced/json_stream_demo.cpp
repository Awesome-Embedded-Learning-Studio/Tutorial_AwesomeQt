/// @file    json_stream_demo.cpp
/// @brief   JsonStreamDemo 类的实现：JSON/CBOR 高级用法演示。
///
/// 对应教程：进阶层 01-QtBase/16-JSON 与 XML 解析。

#include "json_stream_demo.h"

QJsonDocument JsonStreamDemo::buildComplexJson()
{
    qDebug() << "  [QJsonDocument] 构建复杂嵌套 JSON 文档";
    qDebug() << "  " << QString(46, '-');

    // 从内到外构建嵌套结构

    // 地址对象（嵌套在最深层）
    QJsonObject address;
    address["street"] = QString::fromUtf8("中关村大街 1 号");
    address["city"] = QString::fromUtf8("北京");
    address["province"] = QString::fromUtf8("北京市");
    address["postalCode"] = "100080";

    // 联系方式对象
    QJsonObject contact;
    contact["email"] = "zhangsan@example.com";
    contact["phone"] = "+86-138-0000-0000";
    contact["wechat"] = "zhangsan_wx";

    // 技能数组
    QJsonArray skills;
    skills.append("C++");
    skills.append("Qt");
    skills.append("Python");
    skills.append("Linux");
    skills.append("Embedded Systems");

    // 项目经验数组（包含对象）
    QJsonArray projects;
    {
        QJsonObject proj1;
        proj1["name"] = QString::fromUtf8("工业控制系统");
        proj1["role"] = QString::fromUtf8("技术负责人");
        proj1["duration"] = "2023-2025";
        proj1["description"] = QString::fromUtf8("基于 Qt6 开发的工业 HMI 系统");
        projects.append(proj1);

        QJsonObject proj2;
        proj2["name"] = QString::fromUtf8("物联网网关");
        proj2["role"] = QString::fromUtf8("核心开发者");
        proj2["duration"] = "2022-2023";
        proj2["description"] = QString::fromUtf8("嵌入式 Linux 平台的 MQTT 网关");
        projects.append(proj2);
    }

    // 顶层对象：用户信息
    QJsonObject user;
    user["id"] = 1001;
    user["name"] = QString::fromUtf8("张三");
    user["age"] = 30;
    user["active"] = true;
    user["address"] = address;
    user["contact"] = contact;
    user["skills"] = skills;
    user["projects"] = projects;
    user["lastLogin"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    return QJsonDocument(user);
}

void JsonStreamDemo::demoJsonMemoryAnalysis(const QJsonDocument& doc)
{
    qDebug() << "";
    qDebug() << "  " << QString::fromUtf8("[内存分析] JSON 格式对比");
    qDebug() << "  " << QString(46, '-');

    // QJsonDocument::toJson 两种格式：
    // - Indented: 带缩进换行，人类可读
    // - Compact:  紧凑格式，最小体积
    QByteArray indented = doc.toJson(QJsonDocument::Indented);
    QByteArray compact = doc.toJson(QJsonDocument::Compact);

    qDebug() << "    Indented" << QString::fromUtf8("格式大小:") << indented.size()
             << QString::fromUtf8("字节");
    qDebug() << "    Compact" << QString::fromUtf8("格式大小: ") << compact.size()
             << QString::fromUtf8("字节");
    double savedPercent = 100.0 * (indented.size() - compact.size()) / indented.size();
    qDebug() << QString::fromUtf8("    节省空间:")
             << (indented.size() - compact.size())
             << QString::fromUtf8("字节 (")
             << QString::number(savedPercent, 'f', 1) << "%)";

    // 输出 Compact 格式预览
    qDebug() << "";
    qDebug() << "    Compact" << QString::fromUtf8("预览:");
    QString preview = QString::fromUtf8(compact).left(200);
    qDebug() << "    " << preview << "...";

    // 输出 Indented 格式前几行
    qDebug() << "";
    qDebug() << "    Indented" << QString::fromUtf8("预览（前 8 行）:");
    QStringList lines = QString::fromUtf8(indented).split('\n');
    for (int i = 0; i < qMin(8, lines.size()); ++i) {
        qDebug() << "    " << lines[i];
    }
}

void JsonStreamDemo::demoJsonParsing()
{
    qDebug() << "";
    qDebug() << "  [JSON" << QString::fromUtf8("解析] 解析 JSON 字符串与错误处理");
    qDebug() << "  " << QString(46, '-');

    // ---- 正常 JSON 解析 ----
    QString validJson = R"({
        "name": "Qt6 Tutorial",
        "version": 1.0,
        "modules": ["Core", "Gui", "Widgets", "Network"],
        "config": {
            "debug": true,
            "threadCount": 4,
            "maxConnections": 100
        }
    })";

    // QJsonParseError 用于获取解析错误的详细信息
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(validJson.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << QString::fromUtf8("    解析错误:") << parseError.errorString();
        qDebug() << QString::fromUtf8("    偏移位置:") << parseError.offset;
    } else {
        qDebug() << QString::fromUtf8("    解析成功!");

        QJsonObject root = doc.object();
        qDebug() << "    name:" << root["name"].toString();
        qDebug() << "    version:" << root["version"].toDouble();

        // 遍历数组
        QJsonArray modules = root["modules"].toArray();
        qDebug() << "    modules:" << modules.size()
                 << QString::fromUtf8("个");
        for (const auto& mod : modules) {
            qDebug() << "      -" << mod.toString();
        }

        // 遍历嵌套对象
        QJsonObject config = root["config"].toObject();
        qDebug() << "    config:";
        for (auto it = config.begin(); it != config.end(); ++it) {
            qDebug() << "      " << it.key() << "="
                     << it.value().toVariant().toString();
        }
    }

    // ---- 错误 JSON 解析 ----
    qDebug() << "";
    qDebug() << "    " << QString::fromUtf8("[错误处理演示]");

    // 错误 1：缺少引号
    QString badJson1 = R"({"name": test})";
    QJsonParseError err1;
    QJsonDocument::fromJson(badJson1.toUtf8(), &err1);
    qDebug() << QString::fromUtf8("      错误 1:") << err1.errorString()
             << QString::fromUtf8("(偏移:") << err1.offset << ")";

    // 错误 2：多余的逗号
    QString badJson2 = R"({"a": 1, "b": 2,})";
    QJsonParseError err2;
    QJsonDocument::fromJson(badJson2.toUtf8(), &err2);
    qDebug() << QString::fromUtf8("      错误 2:") << err2.errorString()
             << QString::fromUtf8("(偏移:") << err2.offset << ")";

    // 错误 3：缺少大括号
    QString badJson3 = R"({"name": "test")";
    QJsonParseError err3;
    QJsonDocument::fromJson(badJson3.toUtf8(), &err3);
    qDebug() << QString::fromUtf8("      错误 3:") << err3.errorString()
             << QString::fromUtf8("(偏移:") << err3.offset << ")";
}

void JsonStreamDemo::demoCborBinaryJson(const QJsonDocument& jsonDoc)
{
    qDebug() << "";
    qDebug() << "  [CBOR]" << QString::fromUtf8("二进制 JSON 写入与读取");
    qDebug() << "  " << QString(46, '-');

    // 从 QJsonDocument 转换为 CBOR
    QCborValue cborFromJson = QCborValue::fromJsonValue(jsonDoc.object());
    QByteArray cborData = cborFromJson.toCbor();
    QByteArray jsonCompact = jsonDoc.toJson(QJsonDocument::Compact);

    qDebug() << "    JSON Compact" << QString::fromUtf8("大小:") << jsonCompact.size()
             << QString::fromUtf8("字节");
    qDebug() << "    CBOR" << QString::fromUtf8("二进制大小: ") << cborData.size()
             << QString::fromUtf8("字节");

    int saved = qMax(0, jsonCompact.size() - cborData.size());
    double savedPercent = 100.0 * saved / jsonCompact.size();
    qDebug() << "    CBOR" << QString::fromUtf8("比 JSON 节省:")
             << saved << QString::fromUtf8("字节 (")
             << QString::number(savedPercent, 'f', 1) << "%)";

    // 从 CBOR 二进制数据读回
    QCborParserError cborError;
    QCborValue parsedCbor = QCborValue::fromCbor(cborData, &cborError);

    if (cborError.error != QCborError::NoError) {
        qDebug() << "    CBOR" << QString::fromUtf8("解析错误:") << cborError.errorString();
    } else {
        qDebug() << "    CBOR" << QString::fromUtf8("解析成功!");

        QCborMap roundTripped = parsedCbor.toMap();
        qDebug() << QString::fromUtf8("    验证: name =")
                 << roundTripped[QLatin1String("name")].toString();
        qDebug() << QString::fromUtf8("    验证: id =")
                 << roundTripped[QLatin1String("id")].toInteger();
    }

    // CBOR 独有特性：支持原生二进制数据
    qDebug() << "";
    qDebug() << "    [CBOR" << QString::fromUtf8("独有特性] 支持原生二进制数据");
    QCborMap binaryCbor;
    binaryCbor[QLatin1String("name")] = QCborValue("binary-file");
    binaryCbor[QLatin1String("size")] = QCborValue(1024);
    // QByteArray 直接作为 CBOR 字节串存储，无需 Base64 编码
    binaryCbor[QLatin1String("data")] = QCborValue(QByteArray(100, '\x42'));

    QByteArray binaryCborData = QCborValue(binaryCbor).toCbor();
    qDebug() << QString::fromUtf8("      含 100 字节二进制数据的 CBOR 大小:")
             << binaryCborData.size() << QString::fromUtf8("字节");

    // 对比 JSON + Base64 编码
    QJsonObject jsonObj;
    jsonObj["name"] = "binary-file";
    jsonObj["size"] = 1024;
    jsonObj["data"] = QString(QByteArray(100, '\x42').toBase64());
    QByteArray jsonBinaryData = QJsonDocument(jsonObj).toJson(QJsonDocument::Compact);
    qDebug() << QString::fromUtf8("      JSON + Base64 编码大小:")
             << jsonBinaryData.size() << QString::fromUtf8("字节");
    qDebug() << "      CBOR" << QString::fromUtf8("节省:")
             << (jsonBinaryData.size() - binaryCborData.size())
             << QString::fromUtf8("字节");
}

void JsonStreamDemo::runAll()
{
    QJsonDocument doc = buildComplexJson();
    demoJsonMemoryAnalysis(doc);
    demoJsonParsing();
    demoCborBinaryJson(doc);
}
