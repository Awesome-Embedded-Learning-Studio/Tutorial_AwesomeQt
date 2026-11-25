---
title: "5.25 Qt5Compat 进阶：批量迁移策略与自动化检测工具"
description: "入门篇我们介绍了 Qt5Compat 模块的基本用法——引入兼容头文件让 Qt5 代码在 Qt6 上编译通过。进阶篇要站在工程全局视角：哪些 API 有 shim 哪些没有、QHash 随机化行为差异怎么排查、QRegExp 到 QRegularExpression 的批量替换策略、自动化迁移脚本怎么写。"
---

# 现代Qt开发教程（进阶篇）5.25——Qt5Compat 进阶：批量迁移策略与自动化检测工具

## 1. 前言

入门篇我们把 Qt5Compat 模块跑通了——引入 `Qt5Compat` 模块，加几个头文件，Qt5 时代的代码就能在 Qt6 上编译了。但入门篇只解决了「能不能编译」的问题。真正的 Qt5 到 Qt6 迁移远不止加几个 include。

想想一个百万行级别的 Qt5 项目要迁移到 Qt6。你不能一个文件一个文件地手动改。你需要知道哪些改动是机械替换（可以用脚本自动完成），哪些需要人工审查（行为差异）。你需要一套系统化的迁移策略，加上自动化检测工具来保证迁移质量。

Qt5Compat 模块的定位是「过渡期的拐杖」——它让你在迁移期间代码能编译、能运行，但它不是让你永远依赖的。最终目标是移除 Qt5Compat 依赖，用 Qt6 的原生 API 替换所有兼容层代码。这篇就讲怎么系统化地完成这个过程。

## 2. 环境说明

本文档基于 Qt 6.2+ 编写，需要 `Qt6::Core5Compat` 模块。CMake 中通过 `find_package(Qt6 REQUIRED COMPONENTS Core5Compat)` 引入。本篇内容涉及 `clazy` 静态分析工具（Qt 专用的 Clang 插件），需要在系统上安装 `clazy`（大多数 Linux 发行版有包可用，Windows 上需要自行编译或从 Qt 安装器获取）。

## 3. 核心概念讲解

### 3.1 Qt5Compat 模块的定位和边界

Qt5Compat 模块包含了 Qt6 中被移除或改动的 API 的兼容封装。但它不是万能的——只有一部分 API 有 shim，另一部分根本没有兼容层，必须手动迁移。

有 shim 的 API（引入 Qt5Compat 后可直接使用）：`QRegExp`（正则表达式）、`QTextCodec`（文本编解码）、`QLinkedList`（链表容器）、`QHash` 的旧迭代行为（通过宏控制）、`QDir` 的一些旧方法。还有各种被改名或移到不同头文件的函数。

没有 shim 的 API（必须手动修改代码）：`QMap::insertMulti` 改为 `QMultiMap`、`QVector` 和 `QList` 的合并（Qt6 中 `QList` 就是 `QVector`）、`QAction` 和 `QWidget::addAction` 的变化、`QRegExp` 到 `QRegularExpression` 的正则语法差异（虽然有 shim 但语义不完全兼容）、`QEvent` 子类枚举值的变化。

```cmake
# CMakeLists.txt 中引入 Qt5Compat
find_package(Qt6 REQUIRED COMPONENTS Core Core5Compat Widgets)

target_link_libraries(my_app
    PRIVATE
        Qt6::Core
        Qt6::Core5Compat    # 添加兼容模块
        Qt6::Widgets
)
```

引入 Qt5Compat 后，以下头文件可以直接使用：

```cpp
// 这些 Qt5 头文件在 Qt6 中通过 Core5Compat 模块提供
#include <QRegExp>           // 正则表达式（旧版）
#include <QTextCodec>        // 文本编解码（旧版）
#include <QLinkedList>       // 链表（旧版）
#include <QHash>             // 包含旧行为兼容
```

但要注意，这些 API 被标记为 `QT_DEPRECATED`。编译时会有弃用警告（如果你启用了 `QT_DISABLE_DEPRECATED_BEFORE` 宏）。正确的迁移路线是：先引入 Qt5Compat 让代码编译通过，然后逐步用 Qt6 原生 API 替换兼容层代码，最终移除 Qt5Compat 依赖。

### 3.2 QHash 随机化行为差异

Qt5 和 Qt6 中 `QHash` 的迭代顺序有根本性的差异，这可能是迁移过程中最隐蔽的行为变化。

