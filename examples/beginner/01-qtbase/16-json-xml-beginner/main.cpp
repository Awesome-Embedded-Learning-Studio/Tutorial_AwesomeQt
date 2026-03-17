// Qt JSON 与 XML 解析入门示例
// 演示 QJsonDocument、QJsonObject、QJsonArray 解析和构建 JSON
// 演示 QXmlStreamReader、QXmlStreamWriter 解析和写入 XML

#include <QCoreApplication>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFile>

// 演示 JSON 解析基础用法
void demonstrateJsonParsing() {
    qDebug() << "=== JSON 解析演示 ===";

    // 示例 JSON 字符串
    QString jsonString = R"({
        "app": {
            "name": "MyQtApp",
            "version": "1.0.0",
            "debug": true
        },
        "features": ["network", "database", "ui"],
        "settings": {
            "maxConnections": 100,
            "timeout": 30.5
        }
    })";

    // 从字符串解析 JSON
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8(), &error);

    // 检查解析是否成功
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON 解析失败:" << error.errorString();
        qWarning() << "错误位置:" << error.offset;
        return;
    }

    // 检查文档类型
    if (!doc.isObject()) {
        qWarning() << "JSON 根元素不是对象";
        return;
    }

    // 获取根对象
    QJsonObject root = doc.object();

    // 解析嵌套对象
    if (root.contains("app") && root["app"].isObject()) {
        QJsonObject app = root["app"].toObject();
        qDebug() << "应用名称:" << app["name"].toString();
        qDebug() << "版本:" << app["version"].toString();
        qDebug() << "调试模式:" << app["debug"].toBool();
    }

    // 解析数组
    if (root.contains("features") && root["features"].isArray()) {
        QJsonArray features = root["features"].toArray();
        qDebug() << "功能列表:";
        for (const QJsonValue &feature : features) {
            qDebug() << "  -" << feature.toString();
        }
    }

    // 解析数字
    if (root.contains("settings") && root["settings"].isObject()) {
        QJsonObject settings = root["settings"].toObject();
        qDebug() << "最大连接数:" << settings["maxConnections"].toInt();
        qDebug() << "超时时间:" << settings["timeout"].toDouble() << "秒";
    }

    qDebug() << "";
}

// 演示 JSON 构建和序列化
void demonstrateJsonBuilding() {
    qDebug() << "=== JSON 构建演示 ===";

    // 构建嵌套对象
    QJsonObject appObj;
    appObj["name"] = "MyQtApp";
    appObj["version"] = "2.0.0";
    appObj["debug"] = false;

    // 构建数组
    QJsonArray featuresArray;
    featuresArray.append("network");
    featuresArray.append("database");
    featuresArray.append("ui");
    featuresArray.append("logging");

    // 构建设置对象
    QJsonObject settingsObj;
    settingsObj["maxConnections"] = 200;
    settingsObj["timeout"] = 60.0;
    settingsObj["retryCount"] = 3;

    // 组装根对象
    QJsonObject rootObj;
    rootObj["app"] = appObj;
    rootObj["features"] = featuresArray;
    rootObj["settings"] = settingsObj;

    // 创建文档并序列化
    QJsonDocument doc(rootObj);

    // 格式化输出（带缩进）
    qDebug() << "格式化 JSON:";
    qDebug() << doc.toJson(QJsonDocument::Indented);

    // 紧凑输出（无空白字符）
    qDebug() << "紧凑 JSON (前100字符):";
    QString compactJson = doc.toJson(QJsonDocument::Compact);
    qDebug() << compactJson.left(100) << "...";

    qDebug() << "";
}

