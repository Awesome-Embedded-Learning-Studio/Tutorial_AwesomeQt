---
title: JSON 解析与 CBOR 内核源码索引
description: Qt 6.9.1 源码中 QJsonDocument + QJsonPrivate + QCborContainerPrivate + 手写 Parser + Writer 的已验证证据——自 Qt 5.15 起 JSON 内部表示已是 QCborValue、手写递归下降解析器直接产 CBOR、QCborContainerPrivate 的 QSharedData 引用计数与 COW、QVariant 互转零拷贝中介、QJsonValue 把 Integer 合并进 Double 的 CBOR 副产物、死枚举与已删的二进制 API。
---

# JSON 解析与 CBOR 内核源码索引

> 本索引收录 Qt 6.9.1 源码中 JSON 子系统的已验证证据。源码全在 `qtbase/src/corelib/serialization/`。关键结论：自 Qt 5.15 起 JSON 内部表示切到 CBOR，`QJsonDocument` 是 `QCborValue` 的薄壳。

## QJsonDocument 是 QCborValue 的薄壳

源码文件：`qtbase/src/corelib/serialization/qjsondocument.cpp` / `qjsondocument.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QJsonDocumentPrivate 仅包一个 QCborValue | qjsondocument.cpp:52 | `class QJsonDocumentPrivate { ... QCborValue value; };` | JSON 文档内部无独立格式，全面 CBOR 化。 |
| unique_ptr 独占非隐式共享 | qjsondocument.h:110 | `std::unique_ptr<QJsonDocumentPrivate> d;` + `Q_DECLARE_SHARED(QJsonDocument)` | detach 不在 document 层触发，真正共享在 QCborContainerPrivate 层。 |
| 拷贝=新建Private+浅拷贝QCborValue | qjsondocument.cpp:108 | `d = std::make_unique<QJsonDocumentPrivate>(); d->value = other.d->value;` | 几乎零成本，但语义是值类型。 |
| fromJson 走 Parser 返 QCborValue | qjsondocument.cpp:266 | `QJsonPrivate::Parser parser(json.constData(), json.size()); const QCborValue val = parser.parse(error);` | 顶层非 array/map 填 IllegalValue；输入当 UTF-8 字节流。 |
| CBOR 化始于 Qt 5.15 | qjsonvalue.h:128 | `// Assert binary compatibility with pre-5.15 QJsonValue` + static_assert 指针大小 | 非 Qt 6 独创；5.15 切 QCborValue，6.x 继承深化。 |

## CBOR 内部数据布局

源码文件：`qtbase/src/corelib/serialization/qcborvalue_p.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QCborContainerPrivate 继承 QSharedData | qcborvalue_p.h:98 | `class QCborContainerPrivate : public QSharedData` | 引用计数 ref 来自 QSharedData::QAtomicInt（注意是 QSharedData 非 QShareData）。持 elements 数组 + data 缓冲。 |
| Element 16 字节定长 | qcborvalue_p.h:38 | `union { qint64 value; QCborContainerPrivate *container; }; QCborValue::Type type; ValueFlags flags;` | IsContainer 位区分内联值/子容器；HasByteData 表示 value 是 data 偏移。 |
| ByteData 变长紧跟数据 | qcborvalue_p.h:75 | `struct ByteData { QByteArray::size_type len; const char *byte() { return ... (this + 1); } };` | 头部+len 字节；flags 的 StringIsUtf16/StringIsAscii 决定解释方式。 |

## Parser 手写递归下降

源码文件：`qtbase/src/corelib/serialization/qjsonparser.cpp` / `qjsonparser_p.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| Parser 纯手写递归下降 | qjsonparser_p.h:26 | `class Parser { ... bool parseObject/parseArray/parseMember/parseString; QCborValue parseValue/parseNumber; }` | 六互递归方法对应 RFC8259 文法，无 yacc/状态机。 |
| 直接产 QCborValue | qjsonparser_p.h:46 | `QExplicitlySharedDataPointer<QCborContainerPrivate> container;` | Parser 成员即 CBOR 容器，无中间 Data 转换。 |
| parse 入口 eatBOM | qjsonparser.cpp:232 | `if (end - json > 3 && json[0]==0xef && ...) json += 3;` | 只认 UTF-8 BOM；非 ASCII 走 scanUtf8Char 严格解码。 |
| 嵌套上限 1024 | qjsonparser.cpp:16 | `static const int nestingLimit = 1024;` | parseObject/parseArray 入口 `++nestingLevel > nestingLimit` 报 DeepNesting，防栈溢出。 |
| StashedContainer RAII 暂存外层 | qjsonparser.cpp:158 | `StashedContainer(...) : stashed(std::move(*container))` + `intoValue` 返回 `makeValue(type, -1, ...)` | n=-1 用于 container 承载场景（非神秘哨兵，别处同模式）。 |
| parseNumber 三段策略 | qjsonparser.cpp:684 | `toLongLong → toDouble → convertDoubleTo(d,&n) 升级回 qint64` | 无 frac 时 isInt 保持 true；委托 strtoll/strtod，无自实现 IEEE。 |
| key 排序在解析期完成 | qjsonparser.cpp:416 | `std::stable_sort(...) + customAssigningUniqueLast(...)` | stable_sort 字典序 + 去重保最后值；sortContainer 仅 parseObject 收尾调（475），array 不排序。 |
| 非法 UTF-8 立即失败 | qjsonparser.cpp:819 | `if (!scanUtf8Char(...)) { lastError = IllegalUTF8String; return false; }` | 无 Latin-1 回退，与 RFC8259 一致。 |

