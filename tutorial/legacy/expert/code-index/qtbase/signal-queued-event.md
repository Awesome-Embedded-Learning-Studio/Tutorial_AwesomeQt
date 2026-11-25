---
title: 排队事件全链——参数从深拷贝到销毁
description: queuedConnectionTypes 补查逐参数取 QMetaType、DIRECT_CONNECTION_ONLY 单个 static int 哨兵、QMetaCallEvent 的 Data 结构布局与 prealloc_ 三格小缓冲、queued_activate 组装（nargs 多 1 第 0 位返回槽）、接收线程 event case MetaCall 进 placeMetaCall 三路分流、semaphore 在基类析构放行、事件析构逐参数 destroy 闭环。
---

# 排队事件全链——参数从深拷贝到销毁

> 本索引收录 Qt 6.9.1 源码中排队连接参数全链路相关的已验证证据。对应专家篇《18 信号槽深度拆解》正文 queued 参数的完整旅程一节。02 篇的 queued_activate 分流证据见 [signal-dispatch-cleanup](./signal-dispatch-cleanup.md)，不重复收录。

## 类型表补查

源码文件：`qtbase/src/corelib/kernel/qobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| queuedConnectionTypes 逐参数取 QMetaType（非 parameterMetaTypes 复数版） | :76-103 | `static int *queuedConnectionTypes(const QMetaMethod &method) { const auto parameterCount = method.parameterCount(); int *typeIds = new int[parameterCount + 1]; ... parameterMetaType(i)` | parameterCount() 加循环单数版；指针参数映射 QMetaType::VoidStar。 |
| 拿不到 id 时 qCWarning 加 delete[] 返回 nullptr，成功末位补 0 终止符 | :91-99 | `qCWarning(...); delete[] typeIds; return nullptr;`（警告 91-94，delete 95，return 96，`typeIds[parameterCount] = 0` 在 99） | PMF 版连接的警告推迟到第一次 emit 补查时打，且只打一次。 |
| 按 QArgumentType 数组的同名重载 | :105-128 | `static int *queuedConnectionTypes(const QArgumentType *argumentTypes, int argc)` | 字符串版 connect 当场查类型走的是这一支。 |
| DIRECT_CONNECTION_ONLY 是单个 static int，值恒 0 拿地址当哨兵 | :56 | `static int DIRECT_CONNECTION_ONLY = 0;` | 不是数组。queued_activate 补查失败时 argumentTypes 指向它（3965-3966），命中直接 return（3973-3974）；~Connection 里 `if (v != &DIRECT_CONNECTION_ONLY) delete[] v`（1155-1156）免误删。 |

## QMetaCallEvent 布局（6.9 大改）

源码文件：`qtbase/src/corelib/kernel/qobject_p.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 成员全收在 Data 结构 d 里，无独立 types_ 成员 | :440-449 | `struct Data { QtPrivate::SlotObjUniquePtr slotObj_; void **args_; StaticMetaCallFunction callFunction_; int nargs_; ushort method_offset_; ushort method_relative_; } d;` | 常见资料的平铺布局在 6.9 不成立。 |
| QMetaType 数组内联排在 args_ 指针数组后 | :420-421 | types() 访问器从 `d.args_ + d.nargs_` 处 reinterpret_cast 取 | 与 args_ 同一块内存，省一次分配。 |
| prealloc_ 三格小缓冲免堆分配 | :449 | `alignas(void *) char prealloc_[3 * sizeof(void *) + 3 * sizeof(QMetaType)];` | 连返回槽在内不超过三格（两个参数以内）时参数缓冲不碰堆；allocArgs 按 `nargs_ * (sizeof(void*)+sizeof(QMetaType))` 超过 sizeof(prealloc_) 才 calloc（qobject.cpp:488-499）。 |
| semaphore_/sender_/signalId_ 在基类 QAbstractMetaCallEvent | :345-369 | 基类声明 345，semaphore_ 在 365，QMetaCallEvent 本类 369 | BlockingQueued 的放行时机跟着基类析构走。 |
| 6 个构造 + create<T...> 变参模板工厂 | qobject.cpp:507-598（构造）/:398-415（create） | create 用 QMetaType::fromType<Args>() 编译期取类型 | 编译期已知 QMetaType 免运行时查表。 |

## 组装与接收

源码文件：`qtbase/src/corelib/kernel/qobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| nargs 从 1 起数（第 0 位留给返回值），靠终止符 0 数出参数个数 | :3975-3977 | `int nargs = 1; // include return type / while (argumentTypes[nargs - 1]) ++nargs;` | 排队事件的参数表比信号参数多一个返回槽。 |
| 按 isSlotObject 二选一 new QMetaCallEvent，返回槽占位 types[0]=QMetaType()、args[0]=nullptr | :3989-3997 | `new QMetaCallEvent(...)` 两分支 | 槽对象版传 c->slotObj，方法版传 method_offset/method_relative/callFunction。 |
| types[n].create(argv[n]) 深拷贝参数，postEvent 收尾 | :3999-4012 | `args[n] = types[n].create(argv[n]); ... QCoreApplication::postEvent(receiver, ev);` | argv 活在发送线程栈上，排队异步必须拷贝。 |
| 接收线程入口：QObject::event case MetaCall 转 placeMetaCall | :1419-1433 | `case QEvent::MetaCall: { QAbstractMetaCallEvent *mce = ...; ... mce->placeMetaCall(this);` | 先懒加载 ConnectionData（1423-1428），再构造 QObjectPrivate::Sender 恢复 senderContext（1429）。 |
| placeMetaCall 三路分流：slotObj / callFunction（offset 未低于接收者层级，传相对索引）/ metacall（绝对索引） | :619-629 | `if (d.slotObj_) { d.slotObj_->call(...); } else if (d.callFunction_ && d.method_offset_ <= object->metaObject()->methodOffset()) { d.callFunction_(...); }` | 前两路绕开通用分发。 |
| BlockingQueued 的 semaphore 在基类析构释放 | :477-483 | `QAbstractMetaCallEvent::~QAbstractMetaCallEvent() ... if (semaphore_) semaphore_->release();`（480-481） | 槽跑完、事件析构时才放行发送线程，不在 placeMetaCall 里。 |
| 事件析构逐参数 destroy，`args_` 非 `prealloc_` 才 free | :603-615 | `for (int i = 0; i < d.nargs_; ++i) { if (t[i].isValid() && d.args_[i]) t[i].destroy(d.args_[i]); }` | queued_activate 里 create 的副本在接收线程析构销毁，生命周期闭环。 |