// 演示 JSON 错误处理
void demonstrateJsonErrorHandling() {
    qDebug() << "=== JSON 错误处理演示 ===";

    // 测试各种无效 JSON
    QStringList invalidJsonList = {
        "{\"name\": \"test\"",           // 缺少闭合括号
        "{\"name\": test}",              // 值未加引号
        "{name: \"test\"}",              // 键未加引号
        "[1, 2, 3,]"                    // 数组末尾多余逗号
    };

    for (const QString &jsonStr : invalidJsonList) {
        QJsonParseError error;
        QJsonDocument::fromJson(jsonStr.toUtf8(), &error);

        if (error.error != QJsonParseError::NoError) {
            qDebug() << "错误:" << error.errorString();
            qDebug() << "位置:" << error.offset;
            qDebug() << "问题 JSON:" << jsonStr;
            qDebug() << "";
        }
    }

    // 演示安全访问键值
    qDebug() << "安全访问键值演示:";
    QJsonObject obj;
    obj["name"] = "Test";

    // 错误做法：直接访问不存在的键
    QString missing = obj["missing"].toString();  // 返回空字符串
    qDebug() << "直接访问不存在的键:" << missing;

    // 正确做法：先检查键是否存在
    if (obj.contains("missing")) {
        qDebug() << "键存在";
    } else {
        qDebug() << "键不存在，使用默认值";
    }

    // 或者使用 value() + 默认值
    QString safeValue = obj.value("missing").toString("default");
    qDebug() << "使用默认值:" << safeValue;

    qDebug() << "";
}

// 演示从文件读取 JSON
void demonstrateJsonFileIO() {
    qDebug() << "=== JSON 文件读写演示 ===";

    // 先创建一个 JSON 文件
    QJsonObject configObj;
    configObj["windowTitle"] = "Demo Application";
    configObj["width"] = 800;
    configObj["height"] = 600;
    configObj["fullscreen"] = false;

    QJsonDocument writeDoc(configObj);

    QFile writeFile("config.json");
    if (writeFile.open(QIODevice::WriteOnly)) {
        writeFile.write(writeDoc.toJson());
        writeFile.close();
        qDebug() << "配置文件已保存: config.json";
    }

    // 读取刚才保存的文件
    QFile readFile("config.json");
    if (readFile.open(QIODevice::ReadOnly)) {
        QJsonParseError error;
        QJsonDocument readDoc = QJsonDocument::fromJson(readFile.readAll(), &error);
        readFile.close();

        if (error.error == QJsonParseError::NoError && readDoc.isObject()) {
            QJsonObject config = readDoc.object();
            qDebug() << "读取配置:";
            qDebug() << "  窗口标题:" << config["windowTitle"].toString();
            qDebug() << "  宽度:" << config["width"].toInt();
            qDebug() << "  高度:" << config["height"].toInt();
            qDebug() << "  全屏:" << config["fullscreen"].toBool();
        }
    }

    // 清理测试文件
    QFile::remove("config.json");

    qDebug() << "";
}

