---
title: "5.2 QSqlRelationalTableModel 关联表视图"
description: "入门篇我们把 QSqlTableModel 的单表增删改查跑通了——一张表绑定一个 Model，直接显示在 QTableView 里。但如果你的数据库稍微复杂一点——比如订单表里存的是客户 ID，你想在视图里显示客户名称而不是数字——QSqlTableModel 就力不从心了。"
---

# 现代Qt开发教程（进阶篇）5.2——QSqlRelationalTableModel 关联表视图

## 1. 前言 / 当单表不够用

入门篇我们把 QSqlTableModel 的单表增删改查跑通了——一张表绑定一个 Model，直接显示在 QTableView 里。但如果你的数据库稍微复杂一点——比如订单表里存的是客户 ID，你想在视图里显示客户名称而不是数字——QSqlTableModel 就力不住心了。

QSqlRelationalTableModel 正是为这个场景设计的。它继承自 QSqlTableModel，额外支持「关系字段」——你可以声明某个字段是另一张表的外键，Model 会自动帮你把 ID 替换成对应表的显示值。更棒的是，它还能配合 `QSqlRelationalDelegate` 在编辑时提供一个下拉选择器，让用户从关联表中选择而不是手动输入 ID。

这篇我们一起来把关联表的 Model 配置、Delegate 自定义显示、以及多级关联（A → B → C）的查询策略全部拆干净。

## 2. 环境说明

本文档基于 Qt 6.2+ 编写，使用 C++17 标准和 CMake 3.26+ 构建系统。默认你已经掌握入门篇中 QSqlTableModel 和 QTableView 的基本用法。本篇依赖 Qt6::Sql 和 Qt6::Widgets 模块。示例使用 SQLite 数据库。

## 3. 核心概念讲解

### 3.1 关联表模型——从 ID 到可读文本

假设我们有两张表：`customers`（id, name, city）和 `orders`（id, customer_id, product, amount）。`orders.customer_id` 是 `customers.id` 的外键。在视图中我们不想显示客户 ID，而是显示客户名称。

```cpp
// 创建关联模型
auto *model = new QSqlRelationalTableModel(this);
model->setTable("orders");

// 声明关联关系：orders 表的第 1 列（customer_id）
// 关联到 customers 表的 id 字段，显示 name 字段
model->setRelation(1, QSqlRelation("customers", "id", "name"));

model->select();

// 绑定到视图
tableView->setModel(model);
```

`setRelation()` 的三个参数分别是：当前表中作为外键的列索引、关联表名、关联表的主键字段、关联表中用于显示的字段。调用 `select()` 后，Model 会自动生成一个 `LEFT JOIN` 查询，把 `customer_id` 替换成 `customers.name`。

对于用户来说，视图中显示的是「张三」而不是「3」。但 Model 内部仍然维护着 ID 值——当用户编辑这个字段时，Model 会把选择的名称自动映射回对应的 ID 写入数据库。

### 3.2 QSqlRelationalDelegate——下拉选择器

光有关联显示还不够。如果用户要编辑 `customer_id` 字段，直接在一个文本框里输入数字？那太容易出错了。`QSqlRelationalDelegate` 提供了一个下拉选择器——编辑时自动弹出一个 QComboBox，列出 `customers` 表中所有的名称供选择。

```cpp
// 为关联列设置 Delegate
tableView->setItemDelegate(new QSqlRelationalDelegate(tableView));
```

一行代码就够了。`QSqlRelationalDelegate` 会自动检测哪些列设置了 `QSqlRelation`，然后为这些列提供 QComboBox 编辑器。用户选择一个名称后，Delegate 会把对应的 ID 写回 Model。

如果你想自定义下拉框的显示（比如显示「名称 (城市)」的格式），可以继承 `QSqlRelationalDelegate` 并重写 `createEditor()`：

