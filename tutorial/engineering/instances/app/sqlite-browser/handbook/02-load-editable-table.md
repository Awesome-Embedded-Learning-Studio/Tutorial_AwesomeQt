---
title: "Step 2：可编辑表格"
description: "点列表里的表，用 QSqlTableModel setTable + OnFieldChange 把数据填进 QTableView，单元格里直接改数据失焦即提交；切表前先 clearTableModel 防残留。"
---

# Step 2：可编辑表格

← [Step 1 打开库与列表](./01-open-and-list-tables.md) · [手册首页](./index.md) · 下一步 [Step 3 任意 SQL](./03-execute-arbitrary-sql.md) →

## Step 2：点表填可编辑表格（QSqlTableModel OnFieldChange）

### 目标

点左列表里某张表，右边 `QTableView` 出现该表全部行、全部列，**双击单元格能改数据、改完点别处（失焦）就自动写回库**。再点另一张表，旧表数据干净清掉、换上新表，状态栏实时显示表名和行数。

### 提示

- 列表的 `currentRowChanged(row)` 接到一个 `onTableSelected` 槽；row 合法时取表名 `table_list_->item(row)->text()`，调 `loadTable(table)`。
- `loadTable` 第一行**先 `clearTableModel()`**——切表前必须清旧 model，否则新 model 还没建、旧的还残留，`setTable` 可能撞到旧引用。
- 用 `QSqlTableModel`（不是 `QSqlQueryModel`，那个只读）：`new QSqlTableModel(this, db)`，传刚才的命名连接 `db`。
- 装配顺序：`setTable(table)` → `setEditStrategy(QSqlTableModel::OnFieldChange)` → `select()`。**先 setTable 再 select**，别在旧表残留上 select。
- `OnFieldChange` = 单元格失焦即提交，不用额外加「保存」按钮。
- `select()` 失败要查 `lastError()` 写状态栏，并 `delete` 掉刚 new 的 model（避免泄漏）。
- `clearTableModel` 的微操作是防雷核心：**先** `table_view_->setModel(nullptr)` **再** `delete table_model_`。顺序反了 view 还指着已删 model，Qt 打 warning。
- 成功后 `table_view_->setModel(table_model_)`，状态栏刷「文件名 · table: xxx · N row(s)」。

### 检查点

点表 → 右边出该表数据 → 双击单元格改个值 → 点别处 → 关掉程序重开同一库 → **改的值还在** = OnFieldChange 提交链通了。再点另一张表 → 旧表数据干净消失、新表填上，**控制台无 warning** = 切表释放管对了。

> QSqlTableModel 不熟？先读 [QtSql 表格模型](../../../../../beginner/05-other-modules/02-qtsql-tablemodel-beginner.md)。
> Model/View + QTableView 看这里：[Model/View 架构](../../../../../beginner/03-qtwidgets/03-model-view-beginner.md)、[QTableView](../../../../../beginner/03-qtwidgets/51-qtableview-beginner.md)。

### 对照答案

- onTableSelected 取表名调 loadTable：`demo/sqlite_browser_window.cpp:263-272`
- loadTable 装配（clearTableModel → setTable → OnFieldChange → select → fetchMore → setModel）：`demo/sqlite_browser_window.cpp:274-296`
- clearTableModel「先 submitAll 再 setModel(nullptr) 再 delete」：`demo/sqlite_browser_window.cpp:221-229`
- 状态栏刷新表名 + 行数：`demo/sqlite_browser_window.cpp:361-364`

---

下一步：底部 SQL 框写任意 SQL——[Step 3 任意 SQL 执行](./03-execute-arbitrary-sql.md)。
