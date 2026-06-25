---
title: "Step 1：组合骨架 + 按列声明类型"
description: "EditableTable 继承 QWidget 组合 QTableWidget，定义 ColumnType 枚举，实现 addColumn / addRow 给每列挂类型、给每行建默认 item。"
---

# Step 1：组合骨架 + 按列声明类型

← [手册首页](./index.md) · 下一步 [Step 2 委托校验](./02-delegate-and-validation.md) →

这一步把裸 QTableWidget 包进一个 `AwesomeQt::EditableTable` 类，定义列类型枚举，做出能动态加列、加行的骨架。先不要委托、不要校验——能按类型给每列建出合理默认 item 就行。校验是下一步的事。

## Step 1：组合骨架 + ColumnType + addColumn + addRow

### 目标

得到一个 `EditableTable : public QWidget`，构造时 new 出 QTableWidget 挂 parent、塞进 QVBoxLayout。定义 `enum class ColumnType { kText, kInt, kDouble, kCombo, kCheck }` 加 `Q_ENUM`。`addColumn(header, type, min, max, combo)` 往列定义表 `columns_` 追加一条并调 `table_->setColumnCount(+1)`、设表头；`addRow()` 按列类型给每个单元格建出合理默认 item（kCheck 给勾选框、kCombo 给候选项首项、kInt/kDouble 给范围下界、kText 给空串）。

### 提示

- **组合姿势**：`class EditableTable : public QWidget`，私有成员 `QTableWidget* table_{nullptr}`。构造里 `table_ = new QTableWidget(this)`（parent=this 对象树托管）、`new QVBoxLayout(this)` + `addWidget(table_)`。**别继承 QTableWidget**——那会把一堆不该暴露的 API 放出去，组合才好控边界
- **列定义表**：私有 `struct ColumnSpec { QString header; ColumnType type; double min; double max; QStringList combo; }`，存进 `QVector<ColumnSpec> columns_`。顺序即列序
- **addColumn 两步走**：先 `columns_.append(spec)`，再 `setColumnCount(col+1)`，最后 `setHorizontalHeaderItem(col, new QTableWidgetItem(header))`——QTableWidget 要求先有列才能设表头 item
- **addRow 按类型建 item**：kCheck 走 `applyCheckState(row,col,Unchecked)`（给 item 打 `Qt::ItemIsUserCheckable` flag + `setCheckState`）；kCombo 建候选项首项；kInt/kDouble 建范围下界（`min<=max` 时取 min）；kText 建空串。`setRowCount(row+1)` 后逐列建 item
- **勾选列的 flag**：`item->setFlags(item->flags() | Qt::ItemIsUserCheckable)`，再 `setCheckState(state)`。这一步先不管它怎么触发 dataEdited（step 3 再连 cellChanged）

### 关键认知——为什么组合不继承

继承 QTableWidget 看着省事（直接拥有所有方法），但你会把 `setItem` / `setRowCount` / `setItemDelegate` 这堆内部 API 全暴露给外部，封装形同虚设。组合后 table_ 是私有的，外部只能走你批准的接口（addColumn / addRow / setData / data），要拿 QTableWidget 的方法就加薄透传——边界牢牢攥在自己手里。这也是本批 model/view 控件的统一姿势。

### 检查点

跑起来出现一张能动态加列加行的表：调 `addColumn("Name", kText)` + `addColumn("Score", kInt, 0, 100)` + `addRow()` 后，表头显示 Name/Score，新行 Score 列默认值是 0（范围下界），勾选列出现可点的复选框。列宽/布局正常 = 组合对了。

> QTableWidget / 列行管理不熟？[QTableWidget 入门](../../../../../beginner/03-qtwidgets/50-qtablewidget-beginner.md)。组合控件思路？[QWidget 基类](../../../../../beginner/03-qtwidgets/11-qwidget-base-beginner.md)。

### 对照答案

- 类定义 + ColumnType 枚举 + Q_ENUM：`include/editable_table.h:74` / `include/editable_table.h:85`
- 构造组合 QTableWidget + 布局：`src/editable_table.cpp:151`
- addColumn 追加列定义 + 设表头：`src/editable_table.cpp:186`
- applyCheckState 给勾选列打 flag：`src/editable_table.cpp:204`
- addRow 按类型建默认 item：`src/editable_table.cpp:215`

---

下一步是重头戏：[Step 2 挂上委托做编辑校验](./02-delegate-and-validation.md)。
