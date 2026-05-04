# 现代Qt开发教程（新手篇）5.25--Qt5Compat 兼容层与迁移指南

## 1. 前言：Qt 6 不向下兼容，但给了一条活路

Qt 6 在 2020 年发布的时候做了一个非常大胆的决定：大量 Qt 5 时代的 API 被直接移除，不是标记为 deprecated 然后继续可用，而是从头文件里彻底删除了。这在 Qt 的历史上是比较罕见的——从 Qt 4 到 Qt 5 虽然也有 breaking changes，但大部分代码做少量修改就能编译通过。Qt 6 不一样，如果你有一个几万行的 Qt 5 项目直接拿到 Qt 6 环境下编译，大概率会遇到一堆"类未定义""头文件不存在""函数签名不匹配"之类的编译错误，每一处都需要手动修复。

被移除的 API 有几类。第一类是命名和行为不够规范的旧接口被全新的类替代了，比如 QRegExp 被 QRegularExpression 替代，QTextCodec 被 QStringConverter 替代。第二类是历史遗留的数据结构被清理了，比如 QLinkedList 在 Qt 6 中被移除（直接用 std::list）。第三类是一些模块级别的重组，比如 QtXmlPatterns 被整个砍掉，QtScript 被移除，QTextCodec 从 Core 模块中移除。

Qt5Compat 模块就是为了缓解这种迁移痛苦而存在的。它把一部分被 Qt 6 移除的旧 API 重新打包到一个独立的兼容模块中——你只需要在 CMake 中链接 Qt5Compat，就可以继续使用 QRegExp、QTextCodec、QLinkedList 等旧类。这样你的项目可以先在 Qt 6 下编译通过、运行起来，然后再逐步把旧 API 替换成新 API，而不是一次性把所有代码都改完才能看到程序跑起来。

这篇我们要做的是了解 Qt5Compat 包含哪些已废弃 API，把 QRegExp 迁移到 QRegularExpression，把 QTextCodec 迁移到 QStringConverter，以及制定一套渐进式迁移策略。

## 2. 环境说明

本篇基于 Qt 6.9.1，需要引入 Core5Compat 模块（注意模块名是 Core5Compat 而不是 Qt5Compat）。CMake 配置如下：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core Core5Compat)
```

Core5Compat 模块从 Qt 6.0 开始提供，它包含了 Qt 5 Core 模块中被移除但仍有广泛使用的 API。这个模块在 Qt 6 的整个生命周期中都会保持可用，但 Qt 官方的态度很明确：Core5Compat 是过渡方案，新项目不应该依赖它，旧项目应该尽快完成迁移。

工具链方面没有特殊要求：MSVC 2019+、GCC 11+、Clang 14+，C++17 标准，CMake 3.26+。Core5Compat 是纯头文件 + 少量源码的模块，不需要额外的系统依赖。

## 3. 核心概念讲解

### 3.1 Qt5Compat 模块包含哪些已废弃 API

Core5Compat 模块中保留的旧 API 主要包括以下几个大类。正则表达式方面，QRegExp 类完整保留在 Core5Compat 中——它的 API 设计存在不少历史包袱，比如最小匹配的语义和 Perl 正则不一致、不支持命名捕获组、不支持 lookahead/lookbehind 断言等。容器方面，QLinkedList<T> 作为 std::list<T> 的别名保留。编码处理方面，QTextCodec 及其子类（QFileEncodeDetector 等）完整保留。XML 相关方面，QXmlStreamReader/QXmlStreamWriter 的部分旧式 API 保留。

除了 Core5Compat，还有几个模块级别的兼容包值得了解。Qt5Compat 模块（注意和 Core5Compat 的区别）提供了一些 Qt 5 Quick 模块中被移除的 QML 类型和组件。Graphs 模块在 Qt 6.8 之后替代了旧版 QtCharts 的一部分功能，但 QtCharts 本身仍然可用。这些不在本篇讨论范围内，如果你有 Quick/QML 的迁移需求可以查阅对应模块的文档。

一个很关键的认识是：Core5Compat 中的类在 Qt 6 中只是"可以编译通过"，不代表行为完全一致。Qt 6 在底层做了很多重构，比如 QString 的内部编码从 Qt 5 的 UTF-16 改为 Qt 6 的 UTF-16 但存储方式有变化（QString 在 Qt 6 中支持在 UTF-16 和 Latin-1 之间自动选择内部表示以节省内存）。大多数情况下这些底层变化对上层 API 是透明的，但如果你有直接操作 QString 内部数据的代码（比如 data() 指针的直接操作），可能需要注意。

### 3.2 QRegExp 迁移到 QRegularExpression

QRegExp 到 QRegularExpression 的迁移是 Qt 5 到 Qt 6 升级中最常见的工作之一。QRegularExpression 在 Qt 5 时代就已经存在了（从 Qt 5.0 引入），所以如果你的项目已经在 Qt 5 中使用 QRegularExpression，迁移时不需要做任何改动。需要迁移的是那些还在用 QRegExp 的代码。

两者的 API 风格差异比较大。QRegExp 的用法是把正则表达式字符串和匹配选项都传给构造函数，然后调用 indexIn() 在目标字符串中搜索：

```cpp
// QRegExp 旧写法（需要 Core5Compat）
#include <QRegExp>

