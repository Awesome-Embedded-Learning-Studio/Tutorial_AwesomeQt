---
title: QArrayData 容器共享数据头部
description: QArrayData 的字段结构、ref_ 初始值、isShared() 与 needsDetach() 的判断差异、fromRawData 的 d=nullptr 视图。
---

# QArrayData 容器共享数据头部

> 本索引收录 Qt 6.9.1 源码中 QArrayData 相关的已验证证据。QArrayData 是 QString/QByteArray/QList 共享数据的头部结构。

## 字段结构

源码文件：`qtbase/src/corelib/tools/qarraydata.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 三个关键字段：QBasicAtomicInt ref_、ArrayOptions flags、qsizetype alloc | :42-44 | `QBasicAtomicInt ref_; ArrayOptions flags; qsizetype alloc;` | ref_ 是原子引用计数，flags 包含 CapacityReserved 等，alloc 是以元素个数为单位的已分配容量。 |

## ref/deref 接口

源码文件：`qtbase/src/corelib/tools/qarraydata.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| ref() 调用 ref_.ref() 并始终返回 true | :57-61 | `ref_.ref(); return true;` | 返回值始终 true，无实际用途（历史遗留 API）。 |
| deref() 返回 ref_.deref()，false 表示需要释放 | :64-67 | `return ref_.deref();` | false = ref_ 从 1 减到 0，最后一个引用，该释放了。 |

## 判断函数

源码文件：`qtbase/src/corelib/tools/qarraydata.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| isShared() 用 ref_.loadRelaxed() != 1 | :69-72 | `return ref_.loadRelaxed() != 1;` | ref_==0（fromRawData）也返回 true。 |
| needsDetach() 用 ref_.loadRelaxed() > 1 | :77-80 | `return ref_.loadRelaxed() > 1;` | ref_==0 不触发 detach（fromRawData 由上层 QArrayDataPointer 的 !d 判断处理）。标记为非 const——「问就是打算改」。 |
| 两者差异：ref_==0 时 isShared()=true 但 needsDetach()=false | :69-80 | 见上 | fromRawData 创建的视图没有自己的 QArrayData 头部，d=nullptr。 |

## 分配与初始值

源码文件：`qtbase/src/corelib/tools/qarraydata.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| allocate() 中 ref_ 初始化为 1（storeRelaxed） | :141 | `header->ref_.storeRelaxed(1);` | 分配即持有，ref_=1。与 QSharedData 的 ref=0 不同（两套体系）。 |
