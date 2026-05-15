/// @file    regex_perf.cpp
/// @brief   正则表达式性能与高级匹配演示实现。
///
/// 对应教程：进阶层 01-QtBase/15-正则与文本处理。

#include "regex_perf.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include <QStringList>

/// @brief 对比 JIT 预编译和未优化的正则表达式在大量匹配时的性能差异。
static void demoJITOptimization()
{
    qDebug() << "  [JIT 优化] 对比预编译和未优化的匹配性能";
    qDebug() << "  " << QString(46, '-');

    // 测试用的正则表达式：匹配电子邮件地址
    QString pattern = R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})";

    // 构造测试数据：10000 个字符串，约一半包含有效的邮件地址
    QStringList testData;
    testData.reserve(10000);
    for (int i = 0; i < 5000; ++i) {
        testData.append(QString("user%1@example.com").arg(i));
        testData.append(QString("invalid-email-%1").arg(i));
    }

    // 未优化的正则表达式
    QRegularExpression regexNoOpt(pattern);
    QElapsedTimer timer;
    timer.start();
    int matchCount = 0;
    for (const auto& text : testData) {
        if (regexNoOpt.match(text).hasMatch()) {
            ++matchCount;
        }
    }
    qint64 noOptTime = timer.elapsed();

    // JIT 优化的正则表达式
    QRegularExpression regexOptimized(pattern);
    regexOptimized.optimize();    // 触发 JIT 预编译
    timer.restart();
    int matchCount2 = 0;
    for (const auto& text : testData) {
        if (regexOptimized.match(text).hasMatch()) {
            ++matchCount2;
        }
    }
    qint64 optTime = timer.elapsed();

    qDebug() << "    测试数据: 10000 个字符串";
    qDebug() << "    未优化耗时:" << noOptTime << "ms (匹配数:" << matchCount << ")";
    qDebug() << "    JIT优化耗时:" << optTime << "ms (匹配数:" << matchCount2 << ")";

    if (noOptTime > 0 && optTime > 0) {
        qDebug() << "    速度提升: 约" << (qreal(noOptTime) / qMax(optTime, 1))
                 << "倍（JIT 编译为本地机器码）";
    }

    qDebug() << "";
    qDebug() << "    optimize() 适用场景:";
    qDebug() << "      - 同一个正则需要反复使用";
    qDebug() << "      - 正则模式较复杂";
    qDebug() << "      - 需要匹配大量文本";
    qDebug() << "    不适用场景:";
    qDebug() << "      - 只使用一次的正则（optimize 本身有开销）";
}

/// @brief 演示 globalMatch 返回所有匹配结果的迭代器。
///
/// 相比手动循环调用 match()，globalMatch 更高效且语义更清晰。
static void demoGlobalMatch()
{
    qDebug() << "";
    qDebug() << "  [globalMatch] 迭代器模式匹配所有结果";
    qDebug() << "  " << QString(46, '-');

    QString text = "联系我们：admin@example.com 或 support@company.org，"
                   "也可以发邮件给 john.doe@university.edu 或 dev-team@startup.io";

    QRegularExpression emailRegex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");

    // 方法 1：使用 QRegularExpressionMatchIterator（推荐）
    // 惰性求值（lazy evaluation），不会一次性分配所有结果的内存
    qDebug() << "    [迭代器方式]";
    QRegularExpressionMatchIterator it = emailRegex.globalMatch(text);
    int index = 0;
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        qDebug() << "      邮箱" << (++index) << ":" << match.captured(0)
                 << "位置:" << match.capturedStart(0);
    }

    // 方法 2：将迭代器结果收集到 QList
    qDebug() << "    [收集到容器]";
    QList<QRegularExpressionMatch> allMatches;
    it = emailRegex.globalMatch(text);
    while (it.hasNext()) {
        allMatches.append(it.next());
    }
    qDebug() << "      总共找到" << allMatches.size() << "个邮箱地址";

    // 全局匹配也支持命名捕获组
    qDebug() << "";
    qDebug() << "    [结合命名捕获组]";
    QString logText = "[2025-01-15 INFO] 启动服务\n"
                      "[2025-01-15 WARN] 内存使用率高\n"
                      "[2025-01-16 CRIT] 磁盘空间不足";

    QRegularExpression logRegex(R"(\[(?P<date>\d{4}-\d{2}-\d{2}) (?P<level>\w+)\])");
    it = logRegex.globalMatch(logText);
    while (it.hasNext()) {
        auto m = it.next();
        qDebug() << "      日期:" << m.captured("date")
                 << "级别:" << m.captured("level");
    }
}

/// @brief 演示灾难性回溯与超时防护。
///
/// 灾难性回溯（Catastrophic Backtracking）是正则表达式的已知安全问题。
/// 某些模式在遇到特定输入时，回溯尝试次数呈指数级增长。
static void demoCatastrophicBacktracking()
{
    qDebug() << "";
    qDebug() << "  [灾难性回溯防护] setMatchTimeout 防止 ReDoS";
    qDebug() << "  " << QString(46, '-');

    // 经典的灾难性回溯模式：(a+)+$
    // 匹配 "aaaa...ab" 时，回溯尝试次数为 2^n（n 为 a 的个数）
    QString maliciousInput = QString(25, 'a') + "b";
    QString dangerousPattern = "(a+)+$";

    qDebug() << "    危险模式:" << dangerousPattern;
    qDebug() << "    输入长度:" << maliciousInput.length()
             << "字符 (" << QString(25, 'a') << "...b)";

    // 不设超时：可能卡住很长时间（这里用较短输入演示）
    QRegularExpression noTimeout(dangerousPattern);
    QElapsedTimer timer;
    timer.start();
    QRegularExpressionMatch matchNoTimeout = noTimeout.match(maliciousInput);
    qint64 noTimeoutMs = timer.elapsed();
    qDebug() << "    无超时: 耗时" << noTimeoutMs << "ms"
             << "(匹配:" << (matchNoTimeout.hasMatch() ? "是" : "否") << ")";

    // 防御建议
    // Qt 6.6+ 提供 setMatchTimeout(QDeadlineTimer)
    qDebug() << "";
    qDebug() << "    防护建议:";
    qDebug() << "      1. 处理用户输入时始终设置 matchTimeout";
    qDebug() << "      2. 避免嵌套量词：(a+)+, (a*)*, (a|a)* 等";
    qDebug() << "      3. 使用占有量词（PCRE2）或原子组减少回溯";
    qDebug() << "      4. 对复杂模式进行性能测试";
}

void runRegexPerfDemo()
{
    demoJITOptimization();
    demoGlobalMatch();
    demoCatastrophicBacktracking();
}
