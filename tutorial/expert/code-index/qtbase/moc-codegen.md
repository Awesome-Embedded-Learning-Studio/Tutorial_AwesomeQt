---
title: moc generateCode 四段输出
description: 四段顺序——字符串池去重(strreg/registerFunctionStrings→StringRefStorage)；数据表 UintData(signal 先 add 否则索引错)；staticMetaObject 七字段聚合(metaObjectData 模板 + QObject 填 nullptr/子类填 SuperData::link + qt_staticMetaObjectStaticContent)；函数生成(qt_metacall 链式递减 / qt_static_metacall InvokeMetaMethod switch / Read-WriteProperty if+switch)。
---

# moc generateCode 四段输出

> 本索引收录 Qt 6.9.1 源码中 moc 工具 generateCode 四段代码生成相关的已验证证据。对应专家篇《17 MOC 编译器原理》§3.4-§3.7。源码均在 `qtbase/src/tools/moc/generator.cpp`（moc 工具自身源码，行号漂移快）。

## 第一段：字符串池去重

源码文件：`qtbase/src/tools/moc/generator.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 6.9 字符串池重构：strreg/registerFunctionStrings 注册去重 → 吐成 StringRefStorage qt_stringData | :245-248,286-288 | `strreg(cdef->qualified); registerClassInfoStrings(); registerFunctionStrings(cdef->signalList); registerFunctionStrings(cdef->slotList);` | 去重减小 staticMetaObject 体积，名字用整数偏移引用池。 |

## 第二段：数据表 UintData

源码文件：`qtbase/src/tools/moc/generator.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| 6.9 数据表重构：UintData qt_methods（注释要求 signal 先 add 否则索引错）取代 uint[] 平铺 | :290-295 | `fprintf(out, "QtMocHelpers::UintData qt_methods {\n"); // Build signals array first, otherwise the signal indices would be wrong addFunctions(cdef->signalList, "Signal"); addFunctions(cdef->slotList, "Slot"); addFunctions(cdef->methodList, "Method");` | 方法索引空间固定 signal→slot→method 顺序，信号排最前——方法索引漂移坑的根源。 |

## 第三段：staticMetaObject 七字段聚合

源码文件：`qtbase/src/tools/moc/generator.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| metaObjectData 模板函数聚合 stringData+qt_methods+qt_properties+qt_enums 成编译期常量 | :335-336 | `fprintf(out, "return QtMocHelpers::metaObjectData<%s, %s>(%s, qt_stringData, qt_methods, qt_properties, qt_enums%s);\n")` | 6.9 重构后用模板打包多表，比老版平铺 uint[] 更类型安全。 |
| staticMetaObject 七字段聚合初始化：QObject 填 nullptr，子类填 SuperData::link\<基类::staticMetaObject\>() | :439-451 | `if (isQObject) fprintf(out, "nullptr,\n"); else if (...!hasQGadget && !hasQNamespace) fprintf(out, "QMetaObject::SuperData::link<%s::staticMetaObject>(),\n", purestSuperClass)` | superClass 链建立点——子类 superdata 指向父类 staticMetaObject。 |
| moc 填 stringdata/data 槽用 6.9 新方案 qt_staticMetaObjectStaticContent.stringdata/.data 指针 | :451-453 | `fprintf(out, "qt_staticMetaObjectStaticContent%s.stringdata,\n qt_staticMetaObjectStaticContent%s.data,\n")` | 6.9 引入的编译期内容容器。 |

## 第四段：函数生成

源码文件：`qtbase/src/tools/moc/generator.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| `qt_static_metacall` 的 `InvokeMetaMethod` 生成 `switch(_id)`，每个方法一个 `case _t->`方法名(args) | :1001-1041 | `fprintf(out, "if (_c == QMetaObject::InvokeMetaMethod) {\n switch (_id) {\n"); for (...) { fprintf(out, "case %d: ", methodindex); ... fprintf(out, "_t->"); }` | 运行期 switch 跳转表的来源，编译期固化索引→调用映射。 |
| Read/Write/ResetProperty 各自 if(_c==...)+switch(_id)，case 按属性在 propertyList 索引排 | :1148-1149 | `if (needGet) { fprintf(out, "if (_c == QMetaObject::ReadProperty) {\n");` | 每个属性操作类型独立 if+switch。 |
| qt_metacall 跨继承层链式递减：先调基类 qt_metacall 消费基类方法偏移，本类按段 _id-=N 判断 | :849-854,870-878 | `if (!purestSuperClass.isEmpty() && !isQObject) { fprintf(out, "_id = %s::qt_metacall(_c, _id, _a);\n", superClass); }` | 与 01 篇运行期视角互补——01 看怎么被调，17 看怎么被打印。 |
