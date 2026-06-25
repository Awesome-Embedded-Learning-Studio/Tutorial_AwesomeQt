---
title: "卡住怎么办"
description: "按症状查：cpp 漏引 QVBoxLayout、demo 调不到 QTableWidget 方法、委托不生效、setData 回灌假信号、空值写进模型、短行崩——给方向指向成品 file:行号，不直接给答案。"
---

# 卡住怎么办

← [手册首页](./index.md)

按症状查。每条给方向，不给整段答案——成品 repo 在 `widget/editable-table/`，对照着看。

## cpp 编译报 `expected type-specifier before 'QVBoxLayout'`

- 构造里 `new QVBoxLayout(this)`，但 cpp **只引了 `QTableWidget` / `QTableWidgetItem`，漏了 `<QVBoxLayout>`**。Q_OBJECT 类的 cpp 不像头文件那样顺手引布局类，新加布局代码要单独引。→ `src/editable_table.cpp:16`
- 同理检查 `QSpinBox` / `QDoubleSpinBox` / `QComboBox` / `QLineEdit` 这些编辑器头在 cpp 里都引了吗？createEditor 里 new 它们就得 include。→ `src/editable_table.cpp:10-14`
- 进阶排查：[QTableWidget 入门](../../../../../beginner/03-qtwidgets/50-qtablewidget-beginner.md)

## demo 编译报 `'class AwesomeQt::EditableTable' has no member named 'resizeColumnsToContents'`

- `resizeColumnsToContents()` 是 **QTableWidget** 的方法，EditableTable 把 `table_` 私有化了，外部直接调不到。→ 加薄透传 `void resizeColumnsToContents()` 调 `table_->resizeColumnsToContents()`：`src/editable_table.cpp:393`
- 同理 demo 想拿当前选中行却写了不存在的 `currentRowOfTable()` 之类笔误方法名——EditableTable 没有「取当前行」接口。→ 加公有 `int currentRow() const` 透传 `table_->currentRow()`：`src/editable_table.cpp:389`
- 别图省事把 `table_` 指针 public 出去——那等于放弃封装，以后所有 QTableWidget API 都会从外部漏进来

## 双击单元格不弹自定义编辑器（还是默认文本框）

- 委托**真的 `setItemDelegate` 挂上了**吗？构造里漏了 `table_->setItemDelegate(delegate_)`。→ `src/editable_table.cpp:171`
- 委托的 `ColumnSpecProvider` 回调**注入了吗**？没注入 createEditor 拉不到列规格、退化成文本。→ `src/editable_table.cpp:159`
- 回调里 `column` 越界返回 false 了？检查 `columns_` 的下标边界。→ `src/editable_table.cpp:161`
- 委托类是 Q_OBJECT 但 CMake 源列表里**没把它列进去**？AUTOMOC 要在源列表里看到它才生成元对象。→ `widget/editable-table/CMakeLists.txt`
- 进阶排查：[Model/View 进阶](../../../../../advanced/03-qtwidgets/03-model-view-advanced.md)

## 数值列编辑后存了空串 / 非法值

- 你是不是在 setModelData 里判空写漏了分支？正确姿势是**校验前置**：setEditorData 把空值兜成 `box->minimum()`，setModelData 只管写 `box->value()`（SpinBox 自带夹值）。→ `src/editable_table.cpp:90` / `src/editable_table.cpp:118`
- 双击输 `9999` 提交后格内没夹回 100？检查 createEditor 里 kInt 分支有没有 `box->setRange((int)min, (int)max)`。→ `src/editable_table.cpp:55`
- 进阶排查：[QSpinBox](https://doc.qt.io/qt-6/qspinbox.html)

## setData 后外部接到一堆 dataEdited 假通知

- 这是**程序化 `setItem` 触发 cellChanged** 被 onCellChanged 当用户编辑 emit 出去了。→ 用 `suppress_signal_` 布尔：setData/addRow/clear 入口置 true、出口置 false，onCellChanged 入口先查它。→ `src/editable_table.cpp:445`
- onCellChanged **连的是 cellChanged 还是 itemChanged**？连 cellChanged（给 row/col 参数，省得自己算）。→ `src/editable_table.cpp:174`
- 进阶排查：[信号与槽](../../../../../beginner/01-qtbase/02-signal-slot-beginner.md)

## setData 传的某行比列数短，崩或取到脏值

- 短行没建 QTableWidgetItem，`table_->item(r,c)` 返回 null。→ setData 末尾对 `row_data.size() < col_count` 的列补空占位 item。→ `src/editable_table.cpp:327`
- data() 出口也要兜底：每格 `item ? item->text() : QString()`，别假设 item 一定存在。→ `src/editable_table.cpp:357`
- 进阶排查：[QTableWidget](https://doc.qt.io/qt-6/qtablewidget.html)

## moc 报错（Q_PROPERTY / Q_ENUM 不认识）

- 委托类**有没有 `Q_OBJECT`**？ValidatorDelegate 是 Q_OBJECT 类，漏了 moc 不生成元对象。→ `include/editable_table.h:28`
- CMake **有没有开 AUTOMOC**（`set(CMAKE_AUTOMOC ON)`）？→ `widget/CMakeLists.txt`
- Q_ENUM 的 ColumnType **是不是在 EditableTable 类里**、Q_ENUM 紧跟其后？→ `include/editable_table.h:85`
- 进阶排查：[QObject 与元对象系统](../../../../../beginner/01-qtbase/01-qobject-meta-system-beginner.md)
