/// @file    xml_stream_demo.cpp
/// @brief   XmlStreamDemo 类的实现：XML 流式读写高级用法演示。
///
/// 对应教程：进阶层 01-QtBase/16-JSON 与 XML 解析。

#include "xml_stream_demo.h"

#include <QStringList>

QByteArray XmlStreamDemo::demoXmlWriting()
{
    qDebug() << "  [QXmlStreamWriter]" << QString::fromUtf8("生成格式化的 XML 文档");
    qDebug() << "  " << QString(46, '-');

    // 使用 QByteArray + QBuffer 作为输出目标（内存中）
    QByteArray outputBuffer;
    QBuffer buffer(&outputBuffer);
    buffer.open(QIODevice::WriteOnly);

    QXmlStreamWriter writer(&buffer);

    // 配置写入器
    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(4);

    // 写入 XML 声明：<?xml version="1.0" encoding="UTF-8"?>
    writer.writeStartDocument("1.0");

    writer.writeComment(QString::fromUtf8(" 示例：图书目录 XML 文档 "));

    // 根元素（带命名空间）
    writer.writeNamespace("http://example.com/books", "bk");
    writer.writeStartElement("http://example.com/books", "catalog");

    writer.writeAttribute("version", "1.0");
    writer.writeAttribute("language", "zh-CN");

    // 第一本书
    writer.writeStartElement("book");
    writer.writeAttribute("id", "001");
    writer.writeAttribute("category", "programming");

    writer.writeTextElement("title", QString::fromUtf8("Qt6 C++ 开发指南"));
    writer.writeTextElement("author", QString::fromUtf8("张三"));

    // writeTextElement 自动转义特殊字符（<, >, & 等）
    writer.writeTextElement(
        "description",
        QString::fromUtf8("学习 Qt6 与 C++ 的最佳实践，包括 <信号与槽>、&事件系统& 等"));

    writer.writeTextElement("price", "89.90");
    writer.writeTextElement("isbn", "978-7-123456-78-9");

    // 嵌套元素
    writer.writeStartElement("tags");
    writer.writeTextElement("tag", "Qt");
    writer.writeTextElement("tag", "C++");
    writer.writeTextElement("tag", "GUI");
    writer.writeEndElement(); // </tags>

    writer.writeEndElement(); // </book>

    // 第二本书
    writer.writeStartElement("book");
    writer.writeAttribute("id", "002");
    writer.writeAttribute("category", "embedded");

    writer.writeTextElement("title", QString::fromUtf8("嵌入式 Linux 系统编程"));
    writer.writeTextElement("author", QString::fromUtf8("李四"));
    writer.writeTextElement(
        "description",
        QString::fromUtf8("深入理解嵌入式 Linux 系统的 \"底层\" 开发"));
    writer.writeTextElement("price", "79.50");

    writer.writeEndElement(); // </book>

    writer.writeEndElement(); // </catalog>
    writer.writeEndDocument();

    buffer.close();

    // 输出生成的 XML
    QString xmlString = QString::fromUtf8(outputBuffer);
    qDebug() << QString::fromUtf8("  生成的 XML 文档:");
    for (const auto& line : xmlString.split('\n')) {
        if (!line.trimmed().isEmpty()) {
            qDebug() << "   " << line;
        }
    }

    return outputBuffer;
}

