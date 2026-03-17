━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
入门 · QtBase · 15 · 正则与文本处理
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

## 1. 前言

说实话，在 Qt5 时代我也还在用 QRegExp，毕竟老习惯难改。但后来项目里遇到一个复杂的文本解析需求，正则写了一百多字符，调试起来简直要命。那时才发现 QRegularExpression 不仅性能更好，语法也更接近现代正则标准，而且早已不是什么"新东西"了——Qt6 里 QRegExp 直接被扔到了 Qt5Compat 模块，这意味着官方已经明确告诉你：别用了。

正则表达式这东西，学会之前觉得玄学，学会之后简直是文本处理的瑞士军刀。无论是验证邮箱格式、从日志中提取 IP 地址，还是批量重命名文件，一行正则能顶几十行 if-else。但正则也因为其"写时爽，读时火"的恶名被人诟病——尤其是那些一长串符号没有注释的魔法字符串，三个月后连你自己都看不懂。

所以这篇入门文章我会尽量把基础打扎实，不仅告诉你怎么写，更重要的是怎么写得能看懂、怎么调试、怎么避免那些坑。我们不会讲所有正则语法——那够写一本书——但会覆盖日常开发中 80% 的场景。

## 2. 环境说明

本文基于 Qt 6.10+，所有示例使用 CMake 3.26+ 构建系统。QRegularExpression 属于 Qt Core 模块，不需要额外链接其他组件。如果你还在使用 QRegExp，现在是时候迁移了——Qt6 中 QRegExp 已被移至 Qt5Compat 模块。

## 3. 核心概念

### 3.1 从 QRegExp 到 QRegularExpression

先说清楚这个历史包袱。QRegExp 是 Qt 时代的产物，它的正则引擎自己实现的一套，功能和性能都有限。QRegularExpression 从 Qt5 引入，底层用的是 PCRE（Perl Compatible Regular Expressions）库，功能更强大，语法更标准，性能也更好。

Qt6 之后，官方直接把 QRegExp 踢出了核心模块。如果你还在用：

```cpp
#include <QRegExp>  // Qt6 下这个头文件不在 Core 里了
```

你需要链接 Qt5Compat 模块，或者直接改用 QRegularExpression。我强烈建议后者，因为早晚会改，不如趁早。

### 3.2 QRegularExpression 基础用法

最简单的用法就是创建一个正则对象，然后用它匹配字符串：

```cpp
#include <QRegularExpression>

// 创建正则对象，匹配数字
QRegularExpression re("\\d+");
if (re.isValid()) {
    qDebug() << "正则表达式有效";
} else {
    qDebug() << "错误:" << re.errorString();
}

// 匹配字符串
QString text = "价格：123元";
QRegularExpressionMatch match = re.match(text);
if (match.hasMatch()) {
    qDebug() << "匹配到:" << match.captured(0);  // "123"
}
```

这里有个细节要注意：正则字符串里的反斜杠需要转义，所以 `\\d+` 实际上是正则引擎看到的 `\d+`（一个或多个数字）。如果你觉得两层反斜杠很烦，可以用 C++11 的原始字符串字面量：

```cpp
QRegularExpression re(R"(\d+)");  // 原始字符串，不需要转义反斜杠
```

📝 口述回答：用自己的话说说，QRegularExpression 和 QRegExp 有什么区别？为什么 Qt6 推荐使用前者？

### 3.3 匹配模式

match() 方法默认是从字符串开头开始匹配的，但你可以指定不同的匹配类型：

```cpp
QString text = "abc123def456";
QRegularExpression re("\\d+");

// NormalMatch - 从任意位置开始匹配
QRegularExpressionMatch match1 = re.match(text);
qDebug() << match1.captured(0);  // "123"，第一个匹配

// PrefaceMatch - 必须从字符串开头匹配
QRegularExpressionMatch match2 = re.match(text, 0, QRegularExpression::NormalMatch, QRegularExpression::MatchNoOptions);
// 等等，PrefaceMatch 是什么？这里有个更简单的理解：
// PartialMatch 或 NormalMatch 只是语义上的区分，实际行为相同

// 全局匹配 - 找所有匹配项
QRegularExpressionMatchIterator it = re.globalMatch(text);
while (it.hasNext()) {
    QRegularExpressionMatch match = it.next();
    qDebug() << match.captured(0);  // 输出 "123" 然后 "456"
}
```