QRegExp re(QStringLiteral("(\\d+)\\.(\\d+)"));
if (re.indexIn(QStringLiteral("version 3.14 released")) != -1) {
    QString major = re.cap(1);  // "3"
    QString minor = re.cap(2);  // "14"
}
```

等价的 QRegularExpression 写法：

```cpp
// QRegularExpression 新写法（Qt 5.0+ 引入，Qt 6 推荐）
#include <QRegularExpression>

QRegularExpression re(QStringLiteral("(\\d+)\\.(\\d+)"));
QRegularExpressionMatch match
    = re.match(QStringLiteral("version 3.14 released"));
if (match.hasMatch()) {
    QString major = match.captured(1);  // "3"
    QString minor = match.captured(2);  // "14"
}
```

几个关键差异要搞清楚。QRegExp 的 cap(n) 对应 QRegularExpressionMatch 的 captured(n)——函数名变了，但语义相同：n=0 返回整个匹配，n>=1 返回第 n 个捕获组。QRegExp::indexIn() 返回匹配位置的索引，如果没匹配到返回 -1；QRegularExpression::match() 返回一个 QRegularExpressionMatch 对象，通过 hasMatch() 判断是否匹配成功。如果你需要匹配位置，用 match.capturedStart(n) 和 match.capturedEnd(n)。

正则语法本身也有一些差异。QRegExp 使用的是一种和 Perl 不完全兼容的正则方言——比如它的最小匹配（non-greedy）语义和标准正则不同，字符类 \w 的范围在不同模式下不一样。QRegularExpression 使用的是 PCRE（Perl Compatible Regular Expressions）语法，行为和 Perl/Python/JavaScript 的正则一致。如果你在迁移时发现某个正则表达式的行为变了，大概率是正则方言的差异导致的——需要对照文档逐个检查。

全局匹配（查找所有匹配）的迁移方式如下：

```cpp
// QRegExp 旧写法
QRegExp re(QStringLiteral("\\b\\w+\\b"));
int pos = 0;
while ((pos = re.indexIn(text, pos)) != -1) {
    qDebug() << re.cap(0);
    pos += re.matchedLength();
}

// QRegularExpression 新写法
QRegularExpression re(QStringLiteral("\\b\\w+\\b"));
QRegularExpressionMatchIterator it = re.globalMatch(text);
while (it.hasNext()) {
    QRegularExpressionMatch match = it.next();
    qDebug() << match.captured(0);
}
```

QRegularExpression 的全局匹配通过 globalMatch() 返回一个迭代器，语义比 QRegExp 的 indexIn + pos 偏移方式更清晰，也不容易出现"死循环匹配同一位置"的 bug。

### 3.3 QTextCodec 迁移到 QStringConverter

QTextCodec 是 Qt 5 中处理文本编码转换的核心类。它支持数十种字符编码（GBK、Big5、Shift-JIS、ISO-8859 系列等），可以用来把 QByteArray 按某种编码解码成 QString，或者把 QString 编码成某种编码的 QByteArray。Qt 6 把 QTextCodec 从 Core 模块中移除了，替代方案是 QStringConverter 和它的子类 QStringEncoder / QStringDecoder。

QTextCodec 的典型用法是通过 codecForName() 获取一个编码器，然后调用 toUnicode() / fromUnicode() 做转换：

```cpp
// QTextCodec 旧写法（需要 Core5Compat）
#include <QTextCodec>

QTextCodec* gbkCodec = QTextCodec::codecForName("GBK");
QTextCodec* utf8Codec = QTextCodec::codecForName("UTF-8");

// GBK 字节流 → QString
QByteArray gbkData = "...";
QString text = gbkCodec->toUnicode(gbkData);

// QString → GBK 字节流
QByteArray encoded = gbkCodec->fromUnicode(text);
```

等价的 QStringConverter 写法：

```cpp
// QStringConverter 新写法（Qt 6 推荐）
#include <QStringConverter>

// GBK 字节流 → QString（解码）
QByteArray gbkData = "...";
QString text = QStringDecoder(QStringDecoder::encodingForName("GBK").value_or(
    QStringDecoder::Utf8))(gbkData);

// 或者更清晰的写法
auto decOpt = QStringDecoder::encodingForName("GBK");
if (decOpt) {
    QStringDecoder decoder(*decOpt);
    QString text = decoder.decode(gbkData);
}

