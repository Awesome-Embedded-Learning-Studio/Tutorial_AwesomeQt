/// @file    named_capture.cpp
/// @brief   命名捕获组演示实现。
///
/// 对应教程：进阶层 01-QtBase/15-正则与文本处理。

#include "named_capture.h"

#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include <QStringList>

/// @brief 使用命名捕获组解析 URL 的各个组成部分。
///
/// 正则模式说明：
///   (?P<scheme>https?)      - 协议（http 或 https）
///   ://                     - 分隔符
///   (?P<host>[^/:]+)        - 主机名
///   (:(?P<port>\d+))?       - 可选端口号
///   (?P<path>/[^?]*)?       - 可选路径
///   (\?(?P<query>[^#]*))?   - 可选查询字符串
///   (#(?P<fragment>.*))?    - 可选片段标识符
static void demoURLParsing()
{
    qDebug() << "  [URL 解析] 使用命名捕获组解析 URL 组成部分";
    qDebug() << "  " << QString(46, '-');

    // Qt 使用 PCRE2 引擎，支持 Python 风格的命名捕获组 (?P<name>...)
    QRegularExpression urlRegex(
        R"((?P<scheme>https?)://(?P<host>[^/:]+)(:(?P<port>\d+))?(?P<path>/[^?]*)?(\?(?P<query>[^#]*))?(#(?P<fragment>.*))?)"
    );

    QStringList urls = {
        "https://www.example.com:8080/path/to/resource?key=value&lang=zh#section1",
        "http://localhost/api/v1/users",
        "https://qt.io",
        "https://github.com/CharlieChen114514/Tutorial_AwesomeQt?tab=readme",
    };

    for (const auto& url : urls) {
        QRegularExpressionMatch match = urlRegex.match(url);
        if (match.hasMatch()) {
            qDebug() << "  URL:" << url;
            qDebug() << "    scheme  =" << match.captured("scheme");
            qDebug() << "    host    =" << match.captured("host");
            qDebug() << "    port    =" << (match.captured("port").isEmpty()
                                               ? "(default)" : match.captured("port"));
            qDebug() << "    path    =" << (match.captured("path").isEmpty()
                                               ? "/" : match.captured("path"));
            qDebug() << "    query   =" << (match.captured("query").isEmpty()
                                               ? "(none)" : match.captured("query"));
            qDebug() << "    fragment=" << (match.captured("fragment").isEmpty()
                                               ? "(none)" : match.captured("fragment"));
            qDebug() << "";
        } else {
            qDebug() << "  URL 解析失败:" << url;
        }
    }
}

/// @brief 使用命名捕获组解析电子邮件地址（简化版）。
static void demoEmailParsing()
{
    qDebug() << "  [电子邮件解析] 提取用户名和域名";
    qDebug() << "  " << QString(46, '-');

    // 简化版正则，实际项目应使用更严格的 RFC 5322 验证
    QRegularExpression emailRegex(R"((?P<local>[^@]+)@(?P<domain>.+))");

    QStringList emails = {
        "user@example.com",
        "john.doe@company.co.uk",
        "admin+tag@subdomain.example.org",
        "invalid-email",          // 无效格式
        "@missing-local.com",     // 无效格式
    };

    for (const auto& email : emails) {
        QRegularExpressionMatch match = emailRegex.match(email);
        if (match.hasMatch()) {
            qDebug() << "  " << email
                     << "-> 本地部分:" << match.captured("local")
                     << "域名:" << match.captured("domain");
        } else {
            qDebug() << "  " << email << "-> 无效的邮件地址格式";
        }
    }
}

/// @brief 解析常见格式的日志文件行。
///
/// 在实际项目的日志分析中非常有用。
static void demoLogLineParsing()
{
    qDebug() << "";
    qDebug() << "  [日志行解析] 提取日志级别、时间戳和消息";
    qDebug() << "  " << QString(46, '-');

    QRegularExpression logRegex(
        R"(\[(?P<timestamp>\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3})\]\[(?P<level>\w+)\]\s*(?P<message>.*))"
    );

    QStringList logLines = {
        "[2025-06-15 14:30:45.123][INFO ] Server started on port 8080",
        "[2025-06-15 14:30:46.456][WARN ] Connection timeout from 192.168.1.100",
        "[2025-06-15 14:30:47.789][CRIT ] Database connection failed: timeout",
        "[2025-06-15 14:30:48.012][DEBUG] Request: GET /api/v1/users",
    };

    for (const auto& line : logLines) {
        QRegularExpressionMatch match = logRegex.match(line);
        if (match.hasMatch()) {
            qDebug() << "  级别:" << match.captured("level")
                     << "| 时间:" << match.captured("timestamp")
                     << "| 消息:" << match.captured("message");
        }
    }

    // capturedView 返回 QStringView，指向原始字符串的子区域，避免 QString 拷贝
    qDebug() << "";
    qDebug() << "  [capturedView] 零拷贝提取（适用于高频场景）";
    if (auto match = logRegex.match(logLines.first()); match.hasMatch()) {
        qDebug() << "    capturedView(\"level\") =" << match.capturedView("level");
        qDebug() << "    capturedView(\"message\") =" << match.capturedView("message");
    }
}

/// @brief 演示 PatternOptions 常用选项。
static void demoPatternOptions()
{
    qDebug() << "";
    qDebug() << "  [PatternOptions] 正则表达式常用选项";
    qDebug() << "  " << QString(46, '-');

    // CaseInsensitiveOption：忽略大小写
    QRegularExpression caseInsensitive("hello", QRegularExpression::CaseInsensitiveOption);
    qDebug() << "  CaseInsensitive:";
    qDebug() << "    \"HELLO World\" 匹配 \"hello\":"
             << caseInsensitive.match("HELLO World").hasMatch();

    // MultilineOption：^ 和 $ 匹配每行的开头和结尾
    QRegularExpression multiline("^line", QRegularExpression::MultilineOption);
    QString multiText = "first line\nline two\nline three";
    qDebug() << "  Multiline:";
    qDebug() << "    ^line 匹配多行文本中的行首:" << multiline.match(multiText).hasMatch();

    // DotMatchesEverythingOption：. 也匹配换行符 \n
    QRegularExpression dotAll("a.b", QRegularExpression::DotMatchesEverythingOption);
    qDebug() << "  DotMatchesEverything:";
    qDebug() << "    \"a\\nb\" 匹配 \"a.b\":" << dotAll.match("a\nb").hasMatch();

    // ExtendedPatternSyntax：忽略空白和 # 注释
    QRegularExpression extended(
        R"(
            (?P<year>\d{4})   # 4 位年份
            -                 # 分隔符
            (?P<month>\d{2})  # 2 位月份
            -                 # 分隔符
            (?P<day>\d{2})    # 2 位日期
        )",
        QRegularExpression::ExtendedPatternSyntaxOption
    );
    QRegularExpressionMatch dateMatch = extended.match("2025-06-15");
    if (dateMatch.hasMatch()) {
        qDebug() << "  ExtendedPatternSyntax (带注释的正则):";
        qDebug() << "    年:" << dateMatch.captured("year")
                 << "月:" << dateMatch.captured("month")
                 << "日:" << dateMatch.captured("day");
    }
}

void runNamedCaptureDemo()
{
    demoURLParsing();
    demoEmailParsing();
    demoLogLineParsing();
    demoPatternOptions();
}
