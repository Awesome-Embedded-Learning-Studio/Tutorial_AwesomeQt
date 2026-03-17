/*
 *  Qt 6 入门教程 - 示例 03
 *  主题：QString、QByteArray、QStringView 与编码转换
 *
 * 本示例演示：
 * 1. QString 与 QByteArray 的基本用法
 * 2. UTF-8 编码转换（fromUtf8 / toUtf8）
 * 3. GBK 编码转换（QStringDecoder / QStringEncoder）
 * 4. QStringView 零拷贝视图
 * 5. 常用字符串操作：split、contains、replace、arg
 */

#include <QCoreApplication>
#include <QByteArray>
#include <QString>
#include <QStringView>
#include <QStringList>
#include <QStringDecoder>
#include <QStringEncoder>
#include <QDebug>

void demoQStringVsByteArray()
{
    qDebug() << "===== 1. QString vs QByteArray =====";

    // QString 存储文本，内部是 UTF-16
    QString text = "你好世界";  // 中文字符串字面量
    qDebug() << "QString length:" << text.length();  // 4，字符数
    qDebug() << "QString data:" << text;

    // QByteArray 存储原始字节
    QByteArray utf8Bytes = text.toUtf8();  // 转换为 UTF-8 字节序列
    qDebug() << "UTF-8 bytes length:" << utf8Bytes.length();  // 12，每个汉字 3 字节
    qDebug() << "UTF-8 bytes:" << utf8Bytes.toHex(' ');  // 十六进制显示

    // 从 UTF-8 字节构造 QString
    QString fromUtf8 = QString::fromUtf8(utf8Bytes);
    qDebug() << "From UTF-8:" << fromUtf8;

    qDebug() << "";
}

void demoStringView()
{
    qDebug() << "===== 2. QStringView 零拷贝演示 =====";

    QString original = "Hello, Qt 6!";

    // QStringView 是只读视图，不复制数据
    QStringView view(original);
    qDebug() << "Original string:" << original;
    qDebug() << "View content:" << view.toString();
    qDebug() << "View size:" << view.size();

    // 从字符串字面量构造视图
    QStringView literalView(u"String literal");
    qDebug() << "Literal view:" << literalView.toString();

    // 截取子串视图（零拷贝）
    QStringView subView = original.mid(7, 3);  // "Qt"
    qDebug() << "Substring view:" << subView.toString();

    qDebug() << "";
}

void demoStringOperations()
{
    qDebug() << "===== 3. 常用字符串操作 =====";

    QString csv = "apple,banana,cherry,date";

    // split 分割字符串
    QStringList fruits = csv.split(',');
    qDebug() << "Split result:" << fruits;

    // contains 查找
    bool hasCherry = csv.contains("cherry");
    qDebug() << "Contains 'cherry':" << hasCherry;

    // indexOf 获取位置
    int pos = csv.indexOf("banana");
    qDebug() << "Position of 'banana':" << pos;

    // replace 替换
    QString replaced = csv;
    replaced.replace("banana", "blueberry");
    qDebug() << "After replace:" << replaced;

    // arg 格式化（类似 printf）
    QString templateStr = "用户 %1 登录成功，状态码 %2";
    QString formatted = templateStr.arg("张三").arg(200);
    qDebug() << "Formatted string:" << formatted;

    // 数字转换
    QString numStr = QString::number(3.14159, 'f', 2);  // 保留 2 位小数
    qDebug() << "Number to string:" << numStr;

    double value = numStr.toDouble();
    qDebug() << "String to number:" << value;

    qDebug() << "";
}

void demoEncodingConversion()
{
    qDebug() << "===== 4. 编码转换演示 =====";

    // UTF-8 转换（最常用）
    QString original = "中文测试 Hello World";
    QByteArray utf8Data = original.toUtf8();
    QString backFromUtf8 = QString::fromUtf8(utf8Data);
    qDebug() << "UTF-8 round-trip:" << backFromUtf8;

    // QStringDecoder/QStringEncoder（Qt 6 新方式）
    // 注意：GBK 编码可能不可用，取决于系统配置
    QStringDecoder decoder("GBK");
    QStringEncoder encoder("GBK");

    if (decoder.isValid()) {
        qDebug() << "GBK encoding is supported";

        // GBK 转换示例（假设我们有 GBK 编码的字节）
        // 这里用 UTF-8 模拟，实际使用时替换为真实的 GBK 字节
        QByteArray gbkSimulated = "\xd6\xd0\xce\xc4";  // "中文" 的 GBK 编码（示意）
        QString gbkText = decoder(gbkSimulated);
        qDebug() << "GBK decoded:" << gbkText;
    } else {
        qDebug() << "GBK encoding not supported on this system";
    }

    // Latin1 转换（总是可用）
    QString latin1Str = QString::fromLatin1("Hello");
    qDebug() << "From Latin1:" << latin1Str;

    qDebug() << "";
}

void demoStringLiteral()
{
    qDebug() << "===== 5. QStringLiteral 编译期优化 =====";

    // 普通构造（运行时）
    QString runtimeStr = QString("Runtime string");

    // QStringLiteral（编译期生成 UTF-16）
    QString compiletimeStr = QStringLiteral("Compile-time string");

    qDebug() << "Runtime:" << runtimeStr;
    qDebug() << "Compile-time:" << compiletimeStr;

    qDebug() << "";
}

void demoCommonPitfalls()
{
    qDebug() << "===== 6. 常见坑点演示 =====";

    // 坑 1： QByteArray 直接构造 QString 会当作 Latin1
    QByteArray utf8Text = "你好";  // UTF-8 编码的字节
    QString wrongWay(utf8Text);    // 错误：当作 Latin1 解析
    QString rightWay = QString::fromUtf8(utf8Text);  // 正确：按 UTF-8 解析

    qDebug() << "Wrong way (Latin1):" << wrongWay;  // 乱码
    qDebug() << "Right way (UTF-8):" << rightWay;   // 正确

    // 坑 2：QString::number 默认精度
    double pi = 3.1415926535;
    QString lowPrecision = QString::number(pi);      // 默认 6 位
    QString highPrecision = QString::number(pi, 'f', 10);  // 指定 10 位
    qDebug() << "Default precision:" << lowPrecision;
    qDebug() << "High precision:" << highPrecision;

    qDebug() << "";
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // 设置编码用于 qDebug 输出（控制台可能需要）
    // 注意：这在某些平台可能不生效，取决于控制台编码
    qDebug() << "Qt 6 String & Encoding Demo";
    qDebug() << "Qt version:" << QT_VERSION_STR;
    qDebug() << "";

    demoQStringVsByteArray();
    demoStringView();
    demoStringOperations();
    demoEncodingConversion();
    demoStringLiteral();
    demoCommonPitfalls();

    qDebug() << "===== Demo completed =====";
    return 0;
}
