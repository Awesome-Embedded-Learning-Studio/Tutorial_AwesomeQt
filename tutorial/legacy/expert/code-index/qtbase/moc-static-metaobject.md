---
title: moc 产物与运行期对接
description: QMetaObject::Data 七字段是 moc 填充目标；Q_OBJECT 宏声明端声明 staticMetaObject+metaObject/qt_metacast/qt_metacall（moc 负责定义）；qt_metacast strcmp 逐类名比较（本类命中 return this）；运行期 QMetaObject::metacall 普通对象走虚函数 object->qt_metacall 正是 moc 产物——01↔17 对接点。
---

# moc 产物与运行期对接

> 本索引收录 Qt 6.9.1 源码中 moc 产物的填充目标、声明端与运行期对接点相关的已验证证据。对应专家篇《17 MOC 编译器原理》§3.6/§3.8。

## Data 七字段填充目标

源码文件：`qtbase/src/corelib/kernel/qobjectdefs.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QMetaObject::Data 七字段布局（superdata/stringdata/data/static_metacall/relatedMetaObjects/metaTypes/extradata）是 moc 填充目标 | :600-609 | `struct Data { SuperData superdata; const uint *stringdata; const uint *data; StaticMetacallFunction static_metacall; const SuperData *relatedMetaObjects; const QtPrivate::QMetaTypeInterface *const *metaTypes; void *extradata; } d;` | 字段布局详述见 [qmetaobject-static-metadata](./qmetaobject-static-metadata.md)，本条记它是 moc 第三段的填充目标。 |

## Q_OBJECT 声明端

源码文件：`qtbase/src/corelib/kernel/qtmetamacros.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| Q_OBJECT 宏声明端：声明 staticMetaObject + metaObject/qt_metacast/qt_metacall，moc 负责定义 | :118-146 | `#define Q_OBJECT \ public: ... static const QMetaObject staticMetaObject; virtual const QMetaObject *metaObject() const; virtual void *qt_metacast(const char *); virtual int qt_metacall(QMetaObject::Call, int, void **);` | 声明在头文件，定义在 moc 生成的 moc_myclass.cpp。 |

## qt_metacast 类型转换

源码文件：`qtbase/src/tools/moc/generator.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| qt_metacall strcmp 逐类名比较：本类命中 return this，基类/接口命中返回对应指针，都不中递归基类 | :486-489 | `fprintf(out, "if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_%s_t>.strings)) return static_cast<void*>(this),\n")` | qobject_cast 安全向下转换的底层，查 moc 编译期固化的类名列表。 |

## 与运行期 metacall 对接点

源码文件：`qtbase/src/corelib/kernel/qmetaobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 01↔17 对接点：QMetaObject::metacall 普通对象走虚函数 object->qt_metacall，正是 moc 生成的产物 | :343-348 | `int QMetaObject::metacall(QObject *object, Call cl, int idx, void **argv) { if (object->d_ptr->metaObject) return object->d_ptr->metaObject->metaCall(...); else return object->qt_metacall(cl, idx, argv); }` | 运行期 Qt 库代码 metacall 调 moc 生成的 qt_metacall——元对象框架与 moc 胶水的边界。 |
