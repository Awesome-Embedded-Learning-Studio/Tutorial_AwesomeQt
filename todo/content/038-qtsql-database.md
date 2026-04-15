---
id: "038"
title: "其他模块：QtSql 数据库操作"
category: content
priority: P1
status: pending
created: 2026-04-15
assignee: charliechen
depends_on: []
blocks: []
estimated_effort: medium
---

## 目标

掌握 QtSql 模块的使用，包括 QSqlDatabase 数据库连接、QSqlQuery SQL 执行、
QSqlTableModel/QSqlRelationalTableModel 的模型/视图集成。
学会在 Qt 应用中集成数据库功能，支持 SQLite、MySQL、PostgreSQL 等主流数据库。

## 验收标准

- [ ] 教程遵循标准 7 节结构（前言、环境、核心概念、踩坑、测验、练习、链接）
- [ ] 至少 3 个踩坑条目及后果
- [ ] 3 种类型测验嵌入（口头问答、代码填空、调试挑战）
- [ ] 练习项目已定义
- [ ] Qt 官方文档链接已验证
- [ ] 配套示例代码已创建且可编译

## 实施说明

教程应涵盖以下核心概念：
- QSqlDatabase：数据库连接管理
  - addDatabase / removeDatabase
  - 支持的驱动：QSQLITE, QMYSQL, QPSQL, QODBC
  - setDatabaseName / setHostName / setUserName / setPassword
  - open / close / isOpen
  - 连接池概念
- QSqlQuery：SQL 执行
  - exec / prepare / bindValue
  - 位置绑定与命名绑定
  - next / previous / first / last / seek
  - value / record
  - numRowsAffected / size
  - 事务：transaction / commit / rollback
- QSqlError：错误处理
- QSqlTableModel：表格模型
  - setTable / select / submitAll / revertAll
  - setEditStrategy：OnFieldChange, OnRowChange, OnManualSubmit
  - setFilter / setSort
- QSqlRelationalTableModel：关系模型
- QSqlQueryModel：只读查询模型
- 数据库 Schema 设计与迁移

踩坑重点：
1. QSqlDatabase 连接名冲突导致覆盖已有连接
2. prepare + bindValue 时类型不匹配导致查询失败
3. 大结果集未分页加载导致内存溢出

练习项目：实现一个学生信息管理系统，使用 SQLite 数据库，
支持学生信息的增删改查、多条件搜索、数据导出，
使用 QSqlTableModel 与 QTableView 集成展示。

## 涉及文件

- document/tutorials/beginner/05-other-modules/01-qtsql-database-beginner.md
- examples/beginner/05-other-modules/01-qtsql-database-beginner/

## 参考资料

- [Qt SQL Module](https://doc.qt.io/qt-6/qtsql-index.html)
- [QSqlDatabase Class Reference](https://doc.qt.io/qt-6/qsqldatabase.html)
- [QSqlQuery Class Reference](https://doc.qt.io/qt-6/qsqlquery.html)
- [QSqlTableModel Class Reference](https://doc.qt.io/qt-6/qsqltablemodel.html)
