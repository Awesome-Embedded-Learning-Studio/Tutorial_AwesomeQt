---
title: 对象树析构级联与 deleteLater
description: ~QObject 尾部先 deleteChildren() 删子再 setParent_helper(nullptr) 摘己（子先于父）；deleteChildren 故意不用 qDeleteAll 而手写索引 for 循环（子析构可能删兄弟致 double-free），循环体先置 children[i]=nullptr 再 delete 防重入；deleteLater 去重(deleteLaterCalled)+postEvent(QDeferredDeleteEvent)；DeferredDelete 三种时机判定不满足则 re-post 不丢失。
---

# 对象树析构级联与 deleteLater

> 本索引收录 Qt 6.9.1 源码中析构级联与延迟删除相关的已验证证据。对应专家篇《21 对象树与所有权源码拆解》§3.2-§3.4。

## ~QObject 析构顺序

源码文件：`qtbase/src/corelib/kernel/qobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| ~QObject 尾部先 deleteChildren() 删子（子先于父销毁），再 setParent_helper(nullptr) 从父 children 摘己 | :1139-1148 | `if (!d->children.isEmpty()) d->deleteChildren(); ... if (d->parent) d->setParent_helper(nullptr);` | 子先于父销毁保证不留孤儿；自己从父摘除放最后。 |

## deleteChildren 手写循环防 double-free

源码文件：`qtbase/src/corelib/kernel/qobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| deleteChildren 故意不用 qDeleteAll 而手写索引 for 循环——子的析构可能 delete 兄弟，qDeleteAll 会 double-free | :2213-2228 | `// don't use qDeleteAll as the destructor of the child might delete siblings / for (int i = 0; i < children.size(); ++i) { currentChildBeingDeleted = children.at(i); children[i] = nullptr; delete currentChildBeingDeleted; }` | isDeletingChildren 标志+Q_ASSERT 防递归。 |
| deleteChildren 循环体先把 children[i] 置 nullptr 再 delete——子析构反向破坏父 children 列表时安全跳过 | :2213-2228 | `currentChildBeingDeleted = children.at(i); children[i] = nullptr; delete currentChildBeingDeleted;` | 子析构调 setParent_helper(nullptr) 想摘己，置 nullptr 后摘除逻辑识别「正在被父删」安全跳过——析构期重入保护。 |

## deleteLater 投递 DeferredDelete

源码文件：`qtbase/src/corelib/kernel/qobject.cpp` / `qcoreapplication.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| deleteLater 在 post event list mutex 保护下去重读 deleteLaterCalled（已置位直接 return），置位后 postEvent 一个 QDeferredDeleteEvent 到对象亲和线程 | qobject.cpp:2446-2500 | `if (d->deleteLaterCalled) return; d->deleteLaterCalled = true; ... QCoreApplication::postEvent(this, new QDeferredDeleteEvent(loopLevel, scopeLevel));` | 多次调只生效一次；事件带 loopLevel/scopeLevel 供时机判定。 |
| DeferredDelete 三种发送时机（投递 loop 已返回 / 当前 loop 显式请求 / 投递早于最外层 loop），allowDeferredDelete 三段析取判定不满足则 re-post 不丢失 | qcoreapplication.cpp:1824-1841 | `// 1) when the event loop that posted the event has returned; // 2) if explicitly requested ...; // 3) if the event was posted before the outermost event loop. const bool allowDeferredDelete = (... \|\| ... \|\| ...); if (!allowDeferredDelete) { // re-post` | 与 07 篇 event-loop [deferred-delete-and-filters](./deferred-delete-and-filters.md) 同一机制，本条从对象端记录投递时机规则。 |
