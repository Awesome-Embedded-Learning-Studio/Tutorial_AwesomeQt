---
title: QString 内存模型与编译期视图
description: QString 的类成员布局（无 SSO）、Data typedef/QStringPrivate 链、静态空对象 _empty、构造路径、容量管理（resize/reserve/squeeze）、QStringLiteral/_s/fromRawData 编译期视图、QStringView 不持有语义。
---

# QString 内存模型与编译期视图

> 本索引收录 Qt 6.9.1 源码中 QString 内存模型相关的已验证证据。COW 智能指针的通用机制（三成员、detach、needsDetach 短路、fromRawData 工厂）见 [QArrayDataPointer COW 智能指针](./qarraydatapointer.md)；数据头部字段与分配/增长链路见 [QArrayData 容器共享数据头部](./qarraydata.md)；原子引用计数见 [原子引用计数操作](./atomic-operations.md)。本文件只收 QString 自己的内存模型声明与字面量视图机制。

## 类成员布局——QString 没有 SSO

源码文件：`qtbase/src/corelib/text/qstring.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QString 类私有成员只有一个 DataPointer d + 静态 _empty，无内联 buffer | :1099-1100 | `DataPointer d; static const char16_t _empty;` | 全篇立论根基：QString 不走 std::string 式 SSO，所有字符数据要么在堆、要么指向字面量/静态 _empty，从不内联在对象里。 |
| QString 内部数据头 typedef Data = QTypedArrayData<char16_t> | :143 | `typedef QTypedArrayData<char16_t> Data;` | 头部指针的类型名。static_assert 保证 sizeof 不变。 |

源码文件：`qtbase/src/corelib/text/qstringliteral.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| DataPointer 真身是 QStringPrivate = QArrayDataPointer<char16_t> | :24 | `using QStringPrivate = QArrayDataPointer<char16_t>;` | typedef 链定义在此（不在 qstring.h），因 QArrayDataPointer 模板在更底层的 qarraydatapointer.h，头依赖顺序要求。QArrayDataPointer<char16_t> 三字段（d/ptr/size）见 qarraydatapointer.md。 |

## 静态空对象 _empty（Qt6 替代 Qt5 sharedNull）

源码文件：`qtbase/src/corelib/text/qstring.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| `_empty` 是单个 static char16_t（值=0），不是带头部的 sharedNull | :76 | `const char16_t QString::_empty = 0;` | Qt 5 有 QArrayData::sharedNull()（ref==-1 的静态头部），Qt 6.9.1 删了，改用最朴素的 char16_t 静态变量。空 QString 的 d 是 nullptr，ptr 借指 `_empty`。 |

源码文件：`qtbase/src/corelib/text/qstring.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 默认构造 constexpr 空体，零堆分配 | :1410 | `constexpr QString::QString() noexcept {}` | 依赖 DataPointer 默认构造把 d/ptr/size 全置 nullptr/0。连 _empty 都没指，首次 data()/unicode() 才回退 &_empty。这是「空视图」不是 SSO。 |

## 构造路径

源码文件：`qtbase/src/corelib/text/qstring.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QString(const QChar*, size) 三路径构造 | :2491-2504 | `if (!unicode) d.clear(); else if (!size) d = fromRawData(&_empty, 0); else { d = DataPointer(size, size); memcpy(...); d.data()[size] = '\0'; }` | 标准构造：空→空视图；非空→单次堆分配+memcpy+手写 null 终止符。注意 `\0` 由调用方写，不在 allocate 内自动加（PSV：指针可直传 C API）。 |

源码文件：`qtbase/src/corelib/text/qstring.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 拷贝构造浅拷贝 d(other.d)，O(1) | :1340-1341 | `QString(const QString &other) noexcept : d(other.d) { }` | ref 管理下沉到 QArrayDataPointer 拷贝构造（那里调 ref()）。 |

## 容量管理 resize/reserve/squeeze

源码文件：`qtbase/src/corelib/text/qstring.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| resize 调 reallocData(Grow) + 写 null 终止符 | :2664-2672 | `if (d->needsDetach() \|\| needsReallocate(*this, size)) reallocData(size, Grow); d.size = size; if (d->allocatedCapacity()) d.data()[size] = u'\0';` | Grow 模式预留增长空间。空串视图 alloc=0 时不写终止符（_empty 本身是 0）。 |

源码文件：`qtbase/src/corelib/text/qstring.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| reserve 用 KeepSize 扩张 + 置 CapacityReserved | :1413-1419 | `if (d.needsDetach() \|\| asize >= capacity() - d.freeSpaceAtBegin()) reallocData(qMax(asize, size()), KeepSize); if (d.constAllocatedCapacity()) d.setFlag(Data::CapacityReserved);` | KeepSize 不预留额外增长系数。CapacityReserved 标志让后续 detach 时保留预约容量不被截短。 |
| squeeze 收容量到 size + 清 CapacityReserved | :1421-1429 | `if (!d.isMutable()) return; if (d.needsDetach() \|\| size() < capacity()) reallocData(d.size, KeepSize); if (d.constAllocatedCapacity()) d.clearFlag(Data::CapacityReserved);` | reserve 的反操作。不可变对象（fromRawData 视图，isMutable()==false）直接 return。 |

> 容量增长策略（qNextPowerOfTwo 取 2 的幂）、单次连续 malloc、FooterSize 补偿、in-place reallocate 免拷贝，见 [QArrayData · 分配链路与增长策略](./qarraydata.md#分配链路与增长策略)。

## 编译期视图——QStringLiteral / _s / fromRawData

源码文件：`qtbase/src/corelib/text/qstringliteral.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QStringLiteral 不构造头部，是 {nullptr, literal, N-1} 编译期 fromRawData | :24-38 | `qMakeStringPrivate return { nullptr, str, N - 1 };` 宏展开 `QString(qMakeStringPrivate(QT_UNICODE_LITERAL(str)))` | Qt 5 用 Q_STATIC_ARRAY_DATA 在编译期构造 QArrayData 头部（ref=-1）；Qt 6.9.1 完全不构造头部，d=nullptr 无引用计数，ptr 直指 .rodata 字面量，零运行期分配。 |

源码文件：`qtbase/src/corelib/text/qstring.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| operator 字面量后缀 _s 与 QStringLiteral 同构 | :1731-1734 | `return QString(QStringPrivate(nullptr, const_cast<char16_t *>(str), qsizetype(size)));` | 两条字面量转 QString 路径（宏、UDL）走同一套 nullptr-head 模式。 |

源码文件：`qtbase/src/corelib/text/qstring.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QString::fromRawData 包外部指针为视图 | :9381-9384 | `return QString(DataPointer::fromRawData(const_cast<char16_t *>(...unicode), size));` | 不持有不拷贝。DataPointer::fromRawData 工厂（见 qarraydatapointer.md）返回 {nullptr, rawData, length}。 |

> fromRawData 视图触发 detach 的完整闭环（d==nullptr 时 needsDetach 的 `!d` 短路）见 [QArrayDataPointer · ref/deref 与 needsDetach](./qarraydatapointer.md#refderefnull-safety)。

## QStringView 不持有

源码文件：`qtbase/src/corelib/text/qstringview.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QStringView 只存 m_data + m_size 两字段 | :432-439 | `const storage_type *m_data = nullptr; qsizetype m_size = 0;`（Qt6 走 #else 分支 m_size 在前） | 纯只读视图，两 machine word，无 DataPointer/d 头部/引用计数。生命周期由调用方保证（类似 std::string_view）。 |