Qt5 的 `QHash` 使用一个固定的哈希种子，同一份数据每次运行的迭代顺序是确定的。这导致很多 Qt5 代码（尤其是单元测试）隐式依赖了 `QHash` 的迭代顺序——比如用 `QHash::keys()` 获取键列表然后和预期结果比较。

Qt6 的 `QHash` 每次启动使用不同的随机种子，迭代顺序是不确定的。这是安全措施（防止哈希碰撞攻击），但它会破坏所有依赖迭代顺序的代码。

```cpp
/// @brief Qt5 代码中常见的 QHash 顺序依赖。
void qt5_hash_order_dependency()
{
    QHash<QString, int> kScores;
    kScores["Alice"] = 90;
    kScores["Bob"] = 85;
    kScores["Charlie"] = 95;

    // Qt5 中 keys() 的顺序是确定的
    // Qt6 中 keys() 的顺序每次运行不同
    QStringList kNames = kScores.keys();

    // 如果代码依赖 kNames 的顺序，Qt6 下会出错
    // 比如：assert(kNames[0] == "Alice");  // Qt6 下可能失败
}
```

修复方法取决于具体场景。如果只是需要排序后的键列表，加一步排序：

```cpp
/// @brief 正确的做法——不依赖 QHash 的迭代顺序。
void correct_hash_usage()
{
    QHash<QString, int> kScores;
    kScores["Alice"] = 90;
    kScores["Bob"] = 85;
    kScores["Charlie"] = 95;

    // 需要有序遍历时，先排序
    QStringList kNames = kScores.keys();
    kNames.sort();  // 按字母序排序

    // 或者使用 QMap 替代（QMap 始终按 key 排序）
    QMap<QString, int> kOrderedScores;
    kOrderedScores["Alice"] = 90;
    kOrderedScores["Bob"] = 85;
    kOrderedScores["Charlie"] = 95;
    // QMap 的 keys() 始终按字母序返回
}
```

如果你的项目中有大量依赖 `QHash` 顺序的代码，可以在迁移初期用环境变量 `QT_HASH_SEED` 固定种子值（设为 0），暂时恢复确定性行为。但这只是过渡方案，最终还是需要修复所有顺序依赖。

排查方法是用 `QT_HASH_SEED=0` 和 `QT_HASH_SEED=1` 分别运行测试，如果结果不一致就说明存在顺序依赖。

### 3.3 QRegExp 到 QRegularExpression 的批量替换策略

`QRegExp` 到 `QRegularExpression` 是迁移中工作量最大的部分之一。虽然 Qt5Compat 提供了 `QRegExp` 的 shim，但它的语义不完全兼容，且长期依赖 compat 层不是好的工程实践。

两者的主要差异：`QRegExp` 使用的是简化的正则语法（类似 Perl 风格），`QRegularExpression` 使用的是 PCRE2（Perl Compatible Regular Expressions 2）。大多数常用的正则模式可以直接迁移，但有一些语法差异需要注意。

```cpp
/// @brief QRegExp 到 QRegularExpression 的常见迁移模式。

// QRegExp 版本
QRegExp kOldPattern("\\b\\d{3}-\\d{4}\\b");
bool kMatch = kOldPattern.exactMatch("123-4567");

// QRegularExpression 版本
QRegularExpression kNewPattern("^\\b\\d{3}-\\d{4}\\b$");
// exactMatch 对应 anchorMatch + 全字符串匹配
QRegularExpressionMatch kNewMatch = kNewPattern.match("123-4567");
bool kNewResult = kNewMatch.hasMatch();
```

关键迁移点总结。`QRegExp::exactMatch()` 对应 `QRegularExpression::match()` 加上 `^...$` 锚定（或者用 `QRegularExpression::anchoredPattern()` 包装模式字符串）。`QRegExp::indexIn()` 对应 `QRegularExpression::match()` 加上 `QRegularExpression::MatchOption::MatchDefault`。`QRegExp::cap(n)` 对应 `QRegularExpressionMatch::captured(n)`。`QRegExp::pos(n)` 对应 `QRegularExpressionMatch::capturedStart(n)`。正则语法方面，`\b`、`\d`、`\w` 等元字符不变，但某些高级语法（如前瞻断言的贪婪模式）行为可能有细微差异。

