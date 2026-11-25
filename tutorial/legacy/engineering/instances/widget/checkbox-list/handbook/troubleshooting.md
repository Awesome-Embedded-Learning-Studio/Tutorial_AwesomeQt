---
title: "卡住怎么办"
description: "按症状查：批量操作信号刷屏、blockSignals 误伤别的连接、addItem 初始化回灌、invertChecked 反选错乱、setItemChecked 传空崩——给方向指向教程章，不直接给答案。"
---

# 卡住怎么办

← [手册首页](./index.md)

按症状查。每条给方向，不给整段答案——成品 repo 在 `widget/checkbox-list/`，对照着看。

## 按 Check all / 反选后，checkedChanged 槽被刷屏（信号雪崩）

- 批量方法（`checkAll`/`uncheckAll`/`invertChecked`/`addItems`）里**有没有整段 `blockSignals` 守卫**？批量 `setCheckState` 每项都回灌 `itemChanged` → `onItemChanged` → `checkedChanged`，N 项就是 N 次空转。列表版不崩（无递归），但噪音大、性能塌陷。→ `src/checkbox_list.cpp:68`、`:82`、`:97`、`:43`
- `blockSignals` 是不是**保存旧值、改完恢复**（`was_blocked` 模式）？硬写 false 会误伤别的信号连接。→ `src/checkbox_list.cpp:68`
- `onItemChanged` 入口**有没有判空**？`if (item == nullptr) return;` 防御性兜底，别让空项也走转发。→ `src/checkbox_list.cpp:179`
- 进阶排查：[信号与槽](../../../../../beginner/01-qtbase/02-signal-slot-beginner.md)、[QObject::blockSignals](https://doc.qt.io/qt-6/qobject.html#blockSignals)

## 复选框不显示 / setCheckState 设了但没方框

- `addItem` 里**有没有 `setFlags(item->flags() | Qt::ItemIsUserCheckable)`**？不加 `ItemIsUserCheckable`，`setCheckState` 设了状态也不显示方框——经典坑，以为 setCheckState 写错了其实是 flags 没装。→ `src/checkbox_list.cpp:30`
- `setFlags` 是不是用**「或上」**而非「赋值」？`item->setFlags(Qt::ItemIsUserCheckable)` 会把默认的 `ItemIsEnabled|ItemIsSelectable` 全冲掉，项变灰不可选。要用 `item->flags() | Qt::ItemIsUserCheckable`。→ `src/checkbox_list.cpp:30`
- 进阶排查：[Qt::ItemFlag](https://doc.qt.io/qt-6/qt.html#ItemFlag-enum)

## addItem 预勾选（checked=true）时，外部误收到 checkedChanged

- `addItem` 初始化 `setCheckState` **有没有局部守卫**？初始化期的 `setCheckState(Qt::Checked)` 也会触发 `itemChanged` → `onItemChanged` → `checkedChanged`，view 不区分程序化还是用户点击，业务会误以为是用户勾选。→ `src/checkbox_list.cpp:33-35`
- 批量版 `addItems` **有没有整段守卫**？同理。→ `src/checkbox_list.cpp:43-50`
- 进阶排查：[QListWidget 入门](../../../../../beginner/03-qtwidgets/46-qlistwidget-beginner.md)

## 反选（Invert selection）结果错乱 / 部分项没翻对

- `invertChecked` 里读旧态 `item->checkState()` 和写反态 `setCheckState(反态)` **是不是都在 `blockSignals` 屏蔽期内**？不挡信号的话，写新态触发的 `itemChanged` 回灌会干扰下一次 `checkState()` 的读——读到的可能已是回灌改过的值。→ `src/checkbox_list.cpp:97-106`
- 循环里**有没有对每项判空**？`item == nullptr` 要 `continue` 跳过。→ `src/checkbox_list.cpp:100-101`

## setItemChecked 传空指针崩了 / 外部拿不到单项通知

- 传 nullptr 崩：`setItemChecked` 入口**有没有 `if (item == nullptr) return;`**？→ `src/checkbox_list.cpp:54`
- 外部程序化改单项却收不到 `checkedChanged`：检查你是不是**误把 `setItemChecked` 也加了 `blockSignals` 守卫**？单项故意**不守卫**，允许 `checkedChanged` 透传——外部就靠它知道某项被改了。批量才守卫。→ `src/checkbox_list.cpp:53-61`
- 和批量守卫对照看：批量挡、单项放，策略相反。→ `src/checkbox_list.cpp:63` vs `:53`

## Q_PROPERTY 反复 set 同值刷空 NOTIFY / spacing 传负值崩

- setter 入口**有没有无变化早返回**？`if (list_->alternatingRowColors() == enabled) return;` / `if (list_->spacing() == pixels) return;`，外部反复 set 同值不发重复 NOTIFY。→ `src/checkbox_list.cpp:152`、`:167`
- `setSpacing` **有没有负值 clamp**？`if (pixels < 0) return;` 挡掉负行距。→ `src/checkbox_list.cpp:164`
- 进阶排查：[QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)

## checkedTexts / checkedItems 顺序乱或空列表崩

- 汇总是不是**按索引顺序遍历** `list_->item(i)`？要保证输出顺序 = 列表显示顺序，别用 `findItems`。→ `src/checkbox_list.cpp:114-122`、`:130-137`
- 空列表**有没有判 `list_ == nullptr` 早返回空容器**？→ `src/checkbox_list.cpp:111`、`:127`
- 循环里**有没有对每项判空 + 比较状态**？`item != nullptr && item->checkState() == Qt::Checked`。→ `src/checkbox_list.cpp:118`、`:133`

## moc 报错（Q_PROPERTY 不认识）

- 头文件**有没有 `Q_OBJECT`**？→ `include/checkbox_list.h:32`
- CMake **有没有开 AUTOMOC**（`set(CMAKE_AUTOMOC ON)`）？→ `widget/CMakeLists.txt`
- 进阶排查：[QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)
