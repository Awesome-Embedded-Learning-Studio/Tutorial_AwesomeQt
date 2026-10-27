---
title: QMetaObject 静态元数据表
description: QMetaObject 是每个类一个的只读静态表（struct），Data 七槽位固定布局，SuperData 双模式包装父类引用，Q_OBJECT 宏声明 staticMetaObject + 三个虚函数，moc 填 static_metacall 槽。
---

# QMetaObject 静态元数据表

> 本索引收录 Qt 6.9.1 源码中 QMetaObject 静态元数据结构相关的已验证证据。对应专家篇《01 QObject 元对象系统源码拆解》正文 QMetaObject 一节。信号定义打印的引文出自正文风险与故障预防一节的第一类风险，按机制归入本组。

## QMetaObject 本体

源码文件：`qtbase/src/corelib/kernel/qobjectdefs.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QMetaObject 是 struct（默认公有），只读静态元数据表 | :233 | `struct Q_CORE_EXPORT QMetaObject {` | 每个类一个、编译期定好的静态对象，不存对象状态。 |

## Data 七槽位布局

源码文件：`qtbase/src/corelib/kernel/qobjectdefs.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| Data 七槽位固定顺序 | :600-609 | `superdata / stringdata / data / static_metacall / relatedMetaObjects / metaTypes / extradata` | superdata 指父类元对象（superClass 链源头）、stringdata 压缩字符串表、data 元信息整数表、static_metacall 函数指针、其余为关联元对象 / 元类型 / 扩展。 |

## SuperData 双模式（父类引用）

源码文件：`qtbase/src/corelib/kernel/qobjectdefs.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| SuperData 双模式：正常存 direct=g()，QT_NO_DATA_RELOCATION 存 indirect getter | :575-598 | `constexpr SuperData(Getter g) : direct(g()) {}` | direct 持有的是 getter `g()` 调用后取出的父类元对象地址，不是 getter 指针本身；某些动态库平台退化为运行期调 getter。 |
| QT_NO_DATA_RELOCATION 的定义点是 Windows | qsystemdetection.h:137-143 | `#if defined(Q_OS_WIN32) \|\| defined(Q_OS_WIN64) ... // On Windows, pointers to dllimport'ed variables are not constant expressions, ... #  define QT_NO_DATA_RELOCATION` | dllimport 进来的变量指针不是常量表达式，保住 QMetaObject 一类初始化的 constexpr 只好改存函数。 |

## Q_OBJECT 宏展开

源码文件：`qtbase/src/corelib/kernel/qtmetamacros.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| Q_OBJECT 声明 staticMetaObject + metaObject / qt_metacast / qt_metacall 三虚函数 | :133-140 | `static const QMetaObject staticMetaObject; virtual const QMetaObject *metaObject() const; virtual void *qt_metacast(const char *); virtual int qt_metacall(...);` | 四个都是声明，定义由 moc 生成的 moc_myclass.cpp 提供——改了类声明必须重跑 moc。 |

## moc 填 static_metacall 槽

源码文件：`qtbase/src/tools/moc/generator.cpp`（moc 生成器模板，行号随 Qt 升级漂移快）

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| moc 写 Data 第四槽 static_metacall，有静态元调用填 qt_static_metacall 否则 nullptr | :454-455 | `if (hasStaticMetaCall) fprintf(out, "    qt_static_metacall,\n");` | generator.cpp 是 moc 工具源码，用 fprintf 打印生成代码，非运行期 QObject 代码。 |

## relatedMetaObjects 与 extradata 的 moc 打印

源码文件：`qtbase/src/tools/moc/generator.cpp`（moc 生成器模板，行号随 Qt 升级漂移快）

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| relatedMetaObjects 槽空则填 nullptr，非空填 qt_meta_extradata_类名 | :459-462 | `if (extraList.isEmpty()) fprintf(out, "    nullptr,\n"); else fprintf(out, "    qt_meta_extradata_%s,\n", qualifiedClassNameIdentifier.constData());` | 关联元对象收的是属性类型作用域与带作用域枚举的 SuperData::link 记录，不装 Q_INTERFACES 接口。 |
| extradata 槽恒填 nullptr | :467 | `fprintf(out, "    nullptr\n} };\n\n");` | 注释写着 reserved for future use，Qt 6.9 的 moc 没往这一槽放东西。 |

## qt_metacast 按接口名 strcmp

源码文件：`qtbase/src/tools/moc/generator.cpp`（moc 生成器模板，行号随 Qt 升级漂移快）

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| Q_INTERFACES 的匹配走 qt_metacast 逐接口 strcmp | :505-509 | `for (const QList<ClassDef::Interface> &iface : std::as_const(cdef->interfaceList)) { ... fprintf(out, "    if (!strcmp(_clname, %s))\n        return ", iface.at(j).interfaceId.constData());` | 接口名不在 relatedMetaObjects 里，qt_metacast 生成的代码按接口名字符串逐个比对。 |

## 按名字找元对象

源码文件：`qtbase/src/corelib/kernel/qmetaobject.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| QMetaObject_findMetaObject 顺继承链与关联表递归找 | :1054 起 | `static const QMetaObject *QMetaObject_findMetaObject(const QMetaObject *self, QByteArrayView name) { while (self) { if (objectClassName(self) == name) return self; ... self = self->d.superdata; }` | 运行期按名字找元对象：先比类名，再递归 relatedMetaObjects，再沿 superdata 往上走。 |

## 信号定义的打印（正文风险与故障预防一节）

源码文件：`qtbase/src/tools/moc/generator.cpp`（moc 生成器模板，行号随 Qt 升级漂移快）

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| moc 打印信号函数体的开头 | :1296 起 | `fprintf(out, "\n// SIGNAL %d\n%s %s::%s(", index, def->type.name.constData(), cdef->qualified.constData(), def->name.constData());` | 信号的函数体只在 moc 产物里——头文件加了新信号而 moc 没重跑，取地址在链接期报 undefined reference，字符串语法运行期收 signal not found。 |
