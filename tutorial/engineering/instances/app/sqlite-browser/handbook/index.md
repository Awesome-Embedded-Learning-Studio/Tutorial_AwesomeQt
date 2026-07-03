---
title: "SQLite Browser 手搓手册"
description: "从空 QMainWindow 一行行搓出 SQLite 浏览器：QSqlDatabase 命名连接打开库、sqlite_master 列表、QSqlTableModel 可编辑表格、任意 SQL 走只读 QSqlQueryModel、安全关库释放。"
---

# SQLite Browser 手搓手册

> **source**：成品答案在 `app/10-database-tools/sqlite-browser/`（做完对照）· **related**：app 栏数据库工具类整机成品

::: tip 这是「手搓手册」
不是参考手册（查完走），是 workbook（跟着搓）。每个 step 给**目标 → 提示 → 检查点**，成品 repo 当答案钥匙——卡住了去对照，别整段复制。
:::

## 0. 你将学到

搓完这个 SQLite 浏览器，你会打通这几样 Qt 能力（每样后面都有教程深挖，这里先用起来）：

- **QSqlDatabase 命名连接**：`addDatabase("QSQLITE", name)` 打开库、`database(name)` 取回、`removeDatabase(name)` 释放——理解连接生命周期
- **QSqlTableModel 可编辑表**：`setTable` + `OnFieldChange`，单元格里直接改数据、失焦即提交
- **QSqlQueryModel 只读结果集**：任意 SQL 的结果怎么渲染，与可编辑 model 的边界
- **QSqlQuery 执行 + isSelect 分流**：SELECT 出表格、INSERT/UPDATE 报影响行数，一次 exec 判两类
- **Model/View + QTableView**：表格式 view 怎么接 model
- **QMainWindow 整机装配**：菜单栏 / 工具栏 / 状态栏 / QSplitter 分区 / QAction 复用
- **资源释放顺序**：关库前先松手所有 model，再 close，再 removeDatabase——治 `connection still in use`

## 1. 起点

先有个能跑的空主窗口。最小 Qt Widgets 工程，main 里弹个 QMainWindow：

```cpp
#include <QApplication>
#include <QMainWindow>
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QMainWindow w;
    w.resize(1000, 700);
    w.show();
    return app.exec();
}
```

弹出空白主窗口 = 环境通了。QMainWindow 不熟先看 [QMainWindow 主窗口](../../../../../beginner/03-qtwidgets/55-qmainwindow-beginner.md)。

> 注意：工程要链 `Qt6::Sql`（`CMakeLists.txt` 的 `target_link_libraries` 里加），不链的话 `#include <QSqlDatabase>` 都找不到。

## 2. 任务清单

分 3 步（一文件一步），每步：**目标 → 提示 → 检查点**。卡住翻 [卡住怎么办](./troubleshooting.md)。

| Step | 目标 | 进 |
|---|---|---|
| 1 | 命名连接打开库 + sqlite_master 列表 + 关库安全释放 | [01](./01-open-and-list-tables.md) |
| 2 | 点表填可编辑表格（QSqlTableModel OnFieldChange） + 切表清旧 | [02](./02-load-editable-table.md) |
| 3 | 任意 SQL 执行（isSelect 分流，只读 QSqlQueryModel 出结果） | [03](./03-execute-arbitrary-sql.md) |

成品对照：`app/10-database-tools/sqlite-browser/`（按 [成品导览](../) 的「怎么读」顺序对照）。

## 3. 进阶挑战（可选）

搓完基础版想再深一层：

- **分页加载大表**：现在 `select()` 一次性把整表读进 model，几十万行的表会卡。提示：用 `setFilter` + `LIMIT/OFFSET` 翻页，或换 `QSqlRelationalTableModel` 处理外键。
- **事务批量提交**：现在 `OnFieldChange` 每改一格提交一次，批量录入慢。提示：改成 `OnManualSubmit`，加 Begin/Commit 按钮，`submitAll()` 一次性提交、`revertAll()` 回滚。
- **表结构查看**：现在只看数据，看不到列定义。提示：查 `SELECT sql FROM sqlite_master WHERE name=?` 取建表语句，单独开个 dock 显示。
- **导出 CSV**：把当前表或查询结果导成 CSV。提示：遍历 model 的 `rowCount`/`columnCount`，`QFile` + `QTextStream` 写出。
- **BLOB 列预览**：表里有 BLOB（图片/二进制）列时单元格显示乱码。提示：自定义 delegate，BLOB 列画缩略图或显示 `<blob N bytes>`。
- **下一站**：app 栏的 serial-tool / image-viewer——换皮复用 QMainWindow 整机骨架，但引入 QSerialPort / 自定义绘制。
