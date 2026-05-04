#include "migrationutils.h"

#include <QDebug>
#include <QString>
#include <QStringList>

// ========================================
// 旧 API 头文件（Core5Compat，迁移后删除）
// ========================================
#include <QRegExp>
#include <QTextCodec>

// ========================================
// 新 API 头文件（Qt 6 原生，迁移后保留）
// ========================================
#include <QRegularExpression>
#include <QStringConverter>

void demoRegexMigration()
{
    qDebug() << "=== 正则表达式迁移对照 ===";
    qDebug() << "";

    QString input
        = QStringLiteral("Qt version 6.9.1 released on 2025-04-22");

    // --- 旧写法：QRegExp ---
    qDebug() << "[旧 API] QRegExp:";
    QRegExp oldRe(QStringLiteral("(\\d+)\\.(\\d+)\\.(\\d+)"));
    if (oldRe.indexIn(input) != -1) {
        qDebug() << "  完整匹配:" << oldRe.cap(0);
        qDebug() << "  主版本号:" << oldRe.cap(1);
        qDebug() << "  次版本号:" << oldRe.cap(2);
        qDebug() << "  修订号:" << oldRe.cap(3);
        qDebug() << "  匹配位置:" << oldRe.pos(0);
    }

    qDebug() << "";

    // --- 新写法：QRegularExpression ---
    qDebug() << "[新 API] QRegularExpression:";
    QRegularExpression newRe(QStringLiteral("(\\d+)\\.(\\d+)\\.(\\d+)"));
    QRegularExpressionMatch match = newRe.match(input);
    if (match.hasMatch()) {
        qDebug() << "  完整匹配:" << match.captured(0);
        qDebug() << "  主版本号:" << match.captured(1);
        qDebug() << "  次版本号:" << match.captured(2);
        qDebug() << "  修订号:" << match.captured(3);
        qDebug() << "  匹配位置:" << match.capturedStart(0);
    }

    qDebug() << "";
}

void demoRegexGlobalMatchMigration()
{
    qDebug() << "=== 全局匹配迁移对照 ===";
    qDebug() << "";

    QString input = QStringLiteral("apple banana cherry date");

    // --- 旧写法：QRegExp 循环 indexIn ---
    qDebug() << "[旧 API] QRegExp globalMatch:";
    QRegExp oldRe(QStringLiteral("\\b\\w+\\b"));
    int pos = 0;
    QStringList oldWords;
    while ((pos = oldRe.indexIn(input, pos)) != -1) {
        oldWords << oldRe.cap(0);
        pos += oldRe.matchedLength();
    }
    qDebug() << "  匹配结果:" << oldWords;

    qDebug() << "";

    // --- 新写法：QRegularExpression::globalMatch ---
    qDebug() << "[新 API] QRegularExpression::globalMatch:";
    QRegularExpression newRe(QStringLiteral("\\b\\w+\\b"));
    QStringList newWords;
    QRegularExpressionMatchIterator it = newRe.globalMatch(input);
    while (it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        newWords << m.captured(0);
    }
    qDebug() << "  匹配结果:" << newWords;

    qDebug() << "";
}

void demoCodecMigration()
{
    qDebug() << "=== 编码转换迁移对照 ===";
    qDebug() << "";

    QString text = QStringLiteral("你好世界 Hello World");

    // --- 旧写法：QTextCodec ---
    qDebug() << "[旧 API] QTextCodec:";
    QTextCodec* utf8Codec = QTextCodec::codecForName("UTF-8");
    QTextCodec* latin1Codec = QTextCodec::codecForName("ISO-8859-1");

    QByteArray utf8Data = utf8Codec->fromUnicode(text);
    qDebug() << "  UTF-8 编码:" << utf8Data.toHex(' ');
    qDebug() << "  UTF-8 长度:" << utf8Data.size() << "bytes";

    // Latin-1 编码会丢失非 ASCII 字符
    QByteArray latin1Data = latin1Codec->fromUnicode(text);
    qDebug() << "  Latin-1 编码:" << latin1Data.toHex(' ');

    // 解码回 QString
    QString decoded = utf8Codec->toUnicode(utf8Data);
    qDebug() << "  解码回 UTF-8:" << decoded;

    qDebug() << "";

    // --- 新写法：QStringConverter ---
    qDebug() << "[新 API] QStringConverter:";
    QStringEncoder utf8Encoder(QStringConverter::Utf8);
    QByteArray utf8DataNew = utf8Encoder.encode(text);
    qDebug() << "  UTF-8 编码:" << utf8DataNew.toHex(' ');
    qDebug() << "  UTF-8 长度:" << utf8DataNew.size() << "bytes";

    // 解码回 QString
    QStringDecoder utf8Decoder(QStringConverter::Utf8);
    QString decodedNew = utf8Decoder.decode(utf8DataNew);
    qDebug() << "  解码回 UTF-8:" << decodedNew;

    // 使用 encodingForName 查找编码（支持非 UTF 编码）
    auto gbkOpt = QStringConverter::encodingForName("GBK");
    if (gbkOpt) {
        QStringEncoder gbkEncoder(*gbkOpt);
        QByteArray gbkData = gbkEncoder.encode(text);
        qDebug() << "  GBK 编码:" << gbkData.toHex(' ');
        qDebug() << "  GBK 长度:" << gbkData.size() << "bytes";

        QStringDecoder gbkDecoder(*gbkOpt);
        QString gbkDecoded = gbkDecoder.decode(gbkData);
        qDebug() << "  GBK 解码:" << gbkDecoded;
    } else {
        qDebug() << "  GBK 编码不可用（平台不支持）";
    }

    qDebug() << "";
}

void demoMigrationStrategy()
{
    qDebug() << "=== 渐进式迁移策略 ===";
    qDebug() << "";
    qDebug() << "阶段 1：引入 Core5Compat，让 Qt 5 代码在 Qt 6 下编译通过";
    qDebug() << "  find_package(Qt6 ... Core5Compat)";
    qDebug() << "  target_link_libraries(... Qt6::Core5Compat)";
    qDebug() << "  不修改任何业务代码";
    qDebug() << "";
    qDebug() << "阶段 2：逐文件替换旧 API";
    qDebug() << "  #include <QRegExp>  →  #include <QRegularExpression>";
    qDebug() << "  QRegExp::cap(n)     →  QRegularExpressionMatch::captured(n)";
    qDebug() << "  QRegExp::indexIn()  →  QRegularExpression::match()";
    qDebug() << "";
    qDebug() << "  #include <QTextCodec>    →  #include <QStringConverter>";
    qDebug() << "  QTextCodec::fromUnicode() →  QStringEncoder::encode()";
    qDebug() << "  QTextCodec::toUnicode()   →  QStringDecoder::decode()";
    qDebug() << "";
    qDebug() << "阶段 3：移除 Core5Compat 依赖";
    qDebug() << "  确认所有文件不再 include Core5Compat 头文件";
    qDebug() << "  从 CMake 中删除 Qt6::Core5Compat";
    qDebug() << "";
}
