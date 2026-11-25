---
title: Qt 智能指针家族源码索引
description: QSharedPointer 的 ExternalRefCountData 双计数（强+弱）、QWeakPointer 不阻析构与 weak→strong 升级、自定义 deleter 子类存储、QEnableSharedFromThis 回填、QPointer Qt6 实现（委托 QWeakPointer + strongref=-1 哨兵）、QScopedPointer 栈上 RAII、Qt6 deprecated 态度。
---

# Qt 智能指针家族源码索引

> 本索引收录 Qt 6.9.1 源码中 Qt 智能指针家族相关的已验证证据。QSharedData 那条「ref 长在数据类里」的内部引用计数路线另见 [QSharedData 用户自定义共享类路线](./qshareddata.md)（含 QSharedDataPointer/QExplicitlySharedDataPointer 的 detach 差异）；原子操作底层见 [原子引用计数操作](./atomic-operations.md)。本文件只收 QSharedPointer / QWeakPointer / QScopedPointer / QPointer 这条「ref 在外部 refcount 块」的路线。

## ExternalRefCountData——双计数地基

源码文件：`qtbase/src/corelib/tools/qsharedpointer_impl.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| ExternalRefCountData 同时持有 weakref/strongref 两计数 + destroyer 函数指针 | :108-113 | `struct ExternalRefCountData { ... QBasicAtomicInt weakref; QBasicAtomicInt strongref; DestroyerFn destroyer; };` | refcount 块本身是「ref-counted ref-counter」：strongref 管对象寿命、weakref 管 refcount 块自身寿命。是整个家族的地基。 |
| 构造时 strongref=1、weakref=1 | :115-120 | `strongref.storeRelaxed(1); weakref.storeRelaxed(1);` | weakref 多一个兜底——只要还有强引用，块至少被一个 weakref 持有，不会被提前 delete。 |
| 强引用 deref 分阶段：strongref 归零 destroy 对象，weakref 归零 delete 块 | :511-519 | `if (!dd->strongref.deref()) dd->destroy(); if (!dd->weakref.deref()) delete dd;` | 两阶段析构：对象先死、refcount 块后死（weakref 可能仍被 QWeakPointer 持有）。destroy() 仅调 destroyer 函数指针，无复活机制。 |
| ~ExternalRefCountData 断言 weakref==0 且 strongref<=0 | :122 | `Q_ASSERT(!weakref.loadRelaxed()); Q_ASSERT(strongref.loadRelaxed() <= 0);` | 确认块删除的两个前置条件。 |
| strongref/weakref 是 QBasicAtomicInt，不经 RefCount 层 | :111-112 | `QBasicAtomicInt weakref; QBasicAtomicInt strongref;` | 平台原生原子操作；QSharedPointer 计数不经 qrefcount.h 的 -1 静态包装（与 QSharedData::ref 的裸 QAtomicInt、QArrayData 经 RefCount 是三套不同实现）。 |

## QWeakPointer——只动 weakref、不阻析构

源码文件：`qtbase/src/corelib/tools/qsharedpointer_impl.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 析构/拷贝只 ref/deref weakref、不碰 strongref | :618-622, :671-672 | `~QWeakPointer() { if (d && !d->weakref.deref()) delete d; }` / 拷贝 `{ if (d) d->weakref.ref(); }` | 与 QSharedPointer 的 `d->weakref.ref(); d->strongref.ref();` 对位——弱引用加减完全不动 strongref，故不阻对象析构。 |
| weak→strong 升级：toStrongRef/lock 委托构造，CAS 实际在 internalSet | :558-584, :707-709 | `while (tmp > 0) { if (o->strongref.testAndSetRelaxed(tmp, tmp+1)) break; ... } if (tmp > 0) o->weakref.ref(); else o = nullptr;` | CAS 循环只在 strongref>0 时 +1 升级；对象已析构（strongref≤0）则失败、value 置 null。toStrongRef/lock 本体仅一行委托。 |

## 自定义 deleter——子类附加存储

