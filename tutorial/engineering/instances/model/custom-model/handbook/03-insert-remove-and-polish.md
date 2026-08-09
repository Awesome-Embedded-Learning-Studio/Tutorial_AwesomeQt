---
title: "阶段三：增删行 begin/end 信号 + 便捷 API"
description: "重写 insertRows/removeRows 用 beginInsertRows/endInsertRows 包夹容器改动，加 appendTask 便捷 API，配 demo 增删行按钮，收尾打磨。"
---

# 阶段三：增删行 begin/end 信号 + 便捷 API

← [Step 3-4](./02-edit-and-roles.md) · [手册首页](./index.md) →

前两阶段的模型还是「静态」的——行数固定。真实表格要能增删行。这一阶段学自定义模型最容易崩溃的地方：**增删行必须用 begin/end 模型索引信号包夹容器改动**，漏了就 segfault。

## Step 5：insertRows / removeRows 配 begin/end 信号

### 目标

重写 `insertRows(int row, int count, parent)` 和 `removeRows(int row, int count, parent)`，用 `beginInsertRows`/`endInsertRows`、`beginRemoveRows`/`endRemoveRows` 包夹对 `tasks_` 的改动。

### 提示

- `insertRows`：
  - parent 有效返回 false（表格无层级）；count<=0 返回 false；row 越界（`<0` 或 `>tasks_.size()`，末尾追加算合法）返回 false
  - 顺序严格按这三步：① `beginInsertRows(parent, row, row+count-1)` ② 真正改容器（`tasks_.insert(row, Task{})` 循环 count 次）③ `endInsertRows()`
- `removeRows`：同样三步套：`beginRemoveRows` → 循环 `tasks_.removeAt(row)` → `endRemoveRows()`
  - 边界：`row<0` 或 `row+count>tasks_.size()` 返回 false
- **这三步顺序不能乱、不能漏**：begin 在前是告诉视图「我要动了」，改容器是真动，end 在后是告诉视图「动完了」

### 关键认知

- **begin/end 不是优化是协议**：漏掉它们，视图还按旧行数拿数，模型容器已经变了——视图访问越界直接 segfault。这是自定义模型头号崩溃源
- **beginInsertRows 的第二三个参数是要插入的行范围** `[row, row+count-1]`，不是行数。写错范围视图会算错行数
- **removeAt(row) 循环删**：删一个后后面元素自动前移，所以循环里 row 不变，删 count 次就对了

### 检查点

调 `insertRow(rowCount())` 末尾多一行空任务、视图立刻刷新（说明 begin/end 包夹对了）、`removeRow(0)` 删第一行其余行自动上移且无崩溃。

> Model/View 进阶 / begin/end 信号机制不熟？[Model/View 进阶](../../../../../advanced/03-qtwidgets/03-model-view-advanced.md)。

### 对照答案

- insertRows 三步套（begin/改容器/end）：`src/custom-model.cpp:161,174,176,180`
- removeRows 三步套：`src/custom-model.cpp:188,199,201,203`

---

## Step 6：便捷 API appendTask + demo 增删行按钮

### 目标

加个便捷方法 `appendTask(const Task&)`（等价 insertRows + setData 四列，但一次事务更清晰），再在 demo 端加「增行 / 删选中行 / 加示例」三个按钮，把模型全部能力串起来。

### 提示

- `appendTask(task)`：`int row = tasks_.size(); insertRow(row);` 然后对四列各调一次 `setData(index(row, kXxx), task.xxx)`
- `taskAt(int row)`：越界返回默认 `Task{}`，便于 demo 读数据
- `taskCount()`：返回 `tasks_.size()`
- demo 端三个按钮：
  - 增行：`model_->insertRow(model_->rowCount())` + `scrollToBottom()`
  - 删选中行：`selectionModel()->selectedRows()` 拿选中行号，**从大到小排序再删**（避免删一行后后面行号失效），逐个 `removeRow(row)`
  - 加示例：调 `appendTask` 两条
- 函数指针语法 connect：`connect(btn, &QPushButton::clicked, this, &Window::slot)`

### 关键认知

- **删多行要从大到小删**：选中行 [2,5,8]，先删 8 再删 5 再删 2。先删 2 的话，5 和 8 的实际行号会变成 4 和 7，再删原「5」就越界或删错行
- **appendTask 是语法糖**：它内部就是 insertRows + setData×4，没有新协议。提供它是因为「一次事务插一条已知记录」比「先插空行再逐列填」更好读
- demo 用 `setIndexWidget` 装的 SpinBox 只对当前可见行生效——滚动出新行时视图会问模型要。要正经可复用代理得用 `QStyledItemDelegate`，那是下一个挑战

### 检查点

三个按钮都生效：增行末尾多一行、删选中行从屏幕和模型都消失、加示例末尾追加两条且底色/对齐都正常 = 模型全部能力串起来了。

> 信号槽 / 容器不熟？[信号与槽](../../../../../beginner/01-qtbase/02-signal-slot-beginner.md)、[容器](../../../../../beginner/01-qtbase/04-container-beginner.md)。

### 对照答案

- appendTask（insertRow + setData 四列）：`src/custom-model.cpp:243`
- taskAt 越界保护：`src/custom-model.cpp:256`
- demo 增行按钮：`demo/custom-model_window.cpp:106`
- demo 删选中行（从大到小删）：`demo/custom-model_window.cpp:113`
- demo 加示例 appendTask：`demo/custom-model_window.cpp:127`

---

基础版搓完了。想再深一层（排序/过滤/复选框列/自定义委托）→ 回 [手册首页](./index.md) 看「进阶挑战」。成品全部代码在 `model/01-mv-pattern/custom-model/`，对照 [成品导览](../) 的「怎么读」顺序复盘。
