---
title: "卡住怎么办"
description: "按症状查：itemChanged 雪崩栈溢出、QTreeWidgetItemIterator 编译不过、IDE 误报 __or_fn、动态加节点父态不对——给方向指向教程章，不直接给答案。"
---

# 卡住怎么办

← [手册首页](./index.md)

按症状查。每条给方向，不给整段答案——成品 repo 在 `widget/checkbox-tree/`，对照着看。

## 勾一下就崩 / 卡死（itemChanged 雪崩）

- 递归改子项时**有没有挡 `itemChanged`**？程序化 `setCheckState` 会再触发 itemChanged，槽又调 propagateDown，子子孙孙无限递归到栈溢出。两道闸门：`is_propagating_` 入口挡自身触发（`src/checkbox_tree.cpp:183`），批量改写前后 `tree_->blockSignals(true/false)` 切断回环（`src/checkbox_tree.cpp:197`、`228`）。
- `is_propagating_` 的**置位/复位有没有精确包住整段批量修改**？漏复位会永久挡住用户触发的联动。→ `src/checkbox_tree.cpp:196-203`
- `blockSignals` 是不是**保存旧值、改完恢复**（`was_blocked` 模式）？硬写 false 可能误伤别的信号连接。→ `src/checkbox_tree.cpp:197`
- 进阶排查：[信号与槽](../../../../../beginner/01-qtbase/02-signal-slot-beginner.md)、[QObject::blockSignals](https://doc.qt.io/qt-6/qobject.html#blockSignals)

## 用 QTreeWidgetItemIterator 写 `it != end` 编译不过

- 报 `no matching function for call to 'QTreeWidgetItemIterator::QTreeWidgetItemIterator()'`？Qt6 的 `QTreeWidgetItemIterator` **没有默认构造函数**，造不出 end 哨兵——经典的「以为迭代器都有默认构造」想当然。
- 改用从 `topLevelItem(i)` 出发的递归辅助函数 `collectChecked`（放匿名命名空间），深度优先收 Checked。顺手去掉 `<QTreeWidgetItemIterator>` 头依赖。→ `src/checkbox_tree.cpp:16`
- 进阶排查：[容器与迭代器](../../../../../beginner/01-qtbase/04-container-beginner.md)

## collectChecked 上 IDE 飘红（`__or_fn` / `ovl_no_viable_function_in_call`）

- 这是 IDE/clangd 对 Qt6 `QList` 模板参数推导的**瞬时误报**，不是真实编译错误。
- 以实际 `cmake --build` 输出为准：编译干净通过、生成 `checkbox_tree_demo` 就是对的，忽略红波浪线，别为假报错动代码。

## 勾父项子项没跟着变（联动没生效）

- itemChanged 信号**连了吗**？构造里 `connect(tree_, &QTreeWidget::itemChanged, this, &CheckboxTree::onItemChanged)`。→ `src/checkbox_tree.cpp:45`
- onItemChanged 里**有没有真的调 propagateDown**？且只在状态为 Checked/Unchecked 时调（PartiallyChecked 是回算产物，不该向下传播）。→ `src/checkbox_tree.cpp:198-200`
- `propagation_enabled_` 是不是被关掉了？关了就是普通勾选树，不联动是预期行为。→ `src/checkbox_tree.cpp:187`

## 父项三态不对（子全勾父却显示 Partially / 部分勾父却 Checked）

- `aggregateState` 的**三档判断顺序对不对**？任一子 PartiallyChecked 要**立即短路返回**（`src/checkbox_tree.cpp:256`），不能数完再判断。
- recalcUp 是不是**从 `item->parent()` 开始**而不是从 item 本身？从 item 本身开始会把它刚定好的状态又按子项分布覆盖一遍。→ `src/checkbox_tree.cpp:202`
- recalcUp 的 while 循环**有没有一直走到根**（`current = current->parent()` 直到 nullptr）？中途断了上层祖先就不重算。→ `src/checkbox_tree.cpp:225-232`

## 用代码 addItem 加子项后，父项勾选态不更新

- addItem 挂子项后**有没有 recalcUp(parent)**？初始化期子项的 setCheckState 被 blockSignals 守卫了（防雪崩），但父态不会自动跟着算，得显式回算一次。→ `src/checkbox_tree.cpp:61`
- 进阶排查：[Model/View 基础](../../../../../beginner/03-qtwidgets/03-model-view-beginner.md)

## checkAll / uncheckAll 之后深层子孙没跟着变

- 批量入口里**有没有对每个顶层项调 propagateDown**？只改顶层 item 的 setCheckState 不会自动传播到深层，得递归写到底。→ `src/checkbox_tree.cpp:123` / `142`
- 批量改写整段**有没有被 blockSignals 守卫 + is_propagating_ 包住**？不挡就崩。→ `src/checkbox_tree.cpp:117-128`

## moc 报错（Q_PROPERTY 不认识）

- 头文件**有没有 `Q_OBJECT`**？→ `include/checkbox_tree.h:27`
- CMake **有没有开 AUTOMOC**（`set(CMAKE_AUTOMOC ON)`）？→ `widget/CMakeLists.txt`
- 进阶排查：[QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)
