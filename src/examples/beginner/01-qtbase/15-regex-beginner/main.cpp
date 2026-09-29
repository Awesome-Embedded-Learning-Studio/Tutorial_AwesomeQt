// Qt 正则表达式入门示例
// 演示 QRegularExpression 的基本用法：匹配、捕获组、全局匹配、常用模式

#include <QCoreApplication>
#include <QDebug>
#include <QTextStream>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>

// 演示基础匹配用法
void demonstrateBasicMatching() {
    qDebug() << "=== 基础匹配演示 ===";

    // 创建正则表达式，匹配一个或多个数字
    // 使用原始字符串字面量 R"(...)" 避免双重转义
    QRegularExpression digitRe(R"(\d+)");

    // 检查正则表达式是否有效（重要！）
    if (!digitRe.isValid()) {
        qWarning() << "正则表达式无效:" << digitRe.errorString();
        return;
    }

    QString text = "价格：123元，折扣：45%";
    QRegularExpressionMatch match = digitRe.match(text);

    if (match.hasMatch()) {
        qDebug() << "原始文本:" << text;
        qDebug() << "匹配到数字:" << match.captured(0);      // captured(0) 是完整匹配
        qDebug() << "匹配位置:" << match.capturedStart(0);   // 匹配起始位置
        qDebug() << "匹配长度:" << match.capturedLength(0);  // 匹配字符长度
    }

    qDebug() << "";
}

// 演示捕获组的使用
void demonstrateCaptureGroups() {
    qDebug() << "=== 捕获组演示 ===";

    // 匹配日期格式 YYYY-MM-DD，使用捕获组提取各部分
    // (\d{4}) 是第1个捕获组，(\d{2}) 是第2个，以此类推
    QRegularExpression dateRe(R"((\d{4})-(\d{2})-(\d{2}))");

    if (!dateRe.isValid()) {
        qWarning() << "正则表达式无效:" << dateRe.errorString();
        return;
    }

    QString text = "项目启动日期：2025-03-17，截止日期：2025-12-31";
    QRegularExpressionMatch match = dateRe.match(text);

    if (match.hasMatch()) {
        qDebug() << "原始文本:" << text;
        qDebug() << "完整日期 (captured 0):" << match.captured(0);  // "2025-03-17"
        qDebug() << "年份 (captured 1):" << match.captured(1);       // "2025"
        qDebug() << "月份 (captured 2):" << match.captured(2);       // "03"
        qDebug() << "日期 (captured 3):" << match.captured(3);       // "17"
    }

    // 使用命名捕获组提高可读性
    qDebug() << "\n使用命名捕获组:";
    QRegularExpression namedDateRe(R"((?<year>\d{4})-(?<month>\d{2})-(?<day>\d{2}))");
    QRegularExpressionMatch namedMatch = namedDateRe.match(text);

    if (namedMatch.hasMatch()) {
        qDebug() << "年份:" << namedMatch.captured("year");   // 比 captured(1) 清晰
        qDebug() << "月份:" << namedMatch.captured("month");
        qDebug() << "日期:" << namedMatch.captured("day");
    }

    qDebug() << "";
}

// 演示全局匹配（查找所有匹配项）
void demonstrateGlobalMatch() {
    qDebug() << "=== 全局匹配演示 ===";

    QString text = "联系邮箱：alice@example.com 或 bob@test.org，cc: charlie@company.co.uk";

    // 邮箱正则表达式（简化版）
    QRegularExpression emailRe(R"([\w.%+-]+@[\w.-]+\.[a-zA-Z]{2,})");

    if (!emailRe.isValid()) {
        qWarning() << "正则表达式无效:" << emailRe.errorString();
        return;
    }

    qDebug() << "原始文本:" << text;
    qDebug() << "提取到的邮箱:";

    // globalMatch() 返回一个迭代器，可以遍历所有匹配项
    QRegularExpressionMatchIterator it = emailRe.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        qDebug() << "  " << match.captured(0);
    }

    qDebug() << "";
}

// 演示常用正则模式
void demonstrateCommonPatterns() {
    qDebug() << "=== 常用正则模式演示 ===";

    // IPv4 地址
    QRegularExpression ipv4Re(R"((\d{1,3}\.){3}\d{1,3})");
    QString ipText = "服务器 IP：192.168.1.1，网关：10.0.0.1";
    qDebug() << "IPv4 提取:";
    QRegularExpressionMatchIterator ipIt = ipv4Re.globalMatch(ipText);
    while (ipIt.hasNext()) {
        qDebug() << "  " << ipIt.next().captured(0);
    }

    // URL（http/https）
    QRegularExpression urlRe(R"(https?://[^\s/$.?#].[^\s]*)");
    QString urlText = "访问 https://www.qt.io 或 https://doc.qt.io 获取文档";
    qDebug() << "\nURL 提取:";
    QRegularExpressionMatchIterator urlIt = urlRe.globalMatch(urlText);
    while (urlIt.hasNext()) {
        qDebug() << "  " << urlIt.next().captured(0);
    }

    // 十六进制颜色代码
    QRegularExpression colorRe(R"(#([0-9a-fA-F]{3}|[0-9a-fA-F]{6}))");
    QString colorText = "背景色：#ffffff，文字色：#000，强调色：#FF5733";
    qDebug() << "\n颜色代码提取:";
    QRegularExpressionMatchIterator colorIt = colorRe.globalMatch(colorText);
    while (colorIt.hasNext()) {
        qDebug() << "  " << colorIt.next().captured(0);
    }

    qDebug() << "";
}

