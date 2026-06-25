---
title: "Step 2：itemChanged 槽 + 向下传播 + 向上回算（核心）"
description: "连 itemChanged，onItemChanged 里向下传播状态给子孙、从父起逐层向上重算三态。aggregateState 三态推导规则。"
---

# Step 2：itemChanged 槽 + 向下传播 + 向上回算（核心）

← [Step 1](./01-skeleton-and-additem.md) · [手册首页](./index.md) · 下一步 [Step 3 防雪崩闸门 + 收尾](./03-signal-guard-and-polish.md) →

这一步是整个控件的核心——让父子勾选联动起来。诀窍不在算法本身（向下传播 + 向上回算谁都懂），而在「接到 itemChanged 后干什么、从哪开始算」。

## Step 2：itemChanged 槽 + 向下传播 + 向上回算

### 目标

连上 `QTreeWidget::itemChanged`，写 `onItemChanged(QTreeWidgetItem*, int)`。用户点某项后：向下把该项的 Checked/Unchecked 状态传播给所有子孙；再从**该项的父**起逐层向上重算 tri-state。勾父全选子、子部分勾父变 Partially、子全勾父转 Checked。

### 提示（按顺序）

1. **连信号**：构造里 `connect(tree_, &QTreeWidget::itemChanged, this, &CheckboxTree::onItemChanged)`，用函数指针语法别用 SIGNAL/SLOT 宏
2. **`propagateDown(QTreeWidgetItem* item, Qt::CheckState state)`**：递归遍历 item 的所有 child，每个 `setCheckState(0, state)`，再对自己递归 `propagateDown(child, state)` 直到叶子
3. **`aggregateState(const QTreeWidgetItem* item) const`**：遍历 item 的直接子项，数 Checked/Unchecked 计数——
   - 任一子为 PartiallyChecked → 立即返回 PartiallyChecked（短路，别数完）
   - 全 Checked → Checked
   - 全 Unchecked → Unchecked
   - 否则 → PartiallyChecked
4. **`recalcUp(QTreeWidgetItem* item)`**：从 item 起一个 while 循环向上走，每层 `setCheckState(0, aggregateState(当前层))`，然后 `current = current->parent()` 直到 nullptr
5. **`onItemChanged` 串起来**：读 `item->checkState(0)`，若是 Checked/Unchecked 就 `propagateDown(item, state)`，再 `recalcUp(item->parent())`

### 关键认知

- **向下传播只处理 Checked/Unchecked，不处理 PartiallyChecked**：PartiallyChecked 是回算的产物，用户在 QTreeWidget 交互里点不出来（只产生两态），所以 onItemChanged 收到的非 Partially 态才值得传播
- **recalcUp 从 `item->parent()` 开始，不从 item 本身**：item 的状态已经被用户点击定下了，只有它的祖先需要按子项分布重算。从 item 本身开始会把它刚定好的状态又覆盖一遍
- **这步先别管信号雪崩**——你能看到勾父时子项联动、父项三态正确就算成功。但大概率你会撞上「勾一下就崩」，那是下一步要解决的 itemChanged 回环问题，先别慌

### 检查点

勾父项时子项全部跟着勾上 = 向下传播对了；手动把几个子项勾上、父项自动变 PartiallyChecked = 向上回算对了。如果你勾一下就卡死或崩，说明 itemChanged 在递归改子项时被反复触发——**这正是 Step 3 要解决的雪崩**，先去把闸门加上再回来测。

> 信号槽机制不熟？[信号与槽](../../../../../beginner/01-qtbase/02-signal-slot-beginner.md)。三态勾选的 Qt 语义看 [Qt::CheckState](https://doc.qt.io/qt-6/qt.html#CheckState-enum)。

### 对照答案

- itemChanged 连接：`src/checkbox_tree.cpp:45`
- onItemChanged 总入口：`src/checkbox_tree.cpp:177`
- propagateDown 递归传播：`src/checkbox_tree.cpp:208`
- aggregateState 三态推导（短路返回混合）：`src/checkbox_tree.cpp:235`
- recalcUp 从父起逐层重算：`src/checkbox_tree.cpp:223`

---

下一步：[Step 3 给它加上防雪崩的双闸门 + Q_PROPERTY 联动开关](./03-signal-guard-and-polish.md)。