// QString → GBK 字节流（编码）
auto encOpt = QStringEncoder::encodingForName("GBK");
if (encOpt) {
    QStringEncoder encoder(*encOpt);
    QByteArray encoded = encoder.encode(text);
}
```

QStringConverter 的 API 设计比 QTextCodec 更现代。它使用 std::optional 来处理"编码名不存在"的情况（encodingForName 返回 std::optional<QStringConverter::Encoding>），而不是返回一个可能为 nullptr 的指针。QStringDecoder 和 QStringEncoder 是 QStringConverter 的子类，分别负责解码和编码——这种职责分离比 QTextCodec 把读写混在一起更清晰。

一个重要的变化是：QTextCodec::codecForLocale()（获取系统默认编码）在 Qt 6 中没有直接等价物，因为 Qt 6 内部统一使用 UTF-8。如果你需要获取系统编码，可以用 QStringConverter::encodingForData() 来检测，或者直接硬编码 UTF-8（这是 Qt 6 推荐的做法）。实际上，绝大多数现代应用都应该使用 UTF-8，只有在处理遗留数据文件或者对接旧系统接口时才需要处理 GBK/Shift-JIS 这些非 UTF 编码。

对于常见的 UTF-8 编解码，Qt 6 提供了更便捷的方式——QString 的 toUtf8() 和 fromUtf8() 方法保持不变，不需要通过 QStringConverter。只有在处理非 UTF-8 编码时才需要引入 QStringConverter。

### 3.4 渐进式迁移策略

如果你的 Qt 5 项目代码量比较大（几万行甚至几十万行），一次性把所有 QRegExp 和 QTextCodec 都替换成新 API 风险很高——改动量大、测试压力大、容易引入回归 bug。更稳妥的做法是分阶段迁移。

第一阶段的目标是让项目在 Qt 6 下编译通过并能正常运行。具体做法是在 CMake 中引入 Core5Compat 模块，把所有 `#include <QRegExp>` 和 `#include <QTextCodec>` 的地方保持不变，只修复和 Core5Compat 无关的编译错误（比如 QNamespace 的枚举值变化、QMap/QHash 的 API 微调等）。这个阶段不需要改动任何业务逻辑代码。

第二阶段是逐文件替换。每次挑选一个源文件，把其中的 QRegExp 替换成 QRegularExpression，QTextCodec 替换成 QStringConverter，然后编译、运行测试。替换完成后把该文件的 Core5Compat 头文件 include 删掉。通过版本控制每次提交一个或几个文件的迁移，出问题可以快速定位到具体的提交。

第三阶段是当所有文件都不再依赖 Core5Compat 时，从 CMake 的 find_package 和 target_link_libraries 中移除 Core5Compat。这时你的项目就完全迁移到了 Qt 6 的原生 API，没有任何兼容层依赖。

这个策略的核心思想是"先跑起来，再慢慢改"。很多团队在迁移时犯的错误就是试图一次性改完所有代码——结果改了三天还在修编译错误，测试全面崩溃，最后不得不回滚。分阶段迁移虽然看起来慢，但每个阶段都有可验证的成果，风险可控。

## 4. 综合示例：QRegExp / QTextCodec 迁移对照

这个示例不是一个"可运行的应用"，而是一份迁移对照参考。我们在同一个文件中分别用旧 API（Core5Compat）和新 API 实现相同的功能，通过条件编译切换，让你直观看到 API 的差异。实际迁移时你只需要保留新 API 的写法，删除旧 API 的代码和 Core5Compat 依赖即可。

CMake 配置：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core Core5Compat)

qt_add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Core5Compat)
```

main.cpp 的完整代码：

```cpp
/**
 * Qt5Compat 迁移对照示例
 *
 * 本示例演示旧 API（Core5Compat）和新 API 的使用对比：
 * - QRegExp → QRegularExpression 迁移
 * - QTextCodec → QStringConverter 迁移
 * - 渐进式迁移策略的实际操作
 *
 * 示例中 USE_COMPAT 宏控制使用旧 API 还是新 API，
 * 迁移完成后只需删除旧 API 分支和 Core5Compat 依赖即可。
 */

#include <QDebug>
#include <QString>

// ========================================
// 旧 API 头文件（Core5Compat）
// ========================================
#include <QRegExp>
#include <QTextCodec>

// ========================================
// 新 API 头文件（Qt 6 原生）
// ========================================
#include <QRegularExpression>
#include <QStringConverter>

