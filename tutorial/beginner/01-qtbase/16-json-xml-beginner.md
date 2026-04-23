# 现代Qt开发教程（新手篇）1.16——JSON与XML解析

## 1. 前言

说实话，在我刚开始用 Qt 那会儿，处理 JSON/XML 这类数据交换格式简直是一件令人头疼的事。那时候要么自己写解析器，要么依赖第三方库，写出来的代码又长又难维护。后来 Qt5 引入了完整的 JSON 支持，Qt6 又进一步完善，现在处理这些格式已经是家常便饭了。

JSON 和 XML 是两种最常见的数据交换格式。JSON 轻量简洁，适合网络传输和配置文件；XML 结构严谨，适合复杂数据描述和遗留系统对接。在实际项目中，你可能会遇到从 REST API 获取 JSON 响应、解析配置文件、处理遗留的 XML 数据等场景。Qt 提供的 QJsonDocument 和 QXmlStreamReader 让这些任务变得相对轻松。

这篇入门文章不会讲所有 API 细节——那够写一本小册子——但会覆盖日常开发中 80% 的场景。我们会尽量把基础打扎实，不仅告诉你怎么用，更重要的是怎么避免那些坑。

## 2. 环境说明

本文基于 Qt 6.10+，所有示例使用 CMake 3.26+ 构建系统。JSON 相关类（QJsonDocument、QJsonObject、QJsonArray）属于 Qt Core 模块，XML 相关类（QXmlStreamReader、QXmlStreamWriter）需要链接 Qt::Core。在 Qt6 中，JSON 支持已经非常成熟，而 XML 解析虽然不如 JSON 流行，但在某些行业（如金融、医疗）仍然是标准格式。

## 3. 核心概念

### 3.1 JSON 基础

JSON（JavaScript Object Notation）是一种轻量级的数据交换格式。它支持两种数据结构：对象（键值对集合）和数组（有序值列表）。Qt 的 JSON 类设计得很直观，与 JSON 结构一一对应：QJsonDocument 表示整个 JSON 文档，QJsonObject 表示 JSON 对象，QJsonArray 表示 JSON 数组，而 QJsonValue 表示一个 JSON 值，这个值可以是对象、数组、字符串、数字、布尔值或 null 中的任意一种。

解析 JSON 最常见的场景是从字符串或文件读取数据。比如这样一个 JSON 配置：

```json
{
  "app": {
    "name": "MyApp",
    "version": "1.0.0",
    "features": ["network", "database", "ui"]
  },
  "settings": {
    "debug": true,
    "maxConnections": 100
  }
}
```

用 Qt 解析这个 JSON 的基本流程是：

```cpp
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

// 从字符串解析
QString jsonString = R"({"app": {"name": "MyApp", "version": "1.0.0"}})";
QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());

if (doc.isNull() || doc.isEmpty()) {
    qWarning() << "JSON 解析失败";
    return;
}

QJsonObject root = doc.object();
QString appName = root["app"].toObject()["name"].toString();
qDebug() << "应用名称:" << appName;  // "MyApp"

// 从文件解析
QFile file("config.json");
if (file.open(QIODevice::ReadOnly)) {
    QJsonDocument fileDoc = QJsonDocument::fromJson(file.readAll());
    file.close();
    // 使用 fileDoc...
}
```

这里有个细节要注意：`fromJson()` 接受的是 QByteArray，所以需要先把 QString 转成 UTF-8 编码的字节数组。这是因为在 JSON 标准中，字符串默认就是 UTF-8 编码的。

到这里你可以停下来想一想：QJsonDocument、QJsonObject、QJsonArray 各自代表什么？它们之间的包含关系是怎样的？理解了这一点，后面构建和修改 JSON 就会顺畅很多。

### 3.2 构建和序列化 JSON

解析是从 JSON 到 C++ 数据结构，构建则是从 C++ 数据结构到 JSON。两者是反向操作，API 设计也是对称的：

```cpp
// 构建一个 JSON 对象
QJsonObject appObj;
appObj["name"] = "MyApp";
appObj["version"] = "1.0.0";

QJsonArray featuresArray;
featuresArray.append("network");
featuresArray.append("database");
featuresArray.append("ui");
appObj["features"] = featuresArray;

QJsonObject rootObj;
rootObj["app"] = appObj;

// 序列化为 JSON 字符串
QJsonDocument doc(rootObj);
QString jsonString = doc.toJson(QJsonDocument::Indented);
qDebug() << jsonString;

// 输出（格式化后）：
// {
//   "app": {
//     "name": "MyApp",
//     "version": "1.0.0",
//     "features": [
//       "network",
//       "database",
//       "ui"
//     ]
//   }
// }

// 保存到文件
QFile file("output.json");
if (file.open(QIODevice::WriteOnly)) {
    file.write(doc.toJson());
    file.close();
}
```

