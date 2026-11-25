---
title: metacall 元调用分发框架
description: QMetaObject::metacall 总入口做静态/动态二分，Call 枚举列出全部分支类型，moc 生成的 qt_metacall 沿继承链向上传并按 Call 类型 _id-=N 索引递减，qt_static_metacall 用 switch 跳转表 + IndexOfMethod 线性查找。
---

# metacall 元调用分发框架

> 本索引收录 Qt 6.9.1 源码中元调用（metacall）分发机制相关的已验证证据。对应专家篇《01 QObject 元对象系统源码拆解》正文 metacall、qt_metacall、qt_static_metacall 三节。字符串语法 signal not found 告警的引文出自正文风险与故障预防一节的第一类风险，按机制归入本组。

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
| 按 Call 类型 `_id -= N` 链式递减（方法族 + 属性族） | :871-874 / :887-893 | `if (_c == QMetaObject::InvokeMetaMethod) { ... _id -= %d; }` | 866-867 守卫与方法族、属性族两个分支块各自独立；每族处理后减去自己数量。 |

## qt_static_metacall（switch 跳转表 + 线性查找）

源码文件：`qtbase/src/tools/moc/generator.cpp`（moc 生成器模板，行号随 Qt 升级漂移快）

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| qt_static_metacall 生成多个 switch 跳转表 | :965-979 | `if (!cdef->constructorList.isEmpty()) { fprintf(out, "    if (_c == QMetaObject::CreateInstance) {\n"); fprintf(out, "        switch (_id) {\n"); ... fprintf(out, "        default: break;\n");` | CreateInstance 的 case 表在编译期生成、运行期一次跳转；InvokeMetaMethod 的同款表见 :1001-1002。 |
| 生成代码把入参 `_o` 转回具体类指针再调成员 | :944 | `fprintf(out, "    auto \*_t = static_cast<%s *>(_o);\n", cdef->classname.constData());` | 静态元调用虽是每类一份的静态入口，方法执行落在具体对象上。 |
| InvokeMetaMethod 的 switch 跳转表起点 | :1001-1002 | `fprintf(out, "    if (_c == QMetaObject::InvokeMetaMethod) {\n"); fprintf(out, "        switch (_id) {\n");` | qt_metacall 一节那句转交的落点，`_id` 映射到具体的方法调用。 |
| IndexOfMethod 不用 switch，逐信号指针比较 O(N) | :1078-1083 | `if (_c == QMetaObject::IndexOfMethod) { for (...) { if (QtMocHelpers::indexOfMethod<...>) ...` | 输入是成员函数指针非整数，没法 switch；信号多时 connect 查询有线性开销。 |
| indexOfMethod 模板做成员指针比较 | qtmochelpers.h:111-119 | `if (\*candidate != f) return false; \*result = index; return true;` | 候选与传入成员指针逐个比，比上了写回索引；指针当不了 switch 的 case 标签。 |

## 字符串语法连接的 signal not found 告警（正文风险与故障预防一节）

源码文件：`qtbase/src/corelib/kernel/qobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 字符串语法 connect 沿 superClass() 链逐层查不到就告警 | :5242-5248 | `for (; senderMetaObject && signal_index < 0; senderMetaObject = senderMetaObject->superClass()) { senderMetaObject->static_metacall(QMetaObject::IndexOfMethod, 0, args); ... } if (!senderMetaObject) { qCWarning(lcConnect, "QObject::connect: signal not found in %s", sender->metaObject()->className());` | 旧 moc 产物缺新信号时的运行期表现：警告打出、返回空连接、程序照跑收不到信号。 |