// 演示 XML 解析基础用法
void demonstrateXmlParsing() {
    qDebug() << "=== XML 解析演示 ===";

    // XML 声明必须在文档开头，前面不能有空白字符
    QString xmlString = QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                                "<catalog>\n"
                                "    <book id=\"1\" category=\"programming\">\n"
                                "        <title>Qt Programming</title>\n"
                                "        <author>John Doe</author>\n"
                                "        <price>39.99</price>\n"
                                "        <available>true</available>\n"
                                "    </book>\n"
                                "    <book id=\"2\" category=\"database\">\n"
                                "        <title>SQL Essentials</title>\n"
                                "        <author>Jane Smith</author>\n"
                                "        <price>45.50</price>\n"
                                "        <available>false</available>\n"
                                "    </book>\n"
                                "    <book id=\"3\" category=\"ui\">\n"
                                "        <title>UI Design Patterns</title>\n"
                                "        <author>Bob Wilson</author>\n"
                                "        <price>35.00</price>\n"
                                "        <available>true</available>\n"
                                "    </book>\n"
                                "</catalog>\n");

    QXmlStreamReader reader(xmlString);

    // 统计变量
    int bookCount = 0;
    double total_price = 0.0;

    while (!reader.atEnd() && !reader.hasError()) {
        QXmlStreamReader::TokenType token = reader.readNext();

        if (token == QXmlStreamReader::StartElement) {
            // 处理 book 元素开始
            if (reader.name() == QStringLiteral("book")) {
                bookCount++;

                // 读取属性
                QStringView id = reader.attributes().value("id");
                QStringView category = reader.attributes().value("category");
                qDebug() << "图书 ID:" << id << "分类:" << category;
            }
            // 处理 title 元素
            else if (reader.name() == QStringLiteral("title")) {
                QString title = reader.readElementText();
                qDebug() << "  标题:" << title;
            }
            // 处理 author 元素
            else if (reader.name() == QStringLiteral("author")) {
                QString author = reader.readElementText();
                qDebug() << "  作者:" << author;
            }
            // 处理 price 元素
            else if (reader.name() == QStringLiteral("price")) {
                QString priceStr = reader.readElementText();
                double price = priceStr.toDouble();
                total_price += price;
                qDebug() << "  价格:" << price;
            }
            // 处理 available 元素
            else if (reader.name() == QStringLiteral("available")) {
                QString available = reader.readElementText();
                qDebug() << "  有货:" << (available == "true" ? "是" : "否");
            }
        }
        // book 元素结束，输出空行分隔
        else if (token == QXmlStreamReader::EndElement) {
            if (reader.name() == QStringLiteral("book")) {
                qDebug() << "";
            }
        }
    }

    // 检查解析错误
    if (reader.hasError()) {
        qWarning() << "XML 解析错误:" << reader.errorString();
        qWarning() << "行号:" << reader.lineNumber();
        qWarning() << "列号:" << reader.columnNumber();
    } else {
        qDebug() << "解析完成！";
        qDebug() << "图书总数:" << bookCount;
        qDebug() << "总价格:" << total_price;
    }

    qDebug() << "";
}

// 演示 XML 写入基础用法
void demonstrateXmlWriting() {
    qDebug() << "=== XML 写入演示 ===";

    QString output;
    QXmlStreamWriter writer(&output);

    // 设置自动格式化（缩进和换行）
    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(4);

    // 写入文档开始
    writer.writeStartDocument();

    // 写入注释
    writer.writeComment("这是自动生成的图书目录");

    // 写入根元素
    writer.writeStartElement("catalog");

    // 写入第一本图书
    writer.writeStartElement("book");
    writer.writeAttribute("id", "1");
    writer.writeAttribute("category", "programming");

    writer.writeTextElement("title", "C++ Primer");
    writer.writeTextElement("author", "Stanley Lippman");
    writer.writeTextElement("price", "59.99");

    writer.writeEndElement(); // 结束 book

    // 写入第二本图书
    writer.writeStartElement("book");
    writer.writeAttribute("id", "2");
    writer.writeAttribute("category", "database");

    writer.writeTextElement("title", "Database Design");
    writer.writeTextElement("author", "Alice Johnson");
    writer.writeTextElement("price", "49.99");

    writer.writeEndElement(); // 结束 book

    // 结束根元素
    writer.writeEndElement(); // 结束 catalog

    // 写入文档结束
    writer.writeEndDocument();

    qDebug() << "生成的 XML:";
    qDebug() << output;

    // 保存到文件
    QFile file("catalog.xml");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(output.toUtf8());
        file.close();
        qDebug() << "XML 已保存到: catalog.xml";
    }

    // 清理测试文件
    QFile::remove("catalog.xml");

    qDebug() << "";
}

// 演示 XML 错误处理
void demonstrateXmlErrorHandling() {
    qDebug() << "=== XML 错误处理演示 ===";

    // 测试无效 XML（标签未闭合）
    QString invalidXml = R"(
        <root>
            <element>content
        </root>
    )";

    QXmlStreamReader reader(invalidXml);

    while (!reader.atEnd()) {
        QXmlStreamReader::TokenType token = reader.readNext();

        if (reader.hasError()) {
            qDebug() << "XML 解析错误:" << reader.errorString();
            qDebug() << "错误类型:" << reader.error();
            qDebug() << "行号:" << reader.lineNumber();
            qDebug() << "列号:" << reader.columnNumber();
        }
    }

    qDebug() << "";
}

