---
title: "阶段二：多角色 data + 可编辑 setData"
description: "在 data 里按 Qt::ItemDataRole 分发对齐/底色/悬停等多种角色，重写 headerData 给列标题，再加 flags+setData 让单元格双击可编辑、回写后发 dataChanged 刷新。"
---

# 阶段二：多角色 data + 可编辑 setData

← [Step 1-2](./01-skeleton-and-roles.md) · [手册首页](./index.md) · 下一步 [阶段三 增删行](./03-insert-remove-and-polish.md) →

阶段一只让单元格「能显示文本」。但 Qt 的一个单元格远不止于此——它能同时回答「怎么对齐、什么底色、什么前景、什么字体、悬停时弹什么提示」。这一阶段把 data 的 `switch(role)` 补全，再加 `setData`/`flags` 让单元格可编辑。

## Step 3：多角色 data + headerData

### 目标

在 `data` 的 `switch(role)` 里补上：`Qt::EditRole`（编辑值）、`Qt::TextAlignmentRole`（对齐）、`Qt::BackgroundRole`（底色）、`Qt::ForegroundRole`（前景）、`Qt::FontRole`（字体）、`Qt::ToolTipRole`（悬停提示）。再重写 `headerData` 给水平表头列名、垂直表头行号。

### 提示

- `TextAlignmentRole`：优先级/完成态居中（短字段好看），其余靠左。返回 `QVariant(Qt::AlignCenter)` 或 `QVariant(Qt::AlignLeft | Qt::AlignVCenter)`
- `BackgroundRole`：优先级 `>= 2` 的行整行泛浅红——`return QVariant(QBrush(QColor(255,235,235)))`，否则 `return {}`
- `ToolTipRole`：拼一句任务摘要「任务：xxx　负责人：xxx　优先级：x」
- `FontRole`：已完成行加删除线 `QFont font; font.setStrikeOut(true);`
- `ForegroundRole`：已完成行字色变灰 `QColor(150,150,150)`
- 把「某列显示什么文本」抽到私有函数 `displayText`，避免 switch 嵌 switch 难读
- `headerData(section, orientation, role)`：只处理 `DisplayRole`；水平方向按 section 返回列名，垂直方向返回 `section+1`（行号从 1 开始更直观）

### 关键认知

- **一个角色一个返回值**：视图按需问，data 按问的 role 给。视图要底色就问 BackgroundRole，要对齐就问 TextAlignmentRole，互不干扰
- **不关心的 role 走 `default: return {}`**：返回空，视图用默认值。别试图全返回

### 检查点

高优先级行底色泛红、悬停有提示、已完成行字灰带删除线、表头有「任务标题/负责人/优先级/完成」列名 = 多角色对了。

> Model/View 进阶机制不熟？[Model/View 进阶](../../../../../advanced/03-qtwidgets/03-model-view-advanced.md)、[QTableView 进阶](../../../../../advanced/03-qtwidgets/51-qtableview-advanced.md)、[QHeaderView 进阶](../../../../../advanced/03-qtwidgets/52-qheaderview-advanced.md)。

### 对照答案

- TextAlignmentRole 对齐：`src/custom-model.cpp:72`
- BackgroundRole 底色：`src/custom-model.cpp:79`
- ToolTipRole 摘要：`src/custom-model.cpp:93`
- FontRole 删除线 / ForegroundRole 灰字：`src/custom-model.cpp:86,99`
- headerData 列名 + 行号：`src/custom-model.cpp:210`

---

## Step 4：可编辑——flags + setData + dataChanged

### 目标

让单元格双击进编辑态，编辑完回写到 `tasks_` 并刷新屏幕。需要三件套：`flags` 返回 `Qt::ItemIsEditable`、`setData` 落值并发 `dataChanged`、demo 端配个编辑器（这里用 SpinBox 代理演示优先级列）。

### 提示（按顺序）

1. **`flags(index)`**：返回 `Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable`。**漏掉 `ItemIsEditable` 双击就进不了编辑态**——这是最常见的「为什么我编辑不了」
2. **`setData(index, value, role)`**：
   - 先判断 `role == Qt::EditRole`，否则返回 false（CheckState 等角色本例不处理）
   - 边界保护同 data
   - 把 QVariant 落回 `tasks_[row]` 的对应字段（抽到私有 `applyEdit`，priority 夹 0..3）
   - **发 `dataChanged(top_left, bottom_right, roles)`**：列范围给整行 `[0, kColumnCount-1]`，roles 把可能变的角色都列上（DisplayRole/EditRole/BackgroundRole/ForegroundRole/FontRole/ToolTipRole）
   - 返回 true
3. demo 端给优先级列用 `setIndexWidget` 装 `QSpinBox(0..3)`，SpinBox 值变 → 调 `setData(..., Qt::EditRole)`。也可以用 `QStyledItemDelegate` 做正经可复用代理

### 关键认知

- **发 dataChanged 时列范围给整行**：因为改优先级会连带改变底色（priority>=2 泛红）。如果只通知改的那一列，BackgroundRole 不会重查，底色就不变
- **roles 列表要全**：漏列 BackgroundRole 等于告诉视图「这个角色没变」，视图跳过重查
- **返回值语义**：`setData` 返回 true 表示「我真改了」，false 表示「我不接受这次编辑」
- **applyEdit 夹范围**：视图编辑器可能塞越界值（虽然 SpinBox 会拦），模型自己再兜一道，priority 夹 0..3

### 检查点

双击标题能改文字、SpinBox 调优先级后该行**底色跟着泛红/褪红**（说明 dataChanged 的列范围给对了）、改完成态字色变灰 = 编辑链路通了。

> 信号槽函数指针语法 / EditRole 不熟？[信号与槽](../../../../../beginner/01-qtbase/02-signal-slot-beginner.md)、[Model/View 入门](../../../../../beginner/03-qtwidgets/03-model-view-beginner.md)。

### 对照答案

- flags 返回 ItemIsEditable：`src/custom-model.cpp:154`
- setData 落值 + 发 dataChanged（列范围整行、roles 全列）：`src/custom-model.cpp:118,139`
- applyEdit 把 priority 夹 0..3：`src/custom-model.cpp:301`
- demo 端 SpinBox 代理 + setData 回写：`demo/custom-model_window.cpp:87,97`

---

下一步：[阶段三 给模型加增删行能力（begin/end 信号协议）](./03-insert-remove-and-polish.md)。