`toJson()` 方法可以接受一个格式参数，`Indented` 会让输出格式化（带缩进和换行），适合人类阅读；`Compact` 则会去掉所有空白字符，适合网络传输。

现在我们来做一个小练习：下面的代码想从 JSON 数组中提取所有用户名，请补充空白处。

```cpp
QString json = R"({
  "users": [
    {"name": "Alice", "age": 30},
    {"name": "Bob", "age": 25},
    {"name": "Charlie", "age": 35}
  ]
})";

QJsonDocument doc = QJsonDocument::fromJson(json.______());
QJsonObject root = doc.______();
QJsonArray users = root["users"].______();

for (const QJsonValue &user : users) {
    QJsonObject userObj = user.______();
    qDebug() << "用户:" << userObj["name"].______();
}
```

### 3.3 错误处理

解析 JSON 时可能会遇到格式错误、类型错误等问题。QJsonDocument 提供了一些方法来检查：

```cpp
QJsonParseError error;
QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8(), &error);

if (error.error != QJsonParseError::NoError) {
    qWarning() << "JSON 解析错误:" << error.errorString();
    qWarning() << "错误位置:" << error.offset;
    return;
}

// 检查文档类型
if (doc.isObject()) {
    qDebug() << "这是一个 JSON 对象";
} else if (doc.isArray()) {
    qDebug() << "这是一个 JSON 数组";
}

// 安全访问键值
QJsonObject root = doc.object();
if (root.contains("app") && root["app"].isObject()) {
    QJsonObject app = root["app"].toObject();
    QString name = app["name"].toString();
    qDebug() << "应用名称:" << name;
} else {
    qWarning() << "'app' 键不存在或不是对象";
}
```

这里有个常见的坑：直接访问不存在的键会返回一个 null QJsonValue，调用 toString() 会得到空字符串，不会报错。所以最好用 `contains()` 先检查键是否存在，或者用 `value()` 方法配合默认值。

### 3.4 XML 基础与 QXmlStreamReader

XML（eXtensible Markup Language）是一种更古老但仍然重要的数据交换格式。与 JSON 相比，XML 更冗长，但支持命名空间、属性、注释等更丰富的特性。Qt 提供了三种 XML 处理方式：QXmlStreamReader 是快速、低内存的流式解析器，也是我们推荐的首选；QXmlStreamWriter 是配套的流式写入器；QDomDocument 是 DOM 风格的解析器，比较重，不推荐用于大文件。

这里我们重点介绍 QXmlStreamReader，它采用事件驱动模型，一边读取一边解析，内存占用小且速度快。

比如这样一个 XML：

```xml
<catalog>
  <book id="1">
    <title>Qt Programming</title>
    <author>John Doe</author>
    <price>39.99</price>
  </book>
  <book id="2">
    <title>C++ Primer</title>
    <author>Jane Smith</author>
    <price>45.50</price>
  </book>
</catalog>
```

用 QXmlStreamReader 解析的基本流程：

```cpp
#include <QXmlStreamReader>
#include <QFile>

QString xmlString = R"(
<catalog>
  <book id="1">
    <title>Qt Programming</title>
    <author>John Doe</author>
    <price>39.99</price>
  </book>
</catalog>
)";

QXmlStreamReader reader(xmlString);

while (!reader.atEnd() && !reader.hasError()) {
    QXmlStreamReader::TokenType token = reader.readNext();

    if (token == QXmlStreamReader::StartElement) {
        if (reader.name() == QStringLiteral("book")) {
            // 读取属性
            QStringRef id = reader.attributes().value("id");
            qDebug() << "图书 ID:" << id;
        } else if (reader.name() == QStringLiteral("title")) {
            // 读取元素文本
            QString title = reader.readElementText();
            qDebug() << "标题:" << title;
        }
    }
}

if (reader.hasError()) {
    qWarning() << "XML 解析错误:" << reader.errorString();
}
```

QXmlStreamReader 的工作原理是"标记化"（tokenization）：它把 XML 分解成一系列标记（StartElement、EndElement、Characters、Comment 等），然后你根据这些标记做出相应处理。这种模型类似于 SAX 解析器，但 API 更加友好。

### 3.5 QXmlStreamWriter 写入 XML

写入 XML 使用 QXmlStreamWriter，它自动处理转义、编码等细节，确保生成格式正确的 XML：

