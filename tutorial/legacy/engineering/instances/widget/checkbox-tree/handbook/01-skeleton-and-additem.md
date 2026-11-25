---
title: "Step 1：组合 QTreeWidget 骨架 + addItem 挂节点"
description: "继承 QWidget 内含 QTreeWidget 成员，构造 new 出来 parent=this 托管，addItem 挂带复选框的节点。这步先不联动。"
---

# Step 1：组合 QTreeWidget 骨架 + addItem 挂节点

← [手册首页](./index.md) · 下一步 [Step 2 父子联动](./02-propagation-and-recalc.md) →

## Step 1：组合 QTreeWidget 骨架 + addItem 挂节点

### 目标

屏幕上出现一棵**带复选框的树**，每项前面一个方框，能手动勾上/取消。这步**先不做联动**——勾父不会自动勾子，纯粹是「每项一个独立复选框」的原始状态。联动是下一步的重头戏。

### 提示

- 继承 `QWidget`，加私有成员 `QTreeWidget* tree_`
- 构造里 `tree_ = new QTreeWidget(this)`——`this` 当 parent，交给对象树托管，析构自动回收
- `setColumnCount(1)` + `setHeaderHidden(true)` 让它看着像一棵纯勾选树
- new 一个 `QVBoxLayout(this)`，`addWidget(tree_)`，`setContentsMargins(0,0,0,0)` 去白边
- 写 `addItem(QTreeWidgetItem* parent, const QString& text, Qt::CheckState state)`：`new QTreeWidgetItem()` → `setText(0, text)` → `setCheckState(0, state)` → `parent==nullptr` 就 `addTopLevelItem`，否则 `parent->addChild`
- 这一步先**不连 itemChanged**，纯挂节点。连了也没槽可接

### 检查点

跑出来是一棵能展开、每项带可勾选复选框的树，勾哪个是哪个、互不影响 = 骨架对了。勾父项时子项纹丝不动是**正常的**，联动还没写。

> QTreeWidget / QTreeWidgetItem 不熟？[Model/View 基础](../../../../../beginner/03-qtwidgets/03-model-view-beginner.md)、进阶 [Model/View 进阶](../../../../../advanced/03-qtwidgets/03-model-view-advanced.md)。对象树托管机制看 [QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)。

### 对照答案

- 构造 + 布局组合骨架：`src/checkbox_tree.cpp:33-42`
- addItem 挂顶层 vs 挂子项：`src/checkbox_tree.cpp:48-71`

---

下一步是核心：[Step 2 让父子勾选联动起来](./02-propagation-and-recalc.md)。
