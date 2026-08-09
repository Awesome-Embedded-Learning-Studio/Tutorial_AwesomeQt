---
title: "卡住怎么办"
description: "按症状查：表是空的、双击编辑不了、编辑后不刷新、增删行崩溃、rowCount 死循环、moc 报错——给方向指向教程章，不直接给答案。"
---

# 卡住怎么办

← [手册首页](./index.md)

按症状查。每条给方向，不给整段答案——成品 repo 在 `model/01-mv-pattern/custom-model/`，对照着看。

## 表是空的 / 视图没内容

- `rowCount` 返回 0 吗？`tasks_` 构造里**填了种子数据**吗？→ `src/custom-model.cpp:18`
- `columnCount` 返回 0 吗？返回的是 `kColumnCount`（不是 0）吗？→ `src/custom-model.cpp:40`
- demo 端**调了 `setModel`** 吗？没 setModel 视图不知道用哪个模型。→ `demo/custom-model_window.cpp:39`
- 进阶排查：[Model/View 入门](../../../../../beginner/03-qtwidgets/03-model-view-beginner.md)

## 双击单元格进不了编辑态

- `flags` **返回了 `Qt::ItemIsEditable`** 吗？这是「可编辑」的总开关，漏了就双击没反应。→ `src/custom-model.cpp:154`
- 视图的 `editTriggers` 开了吗？至少 `DoubleClicked` 或 `EditKeyPressed`。→ `demo/custom-model_window.cpp:51`
- 进阶排查：[Model/View 入门](../../../../../beginner/03-qtwidgets/03-model-view-beginner.md)

## 编辑后单元格不刷新 / 改优先级底色不变

- `setData` 改完值**发了 `dataChanged`** 吗？不发信号视图不知道要重查。→ `src/custom-model.cpp:139`
- dataChanged 的**列范围**给的是整行（`[0, kColumnCount-1]`）吗？只给改的那一列，旁边的 BackgroundRole 不会重查，底色就不变。→ `src/custom-model.cpp:137`
- dataChanged 的 **roles 列表**把 BackgroundRole/ForegroundRole/FontRole 列上了吗？漏列等于告诉视图「这角色没变」。→ `src/custom-model.cpp:139`
- 进阶排查：[Model/View 进阶](../../../../../advanced/03-qtwidgets/03-model-view-advanced.md)

## 增删行后崩溃（segfault）或显示错乱

- `insertRows`/`removeRows` 改容器**前后有没有 `beginInsertRows`/`endInsertRows`（或 Remove 版）**？漏了就是头号崩溃源。→ `src/custom-model.cpp:174,180`、`src/custom-model.cpp:199,203`
- `beginInsertRows` 的第二三个参数是**行范围 `[row, row+count-1]`** 不是行数？写错视图算错行数。
- 删多行时**从大到小排序再删**了吗？从小删会让后面行号失效。→ `demo/custom-model_window.cpp:121`
- 进阶排查：[Model/View 进阶](../../../../../advanced/03-qtwidgets/03-model-view-advanced.md)

## rowCount 导致视图无限铺行 / 死循环

- `rowCount(parent)` **有没有判断 `parent.isValid()`**？表格模型 parent 有效必须返回 0，否则视图把它当有子项，无限递归铺行。→ `src/custom-model.cpp:34`
- 进阶排查：[QAbstractItemView 基类](../../../../../beginner/03-qtwidgets/15-qabstractitemview-base-beginner.md)

## 滚动或动画时偶发崩溃

- `data`/`setData` **做了 index 边界保护**吗？视图在滚动/动画/异步刷新时会塞进脏 index（越界），不保护就 segfault。→ `src/custom-model.cpp:53`
- 每个 case 里取 `tasks_` 前，先确认 `index.row() < tasks_.size()`。

## moc 报错（Q_OBJECT / Q_ENUM 不认识）

- 头文件**有没有 `Q_OBJECT`**？→ `include/custom-model.h:37`
- CMake **有没有开 AUTOMOC**（`set(CMAKE_AUTOMOC ON)`）？→ `model/01-mv-pattern/custom-model/CMakeLists.txt`
- Q_ENUM 的 `Column` 枚举**是不是在类里**、Q_ENUM 紧跟其后？→ `include/custom-model.h:41-42`
- 进阶排查：[QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)