// 演示代码填空题答案：从 JSON 数组中提取用户名
void demonstrateJsonArrayExtraction() {
    qDebug() << "=== 代码填空题答案：提取用户名 ===";

    QString json = R"({
      "users": [
        {"name": "Alice", "age": 30},
        {"name": "Bob", "age": 25},
        {"name": "Charlie", "age": 35}
      ]
    })";

    // 答案：
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    QJsonObject root = doc.object();
    QJsonArray users = root["users"].toArray();

    qDebug() << "提取的用户列表:";
    for (const QJsonValue &user : users) {
        QJsonObject userObj = user.toObject();
        QString name = userObj["name"].toString();
        int age = userObj["age"].toInt();
        qDebug() << "  " << name << "(年龄:" << age << ")";
    }

    qDebug() << "";
}

// 演示 JSON 和 XML 互转
void demonstrateJsonXmlConversion() {
    qDebug() << "=== JSON 和 XML 互转演示 ===";

    // 原始 JSON
    QString jsonString = R"({
        "person": {
            "name": "Alice",
            "age": 30,
            "hobbies": ["reading", "gaming", "coding"]
        }
    })";

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());
    QJsonObject root = jsonDoc.object();
    QJsonObject person = root["person"].toObject();

    // 转换为 XML
    QString xmlOutput;
    QXmlStreamWriter xmlWriter(&xmlOutput);
    xmlWriter.setAutoFormatting(true);

    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement("person");

    // 写入简单字段
    xmlWriter.writeTextElement("name", person["name"].toString());
    xmlWriter.writeTextElement("age", QString::number(person["age"].toInt()));

    // 写入数组
    QJsonArray hobbies = person["hobbies"].toArray();
    xmlWriter.writeStartElement("hobbies");
    for (const QJsonValue &hobby : hobbies) {
        xmlWriter.writeTextElement("hobby", hobby.toString());
    }
    xmlWriter.writeEndElement(); // hobbies

    xmlWriter.writeEndElement(); // person
    xmlWriter.writeEndDocument();

    qDebug() << "JSON 转 XML 结果:";
    qDebug() << xmlOutput;

    qDebug() << "";
}

// 演示 QJsonValue 类型检查
void demonstrateJsonValueTypeCheck() {
    qDebug() << "=== QJsonValue 类型检查演示 ===";

    QString json = R"({
        "string": "Hello",
        "number": 42,
        "float": 3.14,
        "bool": true,
        "null": null,
        "array": [1, 2, 3],
        "object": {"key": "value"}
    })";

    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    QJsonObject root = doc.object();

    // 遍历所有键，检查类型
    for (const QString &key : root.keys()) {
        QJsonValue value = root.value(key);

        QString typeStr;
        if (value.isString()) {
            typeStr = "String";
        } else if (value.isDouble()) {
            typeStr = "Double";
        } else if (value.isBool()) {
            typeStr = "Bool";
        } else if (value.isNull()) {
            typeStr = "Null";
        } else if (value.isArray()) {
            typeStr = "Array";
        } else if (value.isObject()) {
            typeStr = "Object";
        }

        qDebug() << key << ":" << typeStr;
    }

    qDebug() << "";
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    qDebug() << "Qt JSON 与 XML 解析入门示例";
    qDebug() << "===========================";
    qDebug() << "";

    // JSON 演示
    demonstrateJsonParsing();
    demonstrateJsonBuilding();
    demonstrateJsonErrorHandling();
    demonstrateJsonFileIO();
    demonstrateJsonValueTypeCheck();
    demonstrateJsonArrayExtraction();

    // XML 演示
    demonstrateXmlParsing();
    demonstrateXmlWriting();
    demonstrateXmlErrorHandling();

    // 互转演示
    demonstrateJsonXmlConversion();

    qDebug() << "演示完成！";

    return 0;
}
