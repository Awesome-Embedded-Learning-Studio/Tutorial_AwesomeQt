/// @file    encoding_demo.cpp
/// @brief   EncodingDemo 类的实现——编码转换与隐式构造开销演示。
///
/// 对应教程：进阶层 01-QtBase/03-QString 进阶。

#include "encoding_demo.h"

#include <QByteArray>
#include <QDebug>
#include <QString>
#include <QStringList>

// ============================================================================
// EncodingDemo — 编码转换演示
//
// QString 内部存储 UTF-16 编码。当你从 const char* 或 QByteArray 构造
// QString 时，Qt 必须做编码转换——这个转换的开销在循环中会被放大数万倍。
// ============================================================================

void EncodingDemo::runAll()
{
    demoEncodingConversions();
    demoImplicitConversionOverhead();
    demoLatin1VsUtf8Pitfall();
}

void EncodingDemo::demoEncodingConversions()
{
    qDebug() << "=== 编码转换对比 ===";

    // fromUtf8: 将 UTF-8 编码的 QByteArray 转为 QString
    // UTF-8 是现代 Linux/macOS 的默认编码，也是 Web 的标准编码
    QByteArray utf8Data = "你好，世界";  // 源文件保存为 UTF-8 时
    QString fromUtf8Str = QString::fromUtf8(utf8Data);
    qDebug() << "fromUtf8:    " << fromUtf8Str;

    // fromLatin1: 将 Latin-1 (ISO 8859-1) 编码转为 QString
    // Latin-1 是 ASCII 的超集，每个字节直接映射到一个 Unicode 码位
    // 对于纯 ASCII 字符串，fromLatin1 和 fromUtf8 结果相同
    QByteArray latin1Data = "Hello World";
    QString fromLatin1Str = QString::fromLatin1(latin1Data);
    qDebug() << "fromLatin1:  " << fromLatin1Str;

    // fromUtf16: 从 UTF-16 编码数据构造 QString
    // 这是"零转换"操作——因为 QString 内部就是 UTF-16
    // 注意：char16_t 数组在内存中就是 UTF-16 编码
    const char16_t utf16Data[] = u"UTF-16 string";
    QString fromUtf16Str = QString::fromUtf16(utf16Data);
    qDebug() << "fromUtf16:   " << fromUtf16Str;

    qDebug() << "";
}

void EncodingDemo::demoImplicitConversionOverhead()
{
    qDebug() << "=== 隐式转换的 QByteArray 中间对象 ===";

    // 方式 1: 直接用字符串字面量——每次都创建临时 QString
    // "prefix_" 是 const char*，隐式转为临时 QString
    QString s1 = "prefix_" + QString("value");
    qDebug() << "隐式转换拼接:" << s1;

    // 方式 2: 用 QStringLiteral 避免运行时分配
    // QStringLiteral 在编译期就生成 UTF-16 数据，运行时零分配
    QString s2 = QStringLiteral("prefix_") + QStringLiteral("value");
    qDebug() << "QStringLiteral 拼接:" << s2;

    // 方式 3: 用 QLatin1String 做比较——零分配
    // QLatin1String 只是指向原始数据的轻量包装，不分配内存
    QString className = QStringLiteral("QWidget");
    bool match = (className == QLatin1String("QWidget"));
    qDebug() << "QLatin1String 比较:" << match
             << "(零堆分配，直接逐字符比较)";

    qDebug() << "";
}

void EncodingDemo::demoLatin1VsUtf8Pitfall()
{
    qDebug() << "=== Latin-1 vs UTF-8 陷阱 ===";

    // UTF-8 中文字符占 3 个字节
    QByteArray utf8Chinese = "中文";
    qDebug() << "UTF-8 \"中文\" 字节数:" << utf8Chinese.size();

    // 用 fromUtf8 正确解码
    QString correct = QString::fromUtf8(utf8Chinese);
    qDebug() << "fromUtf8 解码:" << correct << ", 字符数:" << correct.length();

    // 用 fromLatin1 错误解码：每个字节被当作独立的 Latin-1 字符
    // 这会产生乱码，因为 UTF-8 的多字节序列被误读
    QString wrong = QString::fromLatin1(utf8Chinese);
    qDebug() << "fromLatin1 误用（乱码）:" << wrong
             << ", 字符数:" << wrong.length();

    qDebug() << "教训: 源文件编码为 UTF-8 时，必须用 fromUtf8 而非 fromLatin1";
    qDebug() << "";
}
