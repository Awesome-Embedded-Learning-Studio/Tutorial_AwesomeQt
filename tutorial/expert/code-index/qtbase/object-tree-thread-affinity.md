---
title: 对象树线程亲和与 QWidget 快速分支
description: thread() 从 threadData 原子 loadRelaxed 取 thread.loadAcquire()；moveToThread 有父对象直接 qWarning(Cannot move objects with a parent) 返回 false（对象树必须同线程的铁律），widget 也不能搬；QObject protected 构造有 willBeWidget 快速分支——置真时直接裸写 parent+append 绕开 setParent 不发 ChildAdded；willBeWidget 跨模块由 QWidgetPrivate 构造体(qwidget.cpp:182)反向置位。
---

# 对象树线程亲和与 QWidget 快速分支

> 本索引收录 Qt 6.9.1 源码中对象树线程亲和约束与 QWidget 快速分支相关的已验证证据。对应专家篇《21 对象树与所有权源码拆解》§3.5-§3.6。

## 线程亲和查询

源码文件：`qtbase/src/corelib/kernel/qobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| thread() 从 d_func()->threadData 原子 loadRelaxed 取 thread.loadAcquire() 的 QThread 指针，一次原子 load | :1610-1613 | `QThread *QObject::thread() const { return d_func()->threadData.loadRelaxed()->thread.loadAcquire(); }` | 信号槽跨线程分流（02 篇 receiverInSameThread）和事件投递目标线程都用这个字段。 |

## moveToThread 同线程铁律

源码文件：`qtbase/src/corelib/kernel/qobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| moveToThread 有父对象（d->parent != nullptr）直接 qWarning(Cannot move objects with a parent) 返回 false——对象树必须同线程的源码铁律 | :1655-1698 | `if (d->parent != nullptr) { qWarning("QObject::moveToThread: Cannot move objects with a parent"); return false; } if (d->isWidget) { qWarning("...: Widgets cannot be moved to a new thread"); return false; }` | 有 parent 搬走会跨线程析构不安全；widget 也不能搬。要搬先 setParent(nullptr) 或搬整树根。 |

## QWidget willBeWidget 快速分支

源码文件：`qtbase/src/corelib/kernel/qobject.cpp` / `qtbase/src/widgets/kernel/qwidget.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QObject protected 构造有 willBeWidget 快速分支：置真时直接裸写 d->parent + append 进 children，完全绕开 setParent/helper，不发 ChildAdded | qobject.cpp:960-968 | `if (d->willBeWidget) { if (parent) { d->parent = parent; d->parent->d_func()->children.append(this); } // no events sent here, this is done at the end of the QWidget constructor } else { setParent(parent); }` | widget 构造频繁，省事件开销；事件推迟到 QWidget 构造末尾。 |
| willBeWidget 跨模块置位：QWidgetPrivate 构造体在 qwidget.cpp:182 反向置 willBeWidget=true，core 的 QObject 构造读此标志走快速分支 | qwidget.cpp:182 | `willBeWidget = true; // used in QObject's ctor` | 上层模块（Widgets）反向告诉下层（Core）走特殊路径的跨模块协作。 |