## QJsonValue 类型映射

源码文件：`qtbase/src/corelib/serialization/qjsonvalue.h` / `qjsonvalue.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QJsonValue 持整个 QCborValue | qjsonvalue.h:133 | `QCborValue value;` + static_assert 二进制兼容 | 只是 QCborValue 的语义包装层，不做 tagged pointer。 |
| type() 合并 Integer→Double | qjsonvalue.cpp:32 | `case QCborValue::Double: case QCborValue::Integer: return QJsonValue::Double;` | 非 Qt6 独有：JSON 规范只有 number，QJsonValue since 5.0；内部 Integer 标签保 64 位精度。 |
| QJsonValue(double) 整数升级 | qjsonvalue.cpp:147 | `convertDoubleTo<qint64>(v, &n, false) ? return n : return v;` | false 不允许精度退化；既支持 53 位精度又支持 64 位整数。 |

## Writer 序列化

源码文件：`qtbase/src/corelib/serialization/qjsonwriter.cpp` / `qjsonwriter_p.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| toJson 委托 Writer | qjsondocument.cpp:247 | `QJsonPrivate::Value::fromTrustedCbor(d->value).toJson(format==Compact ? ...)` | d 为 null 返空 QByteArray；fromTrustedCbor 私有快捷路径免转换。 |
| Writer 三静态方法 | qjsonwriter_p.h:28 | `static void objectToJson/arrayToJson/valueToJson(..., int indent, bool compact)` | compact 是 Indented/Compact 运行时开关；indent 每级 4 空格。 |
| Compact/Indented 分流 | qjsonwriter.cpp:167 | `json += compact ? "\":" : "\": ";` + `QByteArray indentString(4*indent, ' ')` | 紧凑无换行，缩进加换行+前缀。 |

## QVariant 互转走 CBOR 中介

源码文件：`qtbase/src/corelib/serialization/qjsonobject.cpp` / `qjsonarray.cpp` / `qjsoncbor.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| toVariantMap 经 QCborMap | qjsonobject.cpp:188 | `return QCborMap::fromJsonObject(*this).toVariantMap();` | fromJsonObject 共享 QCborContainerPrivate 零拷贝。 |
| fromVariantMap 经 QCborMap | qjsonobject.cpp:176 | `return QJsonPrivate::Variant::toJsonObject(map);` | qjsoncbor.cpp:474 先 QCborMap::fromVariantMap 再 convertToJsonObject。 |

## CBOR 互转与扩展类型降级

源码文件：`qtbase/src/corelib/serialization/qjsoncbor.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 扩展类型降级为字符串 | qjsoncbor.cpp:173 | `switch(tag){ case Url: ... case DateTime/Base64/Uuid: return maybeEncodeTag(d);} return qt_convertToJson(d,1);` | Url→FullyEncoded、DateTime→RFC3339；未知 tag 忽略转内部值。 |
| NaN/Inf 降级为 null | qjsoncbor.cpp:29 | `return qt_is_finite(v) ? QJsonValue(v) : QJsonValue();` | 与 Writer 写出（qjsonwriter.cpp 非 finite 写 null）一致，源 RFC4627。 |

## COW 与 detach

源码文件：`qtbase/src/corelib/serialization/qcborvalue.cpp` / `qjsonobject.h` / `qjsonarray.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| detach 条件 ref!=1 clone | qcborvalue.cpp:971 | `if (!d \|\| d->ref.loadRelaxed() != 1) return clone(d, reserved);` | clone 拷贝 elements+data 并手动 ref 子容器；所有写操作前调。 |
| begin() 触发 detach | qjsonobject.h:283 | `inline iterator begin() { detach(); return iterator(this, 0); }` | 非 const 遍历即深拷贝；读访问用 constBegin/cbegin 避免。 |

## 死枚举与已删 API

源码文件：`qtbase/src/corelib/serialization/qjsonparseerror.h` / `qjsonparser.cpp` / `qjsondocument.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| ParseError 15 个枚举值 | qjsonparseerror.h:14 | `enum ParseError { NoError=0, ..., DeepNesting, DocumentTooLarge, GarbageAtEnd }; int offset = -1;` | offset 默认 -1；offset 是字节偏移（qjsonparser.cpp:86 文档）。 |
| DocumentTooLarge 死枚举 | qjsonparser.cpp:142 | errorString switch 有 case，但全文件无 `lastError = DocumentTooLarge` 赋值 | 保留为二进制兼容，永不触发。 |
| TerminationByNumber 6.9 不再返回 | qjsonparser.cpp:64 | `\value TerminationByNumber ... (as of 6.9, this is no longer returned)` | 文档明示，枚举保留。 |
| 二进制 API 已直接删除 | qjsondocument.h:29 | `static const uint BinaryFormatTag = ('q')\|('b'<<8)...;` 无 fromBinaryData/toBinaryData/fromRawData 声明 | 非 deprecated 是删除；只剩魔数常量，改用 QCborValue::toCbor/fromCbor。 |
