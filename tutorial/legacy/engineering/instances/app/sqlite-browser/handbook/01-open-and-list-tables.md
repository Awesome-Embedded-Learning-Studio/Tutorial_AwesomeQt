---
title: "Step 1：打开库 + 列表 + 安全关库"
description: "QSqlDatabase 命名连接打开 SQLite 库、查 sqlite_master 列所有表、关库时按 model→close→removeDatabase 顺序安全释放。"
---

# Step 1：打开库 + 列表 + 安全关库

← [手册首页](./index.md) · 下一步 [Step 2 可编辑表格](./02-load-editable-table.md) →

## Step 1：命名连接打开库 + sqlite_master 列表 + 关库安全释放

### 目标

点 `Open Database…` 选一个 `.db` 文件，左边列表里冒出这个库里所有表名；状态栏显示当前打开的文件名。再点 `Close Database`，列表清空、状态栏回到「No database open」，且**控制台不刷 `connection still in use` 警告**。

### 提示

- 开库**别用默认连接**：`QSqlDatabase::addDatabase("QSQLITE", connection_name)`——第二参是连接名。连接名怎么定？用「文件名 + 时间戳」拼一个唯一的，同一个 db 反复打开也不撞名。
- `openDatabase` 开新库前**先调 `closeDatabase()`** 关掉当前库，避免两个连接同时挂着。
- `db.open()` 失败要回滚：`removeDatabase(connection_name)` 把刚 add 的连接撤掉，别留个打不开的空壳。
- 列表查 SQLite 内置的 `sqlite_master` 表：`SELECT name FROM sqlite_master WHERE type='table' ORDER BY name`——`type='table'` 排除 view/index/trigger。
- 关库的**释放顺序是这一步的灵魂**：必须先把所有引用该连接的 model 干掉（这步还没建 model，但要预留 `clearTableModel()` 钩子）→ `db.close()` → `QSqlDatabase::removeDatabase(name)`。顺序反了就触发 `connection still in use`。
- `refreshTableList` 里 `QListWidget::clear()` 会误触发 `currentRowChanged`——填列表全程 `blockSignals(true)`，填完放开。
- 菜单 / 工具栏放 `Open`（Ctrl+O）和 `Close`（Ctrl+W/F4）两个 `QAction`。

### 检查点

打开一个有几个表的 db → 左列表出现表名、状态栏显示文件名 → 关库 → 列表清空、状态栏回到初始。**反复开同一库多次，控制台干净无 warning** = 连接生命周期管对了。

> QSqlDatabase / 命名连接机制不熟？先读 [QtSql 数据库连接](../../../../../beginner/05-other-modules/01-qtsql-database-beginner.md)。
> sqlite_master 是什么看 [官方 schema 表文档](https://www.sqlite.org/schematab.html)。

### 对照答案

- 连接名拼接（文件名 + mtime + 自增序号）：`demo/sqlite_browser_window.cpp:139-146`
- openDatabase 开新库前先关、open 失败回滚：`demo/sqlite_browser_window.cpp:158-182`
- 查 sqlite_master 列表 + blockSignals：`demo/sqlite_browser_window.cpp:234-261`
- closeDatabase 释放顺序（model → 嵌套作用域 close → removeDatabase）：`demo/sqlite_browser_window.cpp:196-219`
- Open / Close QAction 装配：`demo/sqlite_browser_window.cpp:88-96`

---

下一步：点一张表，把它的数据填进可编辑表格——[Step 2 可编辑表格](./02-load-editable-table.md)。
