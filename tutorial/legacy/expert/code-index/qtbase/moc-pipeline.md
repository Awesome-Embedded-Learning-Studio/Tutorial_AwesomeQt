---
title: moc 三段式流水线与中间表示
description: runMoc 三段式（preprocessed 嚼符号 → parse 解析 → generate 生成）；FunctionDef 用 isSlot/isSignal/isInvokable 等布尔标志位记录方法角色；ClassDef 持 superclassList/signalList/slotList/methodList/propertyList 是产物直接投影；generate 遍历 classList 对每个 ClassDef new Generator 调 generateCode。
---

# moc 三段式流水线与中间表示

> 本索引收录 Qt 6.9.1 源码中 moc 工具前端（入口 + parse + IR）相关的已验证证据。对应专家篇《17 MOC 编译器原理》§3.1-§3.3。源码均在 `qtbase/src/tools/moc/`（moc 工具自身源码，行号漂移快）。

## runMoc 三段式入口

源码文件：`qtbase/src/tools/moc/main.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| moc 三段式入口 runMoc：pp.preprocessed 嚼符号 → moc.parse() 解析 → moc.generate 生成 | :543-547 | `moc.symbols += pp.preprocessed(moc.filename, &in); if (!pp.preprocessOnly) { moc.parse(); }` | 经典前端 parse → IR → 后端 generate 三段式编译器结构，产物是 C++ 源代码。 |

## FunctionDef / ClassDef 中间表示

源码文件：`qtbase/src/tools/moc/moc.h`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| FunctionDef 用一组布尔标志位记录方法角色（isSlot/isSignal/isInvokable 等） | :76-110 | `bool isInvokable = false; bool isScriptable = false; bool isSlot = false; bool isSignal = false; bool isPrivateSignal = false;` | parse 阶段根据宏标记和 signals:/slots: 段落位置填上。 |
| ClassDef 持 superclassList/signalList/slotList/methodList/propertyList 等，是产物直接投影 | :177-209 | `struct ClassDef : BaseDef { QList<SuperClass> superclassList; ... QList<FunctionDef> signalList, slotList, methodList, publicList; QList<PropertyDef> propertyList; }` | 整个类的元信息在 moc 眼里的完整画像，generate 阶段逐项翻译。 |

## generate + Generator 类

源码文件：`qtbase/src/tools/moc/moc.cpp` / `generator.h` / `generator.cpp`

| 论点 | 行号 | 原文摘要 | 解读 |
|---|---|---|---|
| generate 遍历 classList，对每个 ClassDef new Generator 并调 generateCode 作为产物总入口 | moc.cpp:1233-1236 | `for (ClassDef &def : classList) { Generator generator(this, &def, ...); generator.generateCode(); }` | 一个头文件可能多个 Q_OBJECT 类，每类一个 Generator。 |
| Generator 类封装 cdef/out/strings/purestSuperClass + generateCode/generateMetacall/generateStaticMetacall 三方法 | generator.h:11-49 | `class Generator { Moc *parser; FILE *out; ClassDef *cdef; QList<uint> meta_data; ... void generateCode(); void generateMetacall(); void generateStaticMetacall(); }` | generateCode 是总入口，按四段顺序输出。 |
| generateCode 开头判断 isQObject/isConstructible，决定后续分支 | generator.cpp:220 | `void Generator::generateCode() { bool isQObject = (cdef->classname == "QObject"); bool isConstructible = !cdef->constructorList.isEmpty(); }` | isQObject 影响要不要往上传 qt_metacall；isConstructible 影响 CreateInstance 分支。 |
