---
title: "Step 2：itemChanged 槽 + 状态汇总 checkedTexts/checkedItems"
description: "连 itemChanged，onItemChanged 去重+边界后转发为 checkedChanged；写 checkedTexts/checkedItems 按列表顺序收已勾选项。这一步先把单项转发跑通。"
---

# Step 2：itemChanged 槽 + 状态汇总 checkedTexts/checkedItems

← [Step 1](./01-skeleton-and-additem.md) · [手册首页](./index.md) · 下一步 [Step 3 批量守卫 + 收尾](./03-batch-guard-and-polish.md) →

这一步把勾选变化转发出去，再补上「把勾选结果收出来」的汇总 API。诀窍在分清两条路：用户点击的勾选要转发，程序化批量改写（下一步才加）要挡住——这一步先把「转发」和「汇总」两件事跑通，批量守卫留到 Step 3。

## Step 2：itemChanged 槽 + 状态汇总

### 目标

连上 `QListWidget::itemChanged`，写 `onItemChanged(QListWidgetItem* item)`：去重 + 空指针边界后，转发为更易用的 `checkedChanged(item, bool)`——外部业务连这个信号就行，不用自己再去读 `checkState`。再写 `checkedTexts()` / `checkedItems()` 两个状态汇总方法，按列表顺序把已勾选项的文本/指针收出来。

### 提示（按顺序）

1. **连信号**：构造里 `connect(list_, &QListWidget::itemChanged, this, &CheckboxList::onItemChanged)`，用函数指针语法别用 SIGNAL/SLOT 宏
2. **`onItemChanged(QListWidgetItem* item)`**：入口先 `if (item == nullptr) return;`（边界），再 `emit checkedChanged(item, item->checkState() == Qt::Checked)`——把 `Qt::CheckState` 折成 `bool`，外部用起来更顺手
3. **声明信号**：`signals: void checkedChanged(QListWidgetItem* item, bool checked);`
4. **`QStringList checkedTexts() const`**：先 `list_->count()` 拿总数、`result.reserve(n)` 预分配；`for` 遍历每项，`item != nullptr && item->checkState() == Qt::Checked` 就 `result.append(item->text())`；空列表直接返回空 `QStringList`
5. **`QList<QListWidgetItem*> checkedItems() const`**：同上，但 append 的是 `item` 指针而非 `text()`
6. **两个方法都要判 `list_ == nullptr`** 早返回空容器——防御性，构造失败时也不崩

### 关键认知

- **itemChanged 不区分用户点击 vs 程序改写**：这是 Qt Item View 的既定行为。这一步只有 `addItem`（初始化期局部守卫）和用户点击会触发，问题不大；但下一-步加批量方法后，程序化 `setCheckState` 也会走这条链——所以 Step 3 要给批量方法加 `blockSignals` 守卫。这一步先把转发跑通，心里记着「程序化改写也会进来」
- **汇总按列表顺序遍历，不用 findItems**：`checkedTexts`/`checkedItems` 要保证输出顺序 = 列表显示顺序，所以用索引 `for` 遍历 `list_->item(i)`，别用 `findItems` 或迭代器——顺序和 `reserve` 预分配的简单性最重要
- **状态用 `==` 比较而非位运算**：本件只用两态，`checkState() == Qt::Checked` 足够；checkbox-tree 才会碰 PartiallyChecked 需要更细的判断

### 检查点

手动勾几个框，外部连 `checkedChanged` 的槽能收到 `(item, true/false)` = 转发对了；调 `checkedTexts()` 返回的文本列表和你勾的完全一致、顺序也和列表显示顺序一致 = 汇总对了。这一步还没批量方法，`Check all` 按钮做不出来是正常的——下一步补。

> 信号槽机制不熟？[信号与槽](../../../../../beginner/01-qtbase/02-signal-slot-beginner.md)。QListWidget 遍历与状态读取看 [QListWidget 入门](../../../../../beginner/03-qtwidgets/46-qlistwidget-beginner.md)。

### 对照答案

- itemChanged 连接：`src/checkbox_list.cpp:25`
- onItemChanged 转发 checkedChanged：`src/checkbox_list.cpp:178-184`
- checkedChanged 信号声明：`include/checkbox_list.h:86`
- checkedTexts 顺序汇总：`src/checkbox_list.cpp:109-123`
- checkedItems 顺序汇总：`src/checkbox_list.cpp:125-138`

---

下一步：[Step 3 给批量方法加 blockSignals 守卫 + Q_PROPERTY 收尾](./03-batch-guard-and-polish.md)。
