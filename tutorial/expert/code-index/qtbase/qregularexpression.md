---
title: QRegularExpression（封装 PCRE2）源码索引
description: QRegularExpression 隐式共享 QExplicitlySharedDataPointer、构造懒编译（首次 match/isValid 才编译 isDirty）、Qt 用 PCRE2 16 位宽 _16 接口不转 UTF-8（pattern.constData 直接传 PCRE2_SPTR16）、强制 PCRE2_UTF、PatternOption 值与 PCRE2 不同需 convertToPcreOptions 映射、JIT Release 默认开（Debug/macOS Rosetta 关）编译时自动 pcre2_jit_compile、Qt 用 pcre2_match_16 非 jit_match（内部自动检测 JIT）、JIT 栈 thread_local、match_context/match_data per-call、ovector 拷贝依赖 PCRE2_UNSET==-1、(?J) 重复命名 qWarning 放行不支持、线程安全三层（隐式共享+mutex+thread_local JIT 栈）、const 对象可跨线程并发 match、anchoredPattern 用 \A(?:...)\z、wildcard 默认文件路径 glob。
---

# QRegularExpression（封装 PCRE2）源码索引

> 本索引收录 Qt 6.9.1 源码中 QRegularExpression 的已验证证据。PCRE2 第三方库本身在 `qtbase/src/3rdparty/pcre2/`，本索引只覆盖 Qt 的封装层。

## 类核心与隐式共享

源码文件：`qtbase/src/corelib/text/qregularexpression.h` / `qregularexpression.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QExplicitlySharedDataPointer | h:176 | `QExplicitlySharedDataPointer<QRegularExpressionPrivate> d` | 隐式共享，拷贝浅拷贝，写时 detach。 |
| Private 布局 | cpp:729-746 | patternOptions/pattern/mutable QMutex/pcre2_code_16 *compiledPattern/isDirty | detach 时 compiledPattern 置 nullptr。 |

## 懒编译（关键纠偏）

源码文件：`qregularexpression.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 【纠偏】构造不编译 | cpp:1348-1353 | 构造函数只 d->pattern=pattern; d->patternOptions=options | 编译推迟到首次 match/isValid；常见说法「构造即编译」错。 |
| compilePattern 懒编译 | cpp:885-916 | `if (!isDirty) return; ... pcre2_compile_16(...); optimizePattern(); getPatternInfo();` | 加锁+isDirty 早退+强制 PCRE2_UTF+编译即 JIT。 |
| match 触发编译 | cpp:1587-1600 | match() 第一行 `d.data()->compilePattern()` | d.data() 绕过 const detach。 |

## PCRE2 16 位宽接口（关键纠偏）

源码文件：`qregularexpression.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 【纠偏】用 _16 不转 UTF-8 | cpp:899-904 | `pcre2_compile_16(PCRE2_SPTR16(pattern.constData()), ...)` | grep _8 零命中；pattern/subject 直接 UTF-16 传。 |
| 【纠偏】PatternOption 值不同需映射 | cpp:669-704 | convertToPcreOptions：CaseInsensitiveOption→PCRE2_CASELESS 等 | Qt 0x1/0x2 vs PCRE2 常量值完全不同。 |

## JIT（关键纠偏）

源码文件：`qregularexpression.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 【纠偏】Release 默认开 | cpp:974-990 | isJitEnabled：QT_ENABLE_REGEXP_JIT 覆盖；QT_DEBUG 返 false；macOS Rosetta 关；else true | 教材说「默认不开要手动 optimize」错。 |
| 编译即自动 JIT | cpp:1001-1011 | optimizePattern：`pcre2_jit_compile_16(...COMPLETE\|PARTIAL_SOFT\|PARTIAL_HARD)` | 在 compilePattern 末尾自动调，三种模式全编。 |
| 【纠偏】用 pcre2_match_16 非 jit_match | cpp:1055-1079 | safe_pcre2_match_16 调 pcre2_match_16 | grep jit_match 零；pcre2_match 内部自动检测 JIT。 |

## 匹配与捕获

源码文件：`qregularexpression.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| doMatch per-call 创建 | cpp:1147-1196 | match_context/match_data per-call + jit_stack_assign | PCRE2 同步阻塞，无异步。 |
| ovector 拷贝 | cpp:1232-1261 | `static_assert(PCRE2_UNSET==-1); capturedOffsets[i]=ovector[i]` | 未捕获组 start/end=-1；partial 回退 lookbehind。 |
| 【纠偏】(?J) 不支持 | cpp:938-943 | PCRE2_INFO_JCHANGED 检测→qWarning+放行 | 重复命名警告但继续，namedCaptureGroups 行为未定义。 |
| named capture name table | cpp:1019-1046 | PCRE2_INFO_NAMETABLE/NAMECOUNT 线性扫 | 每条目首 char16_t 组号+NUL name。 |

## globalMatch + 工具函数

源码文件：`qregularexpression.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| globalMatch 迭代推进 | cpp:1286-1306 | nextMatch 以上次 end 为新 offset + DontCheckSubjectString | 空匹配特殊处理（ANCHORED\|NOTEMPTY_ATSTART）。 |
| anchoredPattern | cpp:2077-2083 | `\A(?:expr)\z` | \A/\z 绝对首尾不受 Multiline 影响。 |
| wildcard 默认文件路径 glob | cpp:1931-1959 | Windows [^/\\]*，Unix [^/]*；外层 (?s:...) 强制 DOTALL | * 不跨目录。 |

## 线程安全三层

源码文件：`qregularexpression.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| setPattern detach+isDirty | cpp:1412-1444 | `d.detach(); d->isDirty = true; d->pattern = pattern;` | 写时复制+强制重编译。 |
| 三层线程安全 | cpp:729-746+887+960 | 隐式共享 + mutex 保护编译 + thread_local JIT 栈 | const 已编译对象可安全跨线程并发 match。 |
