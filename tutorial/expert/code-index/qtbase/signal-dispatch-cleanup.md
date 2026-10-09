---
title: 连接分流、跨线程投递与析构清理
description: 四类连接发射期分流（Direct 快路径 / Auto+Queued 走 queued_activate 深拷贝 postEvent / BlockingQueued 信号量阻塞同线程死锁检测）；QMetaCallEvent 跨线程载体；~QObject 两类连接清理；重入保护两线 + Connection ref_=2 引用计数。
---

# 连接分流、跨线程投递与析构清理

> 本索引收录 Qt 6.9.1 源码中连接类型分流、跨线程调用与析构清理相关的已验证证据。对应专家篇《02 信号槽底层——activate 调用链》正文四类连接、QMetaCallEvent、析构清理、重入保护四节。Cannot queue 警告相关的三处引文出自正文风险与故障预防一节的第二类风险，按机制归入本组。

## Direct 快路径

源码文件：`qtbase/src/corelib/kernel/qobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| Direct 快路径：isSlotObject 直接 slotObj->call，否则 callFunction(InvokeMetaMethod) 同步调用 | :4141-4159 | `if (c->isSlotObject) { obj->call(receiver, argv); } else if (c->callFunction ...) { callFunction(receiver, QMetaObject::InvokeMetaMethod, method_relative, argv); }` | 同步直调，栈上 argv 无深拷贝；元方法分支与 metacall 汇合。 |

## Auto / Queued 走 queued_activate

源码文件：`qtbase/src/corelib/kernel/qobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| Auto 跨线程或显式 Queued 走 queued_activate（Auto 类型是发射期现判 receiverInSameThread） | :4102-4104 | `if ((c->connectionType == Qt::AutoConnection && !receiverInSameThread) \|\| (c->connectionType == Qt::QueuedConnection)) { queued_activate(...); }` | Auto 到底 Direct 还是 Queued 由发射那一刻线程关系决定，这就是「自动适应」。 |
| queued_activate new QMetaCallEvent + types[n].create 深拷贝参数 + postEvent 投递到 receiver 线程 | :3989-4020 | `QMetaCallEvent *ev = ... new QMetaCallEvent(...); for (int n = 1; n < nargs; ++n) args[n] = types[n].create(argv[n]); QCoreApplication::postEvent(receiver, ev);` | 深拷贝是因为 argv 在发送线程栈上，Queued 异步不能共享；自定义类型必须 qRegisterMetaType 否则 create 失败。 |

## Cannot queue 警告与两条触发路径（正文风险与故障预防一节）

源码文件：`qtbase/src/corelib/kernel/qobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 真实警告文本 Cannot queue arguments of type，经 qt.core.qobject.connect 分类输出 | :91-94 | `qCWarning(lcConnect, "QObject::connect: Cannot queue arguments of type '%s'\n(Make sure '%s' is registered using qRegisterMetaType().)", ...)` | 6.9.1 里不存在 QMetaType::create: error copying type 的老说法；失败模式只有槽不触发，不会收到垃圾数据。 |
| 字符串版 connect 当场查 queuedConnectionTypes，查不到打警告并返回空连接 | :3160-3162 | `if ((type == Qt::QueuedConnection) && !(types = queuedConnectionTypes(signal))) return QMetaObject::Connection(nullptr);` | 字符串版（SIGNAL()/SLOT() 宏）connect 时就失败，连接压根建立不起来。 |
| queued_activate 进门补查 argumentTypes，补不出换成 DIRECT_CONNECTION_ONLY 直接 return | :3961-3974 | `argumentTypes = queuedConnectionTypes(m); if (!argumentTypes) argumentTypes = &DIRECT_CONNECTION_ONLY; ... if (argumentTypes == &DIRECT_CONNECTION_ONLY) return;` | PMF 版连接从此每次 emit 被静默跳过；警告只在第一次 emit 打一次（结果被缓存）。 |

## BlockingQueued 死锁检测

源码文件：`qtbase/src/corelib/kernel/qobject.cpp` / `qobject_p.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| BlockingQueued 同线程 qWarning 报死锁（Dead lock detected: Sender is %s(%p), receiver is %s(%p)） | qobject.cpp:4107-4131 | `if (receiverInSameThread) { qWarning("...Dead lock detected... Sender is %s(%p), receiver is %s(%p)", ...); } QSemaphore semaphore; ... ev = new QMetaCallEvent(..., &semaphore); postEvent(receiver, ev); semaphore.acquire();` | 同线程只 warning 不阻止，仍会死锁（事件投到当前队列但线程阻塞在 acquire）；跨线程靠信号量同步等槽执行完。 |
| BlockingQueued 的 argv 不深拷贝，注释声明 args always owned by caller | qobject_p.h:372 | `// blocking queued with semaphore - args always owned by caller` | sender 阻塞到槽返回、栈帧不退，参数全程活着，拷贝是白费功夫；与 Queued 必须深拷贝正相反。 |