/// @brief 演示正则表达式迁移：提取版本号
static void demoRegexMigration()
{
    qDebug() << "=== 正则表达式迁移对照 ===";
    qDebug() << "";

    QString input = QStringLiteral("Qt version 6.9.1 released on 2025-04-22");

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

/// @brief 演示正则表达式迁移：全局匹配
static void demoRegexGlobalMatchMigration()
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

/// @brief 演示编码转换迁移
static void demoCodecMigration()
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

/// @brief 演示迁移策略的实践步骤
static void demoMigrationStrategy()
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

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "Qt5Compat 迁移对照示例";
    qDebug() << "本示例对比展示旧 API（Core5Compat）和新 API 的使用方式";
    qDebug() << "";

    demoRegexMigration();
    demoRegexGlobalMatchMigration();
    demoCodecMigration();
    demoMigrationStrategy();

    qDebug() << "迁移对照演示完成";

    return 0;
}
```

运行程序后会依次输出正则表达式迁移对照、全局匹配迁移对照、编码转换迁移对照和迁移策略说明。对于正则表达式部分，你会看到 QRegExp 和 QRegularExpression 对同样的输入产生完全相同的匹配结果，但 API 调用方式不同。对于编码转换部分，你会看到 QTextCodec 和 QStringConverter 对同样的文本产生相同的编码输出。

几个实现细节说明一下。这个示例同时引入了旧 API 和新 API 的头文件，实际迁移完成后你只需要保留新 API 的头文件。demoRegexMigration() 中的 QRegExp::cap(n) 和 QRegularExpressionMatch::captured(n) 的返回值完全相同，但 QRegularExpression 提供了更丰富的匹配信息——capturedStart() / capturedEnd() 获取匹配位置和长度，capturedView() 获取 QStringView 避免不必要的内存分配。demoCodecMigration() 中的 QStringConverter::encodingForName() 返回 std::optional，你需要检查编码是否可用——某些编码可能不被当前平台支持。GBK 编码在大多数 Linux 发行版上需要安装额外的 locale 数据。

## 5. 练习项目

练习项目：批量迁移脚本。

编写一个 Python 脚本（或者 Qt C++ 程序），扫描指定目录下的所有 .cpp 和 .h 文件，自动检测其中使用了 QRegExp、QTextCodec、QLinkedList 的位置，输出一份迁移报告（文件名、行号、使用的旧 API、建议的新 API）。对于简单的替换（比如 `#include <QRegExp>` → `#include <QRegularExpression>`），脚本可以直接执行替换；对于需要手动调整的替换（比如 `re.cap(1)` → `match.captured(1)` 需要额外声明 match 变量），脚本标记为"需要手动处理"。

完成标准是这样的：脚本扫描一个包含 QRegExp 和 QTextCodec 的目录，正确识别所有旧 API 的使用位置；对于 include 指令能自动替换；对于 cap → captured 这种需要上下文的替换能正确标记为手动处理；输出一份清晰的 CSV 格式报告。

几个实现提示：Python 的 re 模块足以完成文本匹配，不需要用 Qt。QRegExp 的迁移比 QTextCodec 复杂，因为 cap() → captured() 需要把返回值从 QRegExp 改为 QRegularExpressionMatch，这涉及到代码结构的改变，很难全自动处理。先从简单的 include 替换和 using 声明替换开始，复杂的模式匹配留给手动处理。

## 6. 官方文档参考

[Qt 文档 · Qt5Compat 模块](https://doc.qt.io/qt-6/qt5compat-index.html) -- Qt 5 兼容层模块总览

[Qt 文档 · QRegularExpression](https://doc.qt.io/qt-6/qregularexpression.html) -- Perl 兼容正则表达式类（Qt 6 推荐使用）

[Qt 文档 · QStringConverter](https://doc.qt.io/qt-6/qstringconverter.html) -- 字符串编码转换基类（替代 QTextCodec）

[Qt 文档 · QStringEncoder](https://doc.qt.io/qt-6/qstringencoder.html) -- 字符串编码器（QString → QByteArray）

[Qt 文档 · QStringDecoder](https://doc.qt.io/qt-6/qstringdecoder.html) -- 字符串解码器（QByteArray → QString）

[Qt 文档 · Qt 6 迁移指南](https://doc.qt.io/qt-6/portingguide.html) -- Qt 5 到 Qt 6 的完整迁移指南

---

到这里就大功告成了。Qt5Compat 不是什么让人兴奋的新功能模块，它是 Qt 版本更迭留下的技术债务缓冲带。但正因为它的存在，大量 Qt 5 项目才得以平稳过渡到 Qt 6——先链接 Core5Compat 编译通过，再逐文件把 QRegExp 换成 QRegularExpression、QTextCodec 换成 QStringConverter，最后移除 Core5Compat 依赖。这套渐进式迁移策略的核心思想是"每个阶段都有可验证的成果"，比一口气重写所有代码的风险低得多。如果你的项目还在依赖 Qt 5，强烈建议尽早开始迁移——Qt 5 的 LTS 支持终有到期的一天，而 Qt 6 的新功能和性能优化值得你花时间迁移过来。
