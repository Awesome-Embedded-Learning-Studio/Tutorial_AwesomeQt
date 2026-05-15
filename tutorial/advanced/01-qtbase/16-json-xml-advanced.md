---
title: "1.16 JSON/XML 进阶：流式处理大文件"
description: "入门篇我们搞定了 QJsonDocument 的基本解析和构建，也用了 QXmlStreamReader 读过简单的 XML 文件，当时一切都岁月静好。"
---

# 现代Qt开发教程（进阶篇）1.16——JSON/XML 进阶：流式处理大文件

## 1. 前言 / 数据量大到入门方案撑不住

入门篇我们搞定了 QJsonDocument 的基本解析和构建，也用了 QXmlStreamReader 读过简单的 XML 文件，当时一切都岁月静好。说实话，等到你真正在项目里碰上一个 200MB 的 JSON 日志文件或者一个嵌套了七八层的 XML 配置树的时候，那些入门级的代码就不够看了——内存直接飙到好几个 GB 解析一个文件，或者手写的拼接 JSON 字符串里到处都是转义错误，又或者 XML 命名空间搞混了导致整个解析逻辑全盘崩溃。

我之前接手过一个数据采集系统，每天要解析上百个 50MB 以上的 JSON 文件，最开始用 QJsonDocument::fromJson 一把梭，结果内存占用直接翻了十倍，服务器 OOM 杀进程杀到让人怀疑人生。后来切换到流式生成和分块处理后，内存占用降了一个数量级。这篇我们要解决的就是这些「数据量大到入门方案撑不住」的问题。

## 2. 环境说明

本篇基于 Qt 6.5+，CMake 3.26+，C++17 标准。所有 JSON、XML、CBOR 相关类都属于 QtCore 模块，无需额外链接。CBOR 相关类（QCborValue、QCborMap、QCborArray）从 Qt 5.12 引入，Qt 6 中已稳定。代码跨平台通用，不依赖 GUI。

## 3. 核心概念讲解

### 3.1 QJsonDocument 的内存开销——为什么大文件会炸

我们先来直面一个残酷的事实：QJsonDocument::fromJson() 在解析 JSON 的时候，需要把整个 QByteArray 全部加载到内存里，然后构建出一棵完整的 JSON DOM 树。这棵树里的每个 QJsonObject、QJsonArray、QJsonValue 都是一个独立的堆对象。一个 100MB 的 JSON 文件，解析后内存占用轻松达到 300MB 甚至更高——字符串数据会被复制一份存到 QJsonValue 里，数字和布尔值虽然不大但每个都需要对象头管理，再加上 Qt 隐式共享引用计数的额外开销。

更致命的是 readAll() 本身就会分配一整块和文件等大的 QByteArray，再加上 QJsonDocument 解析时构建的 DOM 树，同一份数据在内存里至少存在两份完整拷贝。峰值内存可能是原始文件大小的四五倍。

对于 JSON，Qt 目前没有提供流式解析器。大文件通常要走第三方流式 JSON 库（如 RapidJSON 的 SAX 模式）或者自己拆成小文件。但 JSON 的写入端我们可以做流式优化——不构建完整的 QJsonDocument，而是手动拼接 JSON 字符串直接写入文件。

### 3.2 流式 JSON 生成——不构建 DOM 树的高性能输出

当你需要生成一个大型 JSON 文件时（比如导出数据库查询结果），不要用 QJsonObject 逐层构建再 QJsonDocument::toJson()。那样需要把整个 JSON 结构都驻留在内存中。正确做法是直接用 QTextStream 按格式写入文件。

```cpp
QFile file("export.json");
file.open(QIODevice::WriteOnly);
QTextStream stream(&file);

stream << "[\n";
bool first = true;
for (const auto& record : records) {
    if (!first) stream << ",\n";
    first = false;

    stream << "  {\"id\": " << record.id
           << ", \"name\": " << toJsonString(record.name)
           << ", \"value\": " << record.value
           << "}";
}
stream << "\n]\n";
```

这里的 `toJsonString()` 是一个辅助函数，负责处理字符串转义（双引号、反斜杠、换行符等）。QJsonValue 有一个 `QJsonValue(QString).toVariant().toString()` 的路径可以用，但更轻量的做法是自己写一个转义函数。关键是确保所有特殊字符都被正确转义，否则生成的 JSON 是无效的。

这种流式方法的优势是内存占用恒定——不需要把所有记录都加载到 JSON DOM 树中，写一条释放一条。

### 3.3 QXmlStreamWriter 与 QXmlStreamReader 的完整用法

Qt 对 XML 的流式支持比 JSON 好得多。QXmlStreamReader 和 QXmlStreamWriter 是 Qt 推荐的 XML 处理方式（不建议使用 SAX 和 DOM 模式），它们的内存占用是恒定的，不受文件大小影响。

QXmlStreamWriter 的用法比较直观，它是一个状态机：你按顺序调用 writeStartElement、writeAttribute、writeCharacters、writeEndElement 来生成 XML 结构。它会自动处理命名空间声明和结束标签匹配。

```cpp
QFile file("config.xml");
file.open(QIODevice::WriteOnly);
QXmlStreamWriter writer(&file);
writer.setAutoFormatting(true);  // 自动缩进

writer.writeStartDocument();
writer.writeStartElement("configuration");

writer.writeStartElement("database");
writer.writeAttribute("type", "postgresql");
writer.writeTextElement("host", "localhost");
writer.writeTextElement("port", "5432");
writer.writeEndElement();  // database

writer.writeEndElement();  // configuration
writer.writeEndDocument();
```

