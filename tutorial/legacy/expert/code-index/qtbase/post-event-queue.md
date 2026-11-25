---
title: postEventList 内部——状态字段、二分插入与防重入
description: QPostEvent 仅 receiver/event/priority 三字段（marker 不存在）、QPostEventList 是 QList 子类带 recursion/startOffset/insertionOffset 三状态字段与内嵌 mutex、append/insert 私有化强制 addEvent、addEvent 队尾快路径加 upper_bound 二分（同优先级 FIFO 由等值区段后插保证）、防重入靠派发前置空 event 加 insertionOffset 栅栏、全局路径 i 别名 startOffset 与 CleanUp 批量 erase、removePostedEvents 派发中只置空不缩表、lockThreadPostEventList 随 moveToThread 重试。
---

# postEventList 内部——状态字段、二分插入与防重入

> 本索引收录 Qt 6.9.1 源码中投递事件队列内部机制相关的已验证证据。对应专家篇《20 事件循环源码》正文 postEventList 一节。07 篇的 postEvent 入队与 sendPostedEvents 派发主线见 [event-dispatch](./event-dispatch.md)。

## 结构与插入

源码文件：`qtbase/src/corelib/thread/qthread_p.h` / `qthread.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QPostEvent 三字段，无 marker | qthread_p.h:40-58 | `QObject *receiver; QEvent *event; int priority;` + operator< | 旧文献的 marker 字段 6.9 不存在；operator< 按 priority 反向比较支撑降序。 |
| QPostEventList：QList 子类 + recursion/startOffset/insertionOffset + 内嵌 QMutex | qthread_p.h:60-80 | `// recursion == recursion count for sendPostedEvents() / qsizetype startOffset; qsizetype insertionOffset; QMutex mutex;` | append/insert 用 using 藏进 private 强制走 addEvent；注释明说表必须按优先级有序。 |
| addEvent：队尾追加快路径 + std::upper_bound 二分插入 | qthread.cpp:22-38 | `if (isEmpty() \|\| constLast().priority >= priority \|\| insertionOffset >= size()) { append(ev); } else { ... std::upper_bound(begin() + insertionOffset, end(), ev); insert(at, ev); }` | 同优先级 FIFO 由 upper_bound 等值区段后插语义保证；搜索起点跳过本轮已派发区段。 |

## 派发防重入与清理

源码文件：`qtbase/src/corelib/kernel/qcoreapplication.cpp` / `qobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 派发前把 pe.event 置 nullptr，重入者见 null 即跳过 | :1809-1818 | `const_cast<QPostEvent &>(pe).event = nullptr;` + avoid live-lock 注释与 insertionOffset 栅栏 break | 防重入与防活锁的核心；recursion 计数配套。 |
| 只有全局路径 i 才别名 startOffset，CleanUp 析构一次 erase 复位 | :1792-1804 | `qsizetype &i = (!event_type && !receiver) ? data->postEventList.startOffset : startOffset;` + 析构 erase | 带过滤参数的调用用局部副本，不缩表仅靠置空失效。 |
| removePostedEvents：派发中只置空不缩表，无人派发才压缩加截断（B 修正口径） | :1926-1940 | 匹配项置空收集；`!recursion` 分支下才 qSwap 压缩与 erase | 保迭代不变量；QObject 析构调用点 qobject.cpp:203-204（postedEvents 非零才清）。 |
| lockThreadPostEventList 按 receiver 的 threadData 加锁并随 moveToThread 重试 | :1565-1595 | 循环重试跟随对象迁移 | 对象换线程期间投递不丢锁。 |