void XmlStreamDemo::demoXmlParsing(const QByteArray& xmlData)
{
    qDebug() << "";
    qDebug() << "  [QXmlStreamReader]" << QString::fromUtf8("状态机方式解析 XML");
    qDebug() << "  " << QString(46, '-');

    QXmlStreamReader reader(xmlData);

    int indent = 0;
    int bookCount = 0;

    // 主循环：readNext() 推进到下一个 token
    while (!reader.atEnd()) {
        reader.readNext();

        switch (reader.tokenType()) {
        case QXmlStreamReader::StartDocument:
            qDebug() << "    [StartDocument]" << QString::fromUtf8("版本:")
                     << reader.documentVersion()
                     << QString::fromUtf8("编码:") << reader.documentEncoding();
            break;

        case QXmlStreamReader::Comment:
            qDebug() << "    [Comment]" << reader.text().toString().trimmed();
            break;

        case QXmlStreamReader::StartElement: {
            QString indentStr(indent * 2, ' ');
            QString nsUri = reader.namespaceUri().toString();
            QString elemInfo = reader.name().toString();
            if (!nsUri.isEmpty()) {
                elemInfo += QString(" [ns: %1]").arg(nsUri);
            }

            QXmlStreamAttributes attrs = reader.attributes();
            if (!attrs.isEmpty()) {
                QStringList attrList;
                for (const auto& attr : attrs) {
                    attrList << QString("%1=\"%2\"").arg(
                        attr.name().toString(), attr.value().toString());
                }
                elemInfo += " " + attrList.join(" ");
            }

            qDebug() << "    " << indentStr << "<" << elemInfo << ">";
            indent++;
            break;
        }

        case QXmlStreamReader::EndElement:
            indent--;
            if (reader.name() == QString::fromUtf8("book")) {
                bookCount++;
            }
            break;

        case QXmlStreamReader::Characters:
            // 只显示非空白的文本内容
            if (!reader.isWhitespace()) {
                QString indentStr(indent * 2, ' ');
                qDebug() << "    " << indentStr
                         << QString::fromUtf8("  文本:") << reader.text().toString();
            }
            break;

        default:
            break;
        }
    }

    // 解析完成后检查是否有错误
    if (reader.hasError()) {
        qDebug() << "    " << QString::fromUtf8("[错误] XML 解析失败:")
                 << reader.errorString();
        qDebug() << "    " << QString::fromUtf8("[错误] 行号:") << reader.lineNumber()
                 << QString::fromUtf8("列号:") << reader.columnNumber();
    } else {
        qDebug() << "";
        qDebug() << QString::fromUtf8("    解析完成: 共") << bookCount
                 << QString::fromUtf8("本书");
    }
}

void XmlStreamDemo::demoXmlErrorHandling()
{
    qDebug() << "";
    qDebug() << "  [XML" << QString::fromUtf8("错误处理] 检测格式错误的 XML");
    qDebug() << "  " << QString(46, '-');

    // 错误 1：标签未闭合
    QString malformedXml1 = "<root><element>" + QString::fromUtf8("未闭合的内容") + "</root>";
    QXmlStreamReader reader1(malformedXml1);
    while (!reader1.atEnd()) {
        reader1.readNext();
    }
    if (reader1.hasError()) {
        qDebug() << QString::fromUtf8("    错误 1:") << reader1.errorString();
        qDebug() << QString::fromUtf8("    位置: 行") << reader1.lineNumber()
                 << QString::fromUtf8("列") << reader1.columnNumber();
    }

    // 错误 2：属性值缺少引号
    QString malformedXml2 = "<root attr=value>content</root>";
    QXmlStreamReader reader2(malformedXml2);
    while (!reader2.atEnd()) {
        reader2.readNext();
    }
    if (reader2.hasError()) {
        qDebug() << QString::fromUtf8("    错误 2:") << reader2.errorString();
    }

    // 错误 3：空文档
    QString emptyXml = "";
    QXmlStreamReader reader3(emptyXml);
    while (!reader3.atEnd()) {
        reader3.readNext();
    }
    if (reader3.hasError()) {
        qDebug() << QString::fromUtf8("    错误 3:") << reader3.errorString();
    } else {
        qDebug() << QString::fromUtf8("    空文档: 无错误但无内容");
    }
}

void XmlStreamDemo::runAll()
{
    QByteArray xmlData = demoXmlWriting();
    demoXmlParsing(xmlData);
    demoXmlErrorHandling();
}