源码文件：`qtbase/src/corelib/tools/qsharedpointer_impl.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| ExternalRefCountWithCustomDeleter 子类把用户 deleter + 指针附加在 refcount 块后 | :147-208 | `struct ExternalRefCountWithCustomDeleter: public ExternalRefCountData { CustomDeleter<T,Deleter> extra; ... static Self *create(...) { Self *d = ::operator new(sizeof(Self)); new (&d->extra) CustomDeleter(...); new (d) BaseClass(actualDeleter); } };` | 基类只存 destroyer 函数指针（指静态 deleter），真正用户 deleter 走子类 extra 成员；sizeof 随 Deleter 类型动态，一次 ::operator new 分配。 |

## QEnableSharedFromThis——sharedFromThis 回填

源码文件：`qtbase/src/corelib/tools/qsharedpointer_impl.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 对象持 mutable QWeakPointer weakPointer，构造时回填 | :522-527, :811-831 | `class QEnableSharedFromThis { ... QSharedPointer<T> sharedFromThis() { return QSharedPointer<T>(weakPointer); } ... mutable QWeakPointer<T> weakPointer; };` | QSharedPointer 构造经 enableSharedFromThis 回填该 weak 成员，使对象能安全拿到指向自己的 shared，避免裸 this 再造 QSharedPointer 导致双计数。 |

## QPointer——Qt6 实现（委托 QWeakPointer + strongref=-1 哨兵）

源码文件：`qtbase/src/corelib/kernel/qpointer.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QPointer 是独立模板类（非 typedef），内部唯一数据成员是 QWeakPointer | :17-37, :74-83 | `template <class T> class QPointer { QWeakPointer<QObjectType> wp; public: QPointer(T *p) : wp(p, true) { } T* data() const { return wp.internalData(); } };` | Qt5 是 typedef QWeakPointer<QObject>，Qt6 改独立模板类但本质仍是 QWeakPointer 薄封装。wp(p, true) 走 QWeakPointer 私有构造（friend）。 |

源码文件：`qtbase/src/corelib/tools/qsharedpointer.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| getAndRef 在 QObjectPrivate::sharedRefcount 挂块，strongref=-1 哨兵、weakref=2 | :1528-1556, :331-334 | `x->strongref.storeRelaxed(-1); x->weakref.storeRelaxed(2);  // the QWeakPointer + the QObject itself` | strongref=-1 表示「QObject 不被共享、仅跟踪寿命」。这正是 QPointer 不能 toStrongRef 的根源——internalSet 的 tmp>0 守卫不满足，checkQObjectShared 会 qWarning 拒绝。QObjectPrivate::sharedRefcount 是 QAtomicPointer（qobject_p.h:216）。 |

## QScopedPointer——栈上独占 RAII

源码文件：`qtbase/src/corelib/tools/qscopedpointer.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 模板<T, Cleanup>，析构调 Cleanup::cleanup，禁拷贝移动 | :69-82, :14-30 | `class QScopedPointer { ... ~QScopedPointer() { Cleanup::cleanup(this->d); } Q_DISABLE_COPY_MOVE(QScopedPointer) };` | 对位 std::unique_ptr。默认 Cleanup=QScopedPointerDeleter（delete）。 |
| QScopedArrayPointer 数组特化，delete[] + operator[] | :190-215, :32-49 | `class QScopedArrayPointer : public QScopedPointer<T, Cleanup> { T& operator[](qsizetype i) { return this->d[i]; } };` | 默认 Cleanup=QScopedPointerArrayDeleter（delete[]）。对位 std::unique_ptr<T[]>。 |

## Qt6 deprecated 态度

源码文件：`qtbase/src/corelib/tools/qscopedpointer.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 仅 take()（6.1）/swap()（6.2）成员被 deprecated，类本身未整体 deprecated | :128-143, :177-182, :217-221, :239-243 | `QT_DEPRECATED_VERSION_X_6_1("Use std::unique_ptr instead, and call release().") T *take()` / `QT_DEPRECATED_VERSION_X_6_2(...) void swap(...)` | 只有这两个成员函数触发警告，构造/析构/operator*/data/reset 都无警告。类未整体 deprecated。 |
| QSharedPointer 全文无 deprecated 标记 | qsharedpointer.h 全文 | grep `QT_DEPRECATED` 零结果 | QSharedPointer 仍是官方一等公民，且提供 std::shared_ptr 兼容 API（lock/owner_before/owner_equal/owner_hash/qobject_pointer_cast）。 |