`globalMatch()` 这个方法特别有用，比如从日志里提取所有时间戳、所有 IP 地址这类场景。

🔲 代码填空：下面的代码想从文本中提取所有邮箱地址，请补充空白处。

```cpp
QString text = "联系我：alice@example.com 或 bob@test.org";
QRegularExpression emailRe(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
QRegularExpressionMatchIterator it = emailRe.______(text);
while (it.______) {
    QRegularExpressionMatch match = it.next();
    qDebug() << "找到邮箱:" << match.______(0);
}
```

### 3.4 捕获组

捕获组是正则表达式最强大的功能之一，允许你从匹配结果中提取特定部分。捕获组用圆括号 `()` 定义：

```cpp
// 匹配日期格式 YYYY-MM-DD
QRegularExpression dateRe(R"((\d{4})-(\d{2})-(\d{2}))");
QString text = "今天是2025-03-17";

QRegularExpressionMatch match = dateRe.match(text);
if (match.hasMatch()) {
    qDebug() << "完整日期:" << match.captured(0);  // "2025-03-17"
    qDebug() << "年份:" << match.captured(1);       // "2025"
    qDebug() << "月份:" << match.captured(2);       // "03"
    qDebug() << "日期:" << match.captured(3);       // "17"
}
```

`captured(0)` 永远是整个匹配的字符串，`captured(1)`、`captured(2)` 依次是各个捕获组。如果你觉得记数字很麻烦，可以给捕获组命名：

```cpp
// 命名捕获组
QRegularExpression dateRe(R"((?<year>\d{4})-(?<month>\d{2})-(?<day>\d{2}))");
QRegularExpressionMatch match = dateRe.match("今天是2025-03-17");

if (match.hasMatch()) {
    qDebug() << "年份:" << match.captured("year");   // 比 captured(1) 清晰多了
    qDebug() << "月份:" << match.captured("month");
    qDebug() << "日期:" << match.captured("day");
}
```

命名捕获组虽然在入门层显得有点"高级"，但我还是强烈推荐一开始就养成这个习惯。一周后你再看代码，`captured("year")` 比 `captured(1)` 友好太多了。

### 3.5 常用正则模式速查

这里列几个开发中常用的模式，建议收藏：

```cpp
// 邮箱地址（简化版）
QRegularExpression emailRe(R"([\w.%+-]+@[\w.-]+\.[a-zA-Z]{2,})");

// IPv4 地址
QRegularExpression ipv4Re(R"((\d{1,3}\.){3}\d{1,3})");
// 注意：这个不验证每个数字是否在 0-255 范围内

// URL（http/https）
QRegularExpression urlRe(R"(https?://[^\s/$.?#].[^\s]*)");

// 手机号（中国大陆，简化版）
QRegularExpression phoneRe(R"(1[3-9]\d{9})");

// 十六进制颜色代码 (#RGB 或 #RRGGBB)
QRegularExpression colorRe(R"(#([0-9a-fA-F]{3}|[0-9a-fA-F]{6}))");

// 匹配空白行（纯空白或空行）
QRegularExpression blankLineRe(R"(^\s*$)");

// 匹配双引号字符串（支持转义）
QRegularExpression quotedStringRe(R"("([^"\\]|\\.)*")");
```

这些模式覆盖了大部分日常文本处理需求。但记住：正则表达式不是万能的，比如解析 HTML/XML 这种结构化文本，用专门的解析器更好。

## 4. 踩坑预防清单

> ⚠️ 坑 #1：忘记检查正则表达式有效性
>
> ❌ 错误做法：
> ```cpp
> QRegularExpression re("[abc");  // 括号不匹配
> QRegularExpressionMatch match = re.match("test");
> // 直接用 match，完全不知道正则已经无效了
> ```
>
> ✅ 正确做法：
> ```cpp
> QRegularExpression re("[abc");
> if (!re.isValid()) {
>     qWarning() << "正则表达式错误:" << re.errorString();
>     qWarning() << "错误位置:" << re.errorOffset();
>     return;
> }
> QRegularExpressionMatch match = re.match("test");
> ```
>
> 💥 后果：无效的正则表达式会匹配失败，但没有明显错误信息，调试时浪费大量时间。
>
> 💡 一句话记住：每次创建正则对象后，特别是模式来自用户输入时，务必检查 isValid()。