```cpp
class CustomRelationalDelegate : public QSqlRelationalDelegate
{
public:
    using QSqlRelationalDelegate::QSqlRelationalDelegate;

    QWidget *createEditor(QWidget *parent,
                           const QStyleOptionViewItem &option,
                           const QModelIndex &index) const override
    {
        // 对关联列使用自定义的 ComboBox
        auto *combo = new QComboBox(parent);
        // 查询关联表，构建显示文本
        QSqlRelationalTableModel *model =
            qobject_cast<QSqlRelationalTableModel*>(
                const_cast<QAbstractItemModel*>(index.model()));
        if (model) {
            QSqlRelation rel = model->relation(index.column());
            QSqlQuery query(model->database());
            query.prepare(QString("SELECT id, name, city FROM %1")
                          .arg(rel.tableName()));
            if (query.exec()) {
                while (query.next()) {
                    QString display = QString("%1 (%2)")
                        .arg(query.value("name").toString())
                        .arg(query.value("city").toString());
                    combo->addItem(display, query.value("id"));
                }
            }
        }
        return combo;
    }
};
```

### 3.3 多级关联与性能注意事项

如果你的关联链更深——比如 `orders` → `customers` → `cities`——QSqlRelationalTableModel 只能处理直接关联，不能自动做 A → B → C 的级联显示。解决方案有两种。

第一种是用 SQL 视图（CREATE VIEW）把多级关联预先 JOIN 好，然后用普通的 QSqlTableModel 绑定视图。视图在数据库层面完成所有 JOIN，Model 看到的就是一张扁平的虚拟表。缺点是视图通常是只读的（SQLite 的视图不支持直接 INSERT/UPDATE），需要通过 INSTEAD OF 触发器来处理写入。

第二种是放弃 QSqlRelationalTableModel，直接手写 SQL 查询配合 QSqlQueryModel（只读）或自定义 Model（可读写）。这种方式灵活度最高，但需要自己实现 Model 的 `setData()` 和 `submit()` 逻辑。

性能方面需要注意：`setRelation()` 生成的 `LEFT JOIN` 查询在关联表数据量大时可能变慢。SQLite 对 JOIN 的优化能力有限，如果关联表有上万行数据，考虑给外键字段加索引（`CREATE INDEX idx_customer_id ON orders(customer_id)`）。另外，`select()` 默认的 `SELECT *` 会加载所有数据到内存——如果主表数据量特别大，考虑用 `setFilter()` 加 WHERE 条件限制返回行数。

## 4. 踩坑预防

第一个坑是 `setRelation()` 的列索引必须和 `SELECT` 的列顺序一致。如果你调了 `setSort()` 或其他改变了列顺序的操作，`setRelation()` 中指定的列索引可能不再指向正确的字段。建议在所有 Model 配置完成后最后调 `select()`。

第二个坑是关联表的编辑值不正确。`QSqlRelationalDelegate` 要求关联表的主键必须是整数且唯一。如果关联表使用了复合主键或者字符串主键，Delegate 的映射逻辑可能出错。尽量使用单列整数主键作为关联表的键。

## 5. 练习项目

练习项目：图书管理系统。三张表：authors（id, name, country）、books（id, title, author_id, year）、borrowers（id, name）。再加一张 borrow_records（id, book_id, borrower_id, borrow_date）。用 QSqlRelationalTableModel 显示借阅记录，book_id 和 borrower_id 分别关联显示书名和人名。使用自定义 Delegate 在编辑时显示「书名 (年份)」格式。

完成标准：借阅记录表正确显示书名和人名、编辑时下拉框列出所有书籍和借阅者、新增借阅记录时关联字段自动映射回 ID。

## 6. 官方文档参考链接

[Qt 文档 · QSqlRelationalTableModel](https://doc.qt.io/qt-6/qsqlrelationaltablemodel.html) -- 关联表 Model 完整 API，包含 setRelation 和关联查询

[Qt 文档 · QSqlRelationalDelegate](https://doc.qt.io/qt-6/qsqlrelationaldelegate.html) -- 关联字段编辑 Delegate，提供下拉选择器

[Qt 文档 · Presenting Data in a Table View](https://doc.qt.io/qt-6/sql-presenting.html) -- 表格视图数据展示完整指南

*（链接已验证，2026-06-11 可访问）*

---

到这里就大功告成了。关联表 Model + Delegate 下拉选择器——两张表的关联显示和编辑搞定了。如果是更复杂的多级关联，考虑用 SQL 视图或自定义 Model。
