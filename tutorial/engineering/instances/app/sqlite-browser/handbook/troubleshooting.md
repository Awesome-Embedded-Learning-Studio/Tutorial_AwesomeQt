---
title: "卡住怎么办"
description: "按症状查：打不开库、connection still in use 警告、列表为空、切表残留旧数据、delete model 报 warning、SQL 不报影响行数、改了数据没存、>255 行报假行数、错误信息不全、连接名撞名——给方向指向教程章，不直接给答案。"
---

# 卡住怎么办

← [手册首页](./index.md)

按症状查。每条给方向，不给整段答案——成品 repo 在 `app/10-database-tools/sqlite-browser/`，对照着看。

## 打不开库 / open 报错

- 工程链了 `Qt6::Sql` 吗？不链的话 `QSqlDatabase` 都编译不过。→ `demo/CMakeLists.txt:8-13`
- 文件路径对不对？`QFileDialog` 拿到的 path 直接喂给 `db.setDatabaseName`，别再拼绝对路径。
- `db.open()` 失败有没有读 `db.lastError().text()`？空错误也要兜个通用提示。→ `demo/sqlite_browser_window.cpp:168-176`
- 进阶排查：[QtSql 数据库连接](../../../../../beginner/05-other-modules/01-qtsql-database-beginner.md)

## 关库时控制台刷 `connection still in use`

- 这是 Qt 最经典的 SQL 警告：`removeDatabase` 时还有 `QSqlDatabase` 副本没销毁（model 内部握着一份；取回的 `db` 副本也没出作用域）。
- 关库前有没有**先清所有 model**？`db.close()` 必须放在**嵌套作用域**里，让 `QSqlDatabase` 副本随作用域析构、引用数回落到 1，再 `removeDatabase`。顺序：`clearTableModel` + 清 query_model → 嵌套作用域 `{ db.close(); }` → `removeDatabase`。→ `demo/sqlite_browser_window.cpp:196-219`
- 进阶排查：[QtSql 数据库连接](../../../../../beginner/05-other-modules/01-qtsql-database-beginner.md)

## 左列表是空的（库明明有表）

- 查 `sqlite_master` 的 SQL 对不对？要 `WHERE type='table'`，别漏，否则会把 view/index/trigger 也列出来甚至全空。→ `demo/sqlite_browser_window.cpp:252`
- `query.exec` 失败有没有读 `query.lastError()` 写状态栏？→ `demo/sqlite_browser_window.cpp:252-255`
- 库真打开了吗？`refreshTableList` 开头有没有 `QSqlDatabase::contains(connection_name_)` 兜底？→ `demo/sqlite_browser_window.cpp:239-242`

## 切表后表格显示旧表数据 / 报错

- `loadTable` 第一行**有没有先 `clearTableModel()`**？切表不清旧 model 就会残留。→ `demo/sqlite_browser_window.cpp:275`
- `setTable` 和 `select` 顺序对不对？必须**先 setTable 再 select**，别在旧表上 select。→ `demo/sqlite_browser_window.cpp:283-285`
- 进阶排查：[QtSql 表格模型](../../../../../beginner/05-other-modules/02-qtsql-tablemodel-beginner.md)

## delete model 后 Qt 打 `QAbstractItemView` warning

- view 还指着已 delete 的 model——**先** `view->setModel(nullptr)` **再** `delete model`，顺序反了就 warning。→ `demo/sqlite_browser_window.cpp:225-227`
- `closeDatabase` 和 `clearTableModel` 都要走这个「先松手再 delete」套路。

## 跑 INSERT/UPDATE 状态栏不报影响行数 / 结果区空着

- 是不是直接 `QSqlQueryModel::setQuery(sql)` 把 DML 当无结果查询了？DML 读不到行数。
- 要先用 `QSqlQuery::exec(sql)` 探路，`!query.isSelect()` 时读 `numRowsAffected()` 报状态栏。→ `demo/sqlite_browser_window.cpp:324-336`
- 进阶排查：[QtSql 表格模型（进阶）](../../../../../advanced/05-other-modules/02-qtsql-tablemodel-advanced.md)

## 双击单元格改了数据，重开库发现没存

- `setEditStrategy` 设的是 `OnFieldChange` 吗？设成 `OnManualSubmit` 又没调 `submitAll()` 就不会存。→ `demo/sqlite_browser_window.cpp:284`
- 进阶排查：[QtSql 表格模型](../../../../../beginner/05-other-modules/02-qtsql-tablemodel-beginner.md)

## 切库瞬间状态栏乱跳 / 表格被误清

- `refreshTableList` 里 `QListWidget::clear()` 会触发 `currentRowChanged(-1)` 串到 `onTableSelected`。
- 填列表全程有没有 `blockSignals(true)`，填完再放开？→ `demo/sqlite_browser_window.cpp:235-260`

## 表 >255 行时状态栏只报 256 行 / 翻不到底

- SQLite 驱动 `QuerySize=false`，`QSqlTableModel`/`QSqlQueryModel` 初始只 prefetch 255 行，此时 `rowCount()` 只反映已取部分。
- `select()`（表）/`setQuery()`（任意 SQL）之后有没有循环 `while (canFetchMore()) fetchMore();` 拉全量？没拉就报假行数。→ 表 `demo/sqlite_browser_window.cpp:291-294`；任意 SQL `demo/sqlite_browser_window.cpp:342-345`。

## OnFieldChange 编辑器还持焦时切表/关库，最后一条改动丢了

- `OnFieldChange` 靠失焦提交，切表/关库动作可能抢在失焦前，最后一条没落库。
- `clearTableModel` 里**有没有先 `submitAll()` 兜底**提交未落库编辑，再松手 delete model？→ `demo/sqlite_browser_window.cpp:222-227`。

## SQL 出错时错误信息只显示一半

- 手动只取了 `lastError().databaseText()` 或 `driverText()` 其中一层，漏了另一层（驱动报的错和数据库报的错是两份）。
- 错误文本统一用 `err.text()`——它就是 `databaseText()` + `driverText()` 的标准拼接。→ `demo/sqlite_browser_window.cpp:326-327`。

## 同一个 db 反复打开 / 多窗口打开撞名文件报 `connection already exists`

- `QSqlDatabase` 连接注册表是**进程级**，用默认连接、固定连接名、或仅靠「文件名+mtime」都可能撞名（多窗口打开 basename 相同且 mtime 同秒的文件尤甚），静默串到旧连接。
- 连接名有没有加 `QAtomicInt` 自增序号？`makeConnectionName` 用「文件名 + mtime + 自增序号」拼唯一名。→ `demo/sqlite_browser_window.cpp:139-146`。

## moc 报错 / QSqlDatabase 不认识

- `SqliteBrowserWindow` 头里**有没有 Q_OBJECT**？（虽然这例没自定义信号槽用了 Q_OBJECT 也无害，且 `openDatabase` 是普通成员）
- CMake **开了 AUTOMOC 吗**？
- 进阶排查：[QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)
