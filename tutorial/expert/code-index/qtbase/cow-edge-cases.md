---
title: COW 边界情况与对比
description: 空 QString 处理、fromRawData 视图、QVarLengthArray 对比、线程安全边界。
---

# COW 边界情况与对比

> 本索引收录 Qt 6.9.1 源码中 COW 机制的边界情况和对比类证据。

## 空 QString

源码文件：`qtbase/src/corelib/text/qstring.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 空 QString detach 时 alloc==0，回到 fromRawData(&_empty, 0) | :2783-2785 | `d = DataPointer::fromRawData(&_empty, 0);` | `_empty` 是静态 char16_t，所有空 QString 共享，ref=-1，永不释放。空字符串 detach 后仍然是空字符串。 |

## fromRawData

源码文件：`qtbase/src/corelib/tools/qarraydatapointer.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| fromRawData 创建 d=nullptr 的视图 | :63 | `static QArrayDataPointer fromRawData(...)` | 不拥有数据，非 const 操作触发完整深拷贝。 |

## QVarLengthArray（不用 COW 的容器）

源码文件：`qtbase/src/corelib/tools/qvarlengtharray.h`（1063 行）

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 完全不用 COW | 全文 | grep detach/ref/deref/QSharedData/QAtomicInt/isShared：无匹配 | 栈上预分配缓冲区，超出才堆分配。每次拷贝都是深拷贝。适合临时性、局部短数组。 |

## 线程安全边界

| 论点 | 依据 | 置信度 | 解读 |
|---|---|---|---|
| COW 保证「不同线程各自持有副本可安全并发读」 | 基于 ref/deref 原子性 + detach 后各持独立副本的推理 | 中（推理结论，非 Qt 显式声明） | Qt 标记隐式共享类为 reentrant 而非 thread-safe。同一对象的并发非 const 访问不安全。 |
