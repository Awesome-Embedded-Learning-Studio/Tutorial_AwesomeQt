---
title: QVariant 类型擦除与 QMetaType 源码索引
description: QVariant 的 packedType 位域类型擦除、SSO-like 存储（3*sizeof(void*) 内联阈值）、QMetaTypeInterface 虚表、QMetaType 句柄与延迟注册、setValue/value 双路、函数指针短路、convert 三层 fallback、Qt6 完全基于 QMetaType 重构、isNull/equals 两处 Qt5→Qt6 breaking、拷贝三路（PrivateShared COW）。
---

# QVariant 类型擦除与 QMetaType 源码索引

> 本索引收录 Qt 6.9.1 源码中 QVariant + QMetaType 的已验证证据。

## 类型擦除（packedType 位域）

源码文件：`qtbase/src/corelib/kernel/qvariant.h` / `qvariant_p.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| packedType 位域右移 2 位存 iface 指针 | qvariant.h:118, qvariant_p.h:77-79 | `quintptr packedType : sizeof(QMetaType)*8 - 2` / 构造 `packedType(quintptr(iface) >> 2)` / `Q_ASSERT((quintptr(iface) & 0x3) == 0)` | 依赖 iface 4 字节对齐（低 2 位为 0），右移 2 位省 2 bit；typeInterface() 左移 2 位还原。无模板参数，靠运行时 iface 知类型。 |

## SSO-like 存储

源码文件：`qtbase/src/corelib/kernel/qvariant.h` / `qvariant_p.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| Data 联合体 + 内联阈值 3*sizeof(void*) | qvariant.h:99-125 | `MaxInternalSize = 3*sizeof(void*)` / `union { uchar data[MaxInternalSize]; PrivateShared *shared; double _forAlignment; }` | 64 位=24 字节。double 成员强制 32 位系统也 8 字节对齐。 |
| canUseInternalSpace 三条件 | qvariant.h:103-108 | `RelocatableType && size<=MaxInternalSize && alignof(T)<=alignof(double)=8` | 三条满足 placement-new 到 data.data，否则 customConstructShared 堆分配 + is_shared。 |

## QMetaTypeInterface 虚表 + QMetaType 句柄

源码文件：`qtbase/src/corelib/kernel/qmetatype.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QMetaTypeInterface 是类型虚表 | :270-312 | `size/alignment/flags/typeId + defaultCtr/copyCtr/moveCtr/dtor/equals/lessThan/debugStream/dataStream` 函数指针 | 注意无 canConvert/convert 字段（转换走全局注册表）。 |
| QMetaType 是 iface 轻量句柄 | :338-480 | `QMetaType(const QMetaTypeInterface *d) : d_ptr(d) {}` / `registerType/id` 延迟注册 | 只持 d_ptr 指针拷贝廉价。首次 id() 才进全局表。 |

## setValue/value 双路 + 函数指针短路

源码文件：`qtbase/src/corelib/kernel/qvariant.h` / `qmetatype_p.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| setValue fast path / value fast path | qvariant.h:491-503, :752-767 | setValue: `isDetached && type 匹配 → in-place assign`，否则 fromValue / value: `type 匹配 → static_cast`，否则 convert | fast path 零开销，slow path 走 construct/convert。 |
| construct 函数指针短路 | qmetatype_p.h:166-206 | `defaultCtr 为 null → memset` / `copyCtr 为 null → memcpy` | trivially copyable 类型绕过函数指针，Qt 内置类型快的根源。 |

## convert 三层 fallback

源码文件：`qtbase/src/corelib/kernel/qmetatype.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| canConvert/convert 三层 | :2390-2451, :2611-2672 | moduleHelper（内置转换表）→ customTypesConversionRegistry（用户注册）→ 特殊路径（枚举/容器/MetaObject） | canConvert 用 nullptr 探测不实际转换。 |

## Qt6 完全基于 QMetaType + 两处 breaking

源码文件：`qtbase/src/corelib/kernel/qvariant.h` / `qvariant.cpp` / `qmetatype.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QVariant::Type 全 deprecated | qvariant.h:141-216, :687 | `QT_DEPRECATED_VERSION_X_6_0("Use QMetaType::Type instead.") enum Type` / `QVariant(QMetaType::Type) = delete` | Qt6 臣服 QMetaType。枚举值别名到 QMetaType::Xxx。 |
| 【Qt6 breaking】isNull 不再调类型 isNull() | qvariant.cpp:2529-2547 | `if (d.is_null \|\| !metaType().isValid()) return true; if (flags & IsPointer) return get<void*>()==nullptr; return false;` + 注释 `This behavior has been changed from Qt 5` | Qt5 时空 QString/QDate 等 isNull() 返 true；Qt6 不再代理，breaking。 |
| 【Qt6 breaking】equals 类型不同不再通用 convert | qvariant.cpp:2405-2426 | 类型不同 → 仅 `canBeNumericallyCompared→numericCompare` 和 `qvCanConvertMetaObject→pointerCompare`，否则 false | Qt5 会 convert 后比；Qt6 只数值/指针特殊比较，breaking。 |
| fromValue 强制 registerType | qvariant.h:552, qvariant.cpp:2753 | `mt.registerType();` 注释 `we want the type stored in QVariant to always be registered` | 保证 typeId() 可用。 |

## 拷贝三路（PrivateShared COW）

源码文件：`qtbase/src/corelib/kernel/qvariant.cpp` / `qvariant.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| clonePrivate 三路 | qvariant.cpp:302-322 | `is_shared → ref.ref()`（COW）/ 小 trivially 隐式 memcpy / 小非 trivially copyConstruct | 大类型用 QVariant::PrivateShared（alignas(8) QAtomicInt ref，非 QSharedData）；小类型不走 COW 每次 copy construct。 |