批量替换的思路是写一个脚本扫描所有 `.cpp` 和 `.h` 文件，把常见的 `QRegExp` 用法模式替换为 `QRegularExpression` 对应代码。不是所有替换都能自动化——正则模式字符串本身可能需要人工审查（特别是包含特殊语法的正则）。

```bash
# 使用 sed 做简单的机械替换（仅处理最简单的情况）
# 替换头文件 include
find src/ -name "*.cpp" -o -name "*.h" | xargs sed -i 's/#include <QRegExp>/#include <QRegularExpression>/g'

# 替换类型声明
find src/ -name "*.cpp" -o -name "*.h" | xargs sed -i 's/QRegExp /QRegularExpression /g'

# 替换 cap(n) 调用
find src/ -name "*.cpp" -o -name "*.h" | xargs sed -i 's/\.cap(\([0-9]\))/.captured(\1)/g'

# 替换 exactMatch（这个需要人工审查，不能简单替换）
# exactMatch 对应的是 anchoredPattern，不是简单的 match
```

上面这些 `sed` 命令只能处理最简单的情况。复杂的迁移（比如 `indexIn` + `cap` 的组合用法、带全局匹配的循环、`setMinimal` 的最小匹配模式）需要人工审查和重写。

### 3.4 自动化迁移脚本思路（正则匹配 + sed/clazy）

自动化迁移的核心思路是：先跑静态分析找出所有需要迁移的代码位置，然后用脚本做机械替换，最后人工审查脚本无法处理的部分。

`clazy` 是 Qt 专用的 Clang 静态分析插件。它有一个专门的检查器 `qt6-deprecated-api-fixes` 可以检测 Qt5Compat 中使用的已弃用 API，并给出修复建议。

```bash
# 使用 clazy 检测项目中的 Qt5 兼容层 API
# 需要先配置编译器为 clazy
export CXX=clazy
cmake -B build -DCMAKE_CXX_COMPILER=clazy
cmake --build build 2>&1 | tee clazy_report.txt

# 过滤出与迁移相关的检查结果
grep "qt6-deprecated-api-fixes\|qregexp\|qtextcodec" clazy_report.txt
```

`clazy` 会输出每个需要修改的文件和行号，以及建议的替换方式。这比手动搜索要全面得多。

对于脚本无法自动处理的复杂情况，推荐的工作流程是：

```bash
# 第一步：用 clazy 生成完整的迁移清单
clazy ... 2>&1 | grep "qt6" > migration_todo.txt

# 第二步：用脚本处理简单替换
# （QRegExp 头文件替换、简单的类型声明替换等）

# 第三步：用 grep 找出需要人工审查的复杂模式
grep -rn "indexIn\|setMinimal\|cap(" src/ --include="*.cpp" --include="*.h"

# 第四步：逐个文件人工审查和修复

# 第五步：运行测试套件确认迁移正确性
ctest --test-dir build --output-on-failure
```

还有一个实用技巧是用 `git diff` 配合编译来分批迁移。每次迁移一个模块或一个文件，编译确认通过后再继续下一个。不要一次改完所有文件——出问题后很难定位是哪次改动引入的。

```bash
# 分批迁移策略
# 1. 先迁移独立的工具类（不依赖其他模块）
# 2. 再迁移核心数据层（Model、数据库）
# 3. 最后迁移 UI 层（Widget、QML）

# 每迁移一个模块后
git add -p           # 交互式选择要提交的改动
cmake --build build  # 确认编译通过
ctest --test-dir build  # 确认测试通过
git commit -m "migrate: XXX module to Qt6 native API"
```

现在有个思考题：你用 `sed` 把所有 `QRegExp::exactMatch` 替换成了 `QRegularExpression::match`，但测试失败了。可能是什么原因？

原因是 `QRegExp::exactMatch` 要求整个字符串完全匹配正则表达式（自带 `^...$` 锚定），而 `QRegularExpression::match` 默认做的是子串搜索。如果你直接替换，原来 `exactMatch("123")` 匹配 `"123-4567"` 会返回 false，但 `match("123-4567")` 会返回 true（因为 `"123"` 是子串）。正确的做法是用 `QRegularExpression::anchoredPattern()` 包装模式字符串，或者手动在模式前后加 `^` 和 `$`。

## 4. 踩坑预防