```cpp
#include <QXmlStreamWriter>
#include <QFile>

QString output;
QXmlStreamWriter writer(&output);

// 设置自动格式化（缩进和换行）
writer.setAutoFormatting(true);

// 写入文档开始
writer.writeStartDocument();

// 写入根元素
writer.writeStartElement("catalog");

// 写入第一个图书
writer.writeStartElement("book");
writer.writeAttribute("id", "1");

writer.writeTextElement("title", "Qt Programming");
writer.writeTextElement("author", "John Doe");
writer.writeTextElement("price", "39.99");

// 结束 book 元素
writer.writeEndElement(); // 或 writeEndElement()

// 写入第二个图书
writer.writeStartElement("book");
writer.writeAttribute("id", "2");
writer.writeTextElement("title", "C++ Primer");
writer.writeTextElement("author", "Jane Smith");
writer.writeTextElement("price", "45.50");
writer.writeEndElement();

// 结束根元素
writer.writeEndElement();

// 写入文档结束
writer.writeEndDocument();

qDebug() << output;

// 保存到文件
QFile file("catalog.xml");
if (file.open(QIODevice::WriteOnly)) {
    file.write(output.toUtf8());
    file.close();
}
```

QXmlStreamWriter 会自动处理特殊字符的转义（如 `<` 转成 `&lt;`），确保生成的 XML 格式正确。你只需要关注逻辑结构，不需要担心这些细节。

## 4. 踩坑预防

很好，概念讲完了，我们来看看实际开发中最容易踩的坑。这些都是我亲身体验过的，希望大家不要重蹈覆辙。

第一个坑是忘记检查 JSON 解析是否成功。很多人拿到字符串就直接 `fromJson()` 然后访问 object，完全不检查返回值。如果 JSON 格式有问题，解析失败后 `doc.object()` 返回的是空对象，后续访问全都在操作空数据，而且不会报错—— toString() 只会返回空字符串，你以为业务逻辑正常，其实数据全丢了。正确的做法是每次解析后都用 `QJsonParseError` 检查，或者至少用 `isNull()` / `isEmpty()` 判断一下：

```cpp
// 不检查就直接用，出问题了很难排查
QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
QJsonObject root = doc.object();  // 如果解析失败，root 会是空对象
QString name = root["name"].toString();  // 得到空字符串，但不报错

// 正确做法：先检查再使用
QJsonParseError error;
QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8(), &error);

if (error.error != QJsonParseError::NoError) {
    qWarning() << "JSON 解析失败:" << error.errorString();
    qWarning() << "错误位置:" << error.offset;
    return;
}
```

第二个坑是直接访问不存在的 JSON 键。这个坑和第一个有异曲同工之处——访问一个不存在的键不会抛异常，只会返回 null QJsonValue，toString() 返回空字符串。你可能会把空字符串当成合法数据继续往下走，最后整个逻辑链都偏了。所以在访问之前先 `contains()` 检查一下，或者用 `value()` 方法带一个默认值，这样安全得多：

```cpp
QJsonObject root = doc.object();
if (root.contains("nonexistent")) {
    QString value = root["nonexistent"].toString();
} else {
    qWarning() << "'nonexistent' 键不存在";
}

// 或者使用 value() + 默认值
QString value = root.value("nonexistent").toString("default");
```

第三个坑出在 QXmlStreamReader 上——读取文本内容时如果不注意，会捕获大量无意义的空白字符。XML 格式化后元素之间有换行和缩进，这些都会作为 Characters 事件返回。如果你直接处理所有 Characters 事件，输出的文本里会夹带一堆空格和换行。解决方案有两个：要么优先用 `readElementText()` 直接读取指定元素的文本内容，要么在处理 Characters 事件时用 `isWhitespace()` 过滤掉空白字符：

```cpp
// 方案一：readElementText() 直接读文本
QXmlStreamReader reader(xml);
while (!reader.atEnd()) {
    if (reader.readNext() == QXmlStreamReader::StartElement) {
        if (reader.name() == QStringLiteral("title")) {
            QString title = reader.readElementText();
            qDebug() << "标题:" << title;
        }
    }
}

// 方案二：过滤空白字符
while (!reader.atEnd()) {
    QXmlStreamReader::TokenType token = reader.readNext();
    if (token == QXmlStreamReader::Characters && !reader.isWhitespace()) {
        QString text = reader.text().toString();
        qDebug() << "文本内容:" << text;
    }
}
```

第四个坑是 JSON 中的数字精度丢失。JSON 标准里数字是没有类型区分的，而 C++ 里有 int、double、qint64 等多种数值类型。`toDouble()` 在遇到大整数时可能损失精度——超过 2 的 53 次方的整数，double 就没法精确表示了。处理大数字时优先用 `toInteger()`，或者通过 `QVariant` 转成 `qlonglong`，同时留意范围检查：