## QMetaCallEvent 跨线程载体

源码文件：`qtbase/src/corelib/kernel/qobject_p.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QMetaCallEvent 封装槽调用全部信息（method_offset_+method_relative_、slotObj/callFunction、args_），核心方法 placeMetaCall | :369-423 | `class QMetaCallEvent ... { QMetaCallEvent(ushort method_offset, ushort method_relative, ...) ... inline int id() const { return d.method_offset_ + d.method_relative_; } virtual void placeMetaCall(QObject *object) override; }` | receiver 线程事件循环取出事件后调 placeMetaCall 执行槽；id() 拼出槽全局索引。 |

## ~QObject 两类连接清理

源码文件：`qtbase/src/corelib/kernel/qobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| ~QObject 第一类清理：遍历 signal=-1..receiverCount 逐信号 removeConnection 自己发出的连接 | :1060-1074 | `for (int signal = -1; signal < receiverCount; ++signal) { ... while (Connection *c = connectionList.first.loadRelaxed()) { cd->removeConnection(c); } }` | 清「作为 sender」的信号链；-1 是「所有信号」桶。 |
| ~QObject 第二类清理：遍历 cd->senders 反向链，靠 node->sender 定位对方并 senderData->removeConnection | :1077-1105 | `/* Disconnect all senders: */ while (Connection *node = cd->senders) { QObject *sender = node->sender; ... senderData->removeConnection(node); }` | 清「作为 receiver」的 senders 链；靠 node->sender 反查对方连接表。 |

## 重入保护两线 + Connection 引用计数

源码文件：`qtbase/src/corelib/kernel/qobject.cpp` / `qobject_p_p.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 重入保护第一线：doActivate 拍 highestConnectionId 快照，while 条件的 id 检查拦下发射期新增（c->id<=highestConnectionId） | qobject.cpp:4073-4075,4178 | `uint highestConnectionId = connections->currentConnectionId.loadRelaxed();`（声明 4073-4075，id 检查的那一行在 4178，相隔约百行） | 发射期新 connect 的连接 id 必然大于快照，本轮跳过；声明与检查是分开两处。 |
| 重入保护第二线：析构置 currentConnectionId=0 哨兵，激活循环检测到 0 设 senderDeleted 跳过 orphan 清理 | qobject.cpp:1133,4184-4188 | `cd->currentConnectionId.storeRelaxed(0);` | 防止发射途中对象析构导致碰已残缺的连接表。 |
| Connection ref_ 初值 2（内部表+句柄各一份），removeConnection 摘链不释放，靠 deref 归零才 delete | qobject_p_p.h:85-118 | `QAtomicInt ref_{ 2 }; ... void deref() { if (!ref_.deref()) { ...; delete this; } }` | 句柄持有不等于连接有效——disconnect 摘链后 ref_ 减到 1，句柄还在节点不释放但连接已断。 |
