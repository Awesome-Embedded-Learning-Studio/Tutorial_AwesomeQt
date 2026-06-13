---
title: QSharedData 用户自定义共享类路线
description: QSharedData、QSharedDataPointer、QExplicitlySharedDataPointer 的实现——用户自定义类加 COW 的完整路线。
---

# QSharedData 用户自定义共享类路线

> 本索引收录 Qt 6.9.1 源码中 QSharedData 体系相关的已验证证据。这条路线用于用户自定义类的隐式/显式共享，与内置容器的 QArrayData 路线独立。

## QSharedData（数据基类）

源码文件：`qtbase/src/corelib/tools/qshareddata.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| ref 成员是 mutable QAtomicInt | :22 | `mutable QAtomicInt ref;` | mutable 是必要的：const 拷贝也需要修改 ref。 |
| 构造函数 ref 初始化为 0 | :24 | `QSharedData() noexcept : ref(0) { }` | 创建时无指针持有，ref=0。被 QSharedDataPointer 接管后才变 1。与 QArrayData 的 ref_=1 不同。 |
| 拷贝构造也 ref=0 | :25 | `QSharedData(const QSharedData &) noexcept : ref(0) { }` | clone 出的新对象 ref 从 0 开始。 |
| 赋值运算符 = delete | :26 | `QSharedData &operator=(const QSharedData &) = delete;` | 防止意外覆盖引用计数。 |

## QSharedDataPointer（隐式共享智能指针）

源码文件：`qtbase/src/corelib/tools/qshareddata.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| detach 条件：d!=null && ref!=1 | :41 | `if (d && d->ref.loadRelaxed() != 1) detach_helper();` | 注意是 !=1 不是 >1，ref==0 也触发（QAdoptSharedDataTag 场景）。 |
| 拷贝构造：浅拷贝 + ref() | :66-67 | `d(o.d) { if (d) d->ref.ref(); }` | O(1)，与 QArrayDataPointer 对称。 |
| 析构：deref false 时 delete | :57 | `if (d && !d->ref.deref()) delete d.get();` | ref 从 1 减到 0 时释放。 |
| detach_helper：clone → ref → deref 旧 → reset | :243-250 | `T *x = clone(); x->ref.ref(); if (!d.get()->ref.deref()) delete d.get(); d.reset(x);` | clone()=new T(*d)，新对象 ref=0→ref()→1，旧 deref 可能释放。 |
| reset() 有自赋值保护 | :69-78 | `if (ptr != d.get()) { ... }` | ptr==d.get() 时跳过。 |
| operator= 通过 reset() 实现 | :80-84 | `reset(o.d.get());` | 自赋值时 reset 内部跳过。 |

## QExplicitlySharedDataPointer（显式共享）

源码文件：`qtbase/src/corelib/tools/qshareddata.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 非 const operator->() 不调用 detach() | :138-139 | `T *operator->() noexcept { return d.get(); }` | 「显式共享」的核心含义：拿到指针后不会自动深拷贝。 |
| 非 const operator*() 也不调用 detach() | :131 | `T &operator*() const { return *(d.get()); }` | 甚至标记为 const——不修改指针状态。 |

## QAdoptSharedDataTag（零开销所有权转移）

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 构造时不 ref，配合 take() 实现零原子操作转移 | :63-64 | `QSharedDataPointer(T *data, QAdoptSharedDataTag) noexcept : d(data) {}` | 同线程内转移所有权，避免不必要的原子操作。 |