QXmlStreamReader 的读取端是一个基于 token 的拉取式解析器。你反复调用 readNext() 获取下一个 token，然后通过 tokenType() 判断当前 token 的类型（StartElement、Characters、EndElement 等）。

### 3.4 XML 命名空间——工程中的真实坑

XML 命名空间在实际项目中经常出现，特别是在处理第三方 XML 配置文件或 SOAP 消息时。命名空间用 `xmlns` 声明，格式是 `xmlns:prefix="URI"`。带前缀的元素名是 `prefix:localName`。

QXmlStreamReader 对命名空间的处理方式是：对于带前缀的元素，`name()` 返回 localName 部分，`prefix()` 返回前缀，`namespaceUri()` 返回完整的命名空间 URI。如果你在代码里用 `name() == "element"` 来匹配元素名，当元素有前缀时（比如 `ns:element`），`name()` 返回的是 `element`，匹配成功——这看起来没问题，但如果两个不同的命名空间都有 `element` 这个 localName，你就可能匹配到错误的元素。

解决方案是用 `qualifiedName()` 做完整匹配，或者用 `namespaceUri() + name()` 的组合。对于 QXmlStreamWriter，使用 `writeStartElement(namespaceUri, localName)` 的三参数版本，让它自动处理前缀映射。

### 3.5 CBOR——比 JSON 更高效的二进制替代

Qt 6 提供了对 CBOR（Concise Binary Object Representation，RFC 8949）的完整支持。CBOR 是 JSON 的二进制等价物——数据模型完全兼容 JSON，但编码方式是二进制的，解析更快、体积更小。

QCborValue、QCborMap、QCborArray 的 API 和 QJsonValue、QJsonObject、QJsonArray 几乎一一对应，迁移成本很低。CBOR 的优势在于：不需要 UTF-8 到 UTF-16 的字符串转换、数字不需要从文本解析、二进制数据可以直接嵌入不需要 Base64 编码。对于 Qt 内部的数据交换（比如 QML 和 C++ 之间、进程间通信），CBOR 比 JSON 更高效。

现在有一道思考题。如果你需要在两个 Qt 程序之间传递结构化数据，一个是 JSON，一个是 CBOR，你会选哪个？考虑因素是什么？

答案取决于场景：如果数据需要被非 Qt 程序读取或者需要人类可读，选 JSON。如果是 Qt 内部通信且性能敏感，选 CBOR。CBOR 的解析速度通常比 JSON 快 3-5 倍，体积小 20-30%。

## 4. 踩坑预防

第一个坑是 JSON 字符串拼接中的转义遗漏。手动拼接 JSON 字符串时，如果值中包含双引号、反斜杠、换行符或控制字符，必须进行转义。否则生成的 JSON 是无效的，QJsonDocument::fromJson() 会解析失败。后果是导出的数据文件无法被任何 JSON 工具读取。解决方案是写一个专门的转义函数，或者用 QJsonValue::fromVariant() 来生成安全的 JSON 字符串表示。

第二个坑是 QXmlStreamReader 的错误恢复。当 XML 文件格式错误时，readNext() 会返回 Invalid 类型的 token，错误信息可以通过 error() 和 errorString() 获取。但很多开发者忽略了错误检查，继续处理后续 token——结果就是在错误状态下读取到垃圾数据。后果是解析结果不正确但程序没有崩溃，很难发现问题。解决方案是每次 readNext() 后都检查 hasError()，发现错误立即停止解析并报告。

第三个坑是 QXmlStreamWriter 的编码问题。默认情况下 QXmlStreamWriter 使用 UTF-8 编码，但如果你构造的字符串包含非 UTF-8 编码的数据（比如从 Latin-1 源读取的文本），写入的 XML 文件中可能包含非法的 UTF-8 序列。后果是其他 XML 解析器无法读取这个文件。解决方案是在写入前确保所有字符串都是正确的 UTF-8 编码，使用 QString::fromUtf8() 或 QStringConverter 进行显式转换。

## 5. 练习项目

练习项目：大文件日志分析器。我们要实现一个流式的 JSON 日志分析工具，能够处理 GB 级别的日志文件而内存占用保持恒定。

具体要求是：LogAnalyzer 类接受 JSON Lines 格式的日志文件（每行一个 JSON 对象），逐行读取并解析，不需要把整个文件加载到内存。支持过滤条件（级别、时间范围、关键字），匹配的日志行提取关键字段输出到报告文件。完成标准是处理 1GB 日志文件时内存占用不超过 50MB、过滤结果正确、处理速度不低于 100MB/s。

提示几个关键点：用 QFile 逐行读取（QFile::readLine）避免 readAll，每行用 QJsonDocument::fromJson() 独立解析，过滤条件用 QJsonObject 的 value() 方法检查字段值，输出文件用 QTextStream 流式写入。

## 6. 官方文档参考链接

[Qt 文档 · QJsonDocument](https://doc.qt.io/qt-6/qjsondocument.html) -- JSON 文档解析与生成

[Qt 文档 · QXmlStreamReader](https://doc.qt.io/qt-6/qxmlstreamreader.html) -- XML 流式读取器

[Qt 文档 · QXmlStreamWriter](https://doc.qt.io/qt-6/qxmlstreamwriter.html) -- XML 流式写入器

[Qt 文档 · QCborValue](https://doc.qt.io/qt-6/qcborvalue.html) -- CBOR 二进制数据格式

---

到这里，JSON 和 XML 的进阶处理就全部拆完了。流式生成避免 DOM 树内存爆炸、XML 命名空间的正确处理、CBOR 作为 JSON 的高效替代——这些知识在处理工程级数据文件时不可或缺。
