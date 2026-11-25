---
title: "Step 1：组合 QListWidget 骨架 + addItem 装复选框"
description: "继承 QWidget 内含 QListWidget 成员，构造 new 出来 parent=this 托管，addItem 用 setFlags 装复选框、setCheckState 设初值。这步先不接信号。"
---

# Step 1：组合 QListWidget 骨架 + addItem 装复选框

← [手册首页](./index.md) · 下一步 [Step 2 itemChanged + 状态汇总](./02-itemchanged-and-collect.md) →

## Step 1：组合 QListWidget 骨架 + addItem 装复选框

### 目标

屏幕上出现一张**带复选框的扁平清单**，每项前面一个方框，能手动勾上/取消。这一步**先不接 itemChanged**——纯粹是「每项一个独立复选框」的原始状态，勾选互相独立、没有任何联动或转发。信号转发是下一步的事。

### 提示

- 继承 `QWidget`，加私有成员 `QListWidget* list_`（头里前向声明 `class QListWidget;` 就够，避免头里拖进重头文件）
- 构造里 `list_ = new QListWidget(this)`——`this` 当 parent，交给对象树托管，析构自动回收；顺手 `setUniformItemSizes(true)` 提升绘制性能
- new 一个 `QVBoxLayout(this)`，`addWidget(list_)`，`setContentsMargins(0,0,0,0)` 去白边，让列表填满本控件
- 写 `QListWidgetItem* addItem(const QString& text, bool checked = false)`：
  - `auto* item = new QListWidgetItem(text, list_)`——构造时把 `list_` 当 parent，挂进列表
  - `item->setFlags(item->flags() | Qt::ItemIsUserCheckable)`——**用或运算加上** `ItemIsUserCheckable`，保留默认的 `ItemIsEnabled|ItemIsSelectable`，别把默认标志全冲掉
  - `item->setCheckState(checked ? Qt::Checked : Qt::Unchecked)` 设初值
- 这一步**先不连 itemChanged**，纯挂节点。这一步你能看到一张每项带可勾选复选框的清单就算成功

### 关键认知

- **`ItemIsUserCheckable` 是「显示复选框」的开关**：不加这个标志，`setCheckState` 设了状态也不显示方框——很多新手卡在这里，以为是 `setCheckState` 写错了，其实是 flags 没装。`setFlags` 要用「或上」而非「赋值」，否则会丢掉默认的 enabled/selectable
- **`setCheckState` 会触发 itemChanged**：即便这一步没连槽，它也会发信号——只是没人接。下一步连了槽就要小心初始化期回灌，到时候 `blockSignals` 临时挡一下

### 检查点

跑出来是一张每项带可勾选复选框的扁平清单，勾哪个是哪个、互不影响 = 骨架对了。这一步不接信号是**正常的**，转发是下一步的事。

> QListWidget / QListWidgetItem 不熟？[QListWidget 入门](../../../../../beginner/03-qtwidgets/46-qlistwidget-beginner.md)、进阶 [Model/View 进阶](../../../../../advanced/03-qtwidgets/03-model-view-advanced.md)。对象树托管机制看 [QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)。

### 对照答案

- 构造 + 布局组合骨架：`src/checkbox_list.cpp:15-22`
- addItem 装复选框（setFlags 或上 ItemIsUserCheckable）：`src/checkbox_list.cpp:28-38`
- 头文件前向声明 + 成员声明：`include/checkbox_list.h:17` / `:94`

---

下一步：[Step 2 接 itemChanged 转发 checkedChanged + 状态汇总](./02-itemchanged-and-collect.md)。