> ⚠️ 坑 #2：混淆 match() 和 exactMatch()
>
> ❌ 错误做法：
> ```cpp
> QRegularExpression re(R"(\d{3}-\d{4})");  // 匹配 123-4567 格式
> QString phone = "我的电话是123-4567，谢谢";
> if (re.match(phone).hasMatch()) {
>     // 这会匹配成功，因为字符串包含这个模式
> }
> ```
>
> ✅ 正确做法：
> ```cpp
> QRegularExpression re(R"(\d{3}-\d{4})");
> QString phone = "123-4567";
> if (re.match(phone).hasMatch()) {
>     // match() 检查字符串是否包含匹配
> }
>
> // 或者用 anchored pattern 确保全字符串匹配
> QRegularExpression exactRe(R"(^\d{3}-\d{4}$)");
> if (exactRe.match(phone).hasMatch()) {
>     // 只有全字符串符合格式才算匹配
> }
> ```
>
> 💥 后果：当需要验证整个字符串格式时，match() 可能给出假阳性结果。
>
> 💡 一句话记住：验证完整格式用 `^...$`，查找子串用普通 match()。

> ⚠️ 坑 #3：贪婪匹配导致过度捕获
>
> ❌ 错误做法：
> ```cpp
> QString html = "<div>内容1</div><div>内容2</div>";
> QRegularExpression re(R"(<div>.*</div>)");  // 贪婪匹配
> QRegularExpressionMatch match = re.match(html);
> qDebug() << match.captured(0);
> // 输出整个 "<div>内容1</div><div>内容2</div>"，而不是两个独立匹配
> ```
>
> ✅ 正确做法：
> ```cpp
> QRegularExpression re(R"(<div>.*?</div>)");  // 非贪婪匹配
> QRegularExpressionMatchIterator it = re.globalMatch(html);
> while (it.hasNext()) {
>     qDebug() << it.next().captured(0);
> }
> // 分别输出 "<div>内容1</div>" 和 "<div>内容2</div>"
> ```
>
> 💥 后果：贪婪匹配会捕获尽可能多的内容，导致一个匹配"吞掉"多个应该独立匹配的内容。
>
> 💡 一句话记住：需要精确控制匹配范围时，优先用非贪婪量词 `*?`、`+?`、`??`。

🐛 调试挑战：下面的代码有什么问题？

```cpp
QString password = "abc123";
QRegularExpression passwordRe("[A-Za-z0-9]{8,}");  // 至少8位字母数字
if (passwordRe.match(password).hasMatch()) {
    qDebug() << "密码格式正确";
} else {
    qDebug() << "密码格式错误";
}
// 程序输出"密码格式错误"，但开发者困惑为什么
```

## 5. 随堂测验

我们穿插了几个小测验，现在检查一下你的理解：

**📝 口述回答**（前面）：用自己的话说说，QRegularExpression 和 QRegExp 有什么区别？为什么 Qt6 推荐使用前者？

**🔲 代码填空**（前面）：从文本中提取所有邮箱地址的代码。

**🐛 调试挑战**（前面）：密码验证正则为什么失败了？

## 6. 练习项目

🎯 练习项目：日志分析工具

📋 功能描述：实现一个简单的命令行日志分析工具，可以从日志文件中提取特定信息。功能包括：提取所有时间戳、提取所有 IP 地址、统计错误日志数量、根据正则表达式过滤日志行。

✅ 完成标准：程序能够读取一个文本格式的日志文件，使用 QRegularExpression 实现上述提取功能，将结果打印到控制台。时间戳格式至少支持两种常见格式（如 `[2025-03-17 10:30:00]` 和 `17/Mar/2025:10:30:00`）。IP 地址提取能够识别 IPv4 格式。错误日志通过包含 "ERROR" 或 "WARN" 关键字来识别。

💡 提示：
- 用 QFile 和 QTextStream 逐行读取日志文件
- 为每种格式创建专门的 QRegularExpression 对象
- globalMatch() 配合循环处理每一行中的多个匹配项
- 可以用 QHash<QString, int> 统计不同错误类型的出现次数
- 记得检查每个正则对象的 isValid()

## 7. 官方文档参考链接

📎 [Qt 文档 · QRegularExpression Class](https://doc.qt.io/qt-6/qregularexpression.html) · QRegularExpression 完整 API 参考，包含所有匹配选项和枚举类型
📎 [Qt 文档 · QRegularExpressionMatch Class](https://doc.qt.io/qt-6/qregularexpressionmatch.html) · 匹配结果类，讲解如何访问捕获组和匹配位置
