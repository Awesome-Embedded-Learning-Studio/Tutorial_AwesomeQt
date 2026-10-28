---
title: 槽对象——函数指针顶替虚函数表
description: QSlotObjectBase 一个函数指针加原子计数非虚设计（源码注释亲述动机）、Operation 三操作加 NumOperations 哨兵、QCallableObject 接 QFunctorSlotObject 的班（Compare 只认成员函数指针，lambda 落 Q_FALLTHROUGH）、connectImpl 与字符串版两条 UniqueConnection 判重路径、~Connection 经 destroyIfLastRef 销毁槽对象、SlotObjectGuard 发射期持引用。
---

# 槽对象——函数指针顶替虚函数表

> 本索引收录 Qt 6.9.1 源码中槽对象系统相关的已验证证据。对应专家篇《18 信号槽深度拆解》正文槽对象一节。

## QSlotObjectBase 与继任者

源码文件：`qtbase/src/corelib/kernel/qobjectdefs_impl.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 非虚设计动机（源码注释亲述） | :418-427 | `// Don't use virtual functions here; we don't want the compiler to create tons of per-polymorphic-class stuff that we'll never need. We just use one function pointer, and the Operations enum below` | 成员 QAtomicInt m_ref = 1 加 const ImplFn m_impl（425-427）；析构 protected 非虚，销毁统一走 destroyIfLastRef。 |
| Operation 枚举：Destroy/Call/Compare 加 NumOperations 哨兵 | :436-442 | `enum Operation { Destroy, Call, Compare, NumOperations };` | 真实操作 3 个，对应 destroyIfLastRef/call/compare 三个内联封装。 |
| QFunctorSlotObject 已死，继任 QCallableObject | :522-555 | `class QCallableObject : public QSlotObjectBase, private CompactStorage<Functor>` | lambda/仿函数路径的当代类名，全库搜不到 QFunctorSlotObject。 |
| impl 静态函数 switch 分发，Destroy 直接 delete | :530-548 | `switch (which) { case Destroy: delete that; break; case Call: ...` | 虚函数调用换成函数指针加 switch；Qt7 分支签名调整是为尾调用优化（535 行注释）。 |
| Compare 只支持成员函数指针，非 PMF 落 Q_FALLTHROUGH | :550-555 | `case Compare: ... Q_FALLTHROUGH;`（554 行注释 not implemented otherwise） | lambda 配 Qt::UniqueConnection 静默失效的源码根源。 |

## 判重与销毁

源码文件：`qtbase/src/corelib/kernel/qobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| connectImpl 的 unique 分支：receiver 相同且 slotObj->compare(slot) 即拒绝 | :5298-5310 | `if (c2->receiver.loadRelaxed() == receiver && c2->isSlotObject && c2->slotObj->compare(slot)) return QMetaObject::Connection();` | compare 即 m_impl(Compare,...)；空 slot 加 unique 在 5287-5290 显式警告拒绝，两层机制分开。 |
| 字符串版 connect 的 unique 判重是另一段独立代码 | :3617-3628 | `!c2->isSlotObject && ... c2->method() == method_index_absolute` | 按绝对方法索引比，不涉及槽对象。没有叫 ensureUniqueConnection 的函数，两条路径都是就地内联遍历。 |
| ~Connection 经 destroyIfLastRef 销毁槽对象 | :1151-1159 | `if (isSlotObject) slotObj->destroyIfLastRef();` | 节点真正析构要等孤儿链清理，槽对象存活期覆盖所有在途发射。 |
| SlotObjectGuard 在 queued 路径与 lambda 发射路径持槽对象引用 | :3932-3950 / :3986 | `SlotObjectGuard obj{c->slotObj};` 一带 | 发射途中连接被断，槽对象也不提前消失。 |