// 演示非贪婪匹配（避免过度捕获）
void demonstrateNonGreedyMatching() {
    qDebug() << "=== 非贪婪匹配演示 ===";

    QString html = "<div>内容1</div><div>内容2</div>";

    // 贪婪匹配：尽可能多地匹配
    QRegularExpression greedyRe(R"(<div>.*</div>)");
    QRegularExpressionMatch greedyMatch = greedyRe.match(html);
    qDebug() << "贪婪匹配结果:";
    qDebug() << "  " << greedyMatch.captured(0);
    // 输出整个 "<div>内容1</div><div>内容2</div>"

    // 非贪婪匹配：尽可能少地匹配（使用 *? 代替 *）
    QRegularExpression nonGreedyRe(R"(<div>.*?</div>)");
    qDebug() << "\n非贪婪匹配结果:";
    QRegularExpressionMatchIterator it = nonGreedyRe.globalMatch(html);
    while (it.hasNext()) {
        qDebug() << "  " << it.next().captured(0);
    }
    // 分别输出 "<div>内容1</div>" 和 "<div>内容2</div>"

    qDebug() << "";
}

// 演示精确匹配（验证整个字符串格式）
void demonstrateExactMatching() {
    qDebug() << "=== 精确匹配演示 ===";

    // 电话号码格式：XXX-XXXX
    QRegularExpression phoneRe(R"(\d{3}-\d{4})");

    QString validPhone = "123-4567";
    QString invalidText = "我的电话是123-4567，谢谢";

    qDebug() << "验证字符串:" << validPhone;
    if (phoneRe.match(validPhone).hasMatch()) {
        qDebug() << "  包含电话号码格式 ✓";
    }

    qDebug() << "\n验证字符串:" << invalidText;
    if (phoneRe.match(invalidText).hasMatch()) {
        qDebug() << "  包含电话号码格式 ✓";
        // 但这不是我们想要的——我们想验证整个字符串就是电话号码
    }

    // 使用 ^ 和 $ 确保匹配整个字符串
    QRegularExpression exactPhoneRe(R"(^\d{3}-\d{4}$)");

    qDebug() << "\n使用精确匹配验证:";
    qDebug() << "  " << validPhone << "->"
             << (exactPhoneRe.match(validPhone).hasMatch() ? "有效 ✓" : "无效 ✗");
    qDebug() << "  " << invalidText << "->"
             << (exactPhoneRe.match(invalidText).hasMatch() ? "有效 ✓" : "无效 ✗");

    qDebug() << "";
}

// 演示错误处理（正则表达式无效时）
void demonstrateErrorHandling() {
    qDebug() << "=== 错误处理演示 ===";

    // 故意写一个无效的正则表达式（括号不匹配）
    QRegularExpression invalidRe("[abc");

    if (!invalidRe.isValid()) {
        qDebug() << "检测到无效正则表达式！";
        qDebug() << "错误信息:" << invalidRe.errorString();
        // 注意：QRegularExpression 没有 errorOffset() 方法
        // 如果需要定位错误位置，可以尝试使用 QRegularExpression::validate()
        // 或者在外部使用 PCRE2 的错误定位功能
    }

    // 另一个常见错误：无效的转义序列
    QRegularExpression invalidEscapeRe("\\");
    if (!invalidEscapeRe.isValid()) {
        qDebug() << "\n另一个错误:";
        qDebug() << "错误信息:" << invalidEscapeRe.errorString();
    }

    qDebug() << "";
}

// 演示代码填空题答案：从文本中提取所有邮箱
void demonstrateEmailExtraction() {
    qDebug() << "=== 代码填空题答案：邮箱提取 ===";

    QString text = "联系我：alice@example.com 或 bob@test.org";
    QRegularExpression emailRe(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");

    if (!emailRe.isValid()) {
        qWarning() << "正则表达式无效:" << emailRe.errorString();
        return;
    }

    // 答案：
    QRegularExpressionMatchIterator it = emailRe.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        qDebug() << "找到邮箱:" << match.captured(0);
    }

    qDebug() << "";
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    // 使用 QTextStream 输出到 stdout（更可靠）
    QTextStream out(stdout);
    out << "Qt 正则表达式入门示例\n";
    out << "=====================\n";
    out << "\n";

    // 为了方便演示，临时重定向 qDebug 输出
    qInstallMessageHandler([](QtMsgType, const QMessageLogContext&, const QString& msg) {
        QTextStream(stdout) << msg << "\n";
    });

    demonstrateBasicMatching();
    demonstrateCaptureGroups();
    demonstrateGlobalMatch();
    demonstrateCommonPatterns();
    demonstrateNonGreedyMatching();
    demonstrateExactMatching();
    demonstrateErrorHandling();
    demonstrateEmailExtraction();

    qDebug() << "演示完成！";

    return 0;
}
