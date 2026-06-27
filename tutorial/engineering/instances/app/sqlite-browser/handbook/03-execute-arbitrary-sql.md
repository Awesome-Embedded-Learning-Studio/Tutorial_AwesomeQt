---
title: "Step 3：任意 SQL 执行"
description: "底部 QPlainTextEdit 写任意 SQL，QSqlQuery exec 探路 + isSelect 分流：SELECT 走只读 QSqlQueryModel 出表格，INSERT/UPDATE/DDL 报影响行数。"
---

# Step 3：任意 SQL 执行

← [Step 2 可编辑表格](./02-load-editable-table.md) · [手册首页](./index.md) →

## Step 3：任意 SQL 执行（isSelect 分流 + 只读 QSqlQueryModel）

### 目标

底部 `QPlainTextEdit` 里写 `SELECT * FROM users WHERE age > 18`，按 F5 或点 `Execute`，下方 `query_view` 出结果表格（只读），状态栏报「N row(s) × M col(s)」。再写 `UPDATE users SET age=20 WHERE id=1`，按 F5，状态栏报「OK — 1 row(s) affected」，结果区清空。语法错了状态栏报错误文本。

### 提示

- SQL 框接 `Execute` QAction（F5 快捷键），`onExecuteSql` 取 `sql_edit_->toPlainText().trimmed()`，空串直接 return。
- 执行前**先校验库还开着**：`QSqlDatabase::contains(connection_name_)` 且 `db.isOpen()`，否则状态栏提示。
- 关键设计——**两类 SQL 物理隔离**：可编辑表（Step 2）用 `QSqlTableModel` 填 `table_view_`，任意 SQL 结果用只读 `QSqlQueryModel` 填另一个 `query_view_`。别共用一个 view/model，可写和只读混在一起会乱。
- **别直接** `QSqlQueryModel::setQuery(sql)`——它会把 INSERT/UPDATE 当无结果查询，读不到影响行数。先用 `QSqlQuery::exec(sql)` 执行，靠 `query.isSelect()` 分流：
  - `!isSelect()`（DML/DDL）：读 `query.numRowsAffected()`，状态栏报「OK — N row(s) affected」，结果区清空。
  - `isSelect()`（有结果集）：new 一个 `QSqlQueryModel`，`setQuery(std::move(query))`（query 已经 exec 过了，Qt6 这里收右值，move 进去省一次重复执行），`query_view_->setModel(...)`，状态栏报行列数。
- 执行新 SQL 前先释放上一个结果 model：`query_view_->setModel(nullptr)` → `delete query_model_`（同 Step 2 的「先松手再 delete」套路）。
- `query.exec(sql)` 失败：用 `query.lastError()` 的 `text()`（= `databaseText()` + `driverText()` 标准拼接）写状态栏——别只取其中一层，否则错误信息不全。

### 检查点

跑 `SELECT` → 结果表格出现、只读（双击不能改） → 跑 `INSERT/UPDATE` → 状态栏报影响行数、结果区清空 → 跑个 `SELECT` 验证刚才改的数据 → 跑语法错的 SQL → 状态栏报错。**SELECT 结果是只读的，和上面可编辑表是两套独立 view** = 物理隔离成立。

> QSqlQueryModel 只读模型看 [QtSql 表格模型（进阶 QueryModel 部分）](../../../../../advanced/05-other-modules/02-qtsql-tablemodel-advanced.md)。
> Model/View 基础回顾：[Model/View 架构](../../../../../beginner/03-qtwidgets/03-model-view-beginner.md)。

### 对照答案

- onExecuteSql 全流程（校验 → 释放旧 model → exec → isSelect 分流）：`demo/sqlite_browser_window.cpp:301-349`
- DML 报 numRowsAffected：`demo/sqlite_browser_window.cpp:332-336`
- SELECT 走 QSqlQueryModel + move：`demo/sqlite_browser_window.cpp:339-345`
- Execute QAction（F5）装配：`demo/sqlite_browser_window.cpp:98-101`

---

三步搓完，整机骨架就立起来了：命名连接管库、两套 model 隔离可编辑/只读、安全释放链防雷。卡过哪步翻 [卡住怎么办](./troubleshooting.md)，想再深一层看 [手册首页的进阶挑战](./index.md#_3-进阶挑战-可选)。
