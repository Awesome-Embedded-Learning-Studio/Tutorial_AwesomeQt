---
title: QObject 的 d_ptr 与两层数据布局
description: QObject 只持一个 d_ptr 不透明指针（PIMPL 冻结 ABI），QObjectData 抽象基类放对象树三件套，QObjectPrivate 继承并叠实现细节，Q_D/Q_DECLARE_PUBLIC 双向访问。
---

# QObject 的 d_ptr 与两层数据布局

> 本索引收录 Qt 6.9.1 源码中 QObject 私有数据布局相关的已验证证据。对应专家篇《01 QObject 元对象系统源码拆解》§3.1-§3.2。

## d_ptr 不透明指针

源码文件：`qtbase/src/corelib/kernel/qobject.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QObject 只持一个 `QScopedPointer<QObjectData> d_ptr` | :375 | `QScopedPointer<QObjectData> d_ptr;` | PIMPL 手法，冻结 ABI——Qt 内部改 QObjectPrivate 布局不破坏二进制兼容。 |

## 保护构造（绑 d_ptr + 回填 q_ptr）

源码文件：`qtbase/src/corelib/kernel/qobject.cpp` / `qobject.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| protected 构造只有子类可调 | qobject.h:372 | `QObject(QObjectPrivate &dd, QObject *parent = nullptr);` | 外部调不到，子类构造时传自己的 QObjectPrivate 派生类。 |
| 构造体绑 d_ptr + 回填 q_ptr，建双向引用 | qobject.cpp:946-952 | `: d_ptr(&dd) ... d_ptr->q_ptr = this;` | 「公共对象 ↔ 私有数据」双向引用的建立点。 |

## QObjectData 抽象基类（对象树三件套）

源码文件：`qtbase/src/corelib/kernel/qobject.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QObjectData 纯虚析构，不可独立实例化 | :71 | `virtual ~QObjectData() = 0;` | 抽象基类，只定义通用字段集。 |
| 持对象树三件套 q_ptr / parent / children | :72-74 | `QObject *q_ptr; QObject *parent; QObjectList children;` | 放基类层让所有子类继承统一的对象树表示。 |
| children() inline 返回 d_ptr->children 的 const 引用 | :203 | `inline const QObjectList &children() const { return d_ptr->children; }` | 零开销——能这么写正因为 children 在 QObjectData 基类，d_ptr 能直接看到。 |

## QObjectPrivate 实现层

源码文件：`qtbase/src/corelib/kernel/qobject_p.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QObjectPrivate public 继承 QObjectData + Q_DECLARE_PUBLIC 注入 q_func() | :73-76 | `class Q_CORE_EXPORT QObjectPrivate : public QObjectData { public: Q_DECLARE_PUBLIC(QObject)` | 继承对象树三件套，叠上 QtCore 实现细节；Q_DECLARE_PUBLIC 是 Q_D 的对偶，让私有层反查公共层。 |