```cpp
QJsonValue numberValue = root["largeNumber"];
if (numberValue.isDouble()) {
    double doubleValue = numberValue.toDouble();
    // 对于大整数，检查是否在安全范围内
    if (doubleValue > 9007199254740992.0) {  // 2^53
        qWarning() << "数字超出安全整数范围，可能损失精度";
    }
}

// 或者使用 QVariant 转换
QVariant variant = root["largeNumber"].toVariant();
if (variant.canConvert<qlonglong>()) {
    qlonglong longValue = variant.toLongLong();
}
```

第五个坑是 XML 解析时忽略命名空间。如果 XML 文档带命名空间，直接用 `reader.name()` 匹配可能不会按你预期的方式工作。`name()` 返回的是本地名称（不含前缀），而 `qualifiedName()` 返回的是带前缀的完整名称。如果你的 XML 用了命名空间，需要根据实际情况选择匹配方式，或者通过 `addExtraNamespaceDeclaration()` 添加命名空间声明来正确处理：

```cpp
QXmlStreamReader reader(xml);
while (!reader.atEnd()) {
    if (reader.readNext() == QXmlStreamReader::StartElement) {
        // 完全匹配（包括命名空间前缀）
        if (reader.qualifiedName() == QStringLiteral("ns:book")) {
            // ...
        }

        // 只匹配本地名称（忽略命名空间）
        if (reader.name() == QStringLiteral("book")) {
            // ...
        }
    }
}
```

接下来是一道调试挑战：下面的代码有什么问题？

```cpp
QString json = R"({"users": [{"name": "Alice"}, {"name": "Bob"}]})";
QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
QJsonObject root = doc.object();

// 开发者想获取用户数量
int count = root["users"].toArray().count();
qDebug() << "用户数量:" << count;

// 然后遍历用户
for (int i = 0; i <= count; ++i) {
    QJsonObject user = root["users"].toArray()[i].toObject();
    qDebug() << "用户:" << user["name"].toString();
}
```

## 5. 练习项目

我们来做一个综合练习：实现一个简单的命令行配置文件管理工具，支持 JSON 和 XML 两种格式的配置文件，功能包括读取配置文件、修改配置项、添加新配置、保存配置文件，以及 JSON 和 XML 之间的格式转换。

完成标准是：程序能够处理包含嵌套对象的 JSON 和 XML 配置文件，实现上述 CRUD 操作。JSON 配置至少支持字符串、数字、布尔值、数组和嵌套对象类型。XML 配置支持属性和嵌套元素。格式转换能够将 JSON 转换为等效的 XML 表示，反之亦然。程序提供清晰的命令行界面，用户可以通过命令选择操作。

几个提示：JSON 用 QJsonDocument + QJsonObject，XML 用 QXmlStreamReader/QXmlStreamWriter；可以用 `QHash<QString, QVariant>` 存储配置在内存中，简化格式转换逻辑；QXmlStreamWriter 写入时注意设置 `setAutoFormatting(true)` 让输出更易读；处理嵌套结构时可能需要递归函数，分别处理 JSON 对象和 XML 元素；错误处理要完善，包括文件不存在、格式错误、类型不匹配等情况。

## 6. 官方文档参考链接

[QJsonDocument Class | Qt Core 6.10.2](https://doc.qt.io/qt-6/qjsondocument.html) -- QJsonDocument 完整 API 参考，用于读写完整的 JSON 文档

[QJsonObject Class | Qt Core 6.10.2](https://doc.qt.io/qt-6/qjsonobject.html) -- JSON 对象类，演示如何操作键值对集合

[QJsonArray Class | Qt Core 6.10.2](https://doc.qt.io/qt-6/qjsonarray.html) -- JSON 数组类，讲解如何操作有序值列表

[Qt Serialization | Qt Core 6.10.2](https://doc.qt.io/qt-6/qtserialization.html) -- Qt 序列化概述，介绍 JSON 和 XML 在数据交换中的作用

[QXmlStreamReader Class | Qt Core 6.10.2](https://doc.qt.io/qt-6/qxmlstreamreader.html) -- 流式 XML 解析器，适合处理大型 XML 文件

[QXmlStreamWriter Class | Qt Core 6.10.2](https://doc.qt.io/qt-6/qxmlstreamwriter.html) -- 流式 XML 写入器，用于生成格式正确的 XML 文档

[QXmlStreamAttributes Class | Qt Core 6.10.2](https://doc.qt.io/qt-6/qxmlstreamattributes.html) -- XML 属性处理类，讲解如何访问元素属性
