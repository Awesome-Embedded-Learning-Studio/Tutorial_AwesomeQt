---
title: 对象树父子关系建立——setParent_helper
description: setParent 公开方法只做 Q_D+Q_ASSERT(!isWidget) 转手 setParent_helper；setParent_helper 三段式（o==parent return / 旧父 removeAt+双标志发 ChildRemoved / parent=o 改指针 / 新父 append+双标志且非 widget 发 ChildAdded）；sendChildEvents/receiveChildEvents 是 QObjectData 基类的 1 bit 位域，QObjectPrivate 构造默认 true。
---

# 对象树父子关系建立——setParent_helper

> 本索引收录 Qt 6.9.1 源码中 setParent 与父子关系建立相关的已验证证据。对应专家篇《21 对象树与所有权源码拆解》§3.1。

## setParent 公开方法

源码文件：`qtbase/src/corelib/kernel/qobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| setParent 只做 Q_D 取 d_ptr + Q_ASSERT(!d->isWidget) 守卫，真正干活转手 d->setParent_helper | :2206-2211 | `void QObject::setParent(QObject *parent) { Q_D(QObject); Q_ASSERT(!d->isWidget); d->setParent_helper(parent); }` | widget 禁走此路（有 willBeWidget 快速分支）。 |

## setParent_helper 三段式

源码文件：`qtbase/src/corelib/kernel/qobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| setParent_helper 三段式：o==parent 直接 return；旧父 removeAt 摘除 + 双标志门控发 ChildRemoved；parent=o 改指针；新父 append + 双标志且非 widget 发 ChildAdded | :2230-2295 | `if (o == parent) return; if (parent) { parentD->children.removeAt(index); if (sendChildEvents && parentD->receiveChildEvents) { QChildEvent e(QEvent::ChildRemoved, q); ...sendEvent(parent, &e); } } parent = o; if (parent) { parent->d_func()->children.append(q); if (sendChildEvents && parent->d_func()->receiveChildEvents && !isWidget) { QChildEvent e(QEvent::ChildAdded, q); ... } }` | 数据维护（children 增删）与事件通知分离，后者受双标志门控。 |

## 双标志门控

源码文件：`qtbase/src/corelib/kernel/qobject.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QObjectData 基类 sendChildEvents/receiveChildEvents 是 1 bit 位域，QObjectPrivate 构造默认两者置 true | :80-81 | `uint sendChildEvents : 1; uint receiveChildEvents : 1;` | sendChildEvents（子被加/移时发不发）+ receiveChildEvents（parent 收不收）双开才发事件；内部对象可关掉省事件开销。 |
