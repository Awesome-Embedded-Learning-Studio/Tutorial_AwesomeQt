---
title: metacall 元调用分发框架
description: QMetaObject::metacall 总入口做静态/动态二分，Call 枚举列出全部分支类型，moc 生成的 qt_metacall 沿继承链向上传并按 Call 类型 _id-=N 索引递减，qt_static_metacall 用 switch 跳转表 + IndexOfMethod 线性查找。
---

# metacall 元调用分发框架

> 本索引收录 Qt 6.9.1 源码中元调用（metacall）分发机制相关的已验证证据。对应专家篇《01 QObject 元对象系统源码拆解》§3.4-§3.6。

## Call 枚举（元调用类型）

源码文件：`qtbase/src/corelib/kernel/qobjectdefs.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| Call 枚举列出元调用全部分支 | :553-565 | `InvokeMetaMethod / ReadProperty / WriteProperty / ResetProperty / CreateInstance / IndexOfMethod / RegisterMethodArgumentMetaType / BindableProperty / CustomCall / ConstructInPlace` | 一次元调用 = (Call 类型, int 索引, void\*\* 参数)。 |

## metacall 总分发入口

源码文件：`qtbase/src/corelib/kernel/qmetaobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| metacall 只做静态/动态二分 | :343-348 | `if (object->d_ptr->metaObject) return ...metaCall(...); else return object->qt_metacall(...);` | d_ptr->metaObject 非空走动态（QML / 运行期合成元对象），空走静态虚函数 qt_metacall。 |

## qt_metacall（moc 生成，链式调父类 + 索引递减）

源码文件：`qtbase/src/tools/moc/generator.cpp`（moc 生成器模板，行号随 Qt 升级漂移快）

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| qt_metacall 开头先把 `_id` 交基类，QObject 根类不往传（isQObject 守卫） | :849-855 | `if (!purestSuperClass.isEmpty() && !isQObject) { ... fprintf(out, "_id = %s::qt_metacall(_c, _id, _a);\n", ...); }` | 索引空间叠加：派生类先让父类处理其范围。 |
| `_id < 0` 守卫直接 return | :866-867 | `if (_id < 0) return _id;` | 父类认领后 `_id` 变负，本层短路。 |
| 按 Call 类型 `_id -= N` 链式递减（方法族 + 属性族） | :870-895 | `if (_c == QMetaObject::InvokeMetaMethod) { ... _id -= %d; }` | 866-867 守卫与 870 起 Call 分支块是两段独立内容；每族处理后减去自己数量。 |

## qt_static_metacall（switch 跳转表 + 线性查找）

源码文件：`qtbase/src/tools/moc/generator.cpp`（moc 生成器模板，行号随 Qt 升级漂移快）

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| qt_static_metacall 生成多个 switch 跳转表 | :929-967 | `if (_c == QMetaObject::CreateInstance) { switch (_id) { ...` | CreateInstance / InvokeMetaMethod 各自 switch(_id)，编译期生成、运行期一次跳转。 |
| IndexOfMethod 不用 switch，逐信号指针比较 O(N) | :1078-1083 | `if (_c == QMetaObject::IndexOfMethod) { for (...) { if (QtMocHelpers::indexOfMethod<...>) ...` | 输入是成员函数指针非整数，没法 switch；信号多时 connect 查询有线性开销。 |
