---
title: 原子引用计数操作
description: QBasicAtomicInt、QtPrivate::RefCount 的 ref/deref 实现，memory ordering，静态对象标记（-1）。
---

# 原子引用计数操作

> 本索引收录 Qt 6.9.1 源码中与原子引用计数相关的已验证证据。按源码文件组织，供读者对照源码自行验证，也供后续专家篇引用复用。

## QBasicAtomicInteger（底层原子操作）

源码文件：`qtbase/src/corelib/thread/qatomic_cxx11.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| ref() 用 fetch_add(1, memory_order_acq_rel)，返回旧值 != -1 | :259 | `return _q_value.fetch_add(1, std::memory_order_acq_rel) != T(-1);` | 原子 +1。acq_rel 语义保证可见性。返回 true 表示 ref 成功（非静态对象）。 |
| deref() 用 fetch_sub(1, memory_order_acq_rel)，返回旧值 != 1 | :266 | `return _q_value.fetch_sub(1, std::memory_order_acq_rel) != T(1);` | 原子 -1。旧值==1 时返回 false，表示「最后一个引用，该释放了」。 |
| QBasicAtomicInteger 不可拷贝（拷贝构造和赋值 = delete） | `qbasicatomic.h`:154-156 | `QBasicAtomicInteger(const QBasicAtomicInteger &) = delete;` | 防止原子变量被意外拷贝导致引用计数丢失。 |

## QtPrivate::RefCount（高层封装）

源码文件：`qtbase/src/corelib/tools/qrefcount.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| ref() 先检查 count != -1，静态对象跳过递增，始终返回 true | :18-23 | `int count = atomic.loadRelaxed(); if (count != -1) atomic.ref(); return true;` | loadRelaxed + ref 两步之间不是原子 CAS，但静态对象不会运行期变化，所以安全。 |
| deref() 对静态对象（count == -1）返回 true，非静态调 atomic.deref() | :25-30 | `if (count == -1) return true; return atomic.deref();` | 静态对象永不释放。非静态走 fetch_sub。 |
| isShared() 条件：count != 1 && count != 0 | :38-42 | `return (count != 1) && (count != 0);` | count==1 独占，count==0 未被指针接管，两者都不算「共享中」。 |

## 线程安全边界

| 论点 | 依据 | 解读 |
|---|---|---|
| ref/deref 本身线程安全 | fetch_add/fetch_sub 用 memory_order_acq_rel | 原子操作，多线程同时做引用计数不会出问题。 |
| needsDetach() 用 loadRelaxed()，不保证最新值 | qarraydata.h:71,79 | 刻意选择：最坏多一次不必要的 detach，不会数据竞争。 |