第一个坑是 `QHash` 的迭代顺序不一致导致单元测试大面积失败。这是 Qt5 到 Qt6 迁移中最常见的问题，也是最容易被忽略的。很多 Qt5 项目中有大量类似 `QCOMPARE(hash.keys(), expected_keys)` 的断言。在 Qt6 下由于迭代顺序随机化，这些断言每次运行可能通过也可能不通过（取决于当次运行的随机种子）。解决方案是用环境变量 `QT_HASH_SEED=0` 临时恢复确定性行为，然后逐个修复测试用例——把 `QHash::keys()` 的结果排序后再比较，或者改用 `QMap`。

第二个坑是 `QList` 和 `QVector` 的合并。Qt6 中 `QList` 的实现等价于 Qt5 的 `QVector`（连续内存存储），不再有 Qt5 那种根据元素大小选择数组和链表实现的混合策略。对于大多数代码来说这不影响功能，但如果你的代码对 `QList<T>::append` 的摊销复杂度有假设（Qt5 中对于大于指针大小的类型，`QList` 内部存指针，`append` 是 O(1)），在 Qt6 中行为可能不同。另外，Qt5 中 `QList<QObject*>` 的特化行为在 Qt6 中不存在了。这类隐式依赖很难用脚本检测，只能通过代码审查和测试来发现。

第三个坑是过度依赖 Qt5Compat 而不进行后续迁移。Qt5Compat 是过渡工具，不是永久解决方案。如果项目长期依赖 Qt5Compat，一旦 Qt 在某个大版本中移除了 Compat 模块（这在 Qt 的长期规划中是有可能的），项目会面临紧急迁移的压力。建议在迁移完成后设置一个 CI 检查：不定期尝试编译「去掉 Qt5Compat 依赖」的分支，确认剩余的兼容层依赖在可控范围内。

## 5. 练习项目

练习项目是一个 Qt5 项目到 Qt6 的迁移实战。我们提供一个模拟的 Qt5 项目（纯代码，不需要真的用 Qt5 编译），项目中有以下需要迁移的问题：5 处 `QRegExp` 用法需要替换为 `QRegularExpression`、3 个依赖 `QHash` 迭代顺序的函数、2 处使用 `QTextCodec` 的编解码逻辑、1 处使用 `QLinkedList` 的链表操作。

你的任务是编写一个迁移脚本来处理能自动替换的部分，然后手动修复脚本无法处理的部分。最终目标是在保留 Qt5Compat 的情况下让项目编译通过，然后逐步移除 Qt5Compat 依赖（只移除能移除的部分，标注无法移除的）。

完成标准是所有代码迁移到 Qt6 原生 API（QRegExp -> QRegularExpression、QTextCodec -> QStringConverter、QLinkedList -> std::list 或 QLinkedList）、QHash 顺序依赖被修复、最终编译不依赖 Qt5Compat。对于确实无法迁移的部分（比如第三方库依赖），写一份说明文档。

提示几个关键点：用 `clazy` 或 `grep` 找出所有兼容层 API 的使用位置；`QRegExp::exactMatch` 用 `QRegularExpression::anchoredPattern` 替换；`QTextCodec` 用 `QStringEncoder` / `QStringDecoder` 替代；`QLinkedList` 用 `std::list` 替代；每个迁移步骤后编译并运行测试确认。

## 6. 官方文档参考链接

[Qt 文档 · Qt5Compat 模块](https://doc.qt.io/qt-6/qtcore5compat-module.html) -- 兼容模块总览

[Qt 文档 · QRegularExpression](https://doc.qt.io/qt-6/qregularexpression.html) -- Qt6 正则表达式 API

[Qt 文档 · QStringConverter](https://doc.qt.io/qt-6/qstringconverter.html) -- Qt6 文本编解码（替代 QTextCodec）

[Qt 文档 · Qt6 迁移指南](https://doc.qt.io/qt-6/portingguide.html) -- 官方迁移指南

[Clazy 文档](https://invent.kde.org/sdk/clazy) -- Qt 专用 Clang 静态分析工具

---

到这里 Qt5Compat 的进阶内容就拆完了。模块定位和边界、QHash 行为差异、QRegExp 批量替换、自动化迁移工具——掌握了这些，面对百万行级别的 Qt5 到 Qt6 迁移项目时就不会手忙脚乱了。核心策略就一句话：先用 Qt5Compat 让代码编译通过，然后系统化地逐步替换兼容层代码，最终目标是完全移除 Qt5Compat 依赖。迁移是一个工程问题，不是一个技术问题——分批迁移、持续测试、版本控制是你的三大武器。
